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

TEST_GROUP(WrapperUsageGroup) {
	Nan::Persistent<v8::Object> wrapped_obj;
	Wrapper *unwrapped;

	void setup() {
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
				.andReturnValue(true);

		v8::Local<v8::Object> result = Nan::To<v8::Object>(Nan::MakeCallback(
				Nan::GetCurrentContext()->Global(),
				"Wrapper", 1, argv)).ToLocalChecked();
		wrapped_obj.Reset(result);
		unwrapped = Nan::ObjectWrap::Unwrap<Wrapper>(result);
	}
	void teardown() {
		wrapped_obj.Reset();
		mock().checkExpectations();
		mock().clear();
	}
};

TEST(WrapperUsageGroup, AddMembershipShouldFailOnEmptyArguments) {
	v8::Local<v8::Object> wrap = Nan::New(wrapped_obj);
	v8::Local<v8::Value> argv[] = {};

	Nan::TryCatch catchBlock;
	Nan::MakeCallback(wrap, "AddMembership", 0, argv);
	if(!catchBlock.HasCaught()) {
		FAIL("AddMembership didn't threw");
	}
}

TEST(WrapperUsageGroup, AddMembershipShouldFailOnNoAddressOrNonTypeArguments) {
	v8::Local<v8::Object> wrap = Nan::New(wrapped_obj);
	const int argc = 1;
	v8::Local<v8::Value> argv[argc] = {Nan::EmptyString()};

	Nan::TryCatch catchBlock;
	Nan::MakeCallback(wrap, "AddMembership", argc, argv);
	if(!catchBlock.HasCaught()) {
		FAIL("AddMembership didn't threw");
	}
}

TEST(WrapperUsageGroup, AddMembershipShouldFailOnToShortAddressArgument) {
	v8::Local<v8::Object> wrap = Nan::New(wrapped_obj);
	const int argc = 1;
	v8::Local<v8::Value> argv[argc] = {Nan::NewBuffer(4).ToLocalChecked()};

	Nan::TryCatch catchBlock;
	Nan::MakeCallback(wrap, "AddMembership", argc, argv);
	if(!catchBlock.HasCaught()) {
		FAIL("AddMembership didn't threw");
	}
}

TEST(WrapperUsageGroup, AddMembershipShouldFailOnSocketCallFailWithAddress) {
	v8::Local<v8::Object> wrap = Nan::New(wrapped_obj);
	const int argc = 1;
	const unsigned char address[] = {0xde, 0xad, 0x00, 0x00, 0x12, 0x34};
	char *address_for_node_buffer = new char[Socket::ADDRESS_LENGHT];
	memcpy(address_for_node_buffer, address, Socket::ADDRESS_LENGHT);
	v8::Local<v8::Value> argv[argc] = {Nan::NewBuffer(address_for_node_buffer, 6).ToLocalChecked()};

	mock().expectOneCall("add_membership")
			.withIntParameter("type", Socket::MULTICAST)
			.withMemoryBufferParameter("multicast_address", address, Socket::ADDRESS_LENGHT)
			.andReturnValue(false);

	Nan::TryCatch catchBlock;
	Nan::MakeCallback(wrap, "AddMembership", argc, argv);
	if(!catchBlock.HasCaught()) {
		FAIL("AddMembership didn't threw");
	}
}

TEST(WrapperUsageGroup, AddMembershipShouldSucceddWithAddress) {
	v8::Local<v8::Object> wrap = Nan::New(wrapped_obj);
	const int argc = 1;
	const unsigned char address[] = {0xde, 0xad, 0x00, 0x00, 0x12, 0x34};
	char *address_for_node_buffer = new char[Socket::ADDRESS_LENGHT];
	memcpy(address_for_node_buffer, address, Socket::ADDRESS_LENGHT);
	v8::Local<v8::Value> argv[argc] = {Nan::NewBuffer(address_for_node_buffer, 6).ToLocalChecked()};

	mock().expectOneCall("add_membership")
			.withIntParameter("type", Socket::MULTICAST)
			.withMemoryBufferParameter("multicast_address", address, Socket::ADDRESS_LENGHT)
			.andReturnValue(true);

	Nan::TryCatch catchBlock;
	Nan::MakeCallback(wrap, "AddMembership", argc, argv);
	if(catchBlock.HasCaught()) {
		FAIL("AddMembership did threw");
	}
}

