#ifndef _POLLER_H_
#define _POLLER_H_

#include <uv.h>

class Poller {
public:
  enum PollerEvents {
    NONE_EVENT = 0,
    READ_EVENT = UV_READABLE,
    WRITE_EVENT = UV_WRITABLE,
    RW_EVENT = UV_READABLE | UV_WRITABLE
  };
  virtual ~Poller(void) = 0;
  virtual void set_events(PollerEvents events) = 0;
};

class UvPoller : public Poller {
public:
  UvPoller(
    int descriptor,
    void (*read_ready_callback)(void *data),
    void (*write_ready_callback)(void *data),
    void (*error_callback)(void *data, const char *error),
    void *external_data
  );
  virtual ~UvPoller(void);

  void set_events(PollerEvents events);

private:
  void (*read_ready_callback)(void *data);
  void (*write_ready_callback)(void *data);
  void (*error_callback)(void *data, const char *error);
  void *external_data;

  uv_poll_t *uv_poll;
  PollerEvents current_events;

  static void on_close(uv_handle_t *handle);
  static void io_event_wrapper(uv_poll_t* watcher, int status, int revents);
  void io_event(int status, int revents);
};

#endif
