#include <cstdlib>
#include "main.h"
#include "usb.h"

TEST_CASE("USB") {

  SUBCASE("BasicLifetime") {
    SmUSB usb_{};
  }

}
