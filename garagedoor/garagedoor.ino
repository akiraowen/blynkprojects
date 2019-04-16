/*************************************************************
  Download latest Blynk library here:
    https://github.com/blynkkk/blynk-library/releases/latest

  Blynk is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build graphic interfaces for all your
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: http://www.blynk.cc
    Sketch generator:           http://examples.blynk.cc
    Blynk community:            http://community.blynk.cc
    Follow us:                  http://www.fb.com/blynkapp
                                http://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.

 *************************************************************
  This runs directly on NodeMCU.

  Note: This requires ESP8266 support package:
    https://github.com/esp8266/Arduino

  Please be sure to select the right NodeMCU module
  in the Tools -> Board menu!


/*
                    PIN ASSIGNMENTS
   Virtual Pin V1 is the OPEN/CLOSE Button Widget (Momentary)
   Virtual Pin V4 is the Wifi status (LCD)
   Virtual Pin V5 is the OPEN/CLOSE Status Indicator Widget (Value Display)
   8266 GPIO Pin 4 (D2 on nodemcu/05 on breakout board) is the Reed Switch
   8266 GPIO Pin 5 (D1 on nodemcu/04 on breakout board) is the Relay Switch
   Add Push Notification Widget for Controller Offline notifications

                       HARDWARE

    -NodeMCU Chip with necessary power supply

    -Magnetic reed Switch wired NORMALLY CLOSED between GPIO4 pin (Pulled high with 1.5k resistor to VCC) and Ground
     (Switch is opened when door is closed)

    -3v relay controlled by GPIO5 (via resistor to 2N2222 transistor gate) **Don't forget the snubber diode between the +/- leads on the relay

    -Wires spliced into Garage door actuator switch from 3v relay (wired NORMALLY OPEN) for simulating switch press
*/

#include <SoftwareSerial.h>
SoftwareSerial SwSerial(2, 3); // RX, TX
#define BLYNK_PRINT SwSerial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#define REED_SWITCH 4   //D2 nodemcu
#define RELAY_CONTROL 5 //D1 nodemcu

WidgetLCD lcd(V4);

// Blynk App project Auth Token
char auth[] = "";

// Your WiFi credentials.
char ssid[] = "";
char pass[] = "";

SimpleTimer timer;

bool doorOpen = false;
bool lockedOut = false;


void sendConnectionDetails() {
  String ip = String(WiFi.localIP()[0]) + "." + String(WiFi.localIP()[1]) + "." + String(WiFi.localIP()[2]) + "." + String(WiFi.localIP()[3]);
  String rssi = String(WiFi.RSSI());
  lcd.print(0 ,0, ip);
  lcd.print(0 ,1, rssi + "dBm");
}

void sendDoorState()
{
  if(doorOpen) {
    Blynk.virtualWrite(V5, "OPEN");
  }
  if(!doorOpen) {
    Blynk.virtualWrite(V5, "CLOSED");
  }
}

void setup()
{
  SwSerial.begin(9600);
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(1000L, sendConnectionDetails);
  timer.setInterval(1000L, sendDoorState);
  
  pinMode(     REED_SWITCH, INPUT_PULLUP); // Reed Switch GPIO *Add External Pullup 1.5k OHM*
  digitalWrite(REED_SWITCH, HIGH);         // Initial Reed Switch State
  pinMode(     RELAY_CONTROL, OUTPUT);     // Relay Control GPIO
  digitalWrite(RELAY_CONTROL, LOW);        // Initial Relay Control State
  
  while(Blynk.connect() == false) {}

  lcd.clear();
  Blynk.virtualWrite(V5, "-----");
}

void loop()
{
  Blynk.run();
  timer.run();
  doorOpen = digitalRead(REED_SWITCH);

  if(WiFi.status() == WL_DISCONNECTED) ESP.reset();
}

void switchOff ()
{
  digitalWrite(RELAY_CONTROL, LOW);
}

void switchUnLock()
{
  lockedOut = false;
}

// Button Widget WRITING to V1 (OPEN/CLOSE Button Control)
BLYNK_WRITE(V1)
{
  if(param.asInt()) {
    if(!lockedOut) {
      lockedOut = true;
      digitalWrite(RELAY_CONTROL, HIGH);
      timer.setTimeout(1000L, switchOff);     // After 1 second switch off relay
      timer.setTimeout(10000L, switchUnLock); // After 10 seconds unlock
    }
  }
}
