#include "CppUTest/TestHarness.h"
#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTestExt/MockSupport.h"

#include "socket.hh"
#include <exception>
#include <stdexcept>
#include <cstring>


extern "C" {
	#include "CppUTestExt/MockSupport_c.h"
	int socket (int __domain, int __type, int __protocol) {
		mock_c()->actualCall("socket")
				->withIntParameters("domain", __domain)
				->withIntParameters("type", __type)
				->withIntParameters("protocol", __protocol);

		return mock_c()->returnIntValueOrDefault(5);
	}
	int close (int __fd) {
		mock_c()->actualCall("close")
				->withIntParameters("fd", __fd);
		return mock_c()->returnIntValueOrDefault(0);
	}
	int fcntl (int __fd, int __cmd, ...) {
		mock_c()->actualCall("fcntl")
				->withIntParameters("fd", __fd)
				->withIntParameters("cmd", __cmd);
		return mock_c()->returnIntValueOrDefault(0);
	}
	int ioctl (int __fd, unsigned long int __request, ...) {
	    va_list argptr;
	    va_start (argptr, __request);
	    struct ifreq *ifr = va_arg(argptr, struct ifreq *);
	    va_end (argptr);

		mock_c()->actualCall("ioctl")
				->withIntParameters("fd", __fd)
				->withUnsignedLongIntParameters("request", __request)
				->withOutputParameter("interface_index", &ifr->ifr_ifindex)
				->withOutputParameter("hardware_address", ifr->ifr_hwaddr.sa_data);
		return mock_c()->returnIntValueOrDefault(0);
	}
	ssize_t sendto (int __fd, const void *__buf, size_t __n,
			       int __flags, __CONST_SOCKADDR_ARG __addr,
			       socklen_t __addr_len) {
		mock_c()->actualCall("sendto")
				->withIntParameters("fd", __fd)
				->withMemoryBufferParameter("buf", (const unsigned char *)__buf, __n)
				->withUnsignedIntParameters("n", __n)
				->withIntParameters("flags", __flags)
				->withMemoryBufferParameter("addr", (const unsigned char *)__addr, __addr_len)
				->withUnsignedIntParameters("addr_len", __addr_len);
		return mock_c()->returnUnsignedIntValueOrDefault(__n);
	}
}

TEST_GROUP(SocketInit) {
	int socket_descriptor;
	void setup() {
		socket_descriptor = 5;
	}
	void teardown() {
		mock().checkExpectations();
		mock().clear();
	}
};

TEST(SocketInit, ConstructorShouldFailOnSocketFail) {
	mock().expectOneCall("socket").ignoreOtherParameters().andReturnValue(-1);
	mock().expectNoCall("close");
	CHECK_THROWS(std::runtime_error, new Socket("rfm0"));
}
TEST(SocketInit, ConstructorShouldFailOnSettingNonBlockingModeFail) {
	mock().expectOneCall("socket").ignoreOtherParameters().andReturnValue(socket_descriptor);
	mock().expectOneCall("close").withIntParameter("fd", socket_descriptor).andReturnValue(0);
	mock().expectOneCall("fcntl").ignoreOtherParameters().andReturnValue(-1);
	CHECK_THROWS(std::runtime_error, new Socket("rfm0"));
}
TEST(SocketInit, ConstructorShouldFailOnGettingInterfaceFail) {
	mock().expectOneCall("socket").ignoreOtherParameters().andReturnValue(socket_descriptor);
	mock().expectOneCall("close").withIntParameter("fd", socket_descriptor).andReturnValue(0);
	mock().expectNCalls(2, "fcntl").ignoreOtherParameters().andReturnValue(0);
	mock().expectOneCall("ioctl").withIntParameter("request", SIOCGIFINDEX)
			.ignoreOtherParameters().andReturnValue(-1);
	CHECK_THROWS(std::runtime_error, new Socket("rfm0"));
}
TEST(SocketInit, ConstructorShouldFailOnGettingHardwareAddressFail) {
	mock().expectOneCall("socket").ignoreOtherParameters().andReturnValue(socket_descriptor);
	mock().expectOneCall("close").withIntParameter("fd", socket_descriptor).andReturnValue(0);
	mock().expectNCalls(2, "fcntl").ignoreOtherParameters().andReturnValue(0);
	mock().expectOneCall("ioctl").withIntParameter("request", SIOCGIFINDEX)
			.ignoreOtherParameters().andReturnValue(0);
	mock().expectOneCall("ioctl").withIntParameter("request", SIOCGIFHWADDR)
				.ignoreOtherParameters().andReturnValue(-1);
	CHECK_THROWS(std::runtime_error, new Socket("rfm0"));
}
TEST(SocketInit, ConstructorShouldNotThrowOnSuccess) {
	mock().expectOneCall("socket").ignoreOtherParameters().andReturnValue(socket_descriptor);
	mock().expectOneCall("close").withIntParameter("fd", socket_descriptor).andReturnValue(0);
	mock().expectNCalls(2, "fcntl").ignoreOtherParameters().andReturnValue(0);
	mock().expectOneCall("ioctl").withIntParameter("request", SIOCGIFINDEX)
			.ignoreOtherParameters().andReturnValue(0);
	char address[] = {(char)0xde, (char)0xad, (char)0, (char)0, (char)0xbe, (char)0xef};
	mock().expectOneCall("ioctl").withIntParameter("request", SIOCGIFHWADDR)
			.withOutputParameterReturning("hardware_address", address, 6)
			.ignoreOtherParameters().andReturnValue(0);
	try {
		Socket("rfm0");
	} catch(...) {
		FAIL("Catched error");
	}
}

