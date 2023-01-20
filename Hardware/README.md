# NTP GPS Hardware
This is a custom piece of hardware for use with stratum 1 NTP servers to provide a simple and flexible way of getting timing data into the NTP daemon of choice.

This is not a wholly unique device, there are other devices that perform similar functions or some subset thereof. It is certainly not the only way to accomplish the task. It was designed as a means to address the most common shortfalls I have seen and for the enjoyment of making it.

This device uses a good quality GPS receiver with precise Pulse Per Second (PPS) output, and exposes both the NMEA data, as well as the PPS signal on the DCD serial line over both USB (less precise timing) as well as RS232 (more precise timing) simultaneously. This is done to provide maximum flexibility to the user's use case. If a hardware serial port is available, use that as it will provide more precise timing. If only USB is available, that is also functional.

Many inexpensive off the shelf GPS receivers do not expose the PPS signals properly on the DCD serial line making precise timing difficult to capture, or expensive time servers are available which are double to many times the cost of this device. This device was designed to fit the middle ground of a quality time source for a use case where an existing server is available, and can be configured per the instructions in the Software folder of this repository.

The schematic and board files have been created in Eagle, though I cannot guarantee how they will load without all the the associated libraries. There are additionally exported images of the schematic and board designs available for reference without using the Eagle software package.

A parts text file has been included to cover some of the more important, or more specific parts for the design. It is not all inclusive (I didn't include the passives for example), but it covers the main components, as well as some useful accessories (cables, antenna, etc).