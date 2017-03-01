unsigned long LCDTimer = 0;
void updateLCD() {
  if (nightMode) {
    lcd.noBacklight();
  } else {
    if (touchStates[0]) {
      backlightOscillate();
    } else {
      lcd.backlight();
    }
  }

  // Print a message to the LCD.
  if (millis() > LCDTimer) {
    lcd.setCursor(3, 0);
    lcd.print("Edens North #148");
    lcd.setCursor(0, 1);
    lcd.print("M-Detector: " + String(motionDetectorCount));
    lcd.setCursor(0, 2);
    lcd.print("Peephole touch: " + String(peepholeTouchCount));
    lcd.setCursor(0, 3);
    lcd.print("Uptime: " + formatTime((int) (millis() / 3600000)) + ":" + formatTime((int)((millis() / 60000) % 60)));

    LCDTimer = millis() + 2000;
  }
}

String formatTime(int input) {
  if (input < 10)
    return "0" + String(input);
  else
    return String(input);
}

unsigned long oscillateTimer = 0;
bool backlightEnabled = true;
void backlightOscillate() {
  if (millis() > oscillateTimer) {
    if (backlightEnabled) {
      lcd.noBacklight();
      backlightEnabled = false;
    } else {
      lcd.backlight();
      backlightEnabled = true;
    }

    oscillateTimer = millis() + 250;
  }
}

