#ifndef _WRAPPER_HH_
#define _WRAPPER_HH_

#include "node.h"
#include "nan.h"
#include "socket.hh"

class Wrapper : public Nan::ObjectWrap {
	static Nan::Persistent<v8::Function> constructor;

	Socket *socket;

	explicit Wrapper(v8::Local<v8::String> device);
	~Wrapper(void);
public:
	static NAN_MODULE_INIT(Init);

	static NAN_METHOD(New);

	static NAN_METHOD(AddMembership);
	static NAN_METHOD(DropMembership);
};

#endif
