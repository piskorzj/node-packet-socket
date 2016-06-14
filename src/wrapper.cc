#include "wrapper.hh"
#include <stdexcept>

Wrapper::Wrapper(v8::Local<v8::Object> options) {
	Nan::HandleScope scope;

	v8::Local<v8::Value> options_device_value = Nan::Get(options, Nan::New("device").ToLocalChecked()).ToLocalChecked();
	if(!options_device_value->IsString()) {
		throw std::invalid_argument("Options should have string device property");
	}

	v8::Local<v8::Value> options_onRecv_value = Nan::Get(options, Nan::New("onRecv").ToLocalChecked()).ToLocalChecked();
	if(!options_onRecv_value->IsFunction()) {
		throw std::invalid_argument("Options should have callback onRecv property");
	}

	v8::Local<v8::Value> options_onSend_value = Nan::Get(options, Nan::New("onSend").ToLocalChecked()).ToLocalChecked();
	if(!options_onSend_value->IsFunction()) {
		throw std::invalid_argument("Options should have callback onSend property");
	}

	v8::Local<v8::Value> options_onError_value = Nan::Get(options, Nan::New("onError").ToLocalChecked()).ToLocalChecked();
	if(!options_onError_value->IsFunction()) {
		throw std::invalid_argument("Options should have callback onError property");
	}

	onRecvCallback.Reset(options_onRecv_value.As<v8::Function>());
	onSendCallback.Reset(options_onSend_value.As<v8::Function>());
	onErrorCallback.Reset(options_onError_value.As<v8::Function>());

	Nan::Utf8String device_string(options_device_value);
	socket = new Socket(*device_string);
	poller = new UvPoller(socket->get_descriptor(),
		Wrapper::ReadReadyCallback,
		Wrapper::WriteReadyCallback,
		Wrapper::ErrorCallback,
		this);
}
Wrapper::~Wrapper() {
	onRecvCallback.Reset();
	onSendCallback.Reset();
	onErrorCallback.Reset();
	delete poller;
	delete socket;
}

