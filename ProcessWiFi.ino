
void processWiFi()
{   
    char tcpChar = client.read();
    
    if (tcpChar == '\n')
    {
        if      (dataIn.startsWith("CMD 10", 0))
        {
            Serial.println("\n\tL-Swarm_TX: " + dataIn);
            char irMessageIn[SERIAL_BUFFER_SIZE];
            dataIn.toCharArray(irMessageIn, SERIAL_BUFFER_SIZE);
            sendLazerSwarmIR(irMessageIn);      //Send to IR
            dataIn = "";
        }
        
        else if (dataIn.startsWith("ltto"))
        {
            if (receivingData == true && dataIn.startsWith("ltto:P02"))
            {
                Serial.print("Dumping P02 packet");
                dataIn = "";
                return;
            }
            Serial.println("\nLTTO_TX: " + dataIn);
            sendLttoIR(dataIn);      //Send to IR
            dataIn = "";
        }
        
        else if (dataIn.startsWith("TXT", 0))
        {
            //Serial.println("LCD: " + dataIn);
            int lineNumber = (dataIn.charAt(3)-48);                     //grab the 4th character and convert from ASCII to Decimal number
            dataIn.remove(0,5);                                         //remove the TXT#: characters
            bool shallWeClearDisplay = false;
            if(lineNumber == 1) shallWeClearDisplay = true;             //Clear screen if Line# = 1, else don't
            writeDisplay(dataIn, 2, CENTRE_HOR, lineNumber, shallWeClearDisplay);
            dataIn = "";
        }
        
        else if (dataIn.startsWith("DSP",0))
        {
            //full direct access to pixels
            //DSP(xCursor, yCursor, text, fontSize, colour clearDisp)
            int xCurDelim = dataIn.indexOf(',') + 1;
            int yCurDelim = dataIn.indexOf(',', xCurDelim + 1) + 1;
            int textDelim = dataIn.indexOf(',', yCurDelim + 1) + 1;
            int fontDelim = dataIn.indexOf(',', textDelim + 1) + 1;
            int colrDelim = dataIn.indexOf(',', fontDelim + 1) + 1;
            int cDspDelim = dataIn.indexOf(',', colrDelim + 1) + 1;
    
            Serial.print("Strings are : ");
            Serial.print(dataIn.substring(xCurDelim, yCurDelim-1)+" ");
            Serial.print(dataIn.substring(yCurDelim, textDelim-1)+" ");
            Serial.print(dataIn.substring(textDelim, fontDelim-1)+" ");
            Serial.print(dataIn.substring(fontDelim, colrDelim-1)+" ");
            Serial.print(dataIn.substring(colrDelim, cDspDelim-1)+" ");
            Serial.println(dataIn.substring(cDspDelim));
           
            hCursor         = (dataIn.substring(xCurDelim, yCurDelim-1)).toInt();
            vCursor         = (dataIn.substring(yCurDelim, textDelim-1)).toInt();
            String text     = (dataIn.substring(textDelim, fontDelim-1));
            int fontSize    = (dataIn.substring(fontDelim, colrDelim-1)).toInt();
            int colour      = (dataIn.substring(colrDelim, cDspDelim-1)).toInt();
            bool clearDisp  = (dataIn.substring(colrDelim)).toInt();
            writeDSPdisplay(hCursor, vCursor, text, fontSize, colour, clearDisp);
    
            dataIn = "";
        }
        
        else if (dataIn.startsWith("LED",0))
        {
            Serial.print("LED command = ");
            Serial.print(dataIn);
            int RedDelim    = dataIn.indexOf(',') + 1;
            int GreenDelim  = dataIn.indexOf(',', RedDelim   + 1) + 1;
            int BlueDelim   = dataIn.indexOf(',', GreenDelim + 1) + 1;
            bool Red    = (dataIn.substring(RedDelim, GreenDelim-1)).toInt();
            bool Green  = (dataIn.substring(GreenDelim, BlueDelim-1)).toInt();
            bool Blue   = (dataIn.substring(BlueDelim, 1)).toInt();
            
            Serial.print(" : ");
            Serial.print(Red);
            Serial.print(",");
            Serial.print(Green);
            Serial.print(",");
            Serial.print(Blue);
            Serial.println(".");

            rgbLED(Red, Green, Blue);

            dataIn = "";
        }

        else if (dataIn.startsWith("OTA",0))
        {
            lazerTagReceive.enableIRIn(false);
            
            Serial.println("\n-----\nOTA message recv'd\n-----\n");
            //up to here.......
            int ssidDelim = dataIn.indexOf(',') + 1;
            int pswdDelim    = dataIn.indexOf(',', ssidDelim + 1) + 1;

            otaMode             = true;
            String ssidString   = (dataIn.substring(ssidDelim, pswdDelim-1));
            String pswdString   = (dataIn.substring(pswdDelim));
         
            EEPROM.writeByte  (OTA_MODE_OFFSET, otaMode);
            EEPROM.writeString(SSID_OFFSET,     ssidString);
            EEPROM.writeString(PSWD_OFFSET,     pswdString);
            EEPROM.commit();
            delay(2000);
            
            Serial.println("\tCredentials written to EEPROM");
            Serial.print("\tOTA Mode =\t");
            Serial.println(EEPROM.readByte(OTA_MODE_OFFSET));
            Serial.print("\tSSID =\t");
            Serial.println(EEPROM.readString(SSID_OFFSET));
            Serial.print("\tpwd =\t");
            Serial.println(EEPROM.readString(PSWD_OFFSET));

            writeDisplay("U/G mode",             2, CENTRE_HOR, 1, true);
            writeDisplay("selected",             2, CENTRE_HOR, 2, false);
            writeDisplay("Press Reset to start", 1, CENTRE_HOR, 7, false);
            
            // Do not enable this, as they are upgrading.........
            //lazerTagReceive.enableIRIn(true);
        }
        
        else 
        {
            dataIn = "";
        }
    }
    else
    {
        dataIn += tcpChar;
    }
}

