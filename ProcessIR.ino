
void processIR(decode_results *results)
{
    
    if (results->address == TYPE_LAZERTAG_TAG)
    // These are Tags, Packets, Data and Checksums
    {   
        if      ( (results->bits == 9) && (results->value < 256) )
        // This is a Packet header
        {
//TODO: Do I need this line?            
client.println("STOP");

            setIrReceivingState(true);
            fullRxMessage = "P";
            fullRxMessage += String(results->value);
            rxCalculatedCheckSum = results->value;
            irDataIndicator(true, ASTERISK_RX);
        }
        else if ( (results->bits == 9) && (results->value > 256) )
        // This is a Checksum
        {
            fullRxMessage += ",C";
            fullRxMessage += String(results->value);
            fullRxMessage += ",@";
            rxCalculatedCheckSum = rxCalculatedCheckSum%256;
            //if ((rxCalculatedCheckSum) != results->value%256)    Serial.println("\tBAD Checksum !!!");
            Serial.println(fullRxMessage);
            client.println(fullRxMessage);
            irDataIndicator(false, ASTERISK_RX);
            setIrReceivingState(false);
            fullRxMessage = "";
        }
        else if (results->bits == 8)
        //This is Data
        {
            fullRxMessage += ",D";
            fullRxMessage += String(results->value);
            rxCalculatedCheckSum += results->value;
        }        
        else if (results->bits == 7)
        //This is a Tag
        {   
            setIrReceivingState(true);
            irDataIndicator(true, ASTERISK_RX);
            fullRxMessage = "T";
            fullRxMessage += String(results->value);
            fullRxMessage += ",@";
            Serial.println(fullRxMessage);
            client.println(fullRxMessage);
            irDataIndicator(false, ASTERISK_RX);
            setIrReceivingState(false);
            fullRxMessage = "";
        }
        else
        {
            Serial.print("\n\tMISSED Packet = ");
            Serial.print(results->address);
            Serial.print(results->value);
        }
    }
    else if (results->address == TYPE_LAZERTAG_BEACON)
    // These are Beacons
    {   
        if      (results->bits == 5)
        //LTTO Beacon
        {
            setIrReceivingState(true);
            irDataIndicator(true, ASTERISK_RX);
            fullRxMessage = "B";
            fullRxMessage += String(results->value);
            fullRxMessage += ",@";
            Serial.println(fullRxMessage);
            client.println(fullRxMessage);
            irDataIndicator(false, ASTERISK_RX);
            setIrReceivingState(false);
            fullRxMessage = "";
        }
        else if (results->bits == 9)
        //LTAR Enhanced Beacon
        {
            setIrReceivingState(true);
            irDataIndicator(true, ASTERISK_RX);
            fullRxMessage = "E";
            fullRxMessage += String(results->value);
            fullRxMessage += ",@";
            Serial.println(fullRxMessage);
            client.println(fullRxMessage);
            fullRxMessage = "";
            irDataIndicator(false, ASTERISK_RX);
            setIrReceivingState(false);
            fullRxMessage = "";
        }
    }
    else
    // Must be an error
    {
        Serial.print(results->address);
        Serial.print("-RAW ");
        for (int i = 0; i < results->rawlen; i++)
        {
            Serial.print(results->rawbuf[i] * USECPERTICK, DEC);
            if (i < (results->rawlen - 1)) Serial.print(" ");
        }
        Serial.println();
        client.println("X" + String(++rxErrorCount) + ",@");
        setIrReceivingState(false);
        return;
    }
}

void setIrReceivingState (bool state)
{
        rxTimer = millis();
        receivingData = state;
        Serial.print("\tReceivingData = ");
        Serial.println(state);
        if (state == false)
        {
            digitalWrite(LED_PIN, LOW);
        }

        #ifdef DEBUG_LOCAL
            if(fullRxMessage.length() <10)  writeDisplay(fullRxMessage, 2, CENTRE_HOR, CENTRE_VER, true);
            else                            writeDisplay(fullRxMessage, 2, LEFT_HOR,   1,          true);
        #endif
}

