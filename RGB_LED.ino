void rgbLED(bool Red, bool Green, bool Blue)
{
    if(Red)     ledcWrite(RED_CHANNEL,   ledIntensity); else ledcWrite(RED_CHANNEL,   0);
    if(Green)   ledcWrite(GREEN_CHANNEL, ledIntensity); else ledcWrite(GREEN_CHANNEL, 0);
    if(Blue)    ledcWrite(BLUE_CHANNEL,  ledIntensity); else ledcWrite(BLUE_CHANNEL,  0);
}
