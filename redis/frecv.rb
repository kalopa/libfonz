#!/usr/bin/env ruby
#
require 'redis'
require 'json'

$redis = Redis.new

$redis.subscribe('fonz-in') do |on|
  on.message do |channel, msg|
    data = JSON.parse(msg)
    puts "##{channel} - #{data.inspect}"
  end
end
