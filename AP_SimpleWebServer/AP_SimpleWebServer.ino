/*
  WiFi Web Server LED Blink

  A simple web server that lets you blink an LED via the web.
  This sketch will create a new access point (with no password).
  It will then launch a new server and print out the IP address
  to the Serial Monitor. From there, you can open that address in a web browser
  to turn on and off the LED on pin 13.

  If the IP address of your board is yourAddress:
    http://yourAddress/H turns the LED on
    http://yourAddress/L turns it off

  created 25 Nov 2012
  by Tom Igoe
  adapted to WiFi AP by Adafruit
 */

#include <WiFi.h>
#include "arduino_secrets.h" 
#include <ArduinoJson.h>

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

int status = WL_IDLE_STATUS;
WiFiServer server(80);

typedef struct {
    int standard;
    int handicap;
    int echarge;
} ParkingStalls;

ParkingStalls floors[2];

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("Access Point Web Server");

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  WiFi.config(IPAddress(10, 0, 0, 1));

  // print the network name (SSID);
  Serial.print("Creating access point named: ");
  Serial.println(ssid);

  // Create open network. Change this line if you want to create an open network:
  status = WiFi.beginAP(ssid, pass);
  if (status != WL_AP_LISTENING) {
    Serial.println("Creating access point failed");
    // don't continue
    while (true);
  }

  // start the web server on port 80
  server.begin();

  // you're connected now, so print out the status
  printWiFiStatus();

  // Initialize floors
  floors[0].standard = 1;
  floors[0].handicap = 1;
  floors[0].echarge = 1;

  floors[1].standard = 1;
  floors[1].handicap = 1;
  floors[1].echarge = 1;

  // Print floor status
  printFloorStatus();
}


void loop() {
  // compare the previous status to the current status
  if (status != WiFi.status()) {
    // it has changed update the variable
    status = WiFi.status();

    if (status == WL_AP_CONNECTED) {
      // a device has connected to the AP
      Serial.println("Device connected to AP");
    } else {
      // a device has disconnected from the AP, and we are back in listening mode
      Serial.println("Device disconnected from AP");
    }
  }
  
  WiFiClient client = server.accept();   // listen for incoming clients

  if (client) {   
    String lastLine = "";
                                            // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      delayMicroseconds(10);                // This is required for the Arduino Nano RP2040 Connect - otherwise it will loop so fast that SPI will never be served.
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out to the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          }
          else {      // if you got a newline, then clear currentLine:
            lastLine = currentLine;
            currentLine = "";
          }
        }
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disconnected\nLast line: ");
    Serial.println(lastLine);

    handleJson(lastLine);
    printFloorStatus();
  }
}

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

}

void printFloorStatus() {
  for (int i = 0; i < 2; i++) {
      Serial.print("Parking Lot ");
      Serial.println(i);

      Serial.print("Available Car Stalls ");
      Serial.println(floors[i].standard);

      Serial.print("Available Handicap Stalls ");
      Serial.println(floors[i].handicap);

      Serial.print("Available E-Charge Stalls ");
      Serial.println(floors[i].echarge);

      Serial.println();
  }
}

void handleJson(String json) {
  JsonDocument doc;
  deserializeJson(doc, json);
 
  int floor_id = doc["floor_id"];
  int stall_type = doc["stall_type"];
  int counter = doc["counter"];

  Serial.print("floor_id: ");
  Serial.println(floor_id);
  Serial.print("stall_type: ");
  Serial.println(stall_type);
  Serial.print("counter: ");
  Serial.println(counter);

  switch (stall_type) {
    case 0:
      floors[floor_id].standard = counter;
      break;
    case 1:
      floors[floor_id].handicap = counter;
      break;
    case 2:
      floors[floor_id].echarge = counter;
      break;
    default:
      Serial.println("Bad floor_id");
  }
}
