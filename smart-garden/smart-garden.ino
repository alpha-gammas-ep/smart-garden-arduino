#include <SPI.h>
#include <WiFiNINA.h>
#include "Firebase_Arduino_WiFiNINA.h"
#include "smart_garden.h"
#include "secrets.h"
#include "WiFiUdp.h"

int status = WL_IDLE_STATUS;
FirebaseData firebase_data;
plant plant0;
plant plant1;

WiFiUDP Udp;
unsigned int localPort = 2390;      // local port to listen for UDP packets
IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed");
    // don't continue
    while (true);
  }
  // check for latest version firmware
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.print("Please upgrade firmware - Current Version:");
    Serial.print(fv);
    Serial.print(", Latest version: ");
    Serial.println(WIFI_FIRMWARE_LATEST_VERSION);
  }
  // connect to Wifi
  Serial.print("Attempting to connect to: ");
  Serial.println(WIFI_SSID);
  while (status != WL_CONNECTED) {
    status = WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.println(".");
    delay(50);
  }
  
  Serial.print("Connected to: ");
  Serial.println(WIFI_SSID);

  Serial.println("Starting connection to time server...");
  Udp.begin(localPort);
  
  Serial.println("Starting connection to firebase server...");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH, WIFI_SSID, WIFI_PASS);
  Firebase.reconnectWiFi(true);
}

void loop() {
  // put your main code here, to run repeatedly:
  char buf[21];
  unsigned long epoch = get_epoch();
  if (Firebase.getString(firebase_data, "plants/plant_0/last_watered")) {
    if (firebase_data.dataType() == "string")
      Serial.println(firebase_data.stringData());
  }
  else {
    //Failed, then print out the error detail
    Serial.println(firebase_data.errorReason());
  }
  if (Firebase.getString(firebase_data, "plants/plant_0/water_interval")) {
    if (firebase_data.dataType() == "string") {
      Serial.println(firebase_data.stringData());
      firebase_data.stringData().toCharArray(buf, 21);
      Serial.println(string_to_long(buf));
    }
  }
  else {
    //Failed, then print out the error detail
    Serial.println(firebase_data.errorReason());
  }
  long_to_string(buf, epoch);
  if (Firebase.pushString(firebase_data, "plants/plant_0/water_interval", String(buf))) {
    if (firebase_data.dataType() == "string")
      Serial.println(firebase_data.stringData());
  }
  else {
    //Failed, then print out the error detail
    Serial.println(firebase_data.errorReason());
  }
  while (true);
}

boolean load_plant(int num) {
  char path[100];
  int val;
  strcpy(path, "plants/plant");
  path[12] = num;
  path[13] = '\0';
  char* offset = path + 13;
  // 
  if (true) {
    
  } else {
    
  }
  return false;
}

unsigned long get_epoch() {
  sendNTPpacket(timeServer); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);
  if (Udp.parsePacket()) {
    Serial.println("packet received");
    // We've received a packet, read the data from it
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, extract the two words:
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    // Serial.print("Seconds since Jan 1 1900 = ");
    // Serial.println(secsSince1900);
    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(epoch);
    return epoch;
  }
}

unsigned long sendNTPpacket(IPAddress& address) {
  //Serial.println("1");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  //Serial.println("2");
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  //Serial.println("3");
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  //Serial.println("4")
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  //Serial.println("5")
  Udp.endPacket();
  //Serial.println("6");
}

void long_to_string(char* buf, long l) {
  int i = 0;
  while (l != 0) {
    buf[i] = (char) l % 10;
    l = l / 10;
    i++;
  }
  buf[i+1] = '\0';
}

unsigned long string_to_long(char* c) {
  int i = 0;
  unsigned long l = 0;
  while (*c != '\0') {
    l += pow(10, i) * (int) *c;
    i++;
    c++;
  }
  return l;
}
