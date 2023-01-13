/*
 * Telize 4.0.0
 * Copyright (c) 2013-2023, Frederic Cambus
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
	"flag"
	"fmt"
	"log"
	"net/http"
	"os"

	"github.com/go-chi/chi/v5"
	"github.com/oschwald/maxminddb-golang"
)

var asn *maxminddb.Reader
var city *maxminddb.Reader

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

	asn, err = maxminddb.Open("/var/db/GeoIP/GeoLite2-ASN.mmdb")
	if err != nil {
		log.Fatal(err)
	}
	defer asn.Close()

	city, err = maxminddb.Open("/var/db/GeoIP/GeoLite2-City.mmdb")
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

	fmt.Print("Listening on: http://", address+"\n")
	log.Fatal(http.ListenAndServe(address, r))
}
