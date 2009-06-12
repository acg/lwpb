require 'rubygems'
require 'optisms.pb'

network = Optisms::NetworkConfiguration.new
network.clear!
network.number = '#12345'
network.apn = 'gprs.swisscom.ch'
network.username = 'gprs'
network.password = 'gprs'
network.use_peer_dns = true

request = Optisms::SetConfigRequest.new
request.network = network

request.serialize_to_file 'test.dat'
