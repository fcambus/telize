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
	"flag"
	"fmt"
	"github.com/go-chi/chi/v5"
	"io"
	"net"
	"net/http"
	"net/http/fcgi"
	"os"
)

func ip(w http.ResponseWriter, r *http.Request) {
	ip, _, _ := net.SplitHostPort(r.RemoteAddr)

	io.WriteString(w, ip)
}

func main() {
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

	address := *host + ":" + *port

	fmt.Println("Listening on ("+mode+" mode):", address)

	r := chi.NewRouter()
	r.Get("/ip", ip)

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
