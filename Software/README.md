# System Configuration
Configuration on various systems should look roughly similar, but example steps are provided for the OS's below. We will be using the classic NTPd daemon as the reference, but will also discuss Chrony and NTPsec.

- [Ubuntu 22.04 LTS](#ubuntu-2204-lts-and-debian-11-bullseye)
- [Using Chrony](#using-chrony)
- [Using NTPsec](#using-ntpsec)

## Ubuntu 22.04 LTS and Debian 11 Bullseye
### Disable systemd-timesyncd
Ubuntu uses systemd-timesyncd which is a basic SNTP daemon and does not support disciplining or acting as a server for remote clients.
```bash
systemctl disable --now systemd-timesyncd
```
### Install packages
```bash
apt-get update
apt-get install pps-tools gpsd gpsd-clients
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
### Install NTPd
```bash
apt-get install ntp
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
### Disable dhclient from overriding our NTP configuration
If dhclient is active or has been used on this system, it may override our NTP configuration.
```bash
sed -e "s/, ntp-servers//g" -i.backup /etc/dhcp/dhclient.conf
rm /etc/dhcp/dhclient-exit-hooks.d/ntp
rm /run/ntp.conf.dhcp
```
### Configure NTPd
NTPd will get the coarse time and PPS data via SHMs from GPSd. We'll need to include that in the NTPd configuration. This configuration otherwise is very close to defaults.
```bash
curl -o /etc/ntp.conf https://raw.githubusercontent.com/nigelvh/NTP-GPS/main/Software/Files/ntpd.conf
systemctl enable ntp
systemctl restart ntp
```
### Verify NTPd is getting the data
Query the NTP daemon to see what peers it is using. Look for the '*' next to the SHM(1) PPS peer.
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

## Using Chrony
Setting up the system using Chrony will be very similar to using NTPd. Skip the configuring and verifying NTP steps above, and replace them with configuring and verifying Chrony here.

### Configure Chrony
Chrony will (like NTPd) get the coarse time and PPS data via SHMs from GPSd. We'll need to include that in the Chrony configuration. This configuration is otherwise very close to defaults.
```bash
apt-get install chrony
curl -o /etc/chrony/chrony.conf https://raw.githubusercontent.com/nigelvh/NTP-GPS/main/Software/Files/chrony.conf
systemctl enable chrony
systemctl restart chrony
```
### Verify Chrony is tracking PPS
Query the Chrony daemon to see what peers it is using. Look for the '*' next to the PPS peer.
```
test@test:~$ chronyc sources
MS Name/IP address         Stratum Poll Reach LastRx Last sample               
===============================================================================
#x GPS                          15   4   377    13   +247ms[ +247ms] +/-  200ms
#* PPS                           0   4   377    11    +69ns[ +104ns] +/-  436ns
^- ntp2.wiktel.com               1   9   377   154  +2514us[+2514us] +/-   21ms
^- li1150-42.members.linode>     2   9   377   147   +763us[ +763us] +/-   69ms
^- gosanf.hojmark.net            2   9   377    31  -1058us[-1058us] +/-   46ms
^- 44.190.40.123                 2   9   377   155   -889us[ -889us] +/-   33ms
```
We can also see more details about the server state. Such as that we're tracking the PPS reference, and operating as a Stratum 1, as well as a number of timing related parameters.
```
test@test:~$ chronyc tracking
Reference ID    : 50505300 (PPS)
Stratum         : 1
Ref time (UTC)  : Sun Jan 15 04:40:55 2023
System time     : 0.000000047 seconds slow of NTP time
Last offset     : -0.000000040 seconds
RMS offset      : 0.000000484 seconds
Frequency       : 7.842 ppm slow
Residual freq   : +0.003 ppm
Skew            : 0.023 ppm
Root delay      : 0.000000001 seconds
Root dispersion : 0.000023089 seconds
Update interval : 16.0 seconds
Leap status     : Normal
```

## Using NTPsec
NTPsec configuration will look very similar to the NTPd configuration. Skip the configuring and verifying NTP steps above, and replace them with configuring and verifying NTPsec here.

### Configure NTPsec
```bash
apt-get install NTPsec
rm /etc/dhcp/dhclient-exit-hooks.d/ntpsec 
rm /run/ntpsec/ntp.conf.dhcp
curl -o /etc/ntpsec/ntp.conf https://raw.githubusercontent.com/nigelvh/NTP-GPS/main/Software/Files/ntpsec.conf
systemctl enable ntpsec
systemctl restart ntpsec
```
### Verify NTPsec is tracking PPS
Query the NTPsec daemon to see what peers it is using. This matches the command syntax as NTPd. Look for the '*' next to the SHM(1) PPS peer.
```
test@test:~$ ntpq -c lpeers
     remote                                   refid      st t when poll reach   delay   offset   jitter
=======================================================================================================
*SHM(1)                                  .PPS.            0 l   28   64  377   0.0000  -0.0194   1.2577
 SHM(0)                                  .GPS.           15 l   27   64  377   0.0000 -243.975   2.8552
 0.pool.ntp.org                          .POOL.          16 p    -  256    0   0.0000   0.0000   0.0001
 1.pool.ntp.org                          .POOL.          16 p    -  256    0   0.0000   0.0000   0.0001
-rn5.quickhost.hk                        45.79.13.206     3 u   58   64  377  29.5838   6.9985   1.9140
-h69.41.139.40.ip.windstream.net         .GPS.            1 u  128   64   16  82.7081   5.4127   1.0102
+208.67.72.50                            17.253.2.123     2 u   63   64  377  76.8092   0.2502   1.1300
-38.17.55.196                            86.77.84.80      5 u   55   64  177  51.7627   8.4964   5.5212
+c-73-14-129-213.hsd1.co.comcast.net     .PPS.            1 u   55   64  377  42.2095  -0.5344   0.8669
-108.61.73.244                           128.59.0.245     2 u   58   64  377  60.4326   2.3897   1.4830
-50-203-248-23-static.hfc.comcastbusines 130.207.244.240  2 u  123   64    6  79.1322  -2.3248   0.1393
```