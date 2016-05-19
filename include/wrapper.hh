#ifndef _WRAPPER_HH_
#define _WRAPPER_HH_

#include "node.h"
#include "nan.h"
#include "socket.hh"

class Wrapper : public Nan::ObjectWrap {
	static const int MAX_RECEIVE_BUFFER_SIZE = 2048;

	static Nan::Persistent<v8::Function> constructor;

	Socket *socket;

	explicit Wrapper(v8::Local<v8::String> device);
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
};

#endif
