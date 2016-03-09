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

    const char *variable_names[] = {
	"time",
	"real_edit",
	"bool_edit"
    };
    int num_variables = sizeof(variable_names)/sizeof(variable_names[0]);

    double update_period_seconds = 0.1;

    mms_client_watch_variables(update_period_seconds,
			       num_variables,
			       variable_names);

    while(true)
    {
	int i;

	double variable_values[num_variables];

	// Get the values from the server
	mms_client_get_variables(num_variables, variable_values);

	// Show the values
	for(i = 0; i < num_variables; i++)
	    printf("%s=%g ", variable_names[i], variable_values[i]);
	printf("\n");

	// Sleep to simulate the polling delay
	usleep(update_period_seconds * 1e6);
    }

    mms_client_disconnect();

    return 0;
}
