#ifndef _WRAPPER_HH_
#define _WRAPPER_HH_

#include "node.h"
#include "nan.h"
#include "socket.hh"
#include "poller.hh"

class Wrapper : public Nan::ObjectWrap {
	static const int MAX_RECEIVE_BUFFER_SIZE = 2048;

	static Nan::Persistent<v8::Function> constructor;

	Nan::Persistent<v8::Function> onSendCallback;
	Nan::Persistent<v8::Function> onRecvCallback;

	Socket *socket;
	Poller *poller;

	static void ReadReadyCallback(void *data);
	static void WriteReadyCallback(void *data);
	static void ErrorCallback(void *data, const char *error);

	explicit Wrapper(v8::Local<v8::Object> options);
	~Wrapper(void);

	static void ParseMembershipArguments(
			Nan::NAN_METHOD_ARGS_TYPE info,
			Socket::MembershipType *type,
			unsigned char **address);
public:
	static NAN_MODULE_INIT(Init);

	static NAN_METHOD(New);

	static NAN_METHOD(AddMembership);
	static NAN_METHOD(DropMembership);

	static NAN_METHOD(Send);
	static NAN_METHOD(Receive);

	static NAN_METHOD(PauseSending);
	static NAN_METHOD(ResumeSending);
};

#endif
