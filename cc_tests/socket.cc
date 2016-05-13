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

int main(int ac, char** av) {
	return CommandLineTestRunner::RunAllTests(ac, av);
}
