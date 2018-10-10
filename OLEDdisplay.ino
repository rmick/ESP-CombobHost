void writeDisplay(String text, int fontSize, int alignment, int lineNumber, bool erase, bool drawScreen)
{
    if(receivingData == true) return;

//    Serial.print("LCD_WRITE: ");
//    Serial.print(text);
    
    if (erase) display.clearDisplay();
    display.setTextSize(fontSize);
    display.setTextColor(WHITE, BLACK);
    vCursor = ((lineNumber-1)*(fontSize * CHAR_HEIGHT));
    if(lineNumber > 1) vCursor += 1;   // add a gap between lines
    if (lineNumber == CENTRE_VER) vCursor = 32 - ((fontSize * CHAR_HEIGHT)/2);
    int hCursor;
    int numChars = text.length();
    switch (alignment)
    {
        case LEFT_HOR:
            hCursor = 0;
            break;
        case CENTRE_HOR:
            hCursor = 64 - ((numChars)*((fontSize * CHAR_WIDTH)/2));
            break;
        case RIGHT_HOR:
            hCursor = 128 - (numChars*(fontSize * CHAR_WIDTH));
            break;
    }
    display.setCursor(hCursor, vCursor);
    display.println(text);
    if(drawScreen)  display.display();
}

void writeDSPdisplay(int hCursor, int vCursor, String text, int fontSize, int colour, bool clearDsp, bool drawScreen)
{
    if(receivingData == true) return;

    //Serial.print("LCD_WRITE: ");
    //Serial.println(text);
    
    if (clearDsp) display.clearDisplay();
    display.setCursor(hCursor, vCursor);
    display.setTextSize(fontSize);
    display.setTextColor(WHITE, BLACK);
    display.println(text);
    if(drawScreen) display.display();
}

//--------------------------------------------------------------------------------------------------

void irDataIndicator(int state, int location)
{
    //set the flag that stops IR Tx when receiving (via ESP_Loop).
    if (location == ASTERISK_RX)
    {
        if (state == true)  digitalWrite(LED_PIN, HIGH);
        else                digitalWrite(LED_PIN,  LOW);
    }

    //TODO: Remove this return, as it is debug only
    return;
    



    //TODO: Move this the other CPU core, as it takes too long and blocks the IR Tx/Rx code from working correctly.
    
    if (state == true)
    {
        display.setTextSize(2);
        display.setCursor(location, 48);
        display.setTextColor(WHITE, BLACK);
        display.println("*");
        display.display();
        if (location == ASTERISK_TX)
        {
            txStartTime = micros();
            txIndicatorOn = true;
        }
        if (location == ASTERISK_RX)
        {
            rxStartTime = micros();
            rxIndicatorOn = true;
        }
    }
    else if (state == false)
    {
        display.setTextSize(2);
        display.setCursor(location, 48);
        display.setTextColor(BLACK, BLACK);
        display.println("*");
        display.display();

        if (location == ASTERISK_TX) txIndicatorOn = false;
        if (location == ASTERISK_RX) rxIndicatorOn = false;
    }
}

