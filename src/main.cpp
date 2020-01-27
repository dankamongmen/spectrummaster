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
  // FIXME handle CL arguments
  // We wrap the UI object in an aritifical scope to have its destructor called
  // on exit (we need to restore the terminal).
  bool failed = false;
  {
    UI ui{};
    SmUSB usb{};
    while(true){
      auto key = ui.GetKey();
      if(key == (char32_t)-1){
        failed = true;
        break;
      }
      if(key == 'q'){
        break;
      }
    }
  }
  if(failed){
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
