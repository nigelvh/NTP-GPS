# System Configuration
Configuration on various systems should look roughly similar, but example steps are provided for the OS's below. We will be using the classic NTPd daemon as the reference, but will also discuss Chrony and NTPsec.

## Ubuntu 22.04 LTS
### Disable dhclient from overriding our NTP configuration
If dhclient is active or has been used on this system, it may override our NTP configuration.
```bash
sed -e "s/, ntp-servers//g" -i.backup /etc/dhcp/dhclient.conf
rm /etc/dhcp/dhclient-exit-hooks.d/ntp
rm /run/ntp.conf.dhcp
```
### Disable systemd-timesyncd
Ubuntu uses systemd-timesyncd which is a basic SNTP daemon and does not support disciplining or acting as a server for remote clients.
```bash
systemctl disable --now systemd-timesyncd
```
### Install packages
```bash
apt-get update
apt-get install pps-tools gpsd gpsd-clients ntp
```
### Tweak NTPd AppArmor Profile
The packaged apparmor profile (at the time of this writing) appears to be missing the openssl abstraction from the AppArmor profile, causing AppArmor deny messages to be found in the syslog. Comparing to the AppArmor profile from Debian and NTPsec, we just have to add one line to resolve it. However, the errors don't immediately appear to prevent NTPd from running.
```
test@test:~$ diff -c ~/usr.sbin.ntpd.orig /etc/apparmor.d/usr.sbin.ntpd
*** /home/test/usr.sbin.ntpd.orig	2023-01-14 06:55:32.058055004 +0000
--- /etc/apparmor.d/usr.sbin.ntpd	2023-01-14 06:56:01.138298034 +0000
***************
*** 16,21 ****
--- 16,22 ----
  /usr/sbin/ntpd flags=(attach_disconnected) {
    #include <abstractions/base>
    #include <abstractions/nameservice>
+   #include <abstractions/openssl>
    #include <abstractions/user-tmp>
  
    capability ipc_lock,
```
### Configure GPSd
GPSd will interface with the GPS module to parse the NMEA data for coarse time, and attach to the DCD signal as a PPS source. These data streams will be used to discipline the clock via NTPd's shared memory (SHM) interface.
```bash
curl -o /etc/default/gpsd https://raw.githubusercontent.com/nigelvh/NTP-GPS/main/Software/Files/gpsd.conf
systemctl enable gpsd
systemctl restart gpsd
```
### Verify GPSd is recieving the NMEA data and PPS signal
Use the utility `gpsmon` and look for NMEA sentences scrolling, as well as marks for the PPS offset every second. You might also use this opportunity to check the number of satellites being tracked.
<img src="https://github.com/nigelvh/NTP-GPS/raw/main/Software/gpsd_screenshot.jpg" width="484" height="717">
### Configure NTPd
NTPd will get the coarse time and PPS data via SHMs from GPSd. We'll need to include that in the NTPd configuration. This configuration otherwise is very close to defaults.
```bash
curl -o /etc/ntp.conf https://raw.githubusercontent.com/nigelvh/NTP-GPS/main/Software/Files/ntpd.conf
systemctl enable ntp
systemctl restart ntp
```
### Verify NTPd is getting the data
Query the NTP daemon to see what peers it is using. Look for the '*' next to the SHM(1) PPS peer
```
test@test:~$ ntpq -c lpeers
     remote           refid      st t when poll reach   delay   offset  jitter
==============================================================================
 0.pool.ntp.org  .POOL.          16 p    -   64    0    0.000   +0.000   0.000
 1.pool.ntp.org  .POOL.          16 p    -   64    0    0.000   +0.000   0.000
*SHM(1)          .PPS.            0 l   40   64  377    0.000   +0.001   0.006
 SHM(0)          .GPS.           15 l   39   64  377    0.000  -242.60   0.727
-vps-7d02b399.vp 131.188.3.220    2 u   59  128  377  142.990   +0.647   1.257
+155.248.196.28  132.163.97.3     2 u   24   64  377   21.639   +1.285   0.580
+172.98.15.136   69.89.207.99     2 u   42   64  377   46.512   +0.900   0.223
-gotoro.hojmark. 97.183.206.88    2 u   48   64  377   58.146   +2.697   3.304
-clock.isc.org   5.200.6.34       3 u   35  128  377   21.762   -3.684   1.021
-66.85.78.80     172.16.23.153    2 u   53  128  377   50.980   -1.350   1.311
```
### Check that NTPd is reachable
Use `ntpdate -d <server_ip>` or configure NTPd on another host to use your new server, and verify that it is reachable. You're done!
