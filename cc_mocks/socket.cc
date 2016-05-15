#include "CppUTestExt/MockSupport.h"
#include "socket.hh"

Socket::Socket(const char * device) {
	mock().actualCall("socket_constructor")
			.withStringParameter("device", device);
	if(!mock().returnBoolValueOrDefault(true)) {
		throw std::runtime_error("forced creation failure");
	}
}

Socket::~Socket() {}

int Socket::get_descriptor(void) {
	return mock().actualCall("get_descriptor").returnIntValue();
}

int Socket::send_message(const unsigned char *destination_address,
		const char *message, int message_length) {
	return mock().actualCall("send_message").onObject(this)
			.withMemoryBufferParameter("destination_address", destination_address, ETHER_ADDR_LEN)
			.withMemoryBufferParameter("message", (const unsigned char*)message, message_length)
			.returnIntValue();
}

int Socket::receive_message(unsigned char *source_address,
			char *buffer, int buffer_size) {
	return mock().actualCall("receive_message").onObject(this)
			.withMemoryBufferParameter("source_address", source_address, ETHER_ADDR_LEN)
			.withOutputParameter("buffer", buffer)
			.withIntParameter("buffer_size", buffer_size)
			.returnIntValue();
}

void Socket::add_membership(Socket::MembershipType type,
		const unsigned char *multicast_address) {
		mock().actualCall("add_membership")
			.withIntParameter("type", type)
			.withMemoryBufferParameter("multicast_address", multicast_address, ETHER_ADDR_LEN);
		if(!mock().returnBoolValueOrDefault(true)) {
			throw std::runtime_error("forced add_membership failure");
		}
}

void Socket::drop_membership(Socket::MembershipType type,
		const unsigned char *multicast_address) {
	mock().actualCall("drop_membership").onObject(this)
		.withIntParameter("type", type)
		.withMemoryBufferParameter("multicast_address", multicast_address, ETHER_ADDR_LEN);
}
