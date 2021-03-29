/*
 * Telize 3.1.0
 * Copyright (c) 2013-2021, Frederic Cambus
 * https://www.telize.com
 *
 * Created:      2013-08-15
 * Last Updated: 2019-03-13
 *
 * Telize is released under the BSD 2-Clause license.
 * See LICENSE file for details.
 */

#include "telize.h"

int
request_ip(struct http_request *req)
{
	char		ip[INET6_ADDRSTRLEN];

	if (!telize_request_ip(req, ip, sizeof(ip))) {
		http_response(req, HTTP_STATUS_INTERNAL_ERROR, NULL, 0);
		return (KORE_RESULT_OK);
	}

	http_response_header(req, "content-type", "text/plain");
	http_response(req, HTTP_STATUS_OK, ip, strlen(ip));

	return (KORE_RESULT_OK);
}
