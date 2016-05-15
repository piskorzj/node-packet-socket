#include "wrapper.hh"

#include "CppUTest/TestHarness.h"
#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTestExt/MockSupport.h"

TEST_GROUP(WrapperInitGroup) {
	void teardown() {
		mock().checkExpectations();
		mock().clear();
	}
};

TEST(WrapperInitGroup, ConstructShouldFailOnEmptyArguments) {
	v8::Local<v8::Value> argv[] = {};
	Wrapper::Init(Nan::GetCurrentContext()->Global());

	Nan::TryCatch catchBlock;
	Nan::MakeCallback(
					Nan::GetCurrentContext()->Global(),
					"Wrapper", 0, argv);
	if(!catchBlock.HasCaught()) {
		FAIL("Constructor didn't threw");
	}
}

TEST(WrapperInitGroup, ConstructShouldFailOnMissingDevicePropery) {
	const int argc = 1;
	v8::Local<v8::Value> argv[argc] = { Nan::EmptyString() };
	Wrapper::Init(Nan::GetCurrentContext()->Global());

	Nan::TryCatch catchBlock;
	Nan::MakeCallback(
					Nan::GetCurrentContext()->Global(),
					"Wrapper", 1, argv);
	if(!catchBlock.HasCaught()) {
		FAIL("Constructor didn't threw");
	}
}

TEST(WrapperInitGroup, ConstructShouldFailOnMissingOnRecvPropery) {
	const int argc = 1;
	v8::Local<v8::Object> options = Nan::New<v8::Object>();
	Nan::Set(options, Nan::New("device").ToLocalChecked(), Nan::New("rfm0").ToLocalChecked());
	v8::Local<v8::Value> argv[argc] = { options };
	Wrapper::Init(Nan::GetCurrentContext()->Global());

	Nan::TryCatch catchBlock;
	Nan::MakeCallback(
					Nan::GetCurrentContext()->Global(),
					"Wrapper", 1, argv);
	if(!catchBlock.HasCaught()) {
		FAIL("Constructor didn't threw");
	}
}

TEST(WrapperInitGroup, ConstructShouldFailOnMissingOnSendPropery) {
	const int argc = 1;
	v8::Local<v8::Object> options = Nan::New<v8::Object>();
	Nan::Set(options, Nan::New("device").ToLocalChecked(), Nan::New("rfm0").ToLocalChecked());
	Nan::Set(options, Nan::New("onRecv").ToLocalChecked(), Nan::Null());
	v8::Local<v8::Value> argv[argc] = { options };
	Wrapper::Init(Nan::GetCurrentContext()->Global());

	Nan::TryCatch catchBlock;
	Nan::MakeCallback(
					Nan::GetCurrentContext()->Global(),
					"Wrapper", 1, argv);
	if(!catchBlock.HasCaught()) {
		FAIL("Constructor didn't threw");
	}
}

TEST(WrapperInitGroup, ConstructShouldFailOnInvalidDevicePropery) {
	const int argc = 1;
	v8::Local<v8::Object> options = Nan::New<v8::Object>();
	Nan::Set(options, Nan::New("device").ToLocalChecked(), Nan::Null());
	Nan::Set(options, Nan::New("onRecv").ToLocalChecked(), Nan::Null());
	Nan::Set(options, Nan::New("onSend").ToLocalChecked(), Nan::Null());
	v8::Local<v8::Value> argv[argc] = { options };
	Wrapper::Init(Nan::GetCurrentContext()->Global());

	Nan::TryCatch catchBlock;
	Nan::MakeCallback(
					Nan::GetCurrentContext()->Global(),
					"Wrapper", 1, argv);
	if(!catchBlock.HasCaught()) {
		FAIL("Constructor didn't threw");
	}
}

TEST(WrapperInitGroup, ConstructShouldFailOnInvalidOnRecvPropery) {
	const int argc = 1;
	v8::Local<v8::Object> options = Nan::New<v8::Object>();
	Nan::Set(options, Nan::New("device").ToLocalChecked(), Nan::New("rfm0").ToLocalChecked());
	Nan::Set(options, Nan::New("onRecv").ToLocalChecked(), Nan::Null());
	Nan::Set(options, Nan::New("onSend").ToLocalChecked(), Nan::Null());
	v8::Local<v8::Value> argv[argc] = { options };
	Wrapper::Init(Nan::GetCurrentContext()->Global());

	Nan::TryCatch catchBlock;
	Nan::MakeCallback(
					Nan::GetCurrentContext()->Global(),
					"Wrapper", 1, argv);
	if(!catchBlock.HasCaught()) {
		FAIL("Constructor didn't threw");
	}
}

static NAN_METHOD(Noop) {

}
TEST(WrapperInitGroup, ConstructShouldFailOnInvalidOnSendPropery) {
	const int argc = 1;
	v8::Local<v8::Object> options = Nan::New<v8::Object>();
	Nan::Set(options, Nan::New("device").ToLocalChecked(), Nan::New("rfm0").ToLocalChecked());
	Nan::Set(options, Nan::New("onRecv").ToLocalChecked(), Nan::New<v8::Function>(Noop));
	Nan::Set(options, Nan::New("onSend").ToLocalChecked(), Nan::Null());
	v8::Local<v8::Value> argv[argc] = { options };
	Wrapper::Init(Nan::GetCurrentContext()->Global());

	Nan::TryCatch catchBlock;
	Nan::MakeCallback(
					Nan::GetCurrentContext()->Global(),
					"Wrapper", 1, argv);
	if(!catchBlock.HasCaught()) {
		FAIL("Constructor didn't threw");
	}
}

TEST(WrapperInitGroup, ConstructShouldFailOnSocketCreation) {
	const char * expected_device = "rfm0";
	const int argc = 1;
	v8::Local<v8::Object> options = Nan::New<v8::Object>();
	Nan::Set(options, Nan::New("device").ToLocalChecked(), Nan::New(expected_device).ToLocalChecked());
	Nan::Set(options, Nan::New("onRecv").ToLocalChecked(), Nan::New<v8::Function>(Noop));
	Nan::Set(options, Nan::New("onSend").ToLocalChecked(), Nan::New<v8::Function>(Noop));
	v8::Local<v8::Value> argv[argc] = { options };
	Wrapper::Init(Nan::GetCurrentContext()->Global());

	mock().expectOneCall("socket_constructor")
			.withStringParameter("device", expected_device)
			.andReturnValue(false);

	Nan::TryCatch catchBlock;
	Nan::MakeCallback(
					Nan::GetCurrentContext()->Global(),
					"Wrapper", 1, argv);
	if(!catchBlock.HasCaught()) {
		FAIL("Constructor didn't threw");
	}
}

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
