/**
 * @file mms_client.h
 *
 * @brief Library to access the mms_logger server from a C application.
 * @author Denis Dowling
 * @date 9/3/2016
 *
 * Copyright 2016 by Raedyne Pty Ltd. All Rights Reserved
 *
 */
#ifndef MMS_CLIENT_H
#define MMS_CLIENT_H

#include <stdbool.h>

/**
 * @brief Connect to the MMS server application
 * 
 * Connect to the mms server application at address. If address is NULL then
 * connect to the server on the local system
 *
 * @param address The address to connect to. Use NULL for connect to the localhost
 * @return Will return true if successfully connected to the server and false otherwise.
 */
bool mms_client_connect(const char *address);

/**
 * @brief Specify which variables to watch
 *
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
 * @brief Get the current values of all watch variables.
 *
 * Get the current values of all watched variables. This will populate the variable_value array with floating point representation of the variable. 
 *
 * @param num_variables The number of variables to poll. This should match the number of entries in the variable_values array should should also match what was configured in mms_client_watch_variables.
 * @param variable_values An array of doubles where the values of the watched variables will be copied.
 * @returns Will return true if new variables are returned and false if only cached values are available.
 *
 */
bool mms_client_get_variables(int num_variables,
			      double *variable_values);

/**
 * @brief Set supplied variables to the given values
 *
 * Set variables in variable_names array to the corresponding values in variable_values array.
 *
 * @param num_variables The number of variables in the variable_names and valiable_values arrays
 * @param variable_name The names of the variables to set
 * @param variable_values The values to set the variables to
 */
void mms_set_variables(int num_variables,
		       const char **variable_names,
		       const double *variable_values);

/**
 * @brief Disconnect from the server.
 *
 * Stop all active variable watches and disconnect from the MMS server.
 */
void mms_client_disconnect();


#endif
