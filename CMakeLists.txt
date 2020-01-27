cmake_minimum_required(VERSION 3.14)
project(spectrummaster VERSION 0.0.1
  DESCRIPTION "Console-based SDR tool"
  HOMEPAGE_URL "https://nick-black.com/dankwiki/index.php/Spectrummaster"
  LANGUAGES CXX)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)

include(GNUInstallDirs)

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)
find_package(doctest REQUIRED doctest>=2.3.6)
find_package(notcurses REQUIRED notcurses>=1.1.0)
pkg_check_modules(RTLSDR REQUIRED librtlsdr>=0.6.0)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0>=0.1.23)

# spectrummaster binary
file(GLOB SMSRCS CONFIGURE_DEPENDS src/*.cpp)
add_executable(spectrummaster ${SMSRCS})
target_compile_options(spectrummaster
  PRIVATE
    -Wall -Wextra -W -Wshadow
)
target_compile_definitions(spectrummaster
  PRIVATE
    FORTIFY_SOURCE=2
)
target_include_directories(spectrummaster
  PRIVATE
    "${LIBUSB_INCLUDE_DIRS}"
)
target_link_libraries(spectrummaster
  PRIVATE
    Threads::Threads
    notcurses notcurses++
    "${LIBUSB_LIBRARIES}"
)

# documentation
SET(MANSOURCE1 ${CMAKE_CURRENT_SOURCE_DIR}/doc/spectrummaster.1.md)
FIND_PROGRAM(PANDOC pandoc)
if(NOT PANDOC)
  message(WARNING "pandoc not found, won't generate documentation")
else()
  foreach(m ${MANSOURCE1})
    get_filename_component(me ${m} NAME_WLE)
    add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${me}
      DEPENDS ${m}
      COMMAND ${PANDOC}
      ARGS --to man --standalone ${m} > ${CMAKE_CURRENT_BINARY_DIR}/${me}
      COMMENT "Building man page ${me}"
    )
    add_custom_target(${me}.man
      ALL
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${me}
    )
  endforeach()
endif()

# unit tests
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  include(CTest)
endif()

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
    -Wall -Wextra -W -Wshadow
)
target_include_directories(spectrummaster-tester
  PRIVATE
    "${LIBUSB_INCLUDE_DIRS}"
    src
)
target_link_libraries(spectrummaster-tester
  PRIVATE
    Threads::Threads
    doctest::doctest
    notcurses notcurses++
    "${LIBUSB_LIBRARIES}"
)

enable_testing()
add_test(
  NAME spectrummaster-tester
  COMMAND spectrummaster-tester
)

# installation
install(FILES
  ${MANPAGES1}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man/man1
)
install(TARGETS spectrummaster DESTINATION bin)
