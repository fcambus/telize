/*****************************************************************************/
/*                                                                           */
/* Telize 2.0.0                                                              */
/* Copyright (c) 2013-2018, Frederic Cambus                                  */
/* https://www.telize.com                                                    */
/*                                                                           */
/* Created:      2013-08-15                                                  */
/* Last Updated: 2018-10-12                                                  */
/*                                                                           */
/* Telize is released under the BSD 2-Clause license.                        */
/* See LICENSE file for details.                                             */
/*                                                                           */
/*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>

#include <kore/kore.h>
#include <kore/http.h>

#include <maxminddb.h>

#include "location.h"

#define ENTRY_TYPE_UINT32 0
#define ENTRY_TYPE_STRING 1
#define ENTRY_TYPE_DOUBLE 2

MMDB_s asn;
MMDB_s city;

int	init(int);
int	location(struct http_request *);
void	getdata(struct kore_buf *, MMDB_lookup_result_s *, MMDB_entry_data_s *, char *, int, ...);

int
init(int state)
{
	if (MMDB_open("/var/db/GeoIP/GeoLite2-City.mmdb",
	    MMDB_MODE_MMAP, &city) != MMDB_SUCCESS) {
		kore_log(LOG_ERR, "can't open GeoLite2 City database: %s",
		    errno_s);
		return (KORE_RESULT_ERROR);
	}

	if (MMDB_open("/var/db/GeoIP/GeoLite2-ASN.mmdb",
	    MMDB_MODE_MMAP, &asn) != MMDB_SUCCESS) {
		kore_log(LOG_ERR, "can't open GeoLite2 ASN database: %s",
		    errno_s);
		return (KORE_RESULT_ERROR);
	}

	return (KORE_RESULT_OK);
}

void
getdata(struct kore_buf *json, MMDB_lookup_result_s *lookup, MMDB_entry_data_s *entry_data, char *field, int type, ...)
{
	va_list keys;
	va_start(keys, entry_data);

	MMDB_vget_value(&lookup->entry, entry_data, keys);
	if (entry_data->has_data) {
		switch(type) {
		case ENTRY_TYPE_UINT32:
			kore_buf_appendf(json, ",\"%s\":%d", field, entry_data->uint32);
			break;
		case ENTRY_TYPE_STRING:
			kore_buf_appendf(json, ",\"%s\":\"%.*s\"", field, entry_data->data_size, entry_data->utf8_string);
			break;
		case ENTRY_TYPE_DOUBLE:
			kore_buf_appendf(json, ",\"%s\":%.4f", field, entry_data->double_value);
			break;
		}
	}

	va_end(keys);
}

int
location(struct http_request *req)
{
	const char *custom_ip, *visitor_ip, *ip;
	char *answer, *callback, *addr;
	bool is_callback = false;

	int gai_error, mmdb_error;
	MMDB_lookup_result_s lookup;
	MMDB_entry_data_s entry_data;

	time_t rawtime;
	struct tm *info;

	struct sockaddr_in ipv4;
	struct sockaddr_in6 ipv6;

	http_populate_get(req);

	struct kore_buf json;
	kore_buf_init(&json, 4096);

	addr = kore_malloc(INET6_ADDRSTRLEN);

	/* Set response headers */
	http_response_header(req, "Access-Control-Allow-Origin", "*");
	http_response_header(req, "Cache-Control", "no-cache");
	http_response_header(req, "Content-Type", "application/json; charset=utf-8");

	/* IP address of the client originating the request */
	if (req->owner->addrtype == AF_INET)
		inet_ntop(req->owner->addrtype,
		    &(req->owner->addr.ipv4.sin_addr), addr, INET6_ADDRSTRLEN);
	else
		inet_ntop(req->owner->addrtype,
		    &(req->owner->addr.ipv6.sin6_addr), addr, INET6_ADDRSTRLEN);

	/* IP address specified in the "X-Forwarded-For" header */
	if (http_request_header(req, "X-Forwarded-For", &visitor_ip)) {
		strtok(visitor_ip, ",");
		ip = visitor_ip;
	} else {
		ip = addr;
	}

	/* Specific IP passed in the URL */
	if (req->hdlr->type == HANDLER_TYPE_DYNAMIC) {
		if ((custom_ip = strrchr(req->path, '/')) != NULL)
			custom_ip++;

		if (strlen(custom_ip))
			ip = custom_ip;
	}

	/* Check for invalid IP addresses */
	if (!inet_pton(AF_INET, ip, &(ipv4.sin_addr)) &&
	    !inet_pton(AF_INET6, ip, &(ipv6.sin6_addr))) {
		answer = "{\"code\": 401, \"message\": \"Input string is not a valid IP address\"}";
		http_response(req, 400, answer, strlen(answer));
		goto cleanup;
	}

	/* Handle callback parameter */
	if (http_argument_get_string(req, "callback", &callback)) {
		kore_buf_appendf(&json, "%s(", callback);
		is_callback = true;
	}

	kore_buf_appendf(&json, "{\"ip\":\"%s\"", ip);

	/* GeoLite2 City lookup */
	lookup = MMDB_lookup_string(&city, ip, &gai_error, &mmdb_error);

	getdata(&json, &lookup, &entry_data, "continent_code", ENTRY_TYPE_STRING, "continent", "code", NULL);
	getdata(&json, &lookup, &entry_data, "country", ENTRY_TYPE_STRING, "country", "names", "en", NULL);

	MMDB_get_value(&lookup.entry, &entry_data, "country", "iso_code", NULL);
	if (entry_data.has_data) {
		kore_buf_appendf(&json, ",\"country_code\":\"%.*s\"",
		    entry_data.data_size, entry_data.utf8_string);

		for (size_t loop = 0; loop < COUNTRIES; loop++) {
			if (!strncmp(country_code_array[loop], entry_data.utf8_string, 2)) {
				kore_buf_appendf(&json, ",\"country_code3\":\"%s\"", country_code3_array[loop]);
				break;
			}
		}
	}

	getdata(&json, &lookup, &entry_data, "region", ENTRY_TYPE_STRING, "subdivisions", "0", "names", "en", NULL);
	getdata(&json, &lookup, &entry_data, "region_code", ENTRY_TYPE_STRING, "subdivisions", "0", "iso_code", NULL);
	getdata(&json, &lookup, &entry_data, "city", ENTRY_TYPE_STRING, "city", "names", "en", NULL);
	getdata(&json, &lookup, &entry_data, "postal_code", ENTRY_TYPE_STRING, "postal", "code", NULL);
	getdata(&json, &lookup, &entry_data, "latitude", ENTRY_TYPE_DOUBLE, "location", "latitude", NULL);
	getdata(&json, &lookup, &entry_data, "longitude", ENTRY_TYPE_DOUBLE, "location", "longitude", NULL);

	MMDB_get_value(&lookup.entry, &entry_data, "location", "time_zone", NULL);
	if (entry_data.has_data) {
		char *timezone_value = strndup(entry_data.utf8_string, entry_data.data_size);
		kore_buf_appendf(&json, ",\"timezone\":\"%s\"", timezone_value);

		setenv("TZ", timezone_value, 1);
		tzset();
		time(&rawtime);
		info = localtime(&rawtime);
		kore_buf_appendf(&json, ",\"offset\":%d", info->tm_gmtoff);

		free(timezone_value);
	}

	/* GeoLite2 ASN lookup */
	lookup = MMDB_lookup_string(&asn, ip, &gai_error, &mmdb_error);

	getdata(&json, &lookup, &entry_data, "asn", ENTRY_TYPE_UINT32, "autonomous_system_number", NULL);
	getdata(&json, &lookup, &entry_data, "organization", ENTRY_TYPE_STRING, "autonomous_system_organization", NULL);

	kore_buf_append(&json, is_callback ? "});\n" : "}\n", is_callback ? 4 : 2);
	answer = kore_buf_stringify(&json, NULL);

	http_response(req, 200, answer, strlen(answer));

cleanup:
	kore_buf_free(&json);
	kore_free(addr);

	return (KORE_RESULT_OK);
}
