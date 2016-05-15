#include "wrapper.hh"

Wrapper::Wrapper(v8::Local<v8::String> device) {
	Nan::Utf8String device_string(device);
	socket = new Socket(*device_string);
}
Wrapper::~Wrapper() {
	delete socket;
}

Nan::Persistent<v8::Function> Wrapper::constructor;
#include <cstdio>
NAN_MODULE_INIT(Wrapper::Init) {
	v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
	tpl->SetClassName(Nan::New("Wrapper").ToLocalChecked());
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	Nan::SetPrototypeMethod(tpl, "AddMembership", AddMembership);

	constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
	Nan::Set(target, Nan::New("Wrapper").ToLocalChecked(),
			Nan::GetFunction(tpl).ToLocalChecked());
}

NAN_METHOD(Wrapper::New) {
	if(info.IsConstructCall()) {
		Nan::MaybeLocal<v8::Object> options_object_maybe = Nan::To<v8::Object>(info[0]);
		if(options_object_maybe.IsEmpty())
			return Nan::ThrowError("Missing options object");

		v8::Local<v8::Object> options_object = options_object_maybe.ToLocalChecked();

		if(!Nan::HasOwnProperty(options_object, Nan::New("device").ToLocalChecked()).FromMaybe(false)) {
			return Nan::ThrowTypeError("Options should have device property");
		}

		if(!Nan::HasOwnProperty(options_object, Nan::New("onRecv").ToLocalChecked()).FromMaybe(false)) {
			return Nan::ThrowTypeError("Options should have onRecv property");
		}

		if(!Nan::HasOwnProperty(options_object, Nan::New("onSend").ToLocalChecked()).FromMaybe(false)) {
			return Nan::ThrowTypeError("Options should have onSend property");
		}

		v8::Local<v8::Value> options_device_value = Nan::Get(options_object, Nan::New("device").ToLocalChecked()).ToLocalChecked();
		if(!options_device_value->IsString()) {
			return Nan::ThrowTypeError("Options should have string device property");
		}

		v8::Local<v8::Value> options_onRecv_value = Nan::Get(options_object, Nan::New("onRecv").ToLocalChecked()).ToLocalChecked();
		if(!options_onRecv_value->IsFunction()) {
			return Nan::ThrowTypeError("Options should have callback onRecv property");
		}

		v8::Local<v8::Value> options_onSend_value = Nan::Get(options_object, Nan::New("onSend").ToLocalChecked()).ToLocalChecked();
		if(!options_onSend_value->IsFunction()) {
			return Nan::ThrowTypeError("Options should have callback onSend property");
		}

		try {
			Wrapper *object = new Wrapper(Nan::To<v8::String>(options_device_value).ToLocalChecked());
			object->Wrap(info.This());
		} catch(const std::exception &e) {
			return Nan::ThrowError(e.what());
		}
		info.GetReturnValue().Set(info.This());
	} else {
		const int argc = 1;
		v8::Local<v8::Value> argv[argc] = {info[0]};
		v8::Local<v8::Function> emulate_constructor = Nan::New(constructor);

		Nan::TryCatch trycatch;
		Nan::MaybeLocal<v8::Object> constructed_object =
				Nan::NewInstance(emulate_constructor, argc, argv);
		if(trycatch.HasCaught()) {
			trycatch.ReThrow();
			return;
		}
		info.GetReturnValue().Set(constructed_object.ToLocalChecked());
	}
}

NAN_METHOD(Wrapper::AddMembership) {
	Wrapper *obj = Nan::ObjectWrap::Unwrap<Wrapper>(info.Holder());
	Nan::MaybeLocal<v8::Value> type_or_addr_maybe = info[0];
	if(type_or_addr_maybe.IsEmpty())
		return Nan::ThrowError("Missing type or address argument");
	v8::Local<v8::Value> type_or_addr = type_or_addr_maybe.ToLocalChecked();

	unsigned char *address = NULL;
	Socket::MembershipType type = Socket::MULTICAST;

	if(node::Buffer::HasInstance(type_or_addr)) {
		if(node::Buffer::Length(type_or_addr) < Socket::ADDRESS_LENGHT) {
			return Nan::ThrowTypeError("Invalid address length");
		}
		address = reinterpret_cast<unsigned char *>(node::Buffer::Data(type_or_addr));
	} else if(type_or_addr->IsNumber()) {
		type = static_cast<Socket::MembershipType>(Nan::To<int>(type_or_addr).FromJust());
	} else {
		return Nan::ThrowTypeError("Invalid argument");
	}

	try {
		obj->socket->add_membership(type, address);
	} catch(const std::exception &e) {
		return Nan::ThrowError(e.what());
	}
}

NODE_MODULE(packet_socket_addon, Wrapper::Init);
