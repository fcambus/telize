
                               t  e  l  i  z  e
                           _______________________
                     ______\                     /_______
                    \\     \\           ___     //      /
               __    \ ____  \   __    /   \   _____/\ / ____
           ___/  \____/  _//____/  \___\___/___\__   /__/  _//____
          \\__    ____  __/  __     __      ____    ____  __/  __///
            /      \_   |/    \_     /       \/     /_/   |/    \_
          \\\   ___/\___       /____/\_______/\   ___/\___       /
      <0(--- \__/ -h7- \______/   \       .    \__/ ---- \______/ --(0>
                           \      .\     /.      .
                            \      .\   //      /
                             \______\\ //______/
                                      Y

## Description

Telize is a REST API built in Go allowing to get a visitor IP address and
to query location information from any IP address. It outputs JSON-encoded
IP geolocation data, and supports both JSON and JSONP.

Geolocation operations are performed using the MaxMind DB Reader for Go
which caches the database in RAM. Therefore, Telize has very minimal
overhead and should be blazing fast.

## Requirements

Telize requires the following Go libraries:

- chi: lightweight, idiomatic and composable router - https://github.com/go-chi/chi
- maxminddb-golang: MaxMind DB Reader for Go - https://github.com/oschwald/maxminddb-golang

### GeoIP2 databases

Telize requires the free [GeoLite2 databases][1] from MaxMind.

Telize will look for the `GeoLite2 City` and `GeoLite2 ASN` databases in
`/var/db/GeoIP` by default.

## Installation

Build and install with the `go` tool, all dependencies will be automatically
fetched and compiled:

	go build
	go install telize

## Usage

By default, Telize will bind on localhost, port 8080.

	USAGE:
	  -host string
	    	Set the server host (default "127.0.0.1")
	  -port string
	    	Set the server port (default "8080")
	  -version
	    	Display version

## Running Telize at boot time

### Systemd unit file

Telize is bundled with a systemd unit file, see: `systemd/telize.service`

Copy the `systemd/telize.service` file in `/etc/systemd/system` and the Telize
binary in `/usr/local/sbin`.

To launch the daemon at startup, run:

	systemctl enable telize

## Making Queries

For complete API documentation and usage examples, please check the
project site.

Telize supports JSONP callbacks.

### Get IP address in Plain text format

- Example: http://127.0.0.1:8080/ip

### Get IP address in JSON format

- Example (JSON): http://127.0.0.1:8080/jsonip
- Example (JSONP): http://127.0.0.1:8080/jsonip?callback=getip

### Get IP address location in JSON format

Calling the API endpoint without any parameter will return the visitor
IP address:

- Example (JSON): http://127.0.0.1:8080/location
- Example (JSONP): http://127.0.0.1:8080/location?callback=getlocation

Appending an IP address as parameter will return location information for
the given address:

- Example (JSON): http://127.0.0.1:8080/location/46.19.37.108
- Example (JSONP): http://127.0.0.1:8080/location/46.19.37.108?callback=getlocation

## Client Errors

When incorrect user input is entered, the server returns an HTTP 400 Error
(Bad Request), along with a JSON-encoded error message.

- Code 401: Input string is not a valid IP address

## Telize and proxies

Telize handles the 'X-Forwarded-For' HTTP header if present, and returns
data for the first IP address of the list.

## CORS Support (Cross-origin resource sharing)

Telize has CORS enabled by default with the following policy:

	Access-Control-Allow-Origin: *

## Timezone offsets

Since version 3.0.0, Telize now dynamically calculates timezone offsets
(UTC time offset) and adds data to the payload.

## License

Telize is released under the BSD 2-Clause license. See `LICENSE` file
for details.

## Author

Telize is developed by Frederic Cambus.

- Site: https://www.cambus.net

## Resources

Project homepage: https://www.telize.com

Latest tarball release: https://www.statdns.com/telize/telize-3.1.1.tar.gz

GitHub: https://github.com/fcambus/telize

[1]: https://dev.maxmind.com/geoip/geoip2/geolite2/
