#include "poller.hh"

UvPoller::UvPoller(
  int descriptor,
  void (*read_ready_callback)(void *data),
  void (*write_ready_callback)(void *data),
  void (*error_callback)(void *data, const char *error),
  void *external_data
) : read_ready_callback(read_ready_callback),
    write_ready_callback(write_ready_callback),
    error_callback(error_callback),
    external_data(external_data) {
  uv_poll = new uv_poll_t;
  uv_poll_init_socket(uv_default_loop(), uv_poll, descriptor);
  uv_poll->data = this;
  current_events = READ_EVENT;
  uv_poll_start(uv_poll, UV_READABLE, UvPoller::io_event_wrapper);
}

UvPoller::~UvPoller(void) {
  uv_close(reinterpret_cast<uv_handle_t *>(uv_poll), UvPoller::on_close);
}

void UvPoller::on_close(uv_handle_t *handle) {
  delete handle;
}

void UvPoller::io_event_wrapper(uv_poll_t *watcher, int status, int revents) {
  UvPoller *poller = static_cast<UvPoller *>(watcher->data);
  poller->io_event(status, revents);
}

void UvPoller::io_event(int status, int revents) {
  if(status) {
    error_callback(external_data, uv_strerror(status));
  } else {
    if(revents & UV_READABLE)
      read_ready_callback(external_data);
    if(revents & UV_WRITABLE)
      write_ready_callback(external_data);
  }
}

void UvPoller::set_events(PollerEvents events) {
  if(events == current_events) return;
  current_events = events;
  uv_poll_stop(uv_poll);
  if(events)
    uv_poll_start(uv_poll, events, UvPoller::io_event_wrapper);
}

NullPoller::NullPoller(void) {}
NullPoller::~NullPoller(void) {}
void NullPoller::set_events(PollerEvents events) {}
