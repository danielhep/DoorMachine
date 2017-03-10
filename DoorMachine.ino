
/*Copyright (c) 2010 bildr community

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include "mpr121.h"
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>

#define EFFECT_WHEEL 1
#define EFFECT_WATER 2

#define REDPIN D6
#define GREENPIN D5
#define BLUEPIN D7

#define IRQ_PIN D3

#define MOTION_PIN D8

int ledStripEffect = 0;
int effectStrength = 255;
int effectTime = 3000;

boolean touchStates[12]; //to keep track of the previous touch states

int peepholeTouchCount = 0;
int motionDetectorCount = 0;

unsigned long getTimeTimer = 0;
unsigned long timeout = 0;
bool timeoutEnable = true;
int currentTime = 0;
bool nightMode = true;

WiFiServer server(80);
LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup() {
  // Setup serial and i2c
  Serial.begin(9600);
  Wire.begin();

  // Setup trigger pin for capacitive sensor
  pinMode(IRQ_PIN, INPUT);
  digitalWrite(IRQ_PIN, HIGH); //enable pullup resistor

  // Setup motion detector pin
  pinMode(MOTION_PIN, INPUT);


  // Setup the 3 PWM outputs for the LED strip
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);

  // Let us turn on the built in LED!
  pinMode(LED_BUILTIN, OUTPUT);

  // Connect to WiFi
  WiFi.mode(WIFI_STA); // client mode
  WiFi.begin("WWUResTek-alt");
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected, IP Address: ");
  Serial.println(WiFi.localIP());
  server.begin();

  mpr121_setup();

  // initialize the lcd
  lcd.init();
}

void loop() {
  readTouchInputs();
  colorChange();
  iterateTimeout();
  processNetwork();
  checkMotionDetector();
  updateLCD();
}

bool motionDetectorTripped = false;
void checkMotionDetector() {
  if (!touchStates[0]) {
    if (digitalRead(MOTION_PIN) == HIGH) {
      if (!motionDetectorTripped) {
        motionDetectorCount++;
        digitalWrite(LED_BUILTIN, LOW);
        if (nightMode)
          setColor(15, 0, 0, 10000, 200);
        else
          setColor(100, 100, 100, 10000, 200);
        motionDetectorTripped = true;
      }
    } else {
      if (motionDetectorTripped) {
        digitalWrite(LED_BUILTIN, HIGH);
        motionDetectorTripped = false;
      }
    }
  }
}

void set_register(int address, unsigned char r, unsigned char v) {
  Wire.beginTransmission(address);
  Wire.write(r);
  Wire.write(v);
  Wire.endTransmission();
}
