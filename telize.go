/*
 * Telize 3.1.0
 * Copyright (c) 2013-2022, Frederic Cambus
 * https://www.telize.com
 *
 * Created:      2013-08-15
 * Last Updated: 2022-11-21
 *
 * Telize is released under the BSD 2-Clause license.
 * See LICENSE file for details.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"github.com/go-chi/chi/v5"
	"github.com/oschwald/maxminddb-golang"
	"io"
	"log"
	"net"
	"net/http"
	"os"
	"time"
)

type ErrorCode struct {
	Code    int    `json:"code"`
	Message string `json:"message"`
}

var asn *maxminddb.Reader
var city *maxminddb.Reader

func ip(w http.ResponseWriter, r *http.Request) {
	ip, _, _ := net.SplitHostPort(r.RemoteAddr)

	w.Header().Set("Cache-Control", "no-cache")
	w.Header().Set("Content-Type", "text/plain")

	io.WriteString(w, ip)
}

// Return an HTTP Error along with a JSON-encoded error message
func error_code(w http.ResponseWriter, status int, code int, message string) {
	w.Header().Set("Cache-Control", "no-cache")
	w.Header().Set("Content-Type", "application/json")

	if output, err := json.Marshal(ErrorCode{Code: code, Message: message}); err == nil {
		w.WriteHeader(status)
		io.WriteString(w, string(output))
	}
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

func jsonip(w http.ResponseWriter, r *http.Request) {
	ip, _, _ := net.SplitHostPort(r.RemoteAddr)
	jsonip := payload{IP: ip}

	jsonify(w, r, &jsonip)
}

func location(w http.ResponseWriter, r *http.Request) {
	ip := chi.URLParam(r, "ip")

	if ip == "" {
		ip, _, _ = net.SplitHostPort(r.RemoteAddr)
	}

	ip_ := net.ParseIP(ip)

	var asn_record ASN

	err := asn.Lookup(ip_, &asn_record)
	if err != nil {
		error_code(w, 400, 401, "Input string is not a valid IP address")
		return
	}

	var record City

	err = city.Lookup(ip_, &record)
	if err != nil {
		error_code(w, 400, 401, "Input string is not a valid IP address")
		return
	}

	jsonip := payload{
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
		jsonip.Region = record.Subdivisions[0].Names["en"]
		jsonip.RegionCode = record.Subdivisions[0].IsoCode
	}

	if record.Location.TimeZone != "" {
		if tz, err := time.LoadLocation(record.Location.TimeZone); err == nil {
			_, offset := time.Now().In(tz).Zone()
			jsonip.Offset = &offset
		}
	}

	jsonify(w, r, &jsonip)
}

func main() {
	var err error

	host := flag.String("host", "127.0.0.1", "Set the server host")
	port := flag.String("port", "8080", "Set the server port")
	version := flag.Bool("version", false, "Display version")

	flag.Usage = func() {
		fmt.Println("\nUSAGE:")
		flag.PrintDefaults()
	}
	flag.Parse()

	if *version {
		fmt.Println("Telize 4.0.0")
		os.Exit(0)
	}

	asn, err = maxminddb.Open("GeoLite2-ASN.mmdb")
	if err != nil {
		log.Fatal(err)
	}
	defer asn.Close()

	city, err = maxminddb.Open("GeoLite2-City.mmdb")
	if err != nil {
		log.Fatal(err)
	}
	defer city.Close()

	address := *host + ":" + *port

	r := chi.NewRouter()
	r.Get("/ip", ip)
	r.Get("/jsonip", jsonip)
	r.Get("/geoip", location)
	r.Get("/geoip/", location)
	r.Get("/geoip/{ip}", location)
	r.Get("/location", location)
	r.Get("/location/", location)
	r.Get("/location/{ip}", location)

	log.Fatal(http.ListenAndServe(address, r))

	fmt.Println("Listening on:", address)
}
