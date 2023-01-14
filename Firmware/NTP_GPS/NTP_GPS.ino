#include <TinyGPSPlus.h>
#include <TinyGPS++.h>

String SW_VERS = "1.00";
String SW_VERS_NAME = "\n\n\n\nK7NVH NTP GPS - SW Vers " + SW_VERS + "\n\n\n\n";

#define LED_GPS_1 33
#define LED_GPS_2 25
#define LED_GPS_3 26
#define LED_GPS_4 27
#define LED_GPS_5 14

#define LED_STAT 32

#define SER_GPS_TX 17
#define SER_GPS_RX 16
#define SER_USB_TX 1
#define SER_USB_RX 3
#define SER_232_TX 13
#define SER_232_RX 12

#define PPS_INPUT 35

TinyGPSPlus gps;
float f_lat, f_lon;
long alt;
byte hour, minute, second, num_sats, fix_type;

byte flag_gps_output_usb = 1;
byte flag_gps_output_232 = 1;
volatile byte flag_pps_isr = 0;

unsigned long timestamp_last_pps = 0;
unsigned long pps_lock_time = 0;
unsigned long pps_lock_rsts = 0;

byte gps_read_byte = 0;

void IRAM_ATTR pps_isr() {
  unsigned long time_now = millis();
  if(timestamp_last_pps != 0) { // Don't throw an error if we just started up

    // Catch if the PPS is outside of the expected range
    if((time_now - timestamp_last_pps < 900) || (millis() - timestamp_last_pps > 1100)) {
      // ALERT - Got PPS signal at unexpected time! 
      //Serial.print("Time difference: ");
      //Serial.print(time_now - timestamp_last_pps);
      //Serial.println("ms.");
      
      timestamp_last_pps = time_now; // Mark this as the latest timestamp for a PPS pulse for next time around
      pps_lock_time = 0; // Reset the seconds counter for how long we've recieved a continuous valid train of PPS pulses
      pps_lock_rsts++; // Increment the lock resets counter
      return;
    }

    timestamp_last_pps = time_now; // Mark this as the latest timestamp for a PPS pulse for next time around
    pps_lock_time++; // Increment the seconds counter for how long we've recieved a continuous valid train of PPS pulses
  } else { // We've just started up, just set the timestamp for this PPS pulse
    timestamp_last_pps = time_now;
  }

  flag_pps_isr = 1;
}

// Startup setup items
void setup() {
  // Configure the serial ports
  Serial.begin(9600, SERIAL_8N1, SER_USB_RX, SER_USB_TX); // The USB Serial Port
  Serial1.begin(9600, SERIAL_8N1, SER_232_RX, SER_232_TX); // The RS232 Serial Port
  Serial2.begin(9600, SERIAL_8N1, SER_GPS_RX, SER_GPS_TX); // The TTL Serial Port to the GPS Module

  // Configure the GPS status LEDs
  pinMode(LED_GPS_1, OUTPUT); // The leftmost
  pinMode(LED_GPS_2, OUTPUT);
  pinMode(LED_GPS_3, OUTPUT);
  pinMode(LED_GPS_4, OUTPUT);
  pinMode(LED_GPS_5, OUTPUT); // The rightmost

  // Configure the status LED
  pinMode(LED_STAT, OUTPUT);

  // Attach an interrupt to the PPS input pin
  attachInterrupt(PPS_INPUT, pps_isr, RISING);

  delay(1000);
  Serial.println(SW_VERS_NAME);
  Serial1.println(SW_VERS_NAME);
  delay(2000);
  while(Serial2.available()) Serial2.read(); // Clear out any garbage from the input buffer from the GPS
}

// Main loop
void loop() {
  // Read from the GPS and output to the USB and RS232 ports (if enabled)
  if(Serial2.available()) {
    gps_read_byte = Serial2.read();
    if(flag_gps_output_usb) Serial.write(gps_read_byte);
    if(flag_gps_output_232) Serial1.write(gps_read_byte);

    gps.encode(gps_read_byte);
  }

  // Run some stuff every second when we get the PPS interrupt
  if (flag_pps_isr) {
    flag_pps_isr = 0;

    // Send our custom strings. The PPS signal should come through roughly ~100ms before the GPS NMEA data comes
    // But make sure the last byte we got from the GPS was the newline, so we're not dumping our string in the middle of another
    if (gps_read_byte == 10) send_custom_nmea();

    set_gps_leds((byte)gps.satellites.value());
    digitalWrite(LED_STAT, !digitalRead(LED_STAT));
  }

  // Read from the USB and RS232 ports and maybe do something
}

// Update the GPS LEDs based on the number of satellites we have
void set_gps_leds(byte num_sats) {
  if (num_sats > 11){ digitalWrite(LED_GPS_5, HIGH); }else{ digitalWrite(LED_GPS_5, LOW); }
  if (num_sats > 9){ digitalWrite(LED_GPS_4, HIGH); }else{ digitalWrite(LED_GPS_4, LOW); }
  if (num_sats > 7){ digitalWrite(LED_GPS_3, HIGH); }else{ digitalWrite(LED_GPS_3, LOW); }
  if (num_sats > 5){ digitalWrite(LED_GPS_2, HIGH); }else{ digitalWrite(LED_GPS_2, LOW); }
  if (num_sats > 3){ digitalWrite(LED_GPS_1, HIGH); }else{ digitalWrite(LED_GPS_1, LOW); }
}

// Send custom NMEA string(s)
void send_custom_nmea() {
  String str_start = "$";
  String str_stop = "*";
  String custom = "NVDBG," + String(pps_lock_time, DEC) + "," + String(pps_lock_rsts, DEC) + "," + SW_VERS;
  byte checksum = 0;

  // Calculate the string's checksum
  for (uint8_t i = 0; i < custom.length(); i++) {
    checksum = checksum ^ custom[i];
  }
  String str_checksum = String(checksum, HEX);
  str_checksum.toUpperCase();

  String str_final = str_start + custom + str_stop + str_checksum;
  
  Serial.println(str_final);
  Serial1.println(str_final);
}
