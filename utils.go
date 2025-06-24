/*
 * Telize 4.0.0
 * Copyright (c) 2013-2025, Frederic Cambus
 * https://www.telize.com
 *
 * Created:      2013-08-15
 * Last Updated: 2022-11-24
 *
 * Telize is released under the BSD 2-Clause license.
 * See LICENSE file for details.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

package main

import (
	"encoding/json"
	"io"
	"net"
	"net/http"
	"strings"
)

// Return an HTTP Error along with a JSON-encoded error message
func errorCode(w http.ResponseWriter, status int, code int, message string) {
	type Error struct {
		Code    int    `json:"code"`
		Message string `json:"message"`
	}

	w.Header().Set("Cache-Control", "no-cache")
	w.Header().Set("Content-Type", "application/json")

	if output, err := json.Marshal(Error{Code: code, Message: message}); err == nil {
		w.WriteHeader(status)
		io.WriteString(w, string(output))
	}
}

func request_ip(r *http.Request) string {
	var ip string

	if xff := r.Header.Get("X-Forwarded-For"); xff != "" {
		ips := strings.Split(xff, ",")
		ip = ips[0]
	}

	if ip == "" {
		ip, _, _ = net.SplitHostPort(r.RemoteAddr)
	}

	return ip
}

// Generate JSON output
func jsonify(w http.ResponseWriter, r *http.Request, payload *payload) {
	callback := r.URL.Query().Get("callback")

	w.Header().Set("Cache-Control", "no-cache")
	w.Header().Set("Content-Type", "application/json")
	w.Header().Set("Access-Control-Allow-Origin", "*")

	if json, err := json.Marshal(payload); err == nil {
		if callback != "" {
			io.WriteString(w, callback+"("+string(json)+");")
		} else {
			io.WriteString(w, string(json))
		}
	}
}
