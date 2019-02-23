# 5Com

An open-source serial port access application for Linux and Windows written in C++ with Qt5.

## Features
- You can view the serial port output
	- In a human-readable plain text format
	- In hex mode
	- In a verbose mode showing specific information for each byte
- You can directly write a file to the serial port
- You can send custom input to the port (with escape codes support)
- You can export the data read from the port in plain text, hex or raw formats
- Support for custom baud rates (as long as the hardware also supports it)
- Support for custom continuous transmission with a fixed packet interval
- Option to view the pinout signals and manually change the RTS and DTR pins
