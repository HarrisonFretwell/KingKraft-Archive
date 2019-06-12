
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

#include "SparkFunLSM6DS3.h"
#include "Wire.h"
#include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
char host[] = "bad-mule-51.localtunnel.me";    // name address for Google (using DNS)

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiClient client;
double restX;
double restY;
double restZ;
LSM6DS3 myIMU( I2C_MODE, 0x6A );
void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  //Call .begin() to configure the IMUs
  if( myIMU.begin() != 0 )
  {
    Serial.println("Device error");
  }
  else
  {
    Serial.println("Device OK!");
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

  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  if(!hasValidID()){
    registerDevice();
  }
  Serial.print("Arduino with id: ");
  Serial.print(getDeviceId());
  Serial.println(", is initialised");

}

void httpRequest (String req){
  Serial.println("HTTP Request: ");
  Serial.print(req);
  if (client.connect(host, 80)) {
    Serial.println("Connected to server");
    client.println(req + " HTTP/1.1");
    client.print("Host: ");
    client.println("bad-mule-51.localtunnel.me");
    client.println("Connection: close");
    client.println();
  }
  else{
    Serial.println("Connection failed");
  }
}

bool isCycle(double mag){
  return(sqrt(pow(1.0 - mag,2)) > 0.4);
}
double x;
double y;
double z;
const double maxError = 0.2;
void loop() {
  x = myIMU.readFloatAccelX();
  y = myIMU.readFloatAccelY();
  z = myIMU.readFloatAccelZ();
  double mag = sqrt(pow(x,2) + pow(y,2) + pow(z,2));
  if(isCycle(mag)){
    Serial.println("Registering cycle");
    addCycle();
    delay(6000);
  }
}

// Checks if a unique ID is already stored in EEPROM
bool hasValidID() {
  for(int i = 0; i < 5; i++){
    if(EEPROM.get(i) != 255){
      return true;
    }
  }
  return false;
}

//REFACTOR, needs to work with more than 255 devices
int getDeviceId(){
  return EEPROM.get(0);
}

void addCycle(){
  Serial.println("Adding cycle");
  String id = String(getDeviceId());
  String req = "POST /add-cycle?id=" + id;
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
  EEPROM.put(0,id);
  client.stop();
  Serial.println("closing connection");
}
