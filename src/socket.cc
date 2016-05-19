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

int Socket::get_descriptor(void) {
	return socket_descriptor;
}

int Socket::send_message(const unsigned char *destination_address,
		const char *message, int message_length) {

	struct sockaddr_ll addr;
	memset(&addr, 0, sizeof(addr));
	addr.sll_family = AF_PACKET;
	addr.sll_ifindex = interface_index;
	addr.sll_halen = ETHER_ADDR_LEN;
	addr.sll_protocol = htons(ETH_P_802_3);
	memcpy(addr.sll_addr, hardware_address, ETHER_ADDR_LEN);

	size_t overall_packet_length = sizeof(struct ether_header) + message_length;
	char *msg_buffer = new char[overall_packet_length];
	struct ether_header *header = (struct ether_header *)msg_buffer;
	memcpy(header->ether_dhost, destination_address, ETHER_ADDR_LEN);
	memcpy(header->ether_shost, hardware_address, ETHER_ADDR_LEN);
	header->ether_type = htons(message_length);
	char *payload = msg_buffer + sizeof(struct ether_header);
	memcpy(payload, message, message_length);

	int rc = sendto(socket_descriptor, msg_buffer, overall_packet_length, 0,
			(struct sockaddr *)&addr, sizeof(addr));
	delete [] msg_buffer;

	if(rc == -1)
		throw std::runtime_error(strerror(errno));

	if((size_t) rc != overall_packet_length)
		throw std::runtime_error("Failed to send entire packet");

	return message_length;
}

int Socket::receive_message(unsigned char *source_address,
			char *buffer, int buffer_size) {
	struct sockaddr_ll addr;
	memset(&addr, 0, sizeof(addr));
	socklen_t addrlen = sizeof(addr);

	struct ether_header header;

	struct iovec msg_iov[2];
	msg_iov[0].iov_base = &header;
	msg_iov[0].iov_len = sizeof(header);
	msg_iov[1].iov_base = buffer;
	msg_iov[1].iov_len = buffer_size;
	struct msghdr msg;
	msg.msg_name = &addr;
	msg.msg_namelen = addrlen;
	msg.msg_flags = 0;
	msg.msg_iov = msg_iov;
	msg.msg_iovlen = 2;

	int received_bytes = recvmsg(socket_descriptor, &msg, 0);

	if(received_bytes == -1)
		throw std::runtime_error(strerror(errno));

	memcpy(source_address, addr.sll_addr, ETHER_ADDR_LEN);

	return received_bytes - sizeof(header);
}

bool Socket::is_valid_membership_type(Socket::MembershipType type) {
	return (type == Socket::MULTICAST ||
			type == Socket::PROMISCIOUS ||
			type == Socket::ALL_MULTICAST);
}

void Socket::manage_membership(Socket::MembershipAction action,
			Socket::MembershipType type, const unsigned char *multicast_address) {
	if(!is_valid_membership_type(type))
		throw std::invalid_argument("Invalid membership type");

	struct packet_mreq multireq;
	memset(&multireq, 0, sizeof(multireq));
	multireq.mr_ifindex = interface_index;
	multireq.mr_type = type;
	if(type == Socket::MULTICAST) {
		multireq.mr_alen = ETHER_ADDR_LEN;
		memcpy(multireq.mr_address, multicast_address, ETHER_ADDR_LEN);
	}

	if(setsockopt(socket_descriptor, SOL_PACKET, action,
			&multireq, sizeof(multireq)) == -1)
		throw std::runtime_error(strerror(errno));
}

void Socket::add_membership(Socket::MembershipType type,
		const unsigned char *multicast_address) {
	manage_membership(Socket::ADD_MEMBERSHIP, type, multicast_address);
}

void Socket::drop_membership(Socket::MembershipType type,
		const unsigned char *multicast_address) {
	manage_membership(Socket::DROP_MEMBERSHIP, type, multicast_address);
}
