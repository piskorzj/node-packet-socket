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
	ssize_t recvfrom (int __fd, void *__restrict __buf, size_t __n,
				 int __flags, __SOCKADDR_ARG __addr,
				 socklen_t *__restrict __addr_len) {
		mock_c()->actualCall("recvfrom")
				->withIntParameters("fd", __fd)
				->withOutputParameter("buf", __buf)
				->withUnsignedIntParameters("n", __n)
				->withIntParameters("flags", __flags)
				->withOutputParameter("addr", __addr)
				->withOutputParameter("addr_len", __addr_len);
		return mock_c()->returnUnsignedIntValueOrDefault(1);
	}
	ssize_t recvmsg (int __fd, struct msghdr *__message, int __flags) {
		mock_c()->actualCall("recvmsg")
				->withIntParameters("fd", __fd)
				->withIntParameters("flags", __flags)
				->withUnsignedIntParameters("addr_len", __message->msg_namelen)
				->withOutputParameter("addr", __message->msg_name)
				->withOutputParameter("ethernet_header", __message->msg_iov[0].iov_base)
				->withUnsignedIntParameters("ethernet_header_size", __message->msg_iov[0].iov_len)
				->withOutputParameter("message", __message->msg_iov[1].iov_base)
				->withUnsignedIntParameters("message_size", __message->msg_iov[1].iov_len);
		return mock_c()->returnUnsignedIntValueOrDefault(1);
	}
	int setsockopt (int __fd, int __level, int __optname,
			       const void *__optval, socklen_t __optlen) {
		mock_c()->actualCall("setsockopt")
				->withIntParameters("fd", __fd)
				->withIntParameters("level", __level)
				->withIntParameters("optname", __optname)
				->withMemoryBufferParameter("optval", (const unsigned char *)__optval, __optlen)
				->withUnsignedIntParameters("optlen", __optlen);
		return mock_c()->returnIntValueOrDefault(0);
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

TEST(SocketUsage, RecvShouldThrowOnRecvmsgFail) {
	mock().expectOneCall("recvmsg").ignoreOtherParameters().andReturnValue((unsigned int)-1);
	const int receive_buffer_size = 2000;
	char receive_buffer[receive_buffer_size];
	unsigned char from_addres[6];
	CHECK_THROWS(std::runtime_error,
			socket->receive_message(from_addres, receive_buffer, receive_buffer_size));
}
TEST(SocketUsage, RecvShouldCallRecvmsg) {
	struct sockaddr_ll mocked_received_from_address;
	unsigned char source_address[] = {0xde, 0xad, 0, 0, 0xff, 0xff};
	memcpy(mocked_received_from_address.sll_addr, source_address, ETHER_ADDR_LEN);

	const char *mocked_message = "hello";

	struct ether_header ethernet_header;
	ethernet_header.ether_type = htons(strlen(mocked_message));
	memcpy(ethernet_header.ether_shost, source_address, ETHER_ADDR_LEN);
	memcpy(ethernet_header.ether_dhost, this->address, ETHER_ADDR_LEN);

	const int receive_buffer_size = 2000;
	char receive_buffer[receive_buffer_size];
	unsigned char from_addres[6];

	mock().expectOneCall("recvmsg")
			.withIntParameter("fd", this->socket_descriptor)
			.withIntParameter("flags", 0)
			.withUnsignedIntParameter("addr_len", sizeof (mocked_received_from_address))
			.withOutputParameterReturning("addr", &mocked_received_from_address, sizeof(mocked_received_from_address))
			.withOutputParameterReturning("ethernet_header", &ethernet_header, sizeof(ethernet_header))
			.withUnsignedIntParameter("ethernet_header_size", sizeof(ethernet_header))
			.withOutputParameterReturning("message", mocked_message, strlen(mocked_message))
			.withUnsignedIntParameter("message_size", receive_buffer_size)
			.andReturnValue((unsigned int)(strlen(mocked_message) + sizeof(ethernet_header)));

	int received_bytes;


	received_bytes = socket->receive_message(from_addres, receive_buffer, receive_buffer_size);

	MEMCMP_EQUAL(source_address, from_addres, ETHER_ADDR_LEN);
	MEMCMP_EQUAL(mocked_message, receive_buffer, received_bytes);
}

TEST(SocketUsage, AddMembershipShouldThrowOnSetsockoptFail) {
	const unsigned char multicast_address[] = {0xaa, 0xab, 0, 0, 0xff, 0xff};
	Socket::MembershipType type = Socket::PROMISCIOUS;

	mock().expectOneCall("setsockopt")
			.ignoreOtherParameters()
			.andReturnValue(-1);
	CHECK_THROWS(std::runtime_error,
			socket->add_membership(type, multicast_address));
}
TEST(SocketUsage, AddMembershipShouldUseMRAddressOnlyMULTICASTType) {
	const unsigned char multicast_address[] = {0xaa, 0xab, 0, 0, 0xff, 0xff};
	Socket::MembershipType type = Socket::PROMISCIOUS;
	struct packet_mreq multireq;
	memset(&multireq, 0, sizeof(multireq));
	multireq.mr_ifindex = this->socket_index;
	multireq.mr_type = type;

	mock().expectOneCall("setsockopt")
			.withIntParameter("fd", this->socket_descriptor)
			.withIntParameter("level", SOL_PACKET)
			.withIntParameter("optname", PACKET_ADD_MEMBERSHIP)
			.withMemoryBufferParameter("optval", (const unsigned char *)&multireq, sizeof(multireq))
			.withUnsignedIntParameter("optlen", sizeof(multireq))
			.andReturnValue(0);

	try {
		socket->add_membership(type, multicast_address);
	} catch(...) {
		FAIL("Catched error");
	}
}
TEST(SocketUsage, AddMembershipShouldCallSetsockopt) {
	const unsigned char multicast_address[] = {0xaa, 0xab, 0, 0, 0xff, 0xff};
	Socket::MembershipType type = Socket::MULTICAST;
	struct packet_mreq multireq;
	memset(&multireq, 0, sizeof(multireq));
	multireq.mr_ifindex = this->socket_index;
	multireq.mr_type = type;
	multireq.mr_alen = ETHER_ADDR_LEN;
	memcpy(multireq.mr_address, multicast_address, ETHER_ADDR_LEN);

	mock().expectOneCall("setsockopt")
			.withIntParameter("fd", this->socket_descriptor)
			.withIntParameter("level", SOL_PACKET)
			.withIntParameter("optname", PACKET_ADD_MEMBERSHIP)
			.withMemoryBufferParameter("optval", (const unsigned char *)&multireq, sizeof(multireq))
			.withUnsignedIntParameter("optlen", sizeof(multireq))
			.andReturnValue(0);

	try {
		socket->add_membership(type, multicast_address);
	} catch(...) {
		FAIL("Catched error");
	}
}

int main(int ac, char** av) {
	return CommandLineTestRunner::RunAllTests(ac, av);
}
