int toRGB[3] = {0, 0, 0};
float currentRGB[3] = {0, 0, 0};

double slopeRGB[3] = {0, 0, 0};
int interceptRGB[3] = {0, 0, 0};
unsigned long RGBSetTime;
int RGBFadeTime;

void colorChange() {
  if (ledStripEffect == EFFECT_WHEEL)
    colorWheelEffect();

  unsigned long tsnapshot = millis();
  for (int i = 0; i < 3; i++) {
    if (tsnapshot < RGBSetTime + RGBFadeTime) {
      currentRGB[i] = slopeRGB[i] * (tsnapshot - RGBSetTime) + interceptRGB[i];
    } else {
      currentRGB[i] = (RGBFadeTime) * slopeRGB[i] + interceptRGB[i];
    }
  }

  //Serial.println("RGB: " + String(currentRGB[0]) + ", " + String(currentRGB[1]) + ", " + String(currentRGB[2]));

  analogWrite(REDPIN, int(currentRGB[0]));
  analogWrite(GREENPIN, int(currentRGB[1]));
  analogWrite(BLUEPIN, int(currentRGB[2]));
}

void setDefault() {
  setDefault(1000);
}

void setDefault(int fadeTime) {
  if (nightMode)
    setColor(0, 0, 0, 0, fadeTime); // default color
  else {
    //setColor(15, 0, 10, 0, fadeTime);
    setEffect(EFFECT_WATER, 255, 6000);
  }
}

void iterateTimeout() {
  if (millis() > timeout && timeoutEnable) { // If the timeout time has passed and the timeout is enabled
    setDefault();
  }
}

////////////////////
// This section is for setting the color and calculating the values for the color slopes
////////////////////

void setColor(int R, int G, int B, int t, unsigned long fadeTime) {
  setEffect(0, 0, 0); // Get rid of any effect
  setLinearColorGraph(R, G, B, fadeTime); // Set the color fade graph

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

void setEffect(int effect, int strength, int eTime) {
  ledStripEffect = effect;
  effectStrength = strength;
  effectTime = eTime;

  timeoutEnable = false;

  Serial.println("Effect set: " + String(effect) + " at strength: " + String(strength));
}

void setLinearColorGraph(int R, int G, int B, int fadeTime) {
  toRGB[0] = R;
  toRGB[1] = G;
  toRGB[2] = B;

  for (int i = 0; i < 3; i++) {
    slopeRGB[i] = (toRGB[i] - currentRGB[i]) / (fadeTime);
    interceptRGB[i] = currentRGB[i];
  }

  RGBSetTime = millis();
  RGBFadeTime = fadeTime;
}

int wheelFlag = 0;
void colorWheelEffect() {
  int colorTime = millis() % effectTime;
  if (colorTime > 0 && colorTime < effectTime / 3 && wheelFlag != 1) {
    setLinearColorGraph(effectStrength, 0, 0, effectTime / 3);
    wheelFlag = 1;
  } else if (colorTime > effectTime / 3 && colorTime < (2 * effectTime) / 3 && wheelFlag != 2) {
    setLinearColorGraph(0, effectStrength, 0, effectTime / 3);
    wheelFlag = 2;
  } else if (colorTime > (2 * effectTime) / 3 && colorTime < effectTime && wheelFlag != 3) {
    setLinearColorGraph(0, 0, effectStrength, effectTime / 3);
    wheelFlag = 3;
  }
}

void waterWheelEffect() {
  int colorTime = millis() % effectTime;
  int R;
  int G;
  int B;
  
  if (colorTime > 0 && colorTime < effectTime / 2 && wheelFlag != 1) {
    R = 96/255*effectStrength;
    G = 221/255*effectStrength;
    B = 142/255*effectStrength;

    wheelFlag = 1;
  } else if (colorTime > effectTime / 2 && colorTime < effectTime && wheelFlag != 2) {
    R = 20/255*effectStrength;
    G = 17/255*effectStrength;
    B = 99/255*effectStrength;
    
    wheelFlag = 2;
  }

  setLinearColorGraph(R, G, B, effectTime / 3);
}
