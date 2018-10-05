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

#include <jansson.h>
#include <maxminddb.h>

MMDB_s asn;
MMDB_s city;

int		init(int);
int		location(struct http_request *);

int
init(int state)
{
	if (MMDB_open("/var/db/GeoIP/GeoLite2-City.mmdb",
	    MMDB_MODE_MMAP, &city) != MMDB_SUCCESS) {
		kore_log(LOG_ERR, "can't open GeoLite2 City database: %s", errno_s);
		return (KORE_RESULT_ERROR);
	}

	if (MMDB_open("/var/db/GeoIP/GeoLite2-ASN.mmdb",
	    MMDB_MODE_MMAP, &asn) != MMDB_SUCCESS) {
		kore_log(LOG_ERR, "can't open GeoLite2 ASN database: %s", errno_s);
		return (KORE_RESULT_ERROR);
	}

	return (KORE_RESULT_OK);
}

int
location(struct http_request *req)
{
	const char *visitor_ip;
	char *answer, *callback, *json, *ip, addr[INET6_ADDRSTRLEN];
	json_t *output = json_object();

	int gai_error, mmdb_error;
	MMDB_lookup_result_s lookup;
	MMDB_entry_data_s entry_data;

	http_populate_get(req);

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

	/* GeoLite2 City lookup */
	lookup = MMDB_lookup_string(&city, ip, &gai_error, &mmdb_error);

	json = json_dumps(output, JSON_INDENT(3));

	if (http_argument_get_string(req, "callback", &callback)) {
		asprintf(&answer, "%s(%s);", callback, json);
	} else {
		answer = json;
	}

	http_response_header(req, "content-type", "application/json; charset=utf-8");
	http_response(req, 200, answer, strlen(answer));

	return (KORE_RESULT_OK);
}
