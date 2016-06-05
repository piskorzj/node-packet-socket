#include "nan.h"

#include "CppUTest/TestHarness.h"
#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTestExt/MockSupport.h"

#include "poller.hh"

NAN_METHOD(Run) {
	int argc = info.Length();
	char ** argv = new char*[argc];
	for(int i = 0; i < argc; i++) {
		int arg_len = Nan::To<v8::String>(info[i]).ToLocalChecked()->Utf8Length();
		argv[i] = new char[arg_len];
		Nan::To<v8::String>(info[i]).ToLocalChecked()->WriteUtf8(argv[i]);
	}
	MemoryLeakWarningPlugin::turnOffNewDeleteOverloads();
	int rc = CommandLineTestRunner::RunAllTests(argc, argv);
	for(int i = 0; i < argc; i++)
		delete [] argv[i];
	delete [] argv;
	info.GetReturnValue().Set(rc);
}

NAN_MODULE_INIT(TestRunner) {
	Nan::SetMethod(target, "Run", Run);
}

NODE_MODULE(test, TestRunner);
