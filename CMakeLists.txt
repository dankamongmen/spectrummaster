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
