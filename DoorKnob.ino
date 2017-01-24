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

int irqpin = D3;  // Digital 3
int REDPIN = D6;
int GREENPIN = D7;
int BLUEPIN = D5;

boolean touchStates[12]; //to keep track of the previous touch states

int toRGB[3] = {0, 0, 0};
int currentRGB[3] = {0, 0, 0};

int peepholeTouchCount = 0;

unsigned long getTimeTimer = 0;
unsigned long timeout = 0;
bool timeoutEnable = true;
int currentTime = 0;
bool nightMode = true;

WiFiServer server(80);

void setup() {
  pinMode(irqpin, INPUT);
  digitalWrite(irqpin, HIGH); //enable pullup resistor

  Serial.begin(9600);
  Wire.begin();

  WiFi.mode(WIFI_STA);
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

  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);
  mpr121_setup();
}

void loop() {
  readTouchInputs();
  colorChange();
  iterateTimeout();
  processNetwork();
  delay(1);
}

void iterateTimeout() {
  if (millis() > timeout && timeoutEnable) { // If the timeout time has passed and the timeout is enabled
    setDefault();
  }
}

void setDefault() {
  if (nightMode)
    setColor(2, 0, 0, 0); // default color
  else
    setColor(10, 10, 10, 0);
}

void colorChange() {
  for (int i = 0; i < 3; i++) {
    if (toRGB[i] > currentRGB[i]) {
      currentRGB[i]++;
    } else if (toRGB[i] < currentRGB[i]) {
      currentRGB[i]--;
    }
  }

  analogWrite(REDPIN, currentRGB[0]);
  analogWrite(GREENPIN, currentRGB[1]);
  analogWrite(BLUEPIN, currentRGB[2]);
}

void setColor(int R, int G, int B, int t) {
  toRGB[0] = R;
  toRGB[1] = G;
  toRGB[2] = B;

  if (t > 0) { // If the timeout value input is zero, then just disable the timeout
    timeoutEnable = true;
    timeout = millis() + t; // Timeout before reverts to default
  } else {
    timeoutEnable = false;
  }
}

void readTouchInputs() {
  if (!checkInterrupt()) {
    //read the touch state from the MPR121
    Wire.requestFrom(0x5A, 2);

    byte LSB = Wire.read();
    byte MSB = Wire.read();

    uint16_t touched = ((MSB << 8) | LSB); //16bits that make up the touch states

    for (int i = 0; i < 12; i++) { // Check what electrodes were pressed
      if (touched & (1 << i)) { // i is being touches

        if (touchStates[i] == 0) { // It was not set as touched already
          //pin i was just touched
          Serial.print("pin ");
          Serial.print(i);
          Serial.println(" was just touched");
          /*
            if (i == 8) {
            if (touchStates[11] == 0) {
              setColor(1, 1, 1, 0);
            } else {
              setColor(55, 0, 0, 0); // Set the color to red
            }
            }*/
          if (i == 0) {
            peepholeTouchCount++;

            setColor(255, 0, 0, 0);
          }

        } else if (touchStates[i] == 1) {
          //pin i is still being touched
        }

        touchStates[i] = 1;      // Set it as touched
      } else { // i is not being touched
        if (touchStates[i] == 1) { // if it was set as touched before
          Serial.print("pin ");
          Serial.print(i);
          Serial.println(" is no longer being touched");

          if (i == 0) {
            if(!nightMode)
              setColor(0, 55, 0, 2000);
            else
              setDefault();
          }
          /*
            if (i == 8 && touchStates[11] == 1) {
            setColor(55, 0, 55, 2000); // Set color to blue when it is let go of
            }
            if (i == 8) {
            setColor(255, 255, 255, 0);
            }*/

          //pin i is no longer being touched
        }
        touchStates[i] = 0;
      }

    }
    //    processPins(); // A pin was touched, let's process it.
  }
}

void mpr121_setup(void) {

  set_register(0x5A, ELE_CFG, 0x00);

  // Section A - Controls filtering when data is > baseline.
  set_register(0x5A, MHD_R, 0x01);
  set_register(0x5A, NHD_R, 0x01);
  set_register(0x5A, NCL_R, 0x00);
  set_register(0x5A, FDL_R, 0x00);

  // Section B - Controls filtering when data is < baseline.
  set_register(0x5A, MHD_F, 0x01);
  set_register(0x5A, NHD_F, 0x01);
  set_register(0x5A, NCL_F, 0xFF);
  set_register(0x5A, FDL_F, 0x02);

  // Section C - Sets touch and release thresholds for each electrode
  set_register(0x5A, ELE0_T, TOU_THRESH);
  set_register(0x5A, ELE0_R, REL_THRESH);

  set_register(0x5A, ELE1_T, TOU_THRESH);
  set_register(0x5A, ELE1_R, REL_THRESH);

  set_register(0x5A, ELE2_T, TOU_THRESH);
  set_register(0x5A, ELE2_R, REL_THRESH);

  set_register(0x5A, ELE3_T, TOU_THRESH);
  set_register(0x5A, ELE3_R, REL_THRESH);

  set_register(0x5A, ELE4_T, TOU_THRESH);
  set_register(0x5A, ELE4_R, REL_THRESH);

  set_register(0x5A, ELE5_T, TOU_THRESH);
  set_register(0x5A, ELE5_R, REL_THRESH);

  set_register(0x5A, ELE6_T, TOU_THRESH);
  set_register(0x5A, ELE6_R, REL_THRESH);

  set_register(0x5A, ELE7_T, TOU_THRESH);
  set_register(0x5A, ELE7_R, REL_THRESH);

  set_register(0x5A, ELE8_T, TOU_THRESH);
  set_register(0x5A, ELE8_R, REL_THRESH);

  set_register(0x5A, ELE9_T, TOU_THRESH);
  set_register(0x5A, ELE9_R, REL_THRESH);

  set_register(0x5A, ELE10_T, TOU_THRESH);
  set_register(0x5A, ELE10_R, REL_THRESH);

  set_register(0x5A, ELE11_T, TOU_THRESH);
  set_register(0x5A, ELE11_R, REL_THRESH);

  // Section D
  // Set the Filter Configuration
  // Set ESI2
  set_register(0x5A, FIL_CFG, 0x04);

  // Section E
  // Electrode Configuration
  // Set ELE_CFG to 0x00 to return to standby mode
  set_register(0x5A, ELE_CFG, 0x0C);  // Enables all 12 Electrodes


  // Section F
  // Enable Auto Config and auto Reconfig
  /*set_register(0x5A, ATO_CFG0, 0x0B);
    set_register(0x5A, ATO_CFGU, 0xC9);  // USL = (Vdd-0.7)/vdd*256 = 0xC9 @3.3V   set_register(0x5A, ATO_CFGL, 0x82);  // LSL = 0.65*USL = 0x82 @3.3V
    set_register(0x5A, ATO_CFGT, 0xB5);*/  // Target = 0.9*USL = 0xB5 @3.3V

  set_register(0x5A, ELE_CFG, 0x0C);

}


boolean checkInterrupt(void) {
  return digitalRead(irqpin);
}


void set_register(int address, unsigned char r, unsigned char v) {
  Wire.beginTransmission(address);
  Wire.write(r);
  Wire.write(v);
  Wire.endTransmission();
}
