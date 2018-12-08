
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
            if (receivingData == true)
            {
                if (dataIn.startsWith("ltto:P02") || dataIn.startsWith("ltto:P03") || dataIn.startsWith("ltto:P04") ||
                    dataIn.startsWith("ltto:P05") || dataIn.startsWith("ltto:P06") || dataIn.startsWith("ltto:P07") ||
                    dataIn.startsWith("ltto:P08") || dataIn.startsWith("ltto:P09") || dataIn.startsWith("ltto:P10") ||
                    dataIn.startsWith("ltto:P11") || dataIn.startsWith("ltto:P12") || dataIn.startsWith("ltto:P129") )
                {
                    Serial.print("Dumping P02-P12/P129 packet");
                    dataIn = "";
                    return;
                }
            }
            
            //Serial.println("\nLTTO_TX: " + dataIn);
            int stringLength = dataIn.length();
            #ifdef RMT_MODE
                txCheckSum = dataIn.substring( (stringLength - 7), (stringLength-3) );
            #endif
            sendLttoIR(dataIn);      //Send to IR
            dataIn = "";
        }
        
        else if (dataIn.startsWith("TXT", 0))
        {
            //Serial.println("LCD: " + dataIn);
            
            int lineNumber = dataIn.charAt(3)-48;                       //grab the 4th character and convert from ASCII to Decimal number
            dataIn.remove(0,5);                                         //remove the TXT#: characters
            
            bool drawScreen = dataIn.charAt(dataIn.length()-2) - 48;    //grab the last character and convert from ASCII to Decimal number
            dataIn.remove((dataIn.length() -3), 3);                     // remove the :x characters
            
            bool shallWeClearDisplay = false;
            if(lineNumber ==   1) shallWeClearDisplay = true;           //Clear screen if Line# = 1, else don't
            writeDisplay(dataIn, 2, CENTRE_HOR, lineNumber, shallWeClearDisplay, drawScreen);
            dataIn = "";
        }
        
        else if (dataIn.startsWith("DSP",0))
        {
            //full direct access to pixels
            //DSP(xCursor, yCursor, text, fontSize, colour clearDisp, drawScreen)
            int xCurDelim = dataIn.indexOf(',') + 1;
            int yCurDelim = dataIn.indexOf(',', xCurDelim + 1) + 1;
            int textDelim = dataIn.indexOf(',', yCurDelim + 1) + 1;
            int fontDelim = dataIn.indexOf(',', textDelim + 1) + 1;
            int colrDelim = dataIn.indexOf(',', fontDelim + 1) + 1;
            int cDspDelim = dataIn.indexOf(',', colrDelim + 1) + 1;
            int dDspDelim = dataIn.indexOf(',', cDspDelim + 1) + 1;
    
//            Serial.print("Strings are : ");
//            Serial.print(dataIn.substring(xCurDelim, yCurDelim-1)+" ");
//            Serial.print(dataIn.substring(yCurDelim, textDelim-1)+" ");
//            Serial.print(dataIn.substring(textDelim, fontDelim-1)+" ");
//            Serial.print(dataIn.substring(fontDelim, colrDelim-1)+" ");
//            Serial.print(dataIn.substring(colrDelim, cDspDelim-1)+" ");
//            Serial.println(dataIn.substring(cDspDelim));
           
            hCursor         = (dataIn.substring(xCurDelim, yCurDelim-1)).toInt();
            vCursor         = (dataIn.substring(yCurDelim, textDelim-1)).toInt();
            String text     = (dataIn.substring(textDelim, fontDelim-1));
            int fontSize    = (dataIn.substring(fontDelim, colrDelim-1)).toInt();
            int colour      = (dataIn.substring(colrDelim, cDspDelim-1)).toInt();
            bool clearDisp  = (dataIn.substring(cDspDelim, dDspDelim-1)).toInt();
            bool drawScreen = (dataIn.substring(dDspDelim)).toInt();
            writeDSPdisplay(hCursor, vCursor, text, fontSize, colour, clearDisp, drawScreen);
    
            dataIn = "";
        }
        
        else if (dataIn.startsWith("LED",0))
        {
            //Serial.print("LED command = ");
            //Serial.print(dataIn);
            
            int RedDelim    = dataIn.indexOf(',') + 1;
            int GreenDelim  = dataIn.indexOf(',', RedDelim   + 1) + 1;
            int BlueDelim   = dataIn.indexOf(',', GreenDelim + 1) + 1;
                
            bool Red    = (dataIn.substring(RedDelim,   GreenDelim-1)).toInt();
            bool Green  = (dataIn.substring(GreenDelim, BlueDelim-1)).toInt();
            bool Blue   = (dataIn.substring(BlueDelim)).toInt();
            
//            Serial.print(" : ");
//            Serial.print(Red);
//            Serial.print(",");
//            Serial.print(Green);
//            Serial.print(",");
//            Serial.print(Blue);
//            Serial.println(".");

            rgbLED(Red, Green, Blue);

            dataIn = "";
        }

        else if (dataIn.startsWith("OTA",0))
        {
            #ifdef RMT_MODE
            irTx.stopIR();
            irRx.stopIR();
            #else
            lazerTagReceive.enableIRIn(false);
            #endif
            
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

            writeDisplay("U/G mode",   2, CENTRE_HOR, 1,  true, false);
            writeDisplay("selected",   2, CENTRE_HOR, 2, false, false);
            writeDisplay("RESTARTING", 1, CENTRE_HOR, 7, false,  true);
            String otaText = "OTA," + ssidString + "," + pswdString + ",@";
            client.println(otaText);
            delay(1000);
            dataIn = "";
            
            ESP.restart();
        }

        else if (dataIn.startsWith("PING",0))
        {
            String pingText = (dataIn.substring(dataIn.indexOf(',') + 1));
            Serial.print("Reply to Ping");
            String pongText = "PONG," + pingText + ",@";
            client.println(pongText);
            writeDisplay("PING",   2, CENTRE_HOR, 1,  true, false);
            writeDisplay("Reply",  2, CENTRE_HOR, 2, false, false);
            writeDisplay(pingText, 1, CENTRE_HOR, 7, false,  true);
            dataIn = "";
        }
        else if (dataIn.startsWith("HEART",0))
        {
            //Serial.println("*** HeartBeat arrived ***");
            client.println("H-B-Ack,@");
            dataIn = "";
            
        }
        else if (dataIn.startsWith("ESP_RESTART",0))
        {
            Serial.print("ESP_RESTART triggered");
            ESP.restart();
        }
        
        else 
        {
            Serial.println("ERROR: Empty WiFi Message");
            dataIn = "";
        }
    }
    else
    {
        dataIn += tcpChar;
    }
}
