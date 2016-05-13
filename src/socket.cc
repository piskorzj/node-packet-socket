#include "socket.hh"

#include <sys/socket.h>
#include <netinet/if_ether.h>
#include <string.h>
#include <arpa/inet.h> //inet_pton
#include <errno.h>
#include <fcntl.h>
#include <stdexcept>
#include <unistd.h>

Socket::Socket(const char * device) : device_name(device) {
	socket_descriptor = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(socket_descriptor == -1)
		throw std::runtime_error(strerror(errno));

	try {
		//set nonblocking
		int flag = 1;
		if ((flag = fcntl(socket_descriptor, F_GETFL, 0)) == -1)
			throw std::runtime_error(strerror(errno));
		if (fcntl(socket_descriptor, F_SETFL, flag | O_NONBLOCK) == -1)
			throw std::runtime_error(strerror(errno));

		//get interface index
		struct ifreq ifr;
		strcpy(ifr.ifr_name, device);
		if(ioctl(socket_descriptor, SIOCGIFINDEX, &ifr) == -1)
			throw std::runtime_error(strerror(errno));
		interface_index = ifr.ifr_ifindex;

		//set hwaddr
		if(ioctl(socket_descriptor, SIOCGIFHWADDR, &ifr) == -1)
			throw std::runtime_error(strerror(errno));
		memcpy(hardware_address, ifr.ifr_hwaddr.sa_data, 6);


	} catch(...) {
		close(socket_descriptor);
		throw;
	}
}

Socket::~Socket() {
	close(socket_descriptor);
}
