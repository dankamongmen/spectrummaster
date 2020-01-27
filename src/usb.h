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

class USBDev {
public:
USBDev(int bus, int port, int addr, unsigned vendorID, unsigned productID) :
bus_(bus),
port_(port),
addr_(addr),
vendorID_(vendorID),
productID_ (productID) {}

private:
  int bus_;
  int port_;
  int addr_;
  unsigned vendorID_;
  unsigned productID_;
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

struct USBProduct {
  unsigned vendorID;
  unsigned productID;
  const char *name; // FIXME load from usb.ids?
  // FIXME API implementation
};

static constexpr std::array<struct USBProduct, 5> USBProducts {{
  { 0x0bda, 0x2838, "Realtek Semiconductor Corp. RTL2838 DVB-T", },
  { 0x1d50, 0x6108, "OpenMoko, Inc. Myriad-RF LimeSDR", },
  { 0x0403, 0x601f, "Myriad-RF LimeSDR Mini", },
  { 0x2cf0, 0x5246, "BladeRF", },
  { 0x2cf0, 0x5250, "BladeRF 2.0 micro", }
}};

private:
libusb_context *usbctx_;
libusb_hotplug_callback_handle cbhandle_;
std::thread tid;
std::atomic<bool> cancelled_;
std::unordered_map<unsigned, USBDev> devices_;

void USBThread() {
  while(!cancelled_){
    int completed = 0;
    // need to loop on libusb to some degree, or hotplug events never get
    // fired off :/
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

// "hashes" a usb bus+port+addr, all of which take range [0..127]
static unsigned
USBHash(unsigned bus, unsigned port, unsigned addr) {
  return bus * (128 * 128) + port * 128 + addr;
}

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
  auto hidx = USBHash(bus, port, addr);
  if(event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT){
    devices_.erase(devices_.find(hidx), devices_.end());
  }else if(event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED){
    const char *devname = nullptr;
    for(auto it = USBProducts.begin() ; it != USBProducts.end() ; ++it){
      if(it->vendorID == desc.idVendor){
        if(it->productID == desc.idProduct){
          devname = it->name;
        }
      }
    }
    if(!devname){
      return 0; // not a supported device, exit
    }
    std::cout << "NAME: " << devname << std::endl;
    devices_.emplace(hidx, USBDev{ bus, port, addr, desc.idVendor, desc.idProduct});
  }else{
    std::cerr << "Unknown libusb message type " << event << std::endl;
  }
  return 0;
}

// libusb_hotplug_callback_fn. uptr is 'this'
static int devcallback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *uptr){
  return (static_cast<SmUSB*>(uptr))->DevCallback(ctx, dev, event);
}

};

#endif
