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

#include <sys/socket.h>

#include <kore/kore.h>
#include <kore/http.h>

int		ip(struct http_request *);

int
ip(struct http_request *req)
{
	const char *visitor_ip;
	char *ip, *addr;

	addr = kore_malloc(INET6_ADDRSTRLEN);

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

	http_response_header(req, "content-type", "text/plain");
	http_response(req, 200, ip, strlen(ip));

	kore_free(addr);

	return (KORE_RESULT_OK);
}
