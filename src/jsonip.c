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
request_json_ip(struct http_request *req)
{
	struct kore_buf		json;
	char			*answer, *callback;
	char			ip[INET6_ADDRSTRLEN];

	if (!telize_request_ip(req, ip, sizeof(ip))) {
		http_response(req, HTTP_STATUS_INTERNAL_ERROR, NULL, 0);
		return (KORE_RESULT_OK);
	}

	callback = NULL;

	http_populate_get(req);
	kore_buf_init(&json, 4096);

	http_response_header(req, "Access-Control-Allow-Origin", "*");
	http_response_header(req, "Cache-Control", "no-cache");
	http_response_header(req, "Content-Type",
	    "application/json; charset=utf-8");

	if (http_argument_get_string(req, "callback", &callback))
		kore_buf_appendf(&json, "%s(", callback);

	kore_buf_appendf(&json, "{\"ip\":\"%s\"", ip);
	kore_buf_append(&json, callback != NULL ? "});\n" : "}\n",
	    callback != NULL ? 4 : 2);

	answer = kore_buf_stringify(&json, NULL);
	http_response(req, HTTP_STATUS_OK, answer, strlen(answer));
	kore_buf_cleanup(&json);

	return (KORE_RESULT_OK);
}