TEST_GROUP(SocketUsage) {
	Socket *socket;
	int socket_descriptor;
	unsigned int socket_index;
	unsigned char address[ETHER_ADDR_LEN];

	void setup() {
		socket_descriptor = 5;
		socket_index = 12;
		unsigned char _address[] = {0xde, 0xad, 0, 0, 0xbe, 0xef};
		memcpy(address, _address, ETH_ALEN);

		mock().expectOneCall("socket").ignoreOtherParameters().andReturnValue(socket_descriptor);
		mock().expectNCalls(2, "fcntl").ignoreOtherParameters().andReturnValue(0);
		mock().expectOneCall("ioctl").withIntParameter("request", SIOCGIFINDEX)
				.withOutputParameterReturning("interface_index", &socket_index, sizeof(socket_index))
				.ignoreOtherParameters().andReturnValue(0);
		mock().expectOneCall("ioctl").withIntParameter("request", SIOCGIFHWADDR)
				.withOutputParameterReturning("hardware_address", address, 6)
				.ignoreOtherParameters().andReturnValue(0);
		socket = new Socket("rfm0");
	}
	void teardown() {
		mock().expectOneCall("close").withIntParameter("fd", socket_descriptor).andReturnValue(0);
		delete socket;
		mock().checkExpectations();
		mock().clear();
	}
};

TEST(SocketUsage, GetDescriptorShouldReturnDescriptor) {
	LONGS_EQUAL(this->socket_descriptor, socket->get_descriptor());
}

TEST(SocketUsage, SendShouldThrowOnSendtoFail) {
	unsigned char desitnation_address[] = {0xde, 0xad, 0, 0, 0xff, 0xff};
	const char *message = "hello";
	const int message_length = strlen(message);
	mock().expectOneCall("sendto").ignoreOtherParameters().andReturnValue((unsigned int)-1);
	CHECK_THROWS(std::runtime_error,
			socket->send_message(desitnation_address, message, message_length));
}
TEST(SocketUsage, SendShouldThrowOnSendtoNotSendEntirePacket) {
	unsigned char desitnation_address[] = {0xde, 0xad, 0, 0, 0xff, 0xff};
	const char *message = "hello";
	const int message_length = strlen(message);
	mock().expectOneCall("sendto").ignoreOtherParameters().andReturnValue((unsigned int)3);
	CHECK_THROWS(std::runtime_error,
			socket->send_message(desitnation_address, message, message_length));
}
TEST(SocketUsage, SendShouldCallSendto) {
	unsigned char desitnation_address[] = {0xde, 0xad, 0, 0, 0xff, 0xff};
	const char *message = "hello";
	const int message_length = strlen(message);

	struct sockaddr_ll addr;
	memset(&addr, 0, sizeof(addr));
	addr.sll_family = AF_PACKET;
	addr.sll_ifindex = this->socket_index;
	addr.sll_halen = ETHER_ADDR_LEN;
	addr.sll_protocol = htons(ETH_P_802_3);
	memcpy(addr.sll_addr, this->address, ETHER_ADDR_LEN);

	unsigned char send_buffer[2000];
	int send_buffer_index = 0;
	memcpy(send_buffer + send_buffer_index, desitnation_address, ETHER_ADDR_LEN);
	send_buffer_index += ETHER_ADDR_LEN;
	memcpy(send_buffer + send_buffer_index, this->address, ETHER_ADDR_LEN);
	send_buffer_index += ETHER_ADDR_LEN;
	*((uint16_t *) (send_buffer + send_buffer_index)) = htons(message_length);
	send_buffer_index += 2;
	memcpy(send_buffer + send_buffer_index, message, message_length);
	send_buffer_index += message_length;

	mock().expectOneCall("sendto")
			.withIntParameter("fd", this->socket_descriptor)
			.withMemoryBufferParameter("buf", send_buffer, send_buffer_index)
			.withUnsignedIntParameter("n", send_buffer_index)
			.withIntParameter("flags", 0)
			.withMemoryBufferParameter("addr", (const unsigned char *)&addr, sizeof(addr))
			.withUnsignedIntParameter("addr_len", sizeof(addr))
			.andReturnValue(send_buffer_index);

	int rc = socket->send_message(desitnation_address, message, message_length);

	LONGS_EQUAL(message_length, rc);
}

int main(int ac, char** av) {
	return CommandLineTestRunner::RunAllTests(ac, av);
}
