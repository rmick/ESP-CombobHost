void rgbLED(bool Red, bool Green, bool Blue)
{
    if(Red)     ledcWrite(redChannel,   RGB_BRIGHTNESS); else ledcWrite(redChannel,   0);
    if(Green)   ledcWrite(greenChannel, RGB_BRIGHTNESS); else ledcWrite(greenChannel, 0);
    if(Blue)    ledcWrite(blueChannel,  RGB_BRIGHTNESS); else ledcWrite(blueChannel,  0);

    return;
    
    if(Red)     digitalWrite(RED_LED,   HIGH);  else digitalWrite(RED_LED,   LOW);
    if(Green)   digitalWrite(GREEN_LED, HIGH);  else digitalWrite(GREEN_LED, LOW);
    if(Blue)    digitalWrite(BLUE_LED,  HIGH);  else digitalWrite(BLUE_LED,  LOW);
}




