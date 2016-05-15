#include "wrapper.hh"

Wrapper::Wrapper(v8::Local<v8::String> device) {
}
Wrapper::~Wrapper() {
}

Nan::Persistent<v8::Function> Wrapper::constructor;
#include <cstdio>
NAN_MODULE_INIT(Wrapper::Init) {
	v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
	tpl->SetClassName(Nan::New("Wrapper").ToLocalChecked());
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
	Nan::Set(target, Nan::New("Wrapper").ToLocalChecked(),
			Nan::GetFunction(tpl).ToLocalChecked());
}

NAN_METHOD(Wrapper::New) {
	if(info.IsConstructCall()) {
		info.GetReturnValue().Set(info.This());
	} else {
		const int argc = 1;
		v8::Local<v8::Value> argv[argc] = {info[0]};
		v8::Local<v8::Function> emulate_constructor = Nan::New(constructor);

		Nan::MaybeLocal<v8::Object> constructed_object =
				Nan::NewInstance(emulate_constructor, argc, argv);
		info.GetReturnValue().Set(constructed_object.ToLocalChecked());
	}
}

NODE_MODULE(packet_socket_addon, Wrapper::Init);
