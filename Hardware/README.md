# NTP GPS Hardware
This is a custom piece of hardware for use with stratum 1 NTP servers to provide a simple and flexible way of getting timing data into the NTP daemon of choice.

This is not a wholly unique device, there are other devices that perform similar functions or some subset thereof. It is certainly not the only way to accomplish the task. It was designed as a means to address the most common shortfalls I have seen and for the enjoyment of making it.

This device uses a good quality GPS receiver with precise Pulse Per Second (PPS) output, and exposes both the NMEA data, as well as the PPS signal on the DCD serial line over both USB (less precise timing) as well as RS232 (more precise timing) simultaneously. This is done to provide maximum flexibility to the user's use case. If a hardware serial port is available, use that as it will provide more precise timing. If only USB is available, that is also functional.

Many inexpensive off the shelf GPS receivers do not expose the PPS signals properly on the DCD serial line making precise timing difficult to capture, or expensive time servers are available which are double to many times the cost of this device. This device was designed to fit the middle ground of a quality time source for a use case where an existing server is available, and can be configured per the instructions in the Software folder of this repository.

The schematic and board files have been created in Eagle, though I cannot guarantee how they will load without all the the associated libraries. There are additionally exported images of the schematic and board designs available for reference without using the Eagle software package.

A parts text file has been included to cover some of the more important, or more specific parts for the design. It is not all inclusive (I didn't include the passives for example), but it covers the main components, as well as some useful accessories (cables, antenna, etc).

## Design Choices
### ESP32
The ESP32 was chosen as the microcontroller for this project as it is able to handle three simultaneous hardware UARTs, is reasonably inexpensive, and is easily programmed with the Arduino IDE. In some measures it is overkill for this application, but being inexpensive and easily programmable make it more approachable to more people.

### uBlox MAX-M8 GPS
The uBlox MAX-M8 GPS device was chosen for being a quality GPS chipset, that I am familiar with, and has good timing parameters. The MAX series is not the specifically timing oriented line, however it is more than accurate enough for NTP requirements. It is able to track multiple constellations and has good RF sensitivity. I have fitted it with the associated power circuitry to supply an active GPS antenna which is essential for longer runs of cable, which is likely to be required to span between a rooftop antenna placement and a server located inside a building/datacenter.

### FTDI UART and ST232 Serial Driver
There are microcontroller choices available that could do USB natively, as well as there are other RS232 driver chipsets that may be cheaper. These were chosen for being commonly available, widely used and proven, as well as having the required inputs/outputs to handle the PPS signal over the DCD line, which not all devices support.