/* $Id:$
 *
 * Copyright   : (c) 2016 by RAEdyne Pty Ltd.  All Rights Reserved
 * Project     : RAEdyne
 * File        : client_test.c
 *
 * Author      : Denis Dowling
 * Created     : 9/3/2016
 *
 * Description : Test application to drive the mms_client interface to be
 *               used to communication with the mms_logger application.
 */
#include <stdio.h>
#include <unistd.h>
#include "mms_client.h"

int main(int argc, char **argv)
{
    const char *address = NULL;

    if (argc > 1)
	address = argv[1];

    if (!mms_client_connect(address))
    {
	fprintf(stderr, "Cannot connect to mms_logger server at %s\n",
		address ? address : "localhost");
	return 1;
    }

    double update_period_seconds = 1.0/15.0;
    double sleep_period_seconds = 1.0/50.0;

    const char *variable_names[] = {
	"pole_detected",
	"pole_range",
	"pole_theta",
	"wire_detected",
	"wire_range",
	"wire_theta",
	"ground_detected",
	"ground_range",
	"ground_theta"
    };
    int num_variables = sizeof(variable_names)/sizeof(variable_names[0]);

    mms_client_watch_variables(update_period_seconds,
			       num_variables,
			       variable_names);

    while(true)
    {
	int i;

	double variable_values[num_variables];

	// Get the values from the server
	bool res = mms_client_get_variables(num_variables, variable_values);
	printf("res=%d ", res);

	// Show the values
	for(i = 0; i < num_variables; i++)
	    printf("%s=%g ", variable_names[i], variable_values[i]);
	printf("\n");

	// Sleep to simulate the polling delay
	if (sleep_period_seconds != 0)
	    usleep(sleep_period_seconds * 1.0e6);

	if (!mms_client_is_connected())
	{
	    printf("Connection failed\n");
	    break;
	}
    }

    mms_client_disconnect();

    return 0;
}
