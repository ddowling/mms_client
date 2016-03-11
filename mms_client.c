/* $Id:$
 *
 * Copyright   : (c) 2016 by RAEdyne Pty Ltd.  All Rights Reserved
 * Project     : RAEdyne
 * File        : mms_client.c
 *
 * Author      : Denis Dowling
 * Created     : 9/3/2016
 *
 * Description : Library to access the mms_logger server from a C application
 *               like the ArduCopter application
 */
#include "mms_client.h"

#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>

static bool debug_comms = false;
static int server_socket = -1;

#define MAX_WATCHED_VARIABLES 100

// Cached values if don't have a server response in time or we request
// an unknown variable
static double cached_values[MAX_WATCHED_VARIABLES];

// Map from the CSV position to the watched_variables index and variable_value
// index
static int csv_map[MAX_WATCHED_VARIABLES];

static char read_buf[1024];
static int buf_end = 0;
static int buf_start = 0;

static bool fill_buffer()
{
    // recover space at the start of the buffer if possible
    if (buf_start > 0)
    {
	if (buf_end > buf_start)
	    memmove(read_buf, read_buf + buf_start, buf_end - buf_start);
	buf_end -= buf_start;
	buf_start = 0;
    }

    int max_read = sizeof(read_buf) - buf_end - 1;

    int n = read(server_socket, read_buf + buf_end, max_read);
    if (debug_comms)
	printf("n=%d\n", n);

    if (n == 0)
    {
	if (debug_comms)
	    printf("Socket was closed\n");
	close(server_socket);
	server_socket = -1;
	return false;
    }

    if (n < 0)
    {
	if (errno == EWOULDBLOCK || errno == EAGAIN ||
	    errno == EINTR || errno == ETIMEDOUT)
	    return false;

	if (debug_comms)
	    printf("socket read failed : %s\n", strerror(errno));
	close(server_socket);
	server_socket = -1;
	return false;
    }

    buf_end += n;

    read_buf[buf_end] = '\0';
    if (debug_comms)
	printf("fill_buffer now (%d) : %s\n", buf_end, read_buf);

    return true;
}

static char *get_line()
{
    int line_end;
    for(line_end = buf_start + 1; line_end < buf_end; line_end++)
    {
	if (read_buf[line_end] == '\r' || read_buf[line_end] == '\n')
	    break;
    }

    if (debug_comms)
	printf("buf_start=%d buf_end=%d line_end=%d\n",
	       buf_start, buf_end, line_end);

    if (line_end == buf_end)
    {
	if (debug_comms)
	    printf("get_line empty\n");
	return 0;
    }

    // Get the start of the line buffer skipping any end of line characters
    char *line_start = read_buf + buf_start;
    while(*line_start == '\r' || *line_start == '\n')
	line_start++;

    read_buf[line_end] = '\0';
    buf_start = line_end + 1;

    if (debug_comms)
	printf("get_line returned '%s'\n", line_start);

    return line_start;
}

static bool wait_reply(double timeout, int max_reply, char *reply_buf)
{
    while(true)
    {
	struct timeval tv;
	tv.tv_sec = timeout;
	tv.tv_usec = 1000000 * (timeout - tv.tv_sec);;

	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(server_socket, &readfds);

	int res = select(server_socket + 1, &readfds, NULL, NULL, &tv);
	if (res == 0)
	{
	    // Timeout
	    if (debug_comms)
		printf("Read:<timeout>\n");
	    return false;
	}
	else if (res < 0)
	{
	    fprintf(stderr, "Failed to read from client socket : %s\n",
		    strerror(errno));
	    return false;
	}

	fill_buffer();

	char *reply_line = 0;
	while(true)
	{
	    char *line = get_line();
	    if (line == 0)
	    {
		if (debug_comms)
		    printf("Did not get an 'OK' or 'FAIL' response\n");
		return false;
	    }
	    else if (strcmp(line, "OK") == 0)
	    {
		if (reply_buf != 0)
		{
		    if (reply_line != 0 && reply_buf != 0)
			strncpy(reply_buf, reply_line, max_reply);
		    else
			reply_buf[0] = '\0';
		}
		return true;
	    }
	    else if (strcmp(line, "FAIL") == 0)
	    {
		if (debug_comms)
		    printf("Received FAIL\n");

		return false;
	    }
	    else
		reply_line = line;
	}
    }
}

static void send_command(const char *cmd)
{
    write(server_socket, cmd, strlen(cmd));
    write(server_socket, "\r\n", 2);

    if (debug_comms)
	printf("Send:%s\n", cmd);
}

