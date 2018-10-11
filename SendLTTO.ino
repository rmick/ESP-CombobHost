
void sendLttoIR(String fullDataString)
{
    rgbLED(1,0,1);
    
    int delimiterPosition = 0;
    const char *delimiter = ":";

    //check that line starts with 'ltto
    if (fullDataString.startsWith("ltto"))
    {       
        delimiterPosition = fullDataString.indexOf(":");
        fullDataString.remove(0, delimiterPosition + 1);
    }
    else
    {
        Serial.print("ERROR Invalid command header: ");
        delimiterPosition = fullDataString.indexOf(":");
        Serial.print(fullDataString.substring(0, delimiterPosition));
        return;
    }

    #ifdef  RMT_MODE
        isSendingActive = true;
    #endif


    #ifdef  RMT_FAST_MODE
        if(expectingNonP2packet)
        {
            static int counter = 1;
            char     _type = fullDataString.charAt(0);
            uint16_t _data = fullDataString.substring(1,delimiterPosition).toInt();
            if(_type == 'P' && _data == 2 )
            {
                Serial.print("Dumping Host packet - Count =");
                Serial.println(counter++);
                fullDataString = "";
                if(counter >= 5)
                {
                    expectingNonP2packet = false;
                    counter = 1;
                }
                return;
            }
        }
    
        irTx.sendLttoIR(fullDataString);
        delay (150);
        rgbLED(0,1,0);
    #else
        while(fullDataString.length() > 0)
        {
            char packetType = fullDataString.charAt(0);
            fullDataString.remove(0, 1);
            
            delimiterPosition = fullDataString.indexOf(":");
            uint16_t data = fullDataString.substring(0,delimiterPosition).toInt();
            fullDataString.remove(0, delimiterPosition + 1);
    
            #ifdef RMT_MODE
                irTx.sendLttoIR(packetType, data); 
            #else
                IRcontrol(packetType, data);
            #endif
        }
        delay (25);
        rgbLED(0,1,0);
    #endif
}

