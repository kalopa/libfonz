#!/usr/bin/env ruby
#
require 'redis'
require 'json'

$redis = Redis.new

loop do
    msg = STDIN.gets
    $redis.publish "fonz", [msg.to_i, 0x55, 0xaa].to_json
    #$redis.publish "fonz", 0xaa
end
