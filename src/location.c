/*
 * Telize 3.0.0
 * Copyright (c) 2013-2020, Frederic Cambus
 * https://www.telize.com
 *
 * Created:      2013-08-15
 * Last Updated: 2020-10-21
 *
 * Telize is released under the BSD 2-Clause license.
 * See LICENSE file for details.
 */

#include <time.h>

#include "telize.h"
#include "location.h"

#include "assets.h"

static int	response_sent(struct netbuf *);

int
request_location(struct http_request *req)
{
	struct tm		*info;
	struct sockaddr_in	ipv4;
	struct sockaddr_in6	ipv6;
	MMDB_lookup_result_s	lookup;
	time_t			rawtime;
	struct kore_buf		*json, buf;
	MMDB_entry_data_s	entry_data;
	size_t			tz_len, len;
	char			*ptr, *callback, *tz;
	int			slen, gai_error, mmdb_error;
	char			ip[INET6_ADDRSTRLEN], tzenv[256];

	if (!telize_request_ip(req, ip, sizeof(ip))) {
		http_response(req, HTTP_STATUS_INTERNAL_ERROR, NULL, 0);
		return (KORE_RESULT_OK);
	}

	http_populate_get(req);

	/* Set response headers */
	http_response_header(req, "Access-Control-Allow-Origin", "*");
	http_response_header(req, "Cache-Control", "no-cache");
	http_response_header(req, "Content-Type",
	    "application/json; charset=utf-8");

	/* Specific IP passed in the URL */
	if (req->hdlr->type == HANDLER_TYPE_DYNAMIC) {
		if ((ptr = strrchr(req->path, '/')) != NULL) {
			ptr++;
			len = strlen(ptr);

			if (len == 0 || len > sizeof(ip) - 1) {
				http_response(req, HTTP_STATUS_BAD_REQUEST,
				    asset_bad_ip_json, asset_len_bad_ip_json);
				return (KORE_RESULT_OK);
			}

			memcpy(ip, ptr, len);
			ip[len] = '\0';

			if (!inet_pton(AF_INET, ip, &(ipv4.sin_addr)) &&
			    !inet_pton(AF_INET6, ip, &(ipv6.sin6_addr))) {
				http_response(req, HTTP_STATUS_BAD_REQUEST,
				    asset_bad_ip_json, asset_len_bad_ip_json);
				return (KORE_RESULT_OK);
			}
		}
	}

	callback = NULL;
	json = kore_buf_alloc(4096);

	/* Handle callback parameter */
	if (http_argument_get_string(req, "callback", &callback))
		kore_buf_appendf(json, "%s(", callback);

	kore_buf_appendf(json, "{\"ip\":\"%s\"", ip);

	/* GeoLite2 City lookup */
	lookup = MMDB_lookup_string(&telize_city, ip, &gai_error, &mmdb_error);

	telize_getdata(json, &lookup, &entry_data,
	    "continent_code", ENTRY_TYPE_STRING, "continent", "code", NULL);
	telize_getdata(json, &lookup, &entry_data,
	    "country", ENTRY_TYPE_STRING, "country", "names", "en", NULL);

	MMDB_get_value(&lookup.entry, &entry_data, "country", "iso_code", NULL);

	if (entry_data.has_data) {
		kore_buf_appendf(json, ",\"country_code\":\"%.*s\"",
		    entry_data.data_size, entry_data.utf8_string);

		for (size_t loop = 0; loop < COUNTRIES; loop++) {
			if (!strncmp(country_code_array[loop],
			    entry_data.utf8_string, 2)) {
				kore_buf_appendf(json,
				    ",\"country_code3\":\"%s\"",
				    country_code3_array[loop]);
				break;
			}
		}
	}

	telize_getdata(json, &lookup, &entry_data, "region",
	    ENTRY_TYPE_STRING, "subdivisions", "0", "names", "en", NULL);

	telize_getdata(json, &lookup, &entry_data, "region_code",
	    ENTRY_TYPE_STRING, "subdivisions", "0", "iso_code", NULL);

	telize_getdata(json, &lookup, &entry_data, "city",
	    ENTRY_TYPE_STRING, "city", "names", "en", NULL);

	telize_getdata(json, &lookup, &entry_data, "postal_code",
	    ENTRY_TYPE_STRING, "postal", "code", NULL);

	telize_getdata(json, &lookup, &entry_data, "latitude",
	    ENTRY_TYPE_DOUBLE, "location", "latitude", NULL);

	telize_getdata(json, &lookup, &entry_data, "longitude",
	    ENTRY_TYPE_DOUBLE, "location", "longitude", NULL);

	MMDB_get_value(&lookup.entry, &entry_data,
	    "location", "time_zone", NULL);

	if (entry_data.has_data) {
		tz_len = entry_data.data_size;
		tz = strndup(entry_data.utf8_string, tz_len);
		if (!tz) {
			kore_buf_free(json);
			http_response(req, HTTP_STATUS_INTERNAL_ERROR, NULL, 0);
			return (KORE_RESULT_OK);
		}

		kore_buf_init(&buf, tz_len);
		kore_buf_append(&buf, tz, tz_len);
		kore_buf_replace_string(&buf, "/", "\\/", 2);
		kore_buf_appendf(json, ",\"timezone\":\"%s\"",
		    kore_buf_stringify(&buf, NULL));
		kore_buf_cleanup(&buf);

		slen = snprintf(tzenv, sizeof(tzenv), "TZ=%s", tz);
		if (slen == -1 || (size_t)slen >= sizeof(tzenv)) {
			kore_buf_free(json);
			http_response(req, HTTP_STATUS_INTERNAL_ERROR, NULL, 0);
			return (KORE_RESULT_OK);
		}

		putenv(tzenv);
		tzset();
		time(&rawtime);
		info = localtime(&rawtime);
		kore_buf_appendf(json, ",\"offset\":%d", info->tm_gmtoff);

		free(tz);
	}

	/* GeoLite2 ASN lookup */
	lookup = MMDB_lookup_string(&telize_asn, ip, &gai_error, &mmdb_error);

	telize_getdata(json, &lookup, &entry_data, "asn",
	    ENTRY_TYPE_UINT32, "autonomous_system_number", NULL);

	telize_getdata(json, &lookup, &entry_data, "organization",
	    ENTRY_TYPE_STRING, "autonomous_system_organization", NULL);

	kore_buf_append(json, callback != NULL ? "});\n" : "}\n",
	    callback != NULL ? 4 : 2);

	http_response_stream(req, HTTP_STATUS_OK, json->data,
	    json->offset, response_sent, json);

	return (KORE_RESULT_OK);
}

static int
response_sent(struct netbuf *nb)
{
	struct kore_buf		*buf = nb->extra;

	kore_buf_free(buf);

	return (KORE_RESULT_OK);
}
