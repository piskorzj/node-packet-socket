var packet = require ("./build/Release/packet_socket_addon");
var net = require ("net");
var util = require("util");
var events = require('events');

function addressToBuffer(addr) {
	if(Buffer.isBuffer(addr)) return addr;
	if(Array.isArray(addr)) return new Buffer(addr);
	if(addr.indexOf(':') > -1) {
		addr = addr.split(':');
	}
	addr = addr.map(function(v) { return parseInt(v, 16);});
	return new Buffer(addr);
}

function Socket(options) {
	this.requests = [];
	var me = this;

	var binding_options = {
		device: options.device,
		onRecv: function() {
			try {
				me.wrap.Receive(function(source, destination, message) {
					source = source.toString('hex').replace(/(.{2})(?=.*.{2}$)/g, '$1:');
					destination = destination.toString('hex').replace(/(.{2})(?=.*.{2}$)/g, '$1:');
					me.emit ("message", message, source, destination);
				});
			} catch(error) {
				me.emit('error', error);
			}
		},
		onSend: function() {
			if (me.requests.length > 0) {
				var req = me.requests.shift ();
				try {
					me.wrap.Send (req.buffer, req.address, function (bytes) {
						req.afterCallback.call (me, null, bytes);
					});
				} catch (error) {
					req.afterCallback.call (me, error, 0);
				}
			} else {
				me.wrap.PauseSending();
			}
		},
		onError: function(error) {
			me.emit('error', error);
		}
	};
	try {
		this.wrap = new packet.Wrapper(binding_options);
	} catch(error) {
		this.emit('error', error);
	}
};
util.inherits(Socket, events.EventEmitter);

Socket.prototype.close = function() {
	this.wrap.Close();
};

Socket.prototype.joinMC = function(addrortype) {
	try {
		this.wrap.AddMembership( typeof addrortype === 'number' ? addrortype : addressToBuffer(addrortype));
	} catch(error) {
		this.emit("error", error);
	}
}
Socket.prototype.leaveMC = function(addrortype) {
	try {
		this.wrap.DropMembership(typeof addrortype === 'number' ? addrortype : addressToBuffer(addrortype));
	} catch(error) {
		this.emit("error", error);
	}
}

exports.openSocket = function (options) {
	return new Socket(options || {});
};

exports.PROMISCIOUS = packet.PROMISCIOUS;
exports.ALL_MULTICAST = packet.ALL_MULTICAST;

Socket.prototype.send = function (buffer, address, afterCallback) {
	address = addressToBuffer(address);

	var req = {
		buffer: buffer,
		address: address,
		afterCallback: afterCallback
	};
	this.requests.push (req);

	this.wrap.ResumeSending();

	return this;
}
