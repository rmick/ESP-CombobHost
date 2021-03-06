
#ifndef RMT_MODE
//
//void processIR(decode_results *results)
//{
//    
//    if (results->address == TYPE_LAZERTAG_TAG)
//    // These are Tags, Packets, Data and Checksums
//    {   
//        if      ( (results->bits == 9) && (results->value < 256) )
//        // This is a Packet header
//        {
//            fullRxMessage = PACKET;
//            fullRxMessage += String(results->value);
//            rxCalculatedCheckSum = results->value;
//        }
//        else if ( (results->bits == 9) && (results->value > 256) )
//        // This is a Checksum
//        {
//            fullRxMessage += ",C";
//            fullRxMessage += String(results->value);
//            fullRxMessage += ",@";
//            rxCalculatedCheckSum = rxCalculatedCheckSum%256;
//            //Serial.println(fullRxMessage);
//            client.println(fullRxMessage);
//            setIrReceivingState(false);
//            fullRxMessage = "";
//        }
//        else if (results->bits == 8)
//        //This is Data
//        {
//            fullRxMessage += ",D";
//            fullRxMessage += String(results->value);
//            rxCalculatedCheckSum += results->value;
//        }        
//        else if (results->bits == 7)
//        //This is a Tag
//        {   
//            fullRxMessage = TAG;
//            fullRxMessage += String(results->value);
//            fullRxMessage += ",@";
//            Serial.println(fullRxMessage);
//     //TODO:client.println(fullRxMessage);
//            setIrReceivingState(false);
//            fullRxMessage = "";
//        }
//        else
//        {
//            Serial.print("\n\tMISSED Packet = ");
//            Serial.print(results->address);
//            Serial.print(" : ");
//            Serial.print(results->value);
//        }
//    }
//    else if (results->address == TYPE_LAZERTAG_BEACON)
//    // These are Beacons
//    {   
//        if      (results->bits == 5)
//        //LTTO Beacon
//        {
//            fullRxMessage = BEACON;
//            fullRxMessage += String(results->value);
//            fullRxMessage += ",@";
//            Serial.println(fullRxMessage);
//     //TODO:client.println(fullRxMessage);
//            setIrReceivingState(false);
//            fullRxMessage = "";
//        }
//        else if (results->bits == 9)
//        //LTAR Enhanced Beacon
//        {
//            fullRxMessage = LTAR_BEACON;
//            fullRxMessage += String(results->value);
//            fullRxMessage += ",@";
//            Serial.println(fullRxMessage);
//     //TODO:client.println(fullRxMessage);
//            fullRxMessage = "";
//            setIrReceivingState(false);
//            fullRxMessage = "";
//        }
//    }
//    else
//    // Must be an error
//    {
//        Serial.print(results->address);
//        Serial.print("-RAW ");
//        for (int i = 0; i < results->rawlen; i++)
//        {
//            Serial.print(results->rawbuf[i] * USECPERTICK, DEC);
//            if (i < (results->rawlen - 1)) Serial.print(" ");
//        }
//        Serial.println();
//        client.println("X" + String(++rxErrorCount) + ",@");
//        setIrReceivingState(false);
//        return;
//    }
//}

#endif

void setIrReceivingState (bool state)
{
        rxTimer = millis();
        if (receivingData == false && state == true)
        {
            client.println("STOP,@");
            Serial.println("STOP@");
        }
        receivingData = state;
        if (state == true)  rgbLED(1,1,0);
        else                rgbLED(0,0,0);

        //irDataIndicator(state, ASTERISK_RX);

        #ifdef DEBUG_LOCAL
            if(fullRxMessage.length() <10)  writeDisplay(fullRxMessage, 2, CENTRE_HOR, CENTRE_VER, true, true);
            else                            writeDisplay(fullRxMessage, 2, LEFT_HOR,   1,          true, true);
        #endif
}
