
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

Telize is a REST API built in C with Kore allowing to get a visitor IP
address and to query location information from any IP address. It outputs
JSON-encoded IP geolocation data, and supports both JSON and JSONP.

Geolocation operations are performed using libmaxminddb which caches the
database in RAM. Therefore, Telize has very minimal overhead and should
be blazing fast.

## Requirements

### Dependencies

Telize requires [Kore][1] 4.0.0+ and [libmaxminddb][2].

### GeoIP2 databases

Telize requires the free [GeoLite2 databases][3] from MaxMind.

The path to the `GeoLite2 City` and `GeoLite2 ASN` databases can be configured
in `conf/build.conf` via the `GEOIP2DIR` macro, pointing to `/var/db/GeoIP`
by default.

## Building

Using the kodev tool:

	kodev build

## Configuration

By default, Telize will only listen on `127.0.0.1` and `::1`. To add more
listeners, `bind` directives should be added in `conf/telize.conf` in the
server blocks.

## Running

Using the kodev tool:

	kodev run

## Access logs

The default Telize configuration does not have logging enabled, it must be
configured manually. There are commented out `accesslog` directives in
`conf/telize.conf` for each domain block.

If your Telize instance produces lots of logs, [Logswan][4] might be of
interest.

## Telize and proxies

Telize handles the 'X-Forwarded-For' HTTP header if present, and returns
data for the first IP address of the list.

## CORS Support (Cross-origin resource sharing)

Telize has CORS enabled by default with the following policy:

	Access-Control-Allow-Origin: *

## Usage

For complete API documentation and usage examples, please check the
project site.

### Get IP address in Plain text format

- Example: http://127.0.0.1:8080/ip

### Get IP address in JSON format

- Example (JSON): http://127.0.0.1:8080/jsonip
- Example (JSONP): http://127.0.0.1:8080/jsonip?callback=getip

### Get IP address location in JSON format

Calling the API endpoint without any parameter will return the visitor
IP address:

- Example (JSON): http://127.0.0.1:8080/location
- Example (JSONP): http://127.0.0.1:8080/location?callback=getgeoip

Appending an IP address as parameter will return location information for
the given address:

- Example (JSON): http://127.0.0.1:8080/location/46.19.37.108
- Example (JSONP): http://127.0.0.1:8080/location/46.19.37.108?callback=getgeoip

## License

Telize is released under the BSD 2-Clause license. See `LICENSE` file
for details.

## Author

Telize is developed by Frederic Cambus.

- Site: https://www.cambus.net

## Resources

Project homepage: https://www.telize.com

Latest tarball release: https://www.statdns.com/telize/telize-3.0.0.tar.gz

GitHub: https://github.com/fcambus/telize

[1]: https://kore.io
[2]: https://github.com/maxmind/libmaxminddb
[3]: https://dev.maxmind.com/geoip/geoip2/geolite2/
[4]: https://www.logswan.org
