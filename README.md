
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

Telize is a REST API built on Nginx and Lua allowing to get a visitor IP
address and to query location information from any IP address. It outputs
JSON-encoded IP geolocation data, and supports both JSON and JSONP.

Geolocation operations are performed using Nginx GeoIP2 module which caches
the database in RAM. Therefore, Telize has very minimal overhead and should
be blazing fast.

## Requirements

### Nginx modules

Telize requires Nginx 1.7.4+ compiled with the following modules:

- Real IP module (Optional HTTP module: --with-http_realip_module)
- Lua module 0.9.17+ (Third party module: [ngx_http_lua_module][1])
- GeoIP2 module (Third party module: [ngx_http_geoip2_module][2])

For optimal performance, please make sure the HttpLuaModule is built
against LuaJIT:

	ldd $(which nginx) | grep lua

### Lua modules

Telize requires the following Lua module:

- Lua CJSON

Installing via LuaRocks:

	luarocks install lua-cjson

Alternatively, this module can be installed directly via the operating
system's package manager.

### GeoIP2 databases

Telize requires the free [GeoLite2 databases][3] from MaxMind.

	mkdir -p /var/db/GeoIP
	cd /var/db/GeoIP
	wget https://geolite.maxmind.com/download/geoip/database/GeoLite2-City.tar.gz
	wget https://geolite.maxmind.com/download/geoip/database/GeoLite2-ASN.tar.gz
	tar xfz GeoLite2-City.tar.gz
	tar xfz GeoLite2-ASN.tar.gz
	mv */*mmdb .

## Installation

Copy both `country-code3.conf` and `timezone-offset.conf` in the Nginx
configuration files directory.

Edit `nginx.conf` to include those configuration files and to add directives
specifying the path to the GeoIP2 database files, within the http block.

	http {
		...

		include /etc/nginx/country-code3.conf;
		include /etc/nginx/timezone-offset.conf;

		geoip2 /var/db/GeoIP/GeoLite2-City.mmdb {
			$geoip2_continent_code continent code;
			$geoip2_country country names en;
			$geoip2_country_code country iso_code;
			$geoip2_region subdivisions 0 names en;
			$geoip2_region_code subdivisions 0 iso_code;
			$geoip2_city city names en;
			$geoip2_postal_code postal code;
			$geoip2_latitude location latitude;
			$geoip2_longitude location longitude;
			$geoip2_timezone location time_zone;
		}

		geoip2 /var/db/GeoIP/GeoLite2-ASN.mmdb {
			$geoip2_asn autonomous_system_number;
			$geoip2_organization autonomous_system_organization;
		}
	}

Then deploy the API configuration file `telize.conf` to the appropriate
location on your system, and reload Nginx configuration. If Telize is
deployed behind a load balancer, read the next section.

Depending on existing configuration, default values of `map_hash_max_size`
and `map_hash_bucket_size` variables might be too low and Nginx will refuse
to start.

If this happens, please add the following directives in the `http` block:

	map_hash_max_size 8192;
	map_hash_bucket_size 64;

On busy instances, the maximum number of open files limit must be increased
using the `worker_rlimit_nofile` directive in order to avoid running out of
available file descriptors.

## Access and Error logs

The default Telize configuration does not have logging enabled, it must be
configured manually.

If your Telize instance produces lots of logs, this might be of interest:
[Log rotation directly within Nginx configuration file][4].

## Telize and Load Balancers

When using Telize behind a load balancer, uncomment the following directives
in the server block and set the load balancer IP range accordingly:

	# set_real_ip_from 10.0.0.0/8; # Put your load balancer IP range here
	# real_ip_header X-Forwarded-For;

In the `/location` endpoint, replace this directive:

	proxy_set_header X-Real-IP $ip;

By the following directive:

	proxy_set_header X-Forwarded-For $ip;

## CORS Support (Cross-origin resource sharing)

Telize has CORS enabled by default since version 1.02. The following variables
define CORS behavior, within the `telize.conf` configuration file.

	set $cors "true";
	set $cors_origin "*";

## Usage

For complete API documentation and usage examples, please check the
project site.

### Get IP address in Plain text format

- Example: http://127.0.0.1/ip

### Get IP address in JSON format

- Example (JSON): http://127.0.0.1/jsonip
- Example (JSONP): http://127.0.0.1/jsonip?callback=getip

### Get IP address location in JSON format

Calling the API endpoint without any parameter will return the visitor
IP address:

- Example (JSON): http://127.0.0.1/location
- Example (JSONP): http://127.0.0.1/location?callback=getgeoip

Appending an IP address as parameter will return location information for
the given address:

- Example (JSON): http://127.0.0.1/location/46.19.37.108
- Example (JSONP): http://127.0.0.1/location/46.19.37.108?callback=getgeoip

## License

Telize is released under the BSD 2-Clause license. See `LICENSE` file
for details.

## Author

Telize is developed by Frederic Cambus.

- Site: https://www.cambus.net

## Resources

Project homepage: https://www.telize.com

Latest tarball release: https://www.statdns.com/telize/telize-2.0.0.tar.gz

GitHub: https://github.com/fcambus/telize

[1]: https://github.com/openresty/lua-nginx-module
[2]: https://github.com/leev/ngx_http_geoip2_module
[3]: https://dev.maxmind.com/geoip/geoip2/geolite2/
[4]: https://www.cambus.net/log-rotation-directly-within-nginx-configuration-file/
