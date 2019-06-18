
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
#include "SparkFunLSM6DS3.h"
#include <Ultrasonic.h>
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
char server[] = "kingkraft.herokuapp.com";    // name address for Google (using DNS)

// Initialize the Ethernet wifi library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiClient client;
LSM6DS3 myIMU( I2C_MODE, 0x6A );
//DO more with this later
LSM6DS3 myIMU2(SPI_MODE, SPIIMU_SS);
unsigned long lastReading = 0;            // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 5L * 1000L; // delay between updates, in milliseconds
// defines pins numbers
const int trigPin = 9;
const int echoPin = 10;
// defines variables
long duration;
int distance;

void resetEEPROM(){
  for(int i = 0; i < 100; i++){
    EEPROM.update(i,255);
  }
}
double Round(double x, int p)
{
  if (x != 0.0) {
    return ((floor((fabs(x)*pow(double(10.0),p))+0.5))/pow(double(10.0),p))*(x/fabs(x));
  } else {
    return 0.0;
  }
}
double getUS(){
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  return duration*0.034/2;
}
void setup() {
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

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
// this method makes a HTTP connection to the server:
void httpRequest(String req) {
  // close any connection before send a new request.
  // This will free the socket on the Nina module
  client.stop();
  // if there's a successful connection:
  if (client.connect(server, 80)) {
    Serial.println("connecting...");
    // send the HTTP PUT request:
    client.print(req);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(server);
    client.println("User-Agent: Arduinowifi/1.1");
    client.println("Connection: close");
    client.println();

    // note the time that the connection was made:
  } else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}
//Given magnitude of acceleration, returns whether this counts as a cycle
bool isCycle(double mag){
  return(abs(1.0 - mag) > 0.01);
}
double getMag(){
  double x = myIMU.readFloatAccelX();
  double y = myIMU.readFloatAccelY();
  double z = myIMU.readFloatAccelZ();
  return sqrt(pow(x,2) + pow(y,2) + pow(z,2));
}
//Main program loop
struct magUS {
  double mag;
  double us;
};
void postHttp(String data) {
  // close any connection before send a new request.
  // This will free the socket on the Nina module
  client.stop();
  // if there's a successful connection:
  if (client.connect(server, 80)) {
    Serial.println("connecting...");
    // send the HTTP PUT request:
    client.println("POST /input HTTP/1.1");
    client.print("Host:");
    client.println(server);
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Content-type: application/json");
    client.print("Content-length: ");
    client.println(data.length());
    client.println("Connection: close");
    client.println();
    client.println(data);

  } else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}
//Max recording in seconds
const int maxRecordingTime = 5;
magUS data[maxRecordingTime * 10];
magUS dataNode = {};
StaticJsonDocument<1000> doc;
int lastCycle = 0;
void loop() {
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }
  // if ten seconds have passed since your last connection,
  // then connect again and send data:
  if(((millis() - lastReading) > postingInterval) && isCycle(getMag())){
    Serial.println("Mag exceeded");
    unsigned long startTime = millis();
    bool moving = true;
    int i = 0;
    int lastTime = 0;
    while(moving && i < 15){
      //take readings every 0.1 seconds
      if((millis() - lastTime) > 500){
        //Get sensor values
        dataNode.mag = getMag();
        dataNode.us = getUS();
        //If still moving, add to array
        if(i == 0){
          moving = true;
        }
        else{
          moving = fabs((dataNode.us - data[i-1].us) > 0.1);
        }
        if(moving){
          data[i] = dataNode;
          i++;
        }
        lastTime = millis();
      }
    }
    if(i > 1){
      lastReading = millis();
      //Create JSON array
      JsonArray jsonArray = doc.createNestedArray("inputs");
      doc["id"] = getSensorId();
      //definitely needs refactoring
      for(int j = 0; j < i; j++){
        JsonObject input = jsonArray.createNestedObject();
        input["input_id"] = 1;
        input["value"] = Round(data[j].mag,2);
        input = jsonArray.createNestedObject();
        input["input_id"] = 2;
        input["value"] = Round(data[j].us,2);
      }


      String data = "";
      serializeJson(doc, data);
      Serial.println(data);
      postHttp(data);
      doc.clear();
      client.stop();
      /*
      client.post("/input", "application/json", postData.c_str());
      Serial.println("Gone");
      doc.clear();
      lastCycle = millis();


    }
    else{
    Serial.println("Nah");
  }*/
}
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

//Registers device with server
bool registerDevice(){
  httpRequest("GET /register-device");

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
