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
#include "mms_client.h"

#include <stdio.h>
#include <unistd.h>
#include <math.h>

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

    double update_period_seconds = 0.1;

    const char *variable_names[] = {
	"pole_detected",
	"pole_range",
	"pole_theta"
    };
    int num_variables = sizeof(variable_names)/sizeof(variable_names[0]);

    mms_client_watch_variables(update_period_seconds,
			       num_variables,
			       variable_names);

    double t = 0.0;
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

	// Set some values to simulated quantities
	double range = 2.0 + sin(t);
	double theta = sin(t*3.0) * 30.0;
	const char *set_variables[] = {
	    "pole_range",
	    "pole_theta"
        };
	double set_values[] = {
	    range,
	    theta
        };
	mms_set_variables(2, set_variables, set_values);

	// Sleep to simulate the polling delay
	usleep(update_period_seconds * 1e6);

	t += update_period_seconds;
    }

    mms_client_disconnect();

    return 0;
}
