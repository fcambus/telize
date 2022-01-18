/*
 * Telize 3.1.0
 * Copyright (c) 2013-2022, Frederic Cambus
 * https://www.telize.com
 *
 * Created:      2013-08-15
 * Last Updated: 2022-01-18
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
	"net/http/fcgi"
	"os"
)

var asn *maxminddb.Reader

type ASN struct {
	AutonomousSystemNumber       uint   `maxminddb:"autonomous_system_number"`
	AutonomousSystemOrganization string `maxminddb:"autonomous_system_organization"`
}

type payload struct {
	IP           string `json:"ip"`
	ASN          uint   `json:"asn,omitempty"`
	Organization string `json:"organization,omitempty"`
}

func ip(w http.ResponseWriter, r *http.Request) {
	ip, _, _ := net.SplitHostPort(r.RemoteAddr)

	io.WriteString(w, ip)
}

func jsonip(w http.ResponseWriter, r *http.Request) {
	callback := r.URL.Query().Get("callback")

	ip, _, _ := net.SplitHostPort(r.RemoteAddr)
	jsonip := payload{IP: ip}

	if json, err := json.Marshal(jsonip); err == nil {
		if callback != "" {
			io.WriteString(w, callback+"("+string(json)+");")
		} else {
			io.WriteString(w, string(json))
		}
	}
}

func location(w http.ResponseWriter, r *http.Request) {
	callback := r.URL.Query().Get("callback")

	ip := chi.URLParam(r, "ip")

	ip_ := net.ParseIP(ip)

	var asn_record ASN

	err := asn.Lookup(ip_, &asn_record)
	if err != nil {
		log.Panic(err)
	}

	jsonip := payload{
		IP:           ip,
		ASN:          asn_record.AutonomousSystemNumber,
		Organization: asn_record.AutonomousSystemOrganization,
	}

	if json, err := json.Marshal(jsonip); err == nil {
		if callback != "" {
			io.WriteString(w, callback+"("+string(json)+");")
		} else {
			io.WriteString(w, string(json))
		}
	}
}

func main() {
	var err error

	fastcgi := flag.Bool("fastcgi", false, "Enable FastCGI mode")
	host := flag.String("host", "127.0.0.1", "Set the server host")
	port := flag.String("port", "8080", "Set the server port")
	version := flag.Bool("version", false, "Display version")

	mode := "HTTP"

	flag.Usage = func() {
		fmt.Println("\nUSAGE:")
		flag.PrintDefaults()
	}
	flag.Parse()

	if *version {
		fmt.Println("Telize 4.0.0")
		os.Exit(0)
	}

	if *fastcgi {
		mode = "FastCGI"
	}

	asn, err = maxminddb.Open("GeoLite2-ASN.mmdb")
	if err != nil {
		log.Fatal(err)
	}
	defer asn.Close()

	address := *host + ":" + *port

	fmt.Println("Listening on ("+mode+" mode):", address)

	r := chi.NewRouter()
	r.Get("/ip", ip)
	r.Get("/jsonip", jsonip)
	r.Get("/location/{ip}", location)

	if *fastcgi {
		listener, _ := net.Listen("tcp", address)

		if err := fcgi.Serve(listener, r); err != nil {
			fmt.Println("\nERROR:", err)
			os.Exit(1)
		}
	} else {
		if err := http.ListenAndServe(address, r); err != nil {
			fmt.Println("\nERROR:", err)
			os.Exit(1)
		}
	}
}
