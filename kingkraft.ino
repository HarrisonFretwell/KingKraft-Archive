
/*
Web client

This sketch connects to a website (http://www.google.com)
using the WiFi module.

This example is written for a network using WPA encryption. For
WEP or WPA, change the Wifi.begin() call accordingly.

This example is written for a network using WPA encryption. For
WEP or WPA, change the Wifi.begin() call accordingly.

Circuit:
* Board with NINA module (Arduino MKR WiFi 1010, MKR VIDOR 4000 and UNO WiFi Rev.2)

created 13 July 2010
by dlf (Metodo2 srl)
modified 31 May 2012
by Tom Igoe
*/


#include <SPI.h>
#include <WiFiNINA.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

#include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
String host = "wise-ladybug-16.localtunnel.me";    // name address for Google (using DNS)

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiClient client;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("Connected to wifi");
  printWifiStatus();

  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  if(!hasValidID()){
    registerDevice();
  }
  Serial.print("Arduino with id: ");
  Serial.print(getDeviceId());
  Serial.println(", is initialised");
  addCycle();

}
void httpRequest (String req){
  Serial.println("HTTP Request: ");
  Serial.print(req);
  if (client.connect("wise-ladybug-16.localtunnel.me", 80)) {
    Serial.println("Connected to server");
    client.println(req + " HTTP/1.1");
    client.println("Host: wise-ladybug-16.localtunnel.me");
    client.println("Connection: close");
    client.println();
  }
  else{
    Serial.println("Connection failed");
  }
}
void loop() {
  // if there are incoming bytes available
  // from the server, read them and print them:
  /*String json;
  bool readingJSON = false;
  while (client.available()) {
  char c = client.read();
  if(c == '{'){
  readingJSON = !readingJSON;
}
if(readingJSON){
json = json + c;
}
if(c == '}'){
readingJSON = !readingJSON;
}
Serial.print(c);
}*/

// if the server's disconnected, stop the client:
if (!client.connected()) {
  /*  DynamicJsonDocument doc(24);
  deserializeJson(doc, json);
  Serial.print("Id is ");
  int id = doc["id"];
  Serial.print(id);*/
  Serial.println();
  Serial.println("disconnecting from server.");
  client.stop();

  // do nothing forevermore:
  while (true);
}
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

// Checks if a unique ID is already stored in EEPROM
bool hasValidID() {
  for(int i = 0; i < 5; i++){
    if(EEPROM.read(i) != 255){
      return true;
    }
  }
  return false;
}

//REFACTOR, needs to work with more than 255 devices
int getDeviceId(){
  return EEPROM.read(0);
}

void addCycle(){
  Serial.println("Addng cycle");
  int id = getDeviceId();
  String req = "POST /add-cycle?id=40";
  httpRequest(req);
}

//Registers device with server
bool registerDevice(){
  httpRequest("GET /register-device");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 25000) { //Try to fetch response for 25 seconds
      Serial.println(">>> Client Timeout !");
      client.stop();
      return false;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  String json;
  while(client.available()){
    bool readingJSON = false;
    while (client.available()) {
      char c = client.read();
      if(c == '{'){
        readingJSON = !readingJSON;
      }
      if(readingJSON){
        json = json + c;
      }
      if(c == '}'){
        readingJSON = !readingJSON;
      }
      Serial.print(c);
    }
    //Convert to json format and extract id
    DynamicJsonDocument doc(24);
    deserializeJson(doc, json);
    int id = doc["id"];

    //write id to EEPROM
    EEPROM.write(0,id);
    Serial.print("Written to EEPROM at address 0: ");
    int eRead = EEPROM.read(0);
    Serial.print(eRead);

    Serial.println("closing connection");
  }
}