TEST(WrapperUsageGroup, AddMembershipShouldFailOnSocketCallFailWithType) {
	v8::Local<v8::Object> wrap = Nan::New(wrapped_obj);
	const int argc = 1;
	v8::Local<v8::Value> argv[argc] = {Nan::New(Socket::PROMISCIOUS)};

	mock().expectOneCall("add_membership")
			.withIntParameter("type", Socket::PROMISCIOUS)
			.ignoreOtherParameters()
			.andReturnValue(false);

	Nan::TryCatch catchBlock;
	Nan::MakeCallback(wrap, "AddMembership", argc, argv);
	if(!catchBlock.HasCaught()) {
		FAIL("AddMembership didn't threw");
	}
}

TEST(WrapperUsageGroup, AddMembershipShouldSucceddWithType) {
	v8::Local<v8::Object> wrap = Nan::New(wrapped_obj);
	const int argc = 1;
	v8::Local<v8::Value> argv[argc] = {Nan::New(Socket::PROMISCIOUS)};

	mock().expectOneCall("add_membership")
			.withIntParameter("type", Socket::PROMISCIOUS)
			.ignoreOtherParameters()
			.andReturnValue(true);

	Nan::TryCatch catchBlock;
	Nan::MakeCallback(wrap, "AddMembership", argc, argv);
	if(catchBlock.HasCaught()) {
		FAIL("AddMembership did threw");
	}
}

TEST(WrapperUsageGroup, AddMembershipShouldFailWithTypeOutOfRange) {
	v8::Local<v8::Object> wrap = Nan::New(wrapped_obj);
	const int argc = 1;
	v8::Local<v8::Value> argv[argc] = {Nan::New(123)};

	mock().expectOneCall("add_membership")
				.ignoreOtherParameters()
				.andReturnValue(false);

	Nan::TryCatch catchBlock;
	Nan::MakeCallback(wrap, "AddMembership", argc, argv);
	if(!catchBlock.HasCaught()) {
		FAIL("AddMembership didn't threw");
	}
}

TEST(WrapperUsageGroup, DropMembershipShouldFailOnEmptyArguments) {
	v8::Local<v8::Object> wrap = Nan::New(wrapped_obj);
	v8::Local<v8::Value> argv[] = {};

	Nan::TryCatch catchBlock;
	Nan::MakeCallback(wrap, "DropMembership", 0, argv);
	if(!catchBlock.HasCaught()) {
		FAIL("DropMembership didn't threw");
	}
}

TEST(WrapperUsageGroup, DropMembershipShouldFailOnNoAddressOrNonTypeArguments) {
	v8::Local<v8::Object> wrap = Nan::New(wrapped_obj);
	const int argc = 1;
	v8::Local<v8::Value> argv[argc] = {Nan::EmptyString()};

	Nan::TryCatch catchBlock;
	Nan::MakeCallback(wrap, "DropMembership", argc, argv);
	if(!catchBlock.HasCaught()) {
		FAIL("DropMembership didn't threw");
	}
}

TEST(WrapperUsageGroup, DropMembershipShouldFailOnToShortAddressArgument) {
	v8::Local<v8::Object> wrap = Nan::New(wrapped_obj);
	const int argc = 1;
	v8::Local<v8::Value> argv[argc] = {Nan::NewBuffer(4).ToLocalChecked()};

	Nan::TryCatch catchBlock;
	Nan::MakeCallback(wrap, "DropMembership", argc, argv);
	if(!catchBlock.HasCaught()) {
		FAIL("DropMembership didn't threw");
	}
}

TEST(WrapperUsageGroup, DropMembershipShouldFailOnSocketCallFailWithAddress) {
	v8::Local<v8::Object> wrap = Nan::New(wrapped_obj);
	const int argc = 1;
	const unsigned char address[] = {0xde, 0xad, 0x00, 0x00, 0x12, 0x34};
	char *address_for_node_buffer = new char[Socket::ADDRESS_LENGHT];
	memcpy(address_for_node_buffer, address, Socket::ADDRESS_LENGHT);
	v8::Local<v8::Value> argv[argc] = {Nan::NewBuffer(address_for_node_buffer, 6).ToLocalChecked()};

	mock().expectOneCall("drop_membership")
			.withIntParameter("type", Socket::MULTICAST)
			.withMemoryBufferParameter("multicast_address", address, Socket::ADDRESS_LENGHT)
			.andReturnValue(false);

	Nan::TryCatch catchBlock;
	Nan::MakeCallback(wrap, "DropMembership", argc, argv);
	if(!catchBlock.HasCaught()) {
		FAIL("DropMembership didn't threw");
	}
}

