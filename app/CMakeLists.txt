cmake_minimum_required (VERSION 2.8.10.2)
project(xmodem_app)

set(APP_XMODEM 0)
set(APP_XMODEM 1)

if(NOT DEFINED CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE DEBUG)
endif(NOT DEFINED CMAKE_BUILD_TYPE)

if(APPLE)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g -std=c++11")
else(APPLE)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG   "-O0 -ggdb -g3 -std=c++11")
endif(APPLE)

set(XMODEM_APP_SOURCE_FILES main.cpp serialport.cpp)
include_directories(${CMAKE_INCLUDE_PATH} ${ASIO_INCLUDE_PATH})
link_directories(${CMAKE_LIBRARY_PATH})
add_executable(x ${XMODEM_APP_SOURCE_FILES})
link_directories(${INSTALL_DIR}/lib)

if(APPLE)
target_link_libraries(x ${INSTALL_DIR}/lib/libxmodem.a docopt  stdc++ "-framework Foundation" "-framework IOKit" )
else(APPLE)
target_link_libraries(x ${INSTALL_DIR}/lib/libxmodem.a docopt pthread)
endif(APPLE)

install(TARGETS x DESTINATION bin)


