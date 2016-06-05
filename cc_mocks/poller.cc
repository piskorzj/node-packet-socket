#include "CppUTestExt/MockSupport.h"
#include "poller.hh"


Poller::Poller(
  int descriptor,
  void (*read_ready_callback)(void *data),
  void (*write_ready_callback)(void *data),
  void (*error_callback)(void *data, const char *error),
  void *external_data
) {}

Poller::~Poller(void) {}

void Poller::on_close(uv_handle_t *handle) {}

void Poller::io_event_wrapper(uv_poll_t *watcher, int status, int revents) {}

void Poller::io_event(int status, int revents) {}

void Poller::set_events(PollerEvents events) {
  mock().actualCall("set_events").withIntParameter("events", events);
}
