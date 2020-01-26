#ifndef SPECTRUMMASTER_USB
#define SPECTRUMMASTER_USB

#include <libusb.h>

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
}

~SmUSB() {
  libusb_exit(usbctx_);
}

private:
libusb_context *usbctx_;

};

#endif
