#include "CppUTestExt/MockSupport.h"
#include "socket.hh"

Socket::Socket(const char * device) {}

Socket::~Socket() {}

int Socket::get_descriptor(void) {
	return mock().actualCall("get_descriptor").onObject(this).returnIntValue();
}
