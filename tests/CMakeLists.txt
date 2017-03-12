cmake_minimum_required (VERSION 2.8.10.2)
project(xmodem_test)

set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_EXE_LINKER_FLAGS "-static")

if(APPLE)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g")
set(CMAKE_C_FLAGS_RELEASE   "-O3 -g")
set(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g")
#set(CMAKE_C_FLAGS_DEBUG     "-O0 -g -L${INSTALL_LIBRARY_DIR}/libmbedcrypto.a")
else(APPLE)
set(CMAKE_C_FLAGS_RELEASE   "-O3 -ggdb -g3")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -ggdb -g3")
set(CMAKE_C_FLAGS_DEBUG     "-O0 -ggdb -g3")
set(CMAKE_CXX_FLAGS_DEBUG   "-O0 -ggdb -g3")


endif(APPLE)

set(XMODEM_TEST_SOURCES test.cpp XModemTests.cpp)


if(XMODEM_TEST)
include_directories(SYSTEM ${INSTALL_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}  ${INCLUDE_DIR} )
else(XMODEM_TEST)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}  ${INCLUDE_DIR} ${INSTALL_DIR}/include )
endif(XMODEM_TEST)

add_executable(xmodem_test ${XMODEM_TEST_SOURCES} ${XMODEM_TEST_HEADERS})

#find_library(MBEDTLS_LIBRARY mbedcrypto ${INSTALL_LIBRARY_DIR})
link_directories(${INSTALL_DIR}/lib)

if (APPLE)
target_link_libraries(xmodem_test ${INSTALL_DIR}/lib/libxmodem.a ${INSTALL_DIR}/lib/libgtest.a pthread )
else(APPLE)
target_link_libraries(xmodem_test ${INSTALL_DIR}/lib/libxmodem.a ${INSTALL_DIR}/lib/libgtest.a pthread rt)
endif(APPLE)

install(TARGETS xmodem_test DESTINATION bin)

