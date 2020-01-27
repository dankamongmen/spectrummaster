#include <clocale>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include "usb.h"
#include "ui.h"

int main(void){
  if(!setlocale(LC_ALL, "")){
    std::cerr << "Couldn't set locale\n";
    return EXIT_FAILURE;
  }
  // We wrap the UI object in an aritifical scope to have its destructor called
  // on exit (we need to restore the terminal).
  {
    UI ui{};
    SmUSB usb{};
    auto key = ui.GetKey();
  }
  return EXIT_SUCCESS;
}