TEST(WrapperUsageGroup, DropMembershipShouldSucceddWithAddress) {
	v8::Local<v8::Object> wrap = Nan::New(wrapped_obj);
	const int argc = 1;
	const unsigned char address[] = {0xde, 0xad, 0x00, 0x00, 0x12, 0x34};
	char *address_for_node_buffer = new char[Socket::ADDRESS_LENGHT];
	memcpy(address_for_node_buffer, address, Socket::ADDRESS_LENGHT);
	v8::Local<v8::Value> argv[argc] = {Nan::NewBuffer(address_for_node_buffer, 6).ToLocalChecked()};

	mock().expectOneCall("drop_membership")
			.withIntParameter("type", Socket::MULTICAST)
			.withMemoryBufferParameter("multicast_address", address, Socket::ADDRESS_LENGHT)
			.andReturnValue(true);

	Nan::TryCatch catchBlock;
	Nan::MakeCallback(wrap, "DropMembership", argc, argv);
	if(catchBlock.HasCaught()) {
		FAIL("DropMembership did threw");
	}
}

TEST(WrapperUsageGroup, DropMembershipShouldFailOnSocketCallFailWithType) {
	v8::Local<v8::Object> wrap = Nan::New(wrapped_obj);
	const int argc = 1;
	v8::Local<v8::Value> argv[argc] = {Nan::New(Socket::PROMISCIOUS)};

	mock().expectOneCall("drop_membership")
			.withIntParameter("type", Socket::PROMISCIOUS)
			.ignoreOtherParameters()
			.andReturnValue(false);

	Nan::TryCatch catchBlock;
	Nan::MakeCallback(wrap, "DropMembership", argc, argv);
	if(!catchBlock.HasCaught()) {
		FAIL("DropMembership didn't threw");
	}
}

TEST(WrapperUsageGroup, DropMembershipShouldSucceddWithType) {
	v8::Local<v8::Object> wrap = Nan::New(wrapped_obj);
	const int argc = 1;
	v8::Local<v8::Value> argv[argc] = {Nan::New(Socket::PROMISCIOUS)};

	mock().expectOneCall("drop_membership")
			.withIntParameter("type", Socket::PROMISCIOUS)
			.ignoreOtherParameters()
			.andReturnValue(true);

	Nan::TryCatch catchBlock;
	Nan::MakeCallback(wrap, "DropMembership", argc, argv);
	if(catchBlock.HasCaught()) {
		FAIL("DropMembership did threw");
	}
}

TEST(WrapperUsageGroup, DropMembershipShouldFailWithTypeOutOfRange) {
	v8::Local<v8::Object> wrap = Nan::New(wrapped_obj);
	const int argc = 1;
	v8::Local<v8::Value> argv[argc] = {Nan::New(123)};

	mock().expectOneCall("drop_membership")
				.ignoreOtherParameters()
				.andReturnValue(false);

	Nan::TryCatch catchBlock;
	Nan::MakeCallback(wrap, "DropMembership", argc, argv);
	if(!catchBlock.HasCaught()) {
		FAIL("DropMembership didn't threw");
	}
}

TEST(WrapperUsageGroup, SendShouldFailWithNoArguments) {
	v8::Local<v8::Object> wrap = Nan::New(wrapped_obj);
	const int argc = 0;
	v8::Local<v8::Value> argv[argc] = {};

	Nan::TryCatch catchBlock;
	Nan::MakeCallback(wrap, "Send", argc, argv);
	if(!catchBlock.HasCaught()) {
		FAIL("Send didn't threw");
	}
}

TEST(WrapperUsageGroup, SendShouldFailWithNotBufferMessageArgument) {
	v8::Local<v8::Object> wrap = Nan::New(wrapped_obj);
	const int argc = 3;
	v8::Local<v8::Value> argv[argc] = { Nan::EmptyString(), Nan::EmptyString(), Nan::EmptyString()};

	Nan::TryCatch catchBlock;
	Nan::MakeCallback(wrap, "Send", argc, argv);
	if(!catchBlock.HasCaught()) {
		FAIL("Send didn't threw");
	}
}

TEST(WrapperUsageGroup, SendShouldFailWithNotBufferAddressArgument) {
	v8::Local<v8::Object> wrap = Nan::New(wrapped_obj);
	const int argc = 3;
	v8::Local<v8::Value> argv[argc] = { Nan::NewBuffer(5).ToLocalChecked(), Nan::EmptyString(), Nan::EmptyString()};

	Nan::TryCatch catchBlock;
	Nan::MakeCallback(wrap, "Send", argc, argv);
	if(!catchBlock.HasCaught()) {
		FAIL("Send didn't threw");
	}
}

