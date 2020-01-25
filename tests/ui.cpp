#include <cstdlib>
#include "main.h"
#include "ui.h"

TEST_CASE("UI") {

  if(!getenv("TERM")){
    return;
  }

  SUBCASE("BasicLifetime") {
    UI ui_{};
  }

}
