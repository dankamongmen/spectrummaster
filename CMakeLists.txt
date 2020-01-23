cmake_minimum_required(VERSION 3.14)
project(spectrummaster VERSION 0.0.1
  DESCRIPTION "Console-based SDR tool"
  HOMEPAGE_URL "https://nick-black.com/dankwiki/index.php/Spectrummaster"
  LANGUAGES C CXX)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

include(GNUInstallDirs)

find_package(doctest REQUIRED doctest>=2.3.6)

# unit tests
file(GLOB TESTSRCS CONFIGURE_DEPENDS tests/*.cpp)
add_executable(spectrummaster-tester ${TESTSRCS})
target_compile_definitions(spectrummaster-tester
  PRIVATE
    FORTIFY_SOURCE=2
)
target_compile_features(spectrummaster-tester
  PRIVATE
    cxx_std_17
)
target_compile_options(spectrummaster-tester
  PRIVATE
    -Wall -Wextra -W -Wshadow ${DEBUG_OPTIONS}
)
target_link_libraries(spectrummaster-tester
  PRIVATE
    doctest::doctest
)

enable_testing()
add_test(
  NAME spectrummaster-tester
  COMMAND spectrummaster-tester
)
