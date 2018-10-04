/*****************************************************************************/
/*                                                                           */
/* Telize 2.0.0                                                              */
/* Copyright (c) 2013-2018, Frederic Cambus                                  */
/* https://www.telize.com                                                    */
/*                                                                           */
/* Created:      2013-08-15                                                  */
/* Last Updated: 2018-10-04                                                  */
/*                                                                           */
/* Telize is released under the BSD 2-Clause license.                        */
/* See LICENSE file for details.                                             */
/*                                                                           */
/*****************************************************************************/

#include <sys/socket.h>

#include <kore/kore.h>
#include <kore/http.h>

#include <jansson.h>

int		jsonip(struct http_request *);

int
jsonip(struct http_request *req)
{
	const char *visitor_ip;
	char *answer, *ip, addr[INET6_ADDRSTRLEN];
	json_t *output = json_object();

	if (req->owner->addrtype == AF_INET) {
		inet_ntop(req->owner->addrtype, &(req->owner->addr.ipv4.sin_addr), addr, sizeof(addr));
	} else {
		inet_ntop(req->owner->addrtype, &(req->owner->addr.ipv6.sin6_addr), addr, sizeof(addr));
	}

	if (http_request_header(req, "X-Forwarded-For", &visitor_ip)) {
		ip = kore_strdup(visitor_ip);
	} else {
		ip = addr;
	}

	json_object_set_new(output, "ip", json_string(ip));
	answer = json_dumps(output, JSON_INDENT(3));

	http_response_header(req, "content-type", "application/json");
	http_response(req, 200, answer, strlen(answer));

	return (KORE_RESULT_OK);
}
