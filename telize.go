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
var city *maxminddb.Reader

type ASN struct {
	AutonomousSystemNumber       uint   `maxminddb:"autonomous_system_number"`
	AutonomousSystemOrganization string `maxminddb:"autonomous_system_organization"`
}

type City struct {
	Continent struct {
		Code  string            `maxminddb:"code"`
		Names map[string]string `maxminddb:"names"`
	} `maxminddb:"continent"`
	Country struct {
		IsInEuropeanUnion bool              `maxminddb:"is_in_european_union"`
		IsoCode           string            `maxminddb:"iso_code"`
		Names             map[string]string `maxminddb:"names"`
	} `maxminddb:"country"`
	Subdivisions []struct {
		IsoCode string            `maxminddb:"iso_code"`
		Names   map[string]string `maxminddb:"names"`
	} `maxminddb:"subdivisions"`
	City struct {
		Names map[string]string `maxminddb:"names"`
	} `maxminddb:"city"`
	Postal struct {
		Code string `maxminddb:"code"`
	} `maxminddb:"postal"`
	Location struct {
		Latitude  float64 `maxminddb:"latitude"`
		Longitude float64 `maxminddb:"longitude"`
		TimeZone  string  `maxminddb:"time_zone"`
	} `maxminddb:"location"`
}

type payload struct {
	IP                string  `json:"ip"`
	Continent         string  `json:"continent_code"`
	Country           string  `json:"country"`
	CountryCode       string  `json:"country_code"`
	IsInEuropeanUnion bool    `json:"is_in_european_union"`
	Region            string  `json:"region"`
	RegionCode        string  `json:"region_code"`
	City              string  `json:"city"`
	PostalCode        string  `json:"postal_code"`
	Latitude          float64 `json:"latitude"`
	Longitude         float64 `json:"longitude"`
	TimeZone          string  `json:"timezone"`
	ASN               uint    `json:"asn,omitempty"`
	Organization      string  `json:"organization,omitempty"`
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

	var record City

	err = city.Lookup(ip_, &record)
	if err != nil {
		log.Panic(err)
	}

	jsonip := payload{
		IP:                ip,
		Continent:         record.Continent.Code,
		Country:           record.Country.Names["en"],
		CountryCode:       record.Country.IsoCode,
		IsInEuropeanUnion: record.Country.IsInEuropeanUnion,
		Region:            record.Subdivisions[0].Names["en"],
		RegionCode:        record.Subdivisions[0].IsoCode,
		City:              record.City.Names["en"],
		PostalCode:        record.Postal.Code,
		Latitude:          record.Location.Latitude,
		Longitude:         record.Location.Longitude,
		TimeZone:          record.Location.TimeZone,
		ASN:               asn_record.AutonomousSystemNumber,
		Organization:      asn_record.AutonomousSystemOrganization,
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

	city, err = maxminddb.Open("GeoLite2-City.mmdb")
	if err != nil {
		log.Fatal(err)
	}
	defer city.Close()

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
