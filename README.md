Embedded XModem
===============

# Integrator Guide
For integrators looking to build just libXMODEM with dependencies met by integrators build environment.

Setup Environment - Ubuntu 16.10
--------------------------------
```bash
sudo apt-get install build-essential socat lrzsz minicom ant openjdk-8-jdk git libgtest-dev cmake repo 
```

Build and install libXMODEM
--------------------------------
```bash
git clone git@github.com:caseykelso/xmodem.git
cd xmodem
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../installdirectory
make install -j8
```

# Development
For developers looking to extend, bug fix, build, and test libXMODEM with dependencies and test infrastructure included in the source tree. The complete source tree grabs third party repos, standalone command line application code, serial port lib, and gtest.

Setup Environment - OSX
------------------------
* Follow instructions to install git-repo http://source.android.com/source/downloading.html#installing-repo
```bash
brew install ant git Caskroom/versions/java7 socat lrzsz minicom
```

Setup Environment - Ubuntu 16.10
---------------------------------
```bash
sudo apt-get install build-essential socat lrzsz minicom ant git repo openjdk-8-jdk
```


Get Code
-----------------
```bash
mkdir xmodem
cd xmodem
repo init -m xmodem.xml -u git://github.com/caseykelso/xmodem.git
repo sync
```

Build Application - build all dependencies
-----------------
```bash
ant all_app
```

Build Library - for HLOS targets - build all dependencies
------------------
```bash
ant all_lib
```

Build Application Only (development iterations)
------------------
```bash
ant app
```

or

```bash
cd build.app
make
```

Build Library - Bare Metal
--------------------------
TBD

# Tests
## Unit Tests

Build & Run Unit Tests
--------------------
* It is required that the library be built first.
```bash
ant tests
```

## High-level Tests
Open a terminal
```bash
ant tests_system
```

### Transmit State Machine
All transitions are covered via unit tests.

<img src="documentation/xmodem_transmit_fsm.png"  />

### Receive State Machine
All transitions are NOT YET covered via unit tests.

<img src="documentation/xmodem_receive_fsm.png"  />


# Project Structure
## Source Directories
* /source - libxmodem source directory
  * xmodem.h - common header (transmitter and receiver)
  * xmodem.c - common implementation (transmitter and receiver)
  * xmodem_receiver.h - receiver side header
  * xmodem_receiver.c - receiver side implementation
  * xmodem_transmitter.h - transmitter side header
  * xmodem_transmitter.c - transmitter side implementation


* /app    - application level source directory
  * CMakeLists.txt - cmake build configuration for the commandline app
  * main.cpp - main application source code file
  * serialport.cpp - thin abstraction for asio serialport
  * serialport.h - header for thin abstraction for asio serialport


* /tests
  * CMakeLists.txt - cmake build configuration for the test runner
  * test.cpp - libxmodem test implementation
  * XModemTests.cpp - gtest test suite (implementation is in header)
  * XModemTests.h - gtest test suite definition and SetUp() TearDown() implementation


* /tests.tmp - temporary folder for test files and virtual serial port nodes


* /
  * build.xml - ant build configuration for aggregate project (only used for standalone dev purposes not integration). Allows for easy development in Linux, OSX, and Windows
  * CMakeLists.txt - libxmodem cmake build configuration, used for integrators who wish to clone just the libxmodem source tree without using git-repo for aggregation.
  * README.md - this file
  * xmodem.xml - git-repo manifest definition to aggregate all project dependencies for libxmodem development purposes
  * .gitignore - definitions of files and directories to be ignored by git


## Third Party Source Directories
* /asio    - boost asynchronous IO standalone implementation
* /cmake   - latest cmake build tool - tip of master branch
* /docopt  - library for commandline switches
* /gtest   - google test library

## Build Directories
* build - libxmodem library build directory
* build.app - application build directory
* build.docopt - libdocopt build directory
* build.tests - unit test runner build directory
* build.gtest - google test build directory

# Commandline Application Usage
```bash
nanoXmodem:

	    Usage:
	      x --port=<port> --receive  --file=<filename> [--baud=<baudrate>]
	      x --port=<port> --transmit --file=<filename> [--baud=<baudrate>]
              x --enumerate
	      x (-h | --help)
	      x --version

	    Options:
	      -h --help           Show this screen.
	      --version           Show version.
	      --port=<comport>    Path to comport.
	      --baud=<baudrate>   Baudrate [default: 115200].
	      --receive           Start transfer as receiver.
	      --transmit          Start file transmission.
              --enumerate         Enumerate list of available ports.
	      --file=<filename>   File to send.
```
