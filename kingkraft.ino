
/*
Web wifi

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
#include <utility/wifi_drv.h>
#include "SparkFunLSM6DS3.h"
#include <HttpClient.h>
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
char host[] = "wet-pig-87.localtunnel.me";    // name address for Google (using DNS)

// Initialize the Ethernet wifi library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiClient wifi;
HttpClient client = HttpClient(wifi, host, 80);
LSM6DS3 myIMU( I2C_MODE, 0x6A );
//DO more with this later
LSM6DS3 myIMU2(SPI_MODE, SPIIMU_SS);
void resetEEPROM(){
  for(int i = 0; i < 100; i++){
    EEPROM.update(i,255);
  }
}
void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  WiFiDrv::pinMode(25, OUTPUT);  //GREEN
  WiFiDrv::pinMode(26, OUTPUT);  //RED
  WiFiDrv::pinMode(27, OUTPUT);  //BLUE
  WiFiDrv::analogWrite(25, 128);  // for configurable brightness
  WiFiDrv::analogWrite(26, 128);  // for configurable brightness
  WiFiDrv::analogWrite(27, 128);  // for configurable brightness

  int const sensorMax = 1;
  bool sensors[sensorMax];

  //Accelerometer, id of 1
  if( myIMU.begin() != 0 )
  {
    Serial.println("Accel error");
    sensors[1] = false;
  }
  else
  {
    Serial.println("Accel OK!");
    //Accel is enabled
    sensors[1] = true;
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
  Serial.print(getSensorId());
  Serial.println(", is initialised");

}
void LED(int r, int g, int b){
  WiFiDrv::analogWrite(25, r);  // green
  WiFiDrv::analogWrite(26, g);  // red
  WiFiDrv::analogWrite(27, b);  // blue
}
// this method makes a HTTP connection to the server:
void httpRequest(String req) {
  // close any connection before send a new request.
  // This will free the socket on the Nina module
  wifi.stop();
  LED(128,128,0); //Yellow for trying
  // if there's a successful connection:
  if (wifi.connect(host, 80)) {
    Serial.println("connecting...");
    // send the HTTP PUT request:
    LED(0,128,0); //green for success
    wifi.print(req);
    wifi.println(" HTTP/1.1");
    wifi.print("Host: ");
    wifi.println(host);
    wifi.println("User-Agent: ArduinoWiFi/1.1");
    wifi.println("Connection: close");
    wifi.println();

    // note the time that the connection was made:
  } else {
    // if you couldn't make a connection:
    LED(128,0,0); //Red for failure
    Serial.println("connection failed");
  }
}

void jsonPost(String post){
  LED(128,128,0);
  client.beginRequest();
  client.post("/input");
  client.sendHeader("Content-Type", "application/json");
  client.sendHeader("Content-Length", post.length());
  client.beginBody();
  client.print(post);
  client.endRequest();
  LED(0,128,0);
}
//Given magnitude of acceleration, returns whether this counts as a cycle
bool isCycle(double mag){
  return(sqrt(pow(1.0 - mag,2)) > 0.4);
}

double x;
double y;
double z;
void loop() {
  x = myIMU.readFloatAccelX();
  y = myIMU.readFloatAccelY();
  z = myIMU.readFloatAccelZ();
  double mag = sqrt(pow(x,2) + pow(y,2) + pow(z,2));
  if(isCycle(mag)){
    Serial.println("Registering cycle");
    addCycle();
    delay(10000);
    LED(0,0,128); //Resting state
  }
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

//Gets sensor id
int getSensorId(){
  int id;
  EEPROM.get(0,id);
  return id;
}


//Lots of code here thats wildly unneccesary right now, going to keep it
//for future Harrison to sift useful things from
/*void addCycle(){
  Serial.println("Adding cycle");
  String id = String(getSensorId());
  DynamicJsonDocument doc(100);
  //Set sensor id
  doc["id"] = id;
  //Now create array
  int inputLength = 1;
  String jsonArray[inputLength];
  for(int i = 0; i < inputLength; i++){
    jsonArray[i] = "{\"input_id\": 1, \"value\": 1}";
  }
  String jsonArrayS = arrayToString(jsonArray,inputLength);
  //String req = "GET /";
  jsonPost("{\"id\": "+id+", \"inputs\": ["+jsonArrayS+"]}");
}*/

void addCycle(){
  Serial.println("Adding cycle");
  String id = String(getSensorId());
  DynamicJsonDocument doc(64);
  //Set sensor id
  doc["id"] = id;
  jsonPost("{\"id\": "+id+", \"inputs\": [{\"input_id\": 1, \"value\": 1}]}");
}

String arrayToString(String array[],int length){
  String string;
  for(int i = 0; i < length; i++){
    string = string + array[i];
  }
  return string;
}

//Registers device with server
bool registerDevice(){
  httpRequest("GET /register-device");
  unsigned long timeout = millis();
  while (wifi.available() == 0) {
    if (millis() - timeout > 25000) { //Try to fetch response for 25 seconds
      Serial.println(">>> Client Timeout !");
      return false;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  String json;
  bool readingJSON = false;
  while (wifi.available()) {
    char c = wifi.read();
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
  wifi.stop();
  Serial.println("closing connection");
}
