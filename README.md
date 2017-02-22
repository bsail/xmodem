Embedded XModem
===============

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
repo init -m xmodem.xml -u ssh://git-codecommit.us-east-1.amazonaws.com/v1/repos/mutex.io.xmodem
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

Build Library - Bare Metal
-------------------------- 
TBD

