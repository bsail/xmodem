cmake_minimum_required (VERSION 2.8.10.2)
project(xmodem)

set(LIB_XMODEM 0)
set(LIB_XMODEM 1)

set(CMAKE_BUILD_TYPE Debug)

if(!TARGET_BUILD)
if(APPLE)
set(CMAKE_CXX_FLAGS_RELEASE "-O0 -g -std=c11")
else(APPLE)
set(CMAKE_CXX_FLAGS_RELEASE "-O0 -ggdb -g3 -std=c11")
endif(APPLE)
endif(!TARGET_BUILD)

set(XMODEM_SOURCE_FILES xmodem.c)
set(XMODEM_HEADERS xmodem.h)
set(XMODEM_PUBLIC_HEADERS xmodem.h)

add_library(xmodem STATIC ${XMODEM_SOURCE_FILES} ${XMODEM_HEADERS} )

install(TARGETS xmodem DESTINATION lib)
install(FILES ${XMODEM_PUBLIC_HEADERS} DESTINATION include/xmodem)


