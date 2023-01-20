# NTP-GPS
Custom GPS Hardware for use with NTP Stratum 1 servers, along with system configuration documentation.

<img src="https://github.com/nigelvh/NTP-GPS/raw/main/NTP-GPS V1.0.JPG">

## Hardware
The hardware designs for my specific device. There are many GPS options for building a Stratum 1 NTP server. This is just one take on it.

## Firmware
The firmware code and binaries for the device described in the Hardware section. Built in the Arduino IDE to be maximally approachable.

## Software
Configuration guides for setting up NTPd, NTPsec, or Chrony on Ubuntu, Debian, Alma, or Rocky to acquire the PPS signals provided by the GPS and use them to discipline the local clock and act as a Stratum 1 server.
