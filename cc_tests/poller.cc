#include "CppUTest/TestHarness.h"
#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTestExt/MockSupport.h"

#include "poller.hh"

int main(int ac, char** av) {
	return CommandLineTestRunner::RunAllTests(ac, av);
}
