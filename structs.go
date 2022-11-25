/*
 * Telize 4.0.0
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
