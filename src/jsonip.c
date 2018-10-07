/*****************************************************************************/
/*                                                                           */
/* Telize 2.0.0                                                              */
/* Copyright (c) 2013-2018, Frederic Cambus                                  */
/* https://www.telize.com                                                    */
/*                                                                           */
/* Created:      2013-08-15                                                  */
/* Last Updated: 2018-10-05                                                  */
/*                                                                           */
/* Telize is released under the BSD 2-Clause license.                        */
/* See LICENSE file for details.                                             */
/*                                                                           */
/*****************************************************************************/

#include <stdio.h>
#include <stdbool.h>
#include <sys/socket.h>

#include <kore/kore.h>
#include <kore/http.h>

int		jsonip(struct http_request *);

int
jsonip(struct http_request *req)
{
	const char *visitor_ip, *ip;
	char *answer, *callback, *addr;
	bool is_callback = false;

	http_populate_get(req);

	struct kore_buf json;
	kore_buf_init(&json, 0);

	addr = kore_malloc(INET6_ADDRSTRLEN);

	if (req->owner->addrtype == AF_INET) {
		inet_ntop(req->owner->addrtype, &(req->owner->addr.ipv4.sin_addr), addr, INET6_ADDRSTRLEN);
	} else {
		inet_ntop(req->owner->addrtype, &(req->owner->addr.ipv6.sin6_addr), addr, INET6_ADDRSTRLEN);
	}

	if (http_request_header(req, "X-Forwarded-For", &visitor_ip)) {
		strtok(visitor_ip, ",");
		ip = visitor_ip;
	} else {
		ip = addr;
	}

	if (http_argument_get_string(req, "callback", &callback)) {
		kore_buf_appendf(&json, "%s(", callback);
		is_callback = true;
	}

	kore_buf_appendf(&json, "{\"ip\":\"%s\"", ip);

	kore_buf_append(&json, is_callback ? "});\n" : "}\n", is_callback ? 4 : 2);
	answer = kore_buf_stringify(&json, NULL);

	// CORS
	http_response_header(req, "Access-Control-Allow-Origin", "*");
	http_response_header(req, "Cache-Control", "no-cache");

	http_response_header(req, "content-type", "application/json; charset=utf-8");
	http_response(req, 200, answer, strlen(answer));

	kore_buf_free(&json);
	kore_free(addr);

	return (KORE_RESULT_OK);
}
