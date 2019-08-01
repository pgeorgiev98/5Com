# 5Com

An open-source serial port access application for Linux and Windows written in C++ with Qt5.

![Screenshot](screenshots/screenshot-2019-07-05.png?raw=true "5Com screenshot")

## Features
- You can view the serial port output
	- In a human-readable plain text format
	- In hex mode
	- In a verbose mode showing specific information for each byte
- You can directly write a file to the serial port
- You can send custom input to the port (with escape codes support)
- You can export the data read from the port in plain text, hex, raw and csv formats
- Support for custom baud rates (as long as the hardware also supports it)
- Support for custom continuous transmission with a fixed packet interval
- Support for custom sequences of data sending, waiting and RTS/DTR modifying
- Option to view the pinout signals and manually change the RTS and DTR pins
- A lot of other small convenience features

## Installing from source

### Compiling

	git clone https://github.com/pgeorgiev98/5Com
	mkdir 5Com/build
	cd 5Com/build

	# Make sure you're using the qt5 version of qmake. On some distributions
	# you may have to call `qmake -qt=5 ..` instead for example
	qmake ..

	# You may use make -jN where N is the number of threads your CPU has.
	# This should greatly speed up the compilation
	make

	# You can now check if the application works as normal
	app/5Com

### Installing

After compiling you can install it by running

	cd app
	sudo make install

