Embedded XModem
===============

# Development

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

Build Application
-----------------
```bash
ant all_app
```

Build Library - HLOS
------------------
```bash
ant all_lib
```

Build & Run Unit Tests
--------------------
* It is required that the library be built first.
```bash
ant
```

Build Library - Bare Metal
-------------------------- 
TBD

# Tests
## Unit Tests
Unit testing is implemented with Google's GTest.
### Transmit State Machine
All transitions are covered via unit tests.

<img src="documentation/xmodem_transmit_fsm.png"  />

### Receive State Machine
## Integration Tests

