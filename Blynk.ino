BLYNK_WRITE(V1)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable

  if (pinValue == 1) {
    //setColor(255,0,0,1000,100);
    //setDefaultEffect(EFFECT_DND, false);
  } else {
    //setColor(255,255,255,1000,100);
    //setDefaultEffect(EFFECT_WHEEL, false);
  }
}

BLYNK_WRITE(V0)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable

  if (pinValue == 1) {
    //setColor(255,0,0,0,20);
  } else {
    //setDefault();
  }
}