Nan::Persistent<v8::Function> Wrapper::constructor;
#include <cstdio>
NAN_MODULE_INIT(Wrapper::Init) {
	v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
	tpl->SetClassName(Nan::New("Wrapper").ToLocalChecked());
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	Nan::SetPrototypeMethod(tpl, "AddMembership", AddMembership);
	Nan::SetPrototypeMethod(tpl, "DropMembership", DropMembership);
	Nan::SetPrototypeMethod(tpl, "Send", Send);
	Nan::SetPrototypeMethod(tpl, "Receive", Receive);

	Nan::SetPrototypeMethod(tpl, "PauseSending", PauseSending);
	Nan::SetPrototypeMethod(tpl, "ResumeSending", ResumeSending);

	constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
	Nan::Set(target, Nan::New("Wrapper").ToLocalChecked(),
			Nan::GetFunction(tpl).ToLocalChecked());

	Nan::Set(target, Nan::New("PROMISCIOUS").ToLocalChecked(), Nan::New(Socket::PROMISCIOUS));
	Nan::Set(target, Nan::New("ALL_MULTICAST").ToLocalChecked(), Nan::New(Socket::ALL_MULTICAST));
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

		if(!Nan::HasOwnProperty(options_object, Nan::New("onError").ToLocalChecked()).FromMaybe(false)) {
			return Nan::ThrowTypeError("Options should have onError property");
		}

		try {
			Wrapper *object = new Wrapper(options_object);
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

void Wrapper::ReadReadyCallback(void *data) {
	Nan::HandleScope scope;
	Wrapper *wrap = reinterpret_cast<Wrapper *>(data);
	Nan::Callback callback(Nan::New<v8::Function>(wrap->onRecvCallback));
	callback.Call(0, 0);
}
void Wrapper::WriteReadyCallback(void *data) {
	Nan::HandleScope scope;
	Wrapper *wrap = reinterpret_cast<Wrapper *>(data);
	Nan::Callback callback(Nan::New<v8::Function>(wrap->onSendCallback));
	callback.Call(0, 0);
}

void Wrapper::ErrorCallback(void *data, const char *error) {
	Nan::HandleScope scope;
	Wrapper *wrap = reinterpret_cast<Wrapper *>(data);
	Nan::Callback callback(Nan::New<v8::Function>(wrap->onErrorCallback));
	const int argc = 1;
	v8::Local<v8::Value> argv[argc] = {Nan::New(error).ToLocalChecked()};
	callback.Call(argc, argv);
}

void Wrapper::ParseMembershipArguments(
			Nan::NAN_METHOD_ARGS_TYPE info,
			Socket::MembershipType *type,
			unsigned char **address) {
	Nan::HandleScope scope;
	Nan::MaybeLocal<v8::Value> type_or_addr_maybe = info[0];

	if(info.Length() == 0 || type_or_addr_maybe.IsEmpty())
		throw std::invalid_argument("Missing type or address argument");
	v8::Local<v8::Value> type_or_addr = type_or_addr_maybe.ToLocalChecked();

	if(node::Buffer::HasInstance(type_or_addr)) {
		if(node::Buffer::Length(type_or_addr) < Socket::ADDRESS_LENGHT)
			throw std::invalid_argument("Invalid address length");
		*address = reinterpret_cast<unsigned char *>(node::Buffer::Data(type_or_addr));
	} else if(type_or_addr->IsNumber()) {
		*type = static_cast<Socket::MembershipType>(Nan::To<int>(type_or_addr).FromJust());
	} else {
		throw std::invalid_argument("Invalid address length");
	}
}

NAN_METHOD(Wrapper::AddMembership) {
	Wrapper *obj = Nan::ObjectWrap::Unwrap<Wrapper>(info.Holder());

	Socket::MembershipType type = Socket::MULTICAST;
	unsigned char *address = NULL;

	try {
		ParseMembershipArguments(info, &type, &address);
		obj->socket->add_membership(type, address);
	} catch(const std::exception &e) {
		return Nan::ThrowError(e.what());
	}
}

NAN_METHOD(Wrapper::DropMembership) {
	Wrapper *obj = Nan::ObjectWrap::Unwrap<Wrapper>(info.Holder());

	Socket::MembershipType type = Socket::MULTICAST;
	unsigned char *address = NULL;

	try {
		ParseMembershipArguments(info, &type, &address);
		obj->socket->drop_membership(type, address);
	} catch(const std::exception &e) {
		return Nan::ThrowError(e.what());
	}
}

NAN_METHOD(Wrapper::Receive) {
	Wrapper *obj = Nan::ObjectWrap::Unwrap<Wrapper>(info.Holder());

	if(info.Length() != 1)
		return Nan::ThrowError("One argument required");

	if(!info[0]->IsFunction())
		return Nan::ThrowTypeError("Callback argument has to be function");
	v8::Local<v8::Function> callback = info[0].As<v8::Function>();

	int received_bytes = -1;
	unsigned char *source_address = reinterpret_cast<unsigned char *>(malloc(Socket::ADDRESS_LENGHT));
	if(!source_address)
		return Nan::ThrowError("Memory allocation for source address failed");

	unsigned char *destination_address = reinterpret_cast<unsigned char *>(malloc(Socket::ADDRESS_LENGHT));
	if(!destination_address) {
		free(source_address);
		return Nan::ThrowError("Memory allocation for destination address failed");
	}

	char *message_buffer = reinterpret_cast<char *>(malloc(Wrapper::MAX_RECEIVE_BUFFER_SIZE));
	if(!message_buffer) {
		free(source_address);
		free(destination_address);
		return Nan::ThrowError("Memory allocation for buffer failed");
	}

	try {
		received_bytes = obj->socket->receive_message(
				source_address,
				destination_address,
				message_buffer,
				Wrapper::MAX_RECEIVE_BUFFER_SIZE);
	} catch(const std::exception &e) {
		free(destination_address);
		free(source_address);
		free(message_buffer);
		return Nan::ThrowError(e.what());
	}

	char *tmp_buff = reinterpret_cast<char *>(realloc(message_buffer, received_bytes));
	if(tmp_buff)
		message_buffer = tmp_buff;

	Nan::MaybeLocal<v8::Object> source_address_node_buffer = Nan::NewBuffer((char *)source_address, Socket::ADDRESS_LENGHT);
	Nan::MaybeLocal<v8::Object> destination_address_node_buffer = Nan::NewBuffer((char *)destination_address, Socket::ADDRESS_LENGHT);
	Nan::MaybeLocal<v8::Object> message_node_buffer = Nan::NewBuffer(message_buffer, received_bytes);
	if(source_address_node_buffer.IsEmpty() || destination_address_node_buffer.IsEmpty() || message_node_buffer.IsEmpty()) {
		return Nan::ThrowError("Failed to create buffer object");
	}

	const int argc = 3;
	v8::Local<v8::Value> argv[argc] = {
		source_address_node_buffer.ToLocalChecked(),
		destination_address_node_buffer.ToLocalChecked(),
		message_node_buffer.ToLocalChecked()
	};
	Nan::MakeCallback(Nan::GetCurrentContext()->Global(), callback, argc, argv);

}

NAN_METHOD(Wrapper::Send) {
	Wrapper *obj = Nan::ObjectWrap::Unwrap<Wrapper>(info.Holder());

	if(info.Length() != 3)
		return Nan::ThrowError("Three arguments required");

	v8::Local<v8::Value> message_to_send = info[0];
	if(!node::Buffer::HasInstance(message_to_send))
		return Nan::ThrowTypeError("Message argument has to be buffer");

	v8::Local<v8::Value> destination_address = info[1];
	if(!node::Buffer::HasInstance(destination_address))
		return Nan::ThrowTypeError("Address argument has to be buffer");
	if(node::Buffer::Length(destination_address) != Socket::ADDRESS_LENGHT)
		return Nan::ThrowTypeError("Address argument invalid length");

	if(!info[2]->IsFunction())
		return Nan::ThrowTypeError("Callback argument has to be function");
	v8::Local<v8::Function> callback = info[2].As<v8::Function>();

	int send_bytes = -1;
	try {
		send_bytes = obj->socket->send_message(
				reinterpret_cast<unsigned char *>(node::Buffer::Data(destination_address)),
				node::Buffer::Data(message_to_send),
				node::Buffer::Length(message_to_send));
	} catch(const std::exception &e) {
		return Nan::ThrowError(e.what());
	}

	const int argc = 1;
	v8::Local<v8::Value> argv[argc] = { Nan::New(send_bytes) };
	Nan::MakeCallback(Nan::GetCurrentContext()->Global(), callback, argc, argv);
}

NAN_METHOD(Wrapper::PauseSending) {
	Wrapper *obj = Nan::ObjectWrap::Unwrap<Wrapper>(info.Holder());
	obj->poller->set_events(Poller::READ_EVENT);
}

NAN_METHOD(Wrapper::ResumeSending) {
	Wrapper *obj = Nan::ObjectWrap::Unwrap<Wrapper>(info.Holder());
	obj->poller->set_events(Poller::RW_EVENT);
}

NODE_MODULE(packet_socket_addon, Wrapper::Init);
