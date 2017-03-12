cmake_minimum_required (VERSION 2.8.10.2)
project(xmodem)

set(LIB_XMODEM 0)
set(LIB_XMODEM 1)

if(NOT DEFINED CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE )
endif(NOT DEFINED CMAKE_BUILD_TYPE)

#if(!TARGET_BUILD)
if(APPLE)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -std=c++14")
set(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g -std=c++14")
set(CMAKE_C_FLAGS_RELEASE   "-O3 -std=c11")
set(CMAKE_C_FLAGS_DEBUG     "-O0 -g -std=c11")
else(APPLE)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG   "-O0 -ggdb -g3 -std=c++11")
set(CMAKE_C_FLAGS_RELEASE   "-O3 -std=c11")
set(CMAKE_C_FLAGS_DEBUG     "-O0 -ggdb -g3 -std=c11")
endif(APPLE)
#endif(!TARGET_BUILD)

if(DEFINED TRANSMITTER_ONLY)
set(XMODEM_SOURCE_FILES  source/xmodem.c source/xmodem_transmitter.c)
set(XMODEM_HEADERS source/xmodem.h source/xmodem_transmitter.h)
set(XMODEM_PUBLIC_HEADERS source/xmodem.h source/xmodem_transmitter.h)
endif(DEFINED TRANSMITTER_ONLY)

if((DEFINED RECEIVER_ONLY) AND (NOT DEFINED TRANSMITTER_ONLY))
set(XMODEM_SOURCE_FILES  source/xmodem.c source/xmodem_receiver.c)
set(XMODEM_HEADERS source/xmodem.h source/xmodem_receiver.h)
set(XMODEM_PUBLIC_HEADERS source/xmodem.h source/xmodem_receiver.h)
endif((DEFINED RECEIVER_ONLY) AND (NOT DEFINED TRANSMITTER_ONLY))

if(NOT DEFINED RECEIVER_ONLY AND NOT DEFINED TRANSMITTER_ONLY)
set(XMODEM_SOURCE_FILES  source/xmodem.c source/xmodem_transmitter.c source/xmodem_receiver.c)
set(XMODEM_HEADERS source/xmodem.h source/xmodem_transmitter.h source/xmodem_receiver.h)
set(XMODEM_PUBLIC_HEADERS source/xmodem.h source/xmodem_transmitter.h source/xmodem_receiver.h)
endif(NOT DEFINED RECEIVER_ONLY AND NOT DEFINED TRANSMITTER_ONLY)

add_library(xmodem STATIC ${XMODEM_SOURCE_FILES} ${XMODEM_HEADERS} )
install(TARGETS xmodem DESTINATION lib)
install(FILES ${XMODEM_PUBLIC_HEADERS} DESTINATION include/xmodem)


