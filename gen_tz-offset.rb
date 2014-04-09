#!/usr/bin/env ruby
# 2014 - Yann Verry
# Generate nginx offset timezone
#
# gem required: tzinfo
#
require 'tzinfo'

# open file
tz_hdl = File.new('tz_offset.conf','w')
# header
tz_hdl.write("map $geoip_timezone $geoip_timezone_offset {\n")

TZInfo::Timezone.all_country_zone_identifiers.each do |tzname|
  tz = TZInfo::Timezone.get(tzname)
  current = tz.current_period
  offset = current.utc_total_offset / 3600
  tz_hdl.write("\t\"" + tzname.to_s + '" "' +  offset.to_s + "\";\n")
end

# footer
tz_hdl.write("}\n")
