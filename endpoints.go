/*
 * Telize 3.1.0
 * Copyright (c) 2013-2022, Frederic Cambus
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
	"github.com/go-chi/chi/v5"
	"io"
	"net"
	"net/http"
	"time"
)

func ip(w http.ResponseWriter, r *http.Request) {
	ip := request_ip(r)

	w.Header().Set("Cache-Control", "no-cache")
	w.Header().Set("Content-Type", "text/plain")

	io.WriteString(w, ip)
}

func jsonip(w http.ResponseWriter, r *http.Request) {
	ip := request_ip(r)
	jsonip := payload{IP: ip}

	jsonify(w, r, &jsonip)
}

func location(w http.ResponseWriter, r *http.Request) {
	ip := chi.URLParam(r, "ip")

	if ip == "" {
		ip = request_ip(r)
	}

	address := net.ParseIP(ip)

	var asn_record ASN

	err := asn.Lookup(address, &asn_record)
	if err != nil {
		errorCode(w, 400, 401, "Input string is not a valid IP address")
		return
	}

	var record City

	err = city.Lookup(address, &record)
	if err != nil {
		errorCode(w, 400, 401, "Input string is not a valid IP address")
		return
	}

	location := payload{
		IP:                ip,
		Continent:         record.Continent.Code,
		Country:           record.Country.Names["en"],
		CountryCode:       record.Country.IsoCode,
		CountryCode3:      CountryCode3[record.Country.IsoCode],
		IsInEuropeanUnion: record.Country.IsInEuropeanUnion,
		City:              record.City.Names["en"],
		PostalCode:        record.Postal.Code,
		Latitude:          record.Location.Latitude,
		Longitude:         record.Location.Longitude,
		TimeZone:          record.Location.TimeZone,
		ASN:               asn_record.AutonomousSystemNumber,
		Organization:      asn_record.AutonomousSystemOrganization,
	}

	if record.Subdivisions != nil {
		location.Region = record.Subdivisions[0].Names["en"]
		location.RegionCode = record.Subdivisions[0].IsoCode
	}

	if record.Location.TimeZone != "" {
		if tz, err := time.LoadLocation(record.Location.TimeZone); err == nil {
			_, offset := time.Now().In(tz).Zone()
			location.Offset = &offset
		}
	}

	jsonify(w, r, &location)
}
