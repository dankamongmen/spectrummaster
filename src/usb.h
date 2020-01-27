#ifndef SPECTRUMMASTER_USB
#define SPECTRUMMASTER_USB

#include <libusb.h>
#include <functional>

class USBException : public std::runtime_error {
public:
USBException(const std::string& what) :
  std::runtime_error(what) {}
};

class SmUSB {
public:

SmUSB() :
usbctx_(nullptr)
{
  auto e = libusb_init(&usbctx_);
  if(e != LIBUSB_SUCCESS){
    throw USBException(libusb_strerror(static_cast<libusb_error>(e)));
  }
  libusb_set_option(usbctx_, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_WARNING);
  if(!libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG)){
    throw USBException("No hotplug support");
  }
  e = libusb_hotplug_register_callback(usbctx_,
                                       static_cast<libusb_hotplug_event>(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT),
                                       LIBUSB_HOTPLUG_ENUMERATE,
                                       LIBUSB_HOTPLUG_MATCH_ANY, // vendor ID
                                       LIBUSB_HOTPLUG_MATCH_ANY, // product ID
                                       LIBUSB_HOTPLUG_MATCH_ANY, // dev class
                                       devcallback, this, &cbhandle_);
  if(e != LIBUSB_SUCCESS){
    throw USBException(libusb_strerror(static_cast<libusb_error>(e)));
  }
}

~SmUSB() {
  libusb_exit(usbctx_);
}

private:
libusb_context *usbctx_;
libusb_hotplug_callback_handle cbhandle_;

int DevCallback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event){
  // FIXME accumulate (and/or prune) device list
  return 0;
}

// libusb_hotplug_callback_fn. uptr is 'this'
static int devcallback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *uptr){
  return (static_cast<SmUSB*>(uptr))->DevCallback(ctx, dev, event);
}

};

#endif