bool mms_client_connect(const char *address)
{
    assert(server_socket == -1);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
	fprintf(stderr, "Failed to create server socket : %s\n",
		strerror(errno));
	return false;
    }

    if (address == NULL)
	address = "127.0.0.1";

    // FIXME replace with getaddrinfo
    struct hostent *server = gethostbyname(address);

    if (server == NULL)
    {
	fprintf(stderr, "client connect failed : no such host '%s'\n", address);
	return false;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(3001);

    // Now connect to the server
    if (connect(server_socket,
		(struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
	fprintf(stderr, "client connect failed : %s", strerror(errno));

	close(server_socket);
	server_socket = -1;

	return false;
    }

    // Set the socket to non blocking
    int flags = O_NONBLOCK;
    fcntl(server_socket, F_SETFL, flags);

    // Throw away the buffer information after connection
    wait_reply(1, 0, 0);

    send_command("prompt off");
    wait_reply(1, 0, 0);

    return true;
}

bool mms_client_is_connected()
{
    return server_socket >= 0;
}

void mms_client_watch_variables(double update_period_seconds,
				int num_variables,
				const char **variable_names)
{
    assert(server_socket != -1);
    assert(num_variables <= MAX_WATCHED_VARIABLES);

    char send_buf[1024];
    snprintf(send_buf, sizeof(send_buf), "rate %g", update_period_seconds);
    send_command(send_buf);
    wait_reply(1, 0, 0);

    send_command("remove *");
    wait_reply(1, 0, 0);

    int var_index;
    for (var_index = 0; var_index < num_variables; var_index++)
	cached_values[var_index] = 0.0;

    for (var_index = 0; var_index < num_variables; var_index++)
    {
	snprintf(send_buf, sizeof(send_buf), "add %s",
		 variable_names[var_index]);
	send_command(send_buf);
        wait_reply(1, 0, 0);
    }

    char reply_buf[1024];
    send_command("list");
    if (!wait_reply(1, sizeof(reply_buf), reply_buf))
    {
	fprintf(stderr, "Cannot decode variable mappings\n");
	return;
    }

    memset(csv_map, 0, sizeof(csv_map));

    // Decode list response
    int csv_index = 0;
    char *save_ptr;
    const char *token;
    for (token = strtok_r(reply_buf, " \r\n", &save_ptr);
	 token != 0;
	 token = strtok_r(NULL, " \r\n", &save_ptr))
    {
	if (debug_comms)
	    printf("Testing token '%s'\n", token);

	// Ignore the OK response
	if (strcmp(token, "OK") == 0)
	    continue;

	// Find variable in list
	bool found_var = false;
	for (var_index = 0; var_index < num_variables; var_index++)
	{
	    if (strcmp(token, variable_names[var_index]) == 0)
	    {
		csv_map[csv_index] = var_index;
		found_var = true;

		if (debug_comms)
		    printf("Mapped csv_index=%d to var_index=%d\n",
			   csv_index, var_index);
		break;
	    }
	}

	if (!found_var)
	{
	    fprintf(stderr, "Cound not find variable '%s' in expected list\n",
		    token);
	    csv_map[csv_index] = -1;
	}

	csv_index++;
    }

    send_command("csv on");
    wait_reply(1, 0, 0);
    send_command("start");
    wait_reply(1, 0, 0);
}

bool mms_client_get_variables(int num_variables,
			      double *variable_values)
{
    assert(server_socket != -1);
    assert(num_variables <= MAX_WATCHED_VARIABLES);

    // Fill in cache values
    int var_index;
    for (var_index = 0; var_index < num_variables; var_index++)
	variable_values[var_index] = cached_values[var_index];

    fill_buffer();

    while (true)
    {
	char *line = get_line();
	if (line == 0)
	    break;

	char *save_ptr;
	const char *token;
	int csv_index = 0;
	for (token = strtok_r(line, ",", &save_ptr);
	     token != 0;
	     token = strtok_r(NULL, ",", &save_ptr))
	{
	    double v = 0.0;
	    if (strcmp(token, "true") == 0)
		v = 1;
	    else if (strcmp(token, "false") == 0)
		v = 0;
	    else
		v = atof(token);

	    var_index = csv_map[csv_index];

	    if (debug_comms)
		printf("token = '%s' v=%g csv_index=%d var_index=%d\n",
		       token, v, csv_index, var_index);

	    if (var_index >= 0)
	    {
		variable_values[var_index] = v;
		cached_values[var_index] = v;
	    }

	    csv_index++;
	}
    }

    return true;
}

void mms_set_variables(int num_variables,
                       const char **variable_names,
                       const double *variable_values)
{
    assert(server_socket != -1);
    assert(num_variables <= MAX_WATCHED_VARIABLES);

    char send_buf[1024];
    strcpy(send_buf, "set ");
    int p = strlen(send_buf);

    int i;
    for (i = 0; i < num_variables; i++)
    {
        snprintf(send_buf + p, sizeof(send_buf) - p, "%s=%g ",
                 variable_names[i], variable_values[i]);
	p += strlen(send_buf + p);
	assert(p < sizeof(send_buf));
    } 

    send_command(send_buf);
}

void mms_client_disconnect()
{
    if (server_socket != -1)
    {
	send_command("stop");

	close(server_socket);
	server_socket = -1;
    }
}
