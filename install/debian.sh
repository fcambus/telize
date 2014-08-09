# Installing required packages
echo "deb http://http.debian.net/debian wheezy-backports main" >> /etc/apt/sources.list
apt-get update
apt-get install -t wheezy-backports nginx-extras 
apt-get install luarocks
luarocks install lua-cjson

# Copying Telize configuration and enabling it
cp *.conf /etc/nginx/
cp telize /etc/nginx/sites-available
ln -s /etc/nginx/sites-available/telize /etc/nginx/sites-enabled

# Installing GeoIP databases
mkdir -p /usr/share/GeoIP
wget -P /usr/share/GeoIP http://geolite.maxmind.com/download/geoip/database/GeoIPv6.dat.gz
wget -P /usr/share/GeoIP http://geolite.maxmind.com/download/geoip/database/GeoLiteCityv6-beta/GeoLiteCityv6.dat.gz
wget -P /usr/share/GeoIP http://download.maxmind.com/download/geoip/database/asnum/GeoIPASNumv6.dat.gz
gunzip -f /usr/share/GeoIP/*gz

# Adding required directives in nginx.conf
sed -i "s|http {|http {\n\tgeoip_country /usr/share/GeoIP/GeoIPv6.dat;\n\tgeoip_city /usr/share/GeoIP/GeoLiteCityv6.dat;\n\tgeoip_org /usr/share/GeoIP/GeoIPASNumv6.dat;\n\n\tmap_hash_max_size 8192;\n\tmap_hash_bucket_size 64;\n\n\tinclude /etc/nginx/timezone.conf;\n\tinclude /etc/nginx/timezone-offset.conf;\n\n|" /etc/nginx/nginx.conf

# Removing symlink to default site
rm /etc/nginx/sites-enabled/default

# Restarting Nginx
service nginx restart
