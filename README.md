Embedded XModem
===============

# Integrator Guide
For integrators looking to build just libXMODEM with dependencies met by integrators build environment.

Setup Environment - Ubuntu 16.10
--------------------------------
```bash
sudo apt-get install build-essential socat lrzsz minicom ant git libgtest-dev cmake
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
For developers looking to extend, bug fix, build, and test libXMODEM with dependencies and test infrastructure included in the source tree.

Setup Environment - OSX
------------------------
* Follow instructions to install git-repo http://source.android.com/source/downloading.html#installing-repo
```bash
brew install ant git Caskroom/versions/java7 socat lrzsz minicom
```

Setup Environment - Ubuntu 16.10
---------------------------------
```bash
sudo apt-get install build-essential socat lrzsz minicom ant git
```

Features
--------
* small memory and storage footprint

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

Unit testing is implemented with Google's GTest.

### Transmit State Machine
All transitions are covered via unit tests.

<img src="documentation/xmodem_transmit_fsm.png"  />

### Receive State Machine
All transitions are covered via unit tests.

<img src="documentation/xmodem_receive_fsm.png"  />

## High-level Tests
Open a terminal
```bash
ant tests_system
```

