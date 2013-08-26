
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

## Requirements

### Nginx modules

Telize requires Nginx compiled with the following modules :

- GeoIP (Optional HTTP modules : --with-http_geoip_module)
- HttpRealipModule (Optional HTTP module : --with-http_realip_module)
- HttpEchoModule (Third party module (ngx_echo) : http://wiki.nginx.org/HttpEchoModule)
- HttpLuaModule (Third party module (ngx_lua) : http://wiki.nginx.org/HttpLuaModule)

If you are using Debian stable, the `nginx-extras` package have these
modules compiled-in.

### Lua modules

Telize requires the following Lua module :

- Lua CJSON 

Installing via LuaRocks : 

	luarocks install lua-cjson

## Installation

Edit `nginx.conf` to add directives specifying the path to the GeoIP database
files, within the http block :

	http {

		...

		geoip_country  /usr/share/GeoIP/GeoIP.dat;
		geoip_city     /usr/share/GeoIP/GeoLiteCity.dat;
	}

Then deploy the API configuration file `telize` to the appropriate location on
your system, and reload Nginx configuration.

## Usage 

For complete API documentation and JavaScript API usage examples, please check
the project site : http://www.telize.com

### Get IP address in Plain text format :

- Example : http://www.telize.com/ip

### Get IP address in JSON format :

- Example (JSON) : http://www.telize.com/jsonip
- Example (JSONP) : http://www.telize.com/jsonip?callback=getip

### Get IP address location in JSON format :

Calling the API endpoint without any parameter will return the visitor
IP address :

- Example (JSON) : http://www.telize.com/geoip
- Example (JSONP) : http://www.telize.com/geoip?callback=getgeoip

Appending an IP address as parameter will return location information for
this IP address :

- Example (JSON) : http://www.telize.com/geoip/46.19.37.108
- Example (JSONP) : http://www.telize.com/geoip/46.19.37.108?callback=getgeoip

## License

Telize is released under the BSD 3-Clause license. See `LICENSE` file
for details.

## Author

Telize is developed by Frederic Cambus

- Site : http://www.cambus.net
- Twitter: http://twitter.com/fcambus

## Resources

Project Homepage : http://www.telize.com

Latest tarball release : http://www.statdns.com/telize/telize-1.00.tar.gz

GitHub : https://github.com/fcambus/telize
