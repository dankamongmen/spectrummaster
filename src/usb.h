#ifndef SPECTRUMMASTER_USB
#define SPECTRUMMASTER_USB

#include <thread>
#include <atomic>
#include <iomanip>
#include <libusb.h>
#include <iostream>
#include <functional>

class USBException : public std::runtime_error {
public:
USBException(const std::string& what) :
  std::runtime_error(what) {}
};

class SmUSB {
public:

SmUSB() :
cancelled_(false)
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
  tid = std::thread(&SmUSB::USBThread, this);
}

~SmUSB() {
  // FIXME need a way to interrupt thread in libusb_handle_events_completed()
  cancelled_ = true;
  tid.join();
  libusb_exit(usbctx_);
}

private:
libusb_context *usbctx_;
libusb_hotplug_callback_handle cbhandle_;
std::thread tid;
std::atomic<bool> cancelled_;

void USBThread() {
  while(!cancelled_){
    int completed = 0;
    auto e = libusb_handle_events_completed(usbctx_, &completed);
    if(e != LIBUSB_SUCCESS){
      // FIXME...
    }else{
      std::cout << "Completion integer: " << completed << std::endl; // FIXME?
    }
  }
}

struct USBSpeed {
  uint64_t bps;
};

int DevCallback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event){
  static constexpr std::array<struct USBSpeed, 8> speeds {{
    {           0, }, // unknown
    {     1500000, }, // usb1.0 lowspeed
    {    12000000, }, // usb1.1 fullspeed
    {   480000000, }, // usb2 highspeed
    {  5000000000, }, // usb3 superspeed
    { 10000000000, }, // usb3.1 superspeed+
    { 20000000000, }, // usb3.2 superspeed+
    { 40000000000, }, // usb4 superspeed+
  }};
  int bus = libusb_get_bus_number(dev);
  int port = libusb_get_port_number(dev);
  int addr = libusb_get_device_address(dev);
  libusb_device_descriptor desc;
  auto e = libusb_get_device_descriptor(dev, &desc);
  if(e){
    std::cerr << "Couldn't get device descriptor for " << bus << ":" << port << ":" << addr
              << libusb_strerror(static_cast<libusb_error>(e)) << std::endl;
    return 0;
  }
  constexpr int USB_TOPOLOGY_MAXLEN = 7;
  std::array<uint8_t, USB_TOPOLOGY_MAXLEN> numbers;
  auto ret = libusb_get_port_numbers(dev, numbers.data(), numbers.size());
  char prev = std::cout.fill('0');
  int speedidx = libusb_get_device_speed(dev);
  if(speedidx < 0 || static_cast<size_t>(speedidx) >= speeds.size()){
    std::cerr << "Unknown speed (" << speedidx << ") for " << bus << ":"
              << port << ":" << addr
              << libusb_strerror(static_cast<libusb_error>(e)) << std::endl;
    return 0;
  }
  auto& speed = speeds[speedidx];
  std::cout << "E" << event
            << " VendID 0x" << std::hex << std::setw(4) << desc.idVendor
            << " ProdID 0x" << std::setw(4) << desc.idProduct << std::dec
            << " " << ((double)speed.bps / 1000000) // FIXME enmetric()icize
            << " Bus " << std::setw(3) << bus
            << " Port " << std::setw(3) << port
            << " Addr " << std::setw(3) << addr << " ";
  for(auto n = 0 ; n < ret ; ++n){
    std::cout << static_cast<int>(numbers[n]) << (n + 1 < ret ? "." : "");
  }
  std::cout << std::endl;
  std::cout.fill(prev);
  // FIXME accumulate (and/or prune) device list
  return 0;
}

// libusb_hotplug_callback_fn. uptr is 'this'
static int devcallback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *uptr){
  return (static_cast<SmUSB*>(uptr))->DevCallback(ctx, dev, event);
}

};

#endif
