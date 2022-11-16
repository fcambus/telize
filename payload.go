/*
 * Telize 3.1.0
 * Copyright (c) 2013-2022, Frederic Cambus
 * https://www.telize.com
 *
 * Created:      2013-08-15
 * Last Updated: 2022-06-23
 *
 * Telize is released under the BSD 2-Clause license.
 * See LICENSE file for details.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

package main

type payload struct {
	IP                string  `json:"ip"`
	Continent         string  `json:"continent_code,omitempty"`
	Country           string  `json:"country,omitempty"`
	CountryCode       string  `json:"country_code,omitempty"`
	CountryCode3      string  `json:"country_code3,omitempty"`
	IsInEuropeanUnion bool    `json:"is_in_european_union,omitempty"`
	Region            string  `json:"region,omitempty"`
	RegionCode        string  `json:"region_code,omitempty"`
	City              string  `json:"city,omitempty"`
	PostalCode        string  `json:"postal_code,omitempty"`
	Latitude          float64 `json:"latitude,omitempty"`
	Longitude         float64 `json:"longitude,omitempty"`
	TimeZone          string  `json:"timezone,omitempty"`
	Offset            *int    `json:"offset,omitempty"`
	ASN               uint    `json:"asn,omitempty"`
	Organization      string  `json:"organization,omitempty"`
}
