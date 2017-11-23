unsigned long elapsedIRtime = millis();

void processWiFi()
{   
    char tcpChar = client.read();

    if(receivingData)
    {
        //dataIn = "";
        //return;
    }
    
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
            irDataIndicator(true, ASTERISK_TX);
            Serial.println("\nLTTO_TX: " + dataIn);
            elapsedIRtime = millis();
            sendLttoIR(dataIn);      //Send to IR
            //Serial.print("Elapsed Time = ");
            //Serial.println(millis()-elapsedIRtime);
            dataIn = "";
            irDataIndicator(false, ASTERISK_TX);
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
        else 
        {
            Serial.print("\nRaw Data: ");
            Serial.print(dataIn);
            dataIn = "";
        }
    }
    else
    {
        dataIn += tcpChar;
    }
}

