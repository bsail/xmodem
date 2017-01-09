Embedded XModem
===============

Setup Environment - OSX
-----------------
* Follow instructions to install git-repo http://source.android.com/source/downloading.html#installing-repo
```bash
brew install ant git Caskroom/versions/java7 socat lrzsz minicom
```
Features
--------
* small memory and storage footprint

Build Application
-----------------
mkdir xmodem<br/>
cd xmodem<br/>
repo init -m xmodem.xml -u ssh://git-codecommit.us-east-1.amazonaws.com/v1/repos/mutex.io.xmodem<br/>

Build Library - HLOS
------------------

Build Library - Bare Metal
-------------------------- 


