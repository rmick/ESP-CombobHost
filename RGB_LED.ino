void rgbLED(bool Red, bool Green, bool Blue)
{
    if(Red)     pinMode(RED_LED,   INPUT_PULLUP);  else pinMode(RED_LED,   INPUT_PULLDOWN);
    if(Green)   pinMode(GREEN_LED, INPUT_PULLUP);  else pinMode(GREEN_LED, INPUT_PULLDOWN);
    if(Blue)    pinMode(BLUE_LED,  INPUT_PULLUP);  else pinMode(BLUE_LED,  INPUT_PULLDOWN);
}

