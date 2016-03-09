/* $Id:$
 *
 * Copyright   : (c) 2016 by RAEdyne Pty Ltd.  All Rights Reserved
 * Project     : RAEdyne
 * File        : mms_client.h
 *
 * Author      : Denis Dowling
 * Created     : 9/3/2016
 *
 * Description : Library to access the mms_logger server from a C application
 *               like the ArduCopter application
 */
#ifndef MMS_CLIENT_H
#define MMS_CLIENT_H

#include <stdbool.h>

/**
 * Connect to the mms server application at address. If address is NULL then
 * connect to the server on the local system
 *
 * @param address The address to connect to. Use NULL for connect to the localhost
 * @return Will return true if successfully connected to the server and false otherwise.
 */
bool mms_client_connect(const char *address);

/**
 * Specify the variable to watch on the server an the desired update rate for
 * the variables. If variables are polled more frequenty than the update rate
 * then they will return the same values.
 *
 * @param update_period_seconds The number of seconds between successive updates from the MMS server. This should match the expected polling rate from the client.
 * @param num_variables The number of variables to poll. This should match the number of entries in variable_names.
 * @param variable_names An array of variable names to send from the server to this client.
 *
 */
void mms_client_watch_variables(double update_period_seconds,
				int num_variables,
				const char **variable_names);

/**
 * Get the current values of all watch variables.
 *
 * @param num_variable The number of variables to poll. This should match the number of entries in the variable_values array should should also match what was configured in mms_client_watch_variables.
 * @returns Will return true if new variables are returned and false if only cached values are available.
 *
 */
bool mms_client_get_variables(int num_variables,
			      double *variable_values);

/**
 * Disconnect from the server.
 */
void mms_client_disconnect();


#endif
