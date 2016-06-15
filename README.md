# packet-socket
Node.js package implementing socket for receiving OSI layer 2 packets

## API
Assume that packet-socket module is required in following examples.
```
var packet = require('packet-socket')
```

### Opening socket
```
var socket = packet.openSocket({device: 'lo'})
```
openSocket call accepts options object as argument. Option *device* should contain interface name on which open raw packet socket.

### Closing socket
```
socket.close()
```

### Few words about address
Some functions require address as an argument. Formats accepted:
* *String* 'aa:bb:cc:11:22:33'
* *Array* [0xaa, 0xbb, 0xcc, 0x11, 0x22, 0x33]
* *Buffer* new Buffer([0xaa, 0xbb, 0xcc, 0x11, 0x22, 0x33])

### Joining and leaving Multicast Membership
```
socket.joinMC(packet.PROMISCIOUS)
socket.joinMC(packet.ALL_MULTICAST)
socket.joinMC('11:00:de:ad:be:ef')
```
```
socket.leaveMC(packet.PROMISCIOUS)
socket.leaveMC(packet.ALL_MULTICAST)
socket.leaveMC('11:00:de:ad:be:ef')
```
joinMC or leaveMC method accepts as argument either multicast address or integer constant:
* PROMISCIOUS - sets interface to PROMISCIOUS mode, catching all packets
* ALL_MULTICAST - catch all multicast messages

### Sending
```
packet.send(new Buffer('message to send'), '58:00:00:00:00:01', function(err, bytes) {

})
```
Callback function argument will provide information if send failed, then err will contain information, if it is null, then bytes will contain number of bytes transferred.

### Events
Socket returned by openSocket can emit events:
* *message* - this event is emited when packet is received, it contain buffer with message received, source and destination addresses (in string format)
```
packet.on('message', function(message, source, destination) {
});
```
* *error* - this event is emited in case of any error, it contain error object
```
packet.on('error', function(error) {});
```

## Packet socket root reqirement
Opening packet (*man 7 packet*) socket requires root permission or **cap_net_raw** capability

## Testing
Module contains unit test, which can be invoked by command
```
npm test
```
This however requires build suite ready, with CppUTest libraries installed.