TEST(WrapperUsageGroup, SendShouldFailWithAddressArgumentInvalidSize) {
	v8::Local<v8::Object> wrap = Nan::New(wrapped_obj);
	const int argc = 3;
	v8::Local<v8::Value> argv[argc] = {
			Nan::NewBuffer(5).ToLocalChecked(),
			Nan::NewBuffer(5).ToLocalChecked(),
			Nan::EmptyString()};

	Nan::TryCatch catchBlock;
	Nan::MakeCallback(wrap, "Send", argc, argv);
	if(!catchBlock.HasCaught()) {
		FAIL("Send didn't threw");
	}
}

TEST(WrapperUsageGroup, SendShouldFailWithNotFunctionCallbackArgument) {
	v8::Local<v8::Object> wrap = Nan::New(wrapped_obj);
	const int argc = 3;
	v8::Local<v8::Value> argv[argc] = {
			Nan::NewBuffer(5).ToLocalChecked(),
			Nan::NewBuffer(Socket::ADDRESS_LENGHT).ToLocalChecked(),
			Nan::EmptyString()};

	Nan::TryCatch catchBlock;
	Nan::MakeCallback(wrap, "Send", argc, argv);
	if(!catchBlock.HasCaught()) {
		FAIL("Send didn't threw");
	}
}

TEST(WrapperUsageGroup, SendShouldFailWhenSocketSendMessageFail) {
	v8::Local<v8::Object> wrap = Nan::New(wrapped_obj);
	const unsigned char address[] = {0xde, 0xad, 0x00, 0x00, 0x12, 0x34};
	char *address_for_node_buffer = new char[Socket::ADDRESS_LENGHT];
	memcpy(address_for_node_buffer, address, Socket::ADDRESS_LENGHT);
	const char *message = "hello world";
	char *message_for_node_buffer = new char[strlen(message) + 1];
	strcpy(message_for_node_buffer, message);

	const int argc = 3;
	v8::Local<v8::Value> argv[argc] = {
			Nan::NewBuffer(message_for_node_buffer, strlen(message)).ToLocalChecked(),
			Nan::NewBuffer(address_for_node_buffer, Socket::ADDRESS_LENGHT).ToLocalChecked(),
			Nan::New<v8::Function>(Noop)};

	mock().expectOneCall("send_message")
			.withMemoryBufferParameter("destination_address", address, Socket::ADDRESS_LENGHT)
			.withMemoryBufferParameter("message", (const unsigned char*)message, strlen(message))
			.withIntParameter("message_length", strlen(message))
			.andReturnValue(-1);

	Nan::TryCatch catchBlock;
	Nan::MakeCallback(wrap, "Send", argc, argv);
	if(!catchBlock.HasCaught()) {
		FAIL("Send didn't threw");
	}
}

NAN_METHOD(CallbackForSend) {
	if(info.Length() != 1)
		FAIL("Invalid arguments in callback");

	if(!info[0]->IsNumber())
		FAIL("Argument is not a number");

	int return_code = Nan::To<int>(info[0]).FromJust();

	mock().actualCall("CallbackFromSend").withIntParameter("rc", return_code);
}

TEST(WrapperUsageGroup, SendShouldCallCallbackWhenSendMessageSuccedd) {
	v8::Local<v8::Object> wrap = Nan::New(wrapped_obj);
	const unsigned char address[] = {0xde, 0xad, 0x00, 0x00, 0x12, 0x34};
	char *address_for_node_buffer = new char[Socket::ADDRESS_LENGHT];
	memcpy(address_for_node_buffer, address, Socket::ADDRESS_LENGHT);
	const char *message = "hello world";
	int message_len = strlen(message);
	char *message_for_node_buffer = new char[message_len + 1];
	strcpy(message_for_node_buffer, message);

	const int argc = 3;
	v8::Local<v8::Value> argv[argc] = {
			Nan::NewBuffer(message_for_node_buffer, message_len).ToLocalChecked(),
			Nan::NewBuffer(address_for_node_buffer, Socket::ADDRESS_LENGHT).ToLocalChecked(),
			Nan::New<v8::Function>(CallbackForSend)};

	mock().expectOneCall("send_message")
			.withMemoryBufferParameter("destination_address", address, Socket::ADDRESS_LENGHT)
			.withMemoryBufferParameter("message", (const unsigned char*)message, message_len)
			.withIntParameter("message_length", message_len)
			.andReturnValue(message_len);

	mock().expectOneCall("CallbackFromSend").withIntParameter("rc", message_len);

	Nan::TryCatch catchBlock;
	Nan::MakeCallback(wrap, "Send", argc, argv);
	if(catchBlock.HasCaught()) {
		FAIL("Send did threw");
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
