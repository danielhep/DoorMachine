int toRGB[3] = {0, 0, 0};
float currentRGB[3] = {0, 0, 0};

double slopeRGB[3] = {0, 0, 0};
int interceptRGB[3] = {0, 0, 0};
unsigned long RGBSetTime;
int RGBFadeTime;

void setDefault() {
  if (nightMode)
    setColor(2, 0, 0, 0); // default color
  else
    setColor(10, 10, 10, 0);
}

void colorChange() {
  unsigned long tsnapshot = millis();
  for (int i = 0; i < 3; i++) {
    if (tsnapshot < RGBSetTime + RGBFadeTime) {
      currentRGB[i] = slopeRGB[i] * (tsnapshot - RGBSetTime) + interceptRGB[i];
    } else {
      currentRGB[i] = (RGBFadeTime) * slopeRGB[i] + interceptRGB[i];
    }
  }

  analogWrite(REDPIN, int(currentRGB[0]));
  analogWrite(GREENPIN, int(currentRGB[1]));
  analogWrite(BLUEPIN, int(currentRGB[2]));
}

void iterateTimeout() {
  if (millis() > timeout && timeoutEnable) { // If the timeout time has passed and the timeout is enabled
    setDefault();
  }
}

void setColor(int R, int G, int B, int t, unsigned long fadeTime) {
  toRGB[0] = R;
  toRGB[1] = G;
  toRGB[2] = B;

  for (int i = 0; i < 3; i++) {
    slopeRGB[i] = (toRGB[i] - currentRGB[i]) / (fadeTime);

    /*Serial.println("Difference: " + String(toRGB[i] - currentRGB[i]));
    Serial.println("CurrentRGB: " + String(currentRGB[i]));
    Serial.println("ToRGB: " + String(toRGB[i]));
    Serial.println("Fade time: " + String(fadeTime));
    Serial.print("Slope: ");
    Serial.println(slopeRGB[i], 5);*/
    interceptRGB[i] = currentRGB[i];
  }

  RGBSetTime = millis();
  RGBFadeTime = fadeTime;

  if (t > 0) { // If the timeout value input is zero, then just disable the timeout
    timeoutEnable = true;
    timeout = millis() + t; // Timeout before  reverts to default
  } else {
    timeoutEnable = false;
  }
}

void setColor(int R, int G, int B, int t) {
  setColor(R, G, B, t, 1000);
}

int wheelFlag = 0;
void colorWheelCycle() {
  int colorTime = millis() % 3000;
  if (colorTime > 0 && colorTime < 1000 && wheelFlag != 1) {
    setColor(255, 0, 0, 0, 1000);
    wheelFlag = 1;
  } else if (colorTime > 1000 && colorTime < 2000 && wheelFlag != 2) {
    setColor(0, 255, 0, 0, 1000);
    wheelFlag = 2;
  } else if (colorTime > 2000 && colorTime < 3000 && wheelFlag != 3) {
    setColor(0, 0, 255, 0, 1000);
    wheelFlag = 3;
  }
}

