
void processIR(decode_results *results)
{
    if (results->address == TYPE_LAZERTAG_TAG)
    // These are Tags, Packets, Data and Checksums
    {
        if      ( (results->bits == 9) && (results->value < 256) )
        // This is a Packet header
        {
            fullRxMessage = "P";
            fullRxMessage += String(results->value);
            rxCalculatedCheckSum = 0;
            irDataIndicator(true, ASTERISK_RX);
            Serial.print("Receiving IR: ");
            Serial.println(fullRxMessage);
        }
        else if ( (results->bits == 9) && (results->value > 256) )
        // This is a Checksum
        {
            fullRxMessage += ",C";
            fullRxMessage += String(results->value);
            fullRxMessage += ",@";
            if (rxCalculatedCheckSum == results->value)    Serial.print("Good Checksum :-)");
            client.println(fullRxMessage);
            Serial.println(fullRxMessage);
            irDataIndicator(false, ASTERISK_RX);
            digitalWrite(LED_PIN, LOW);
        }
        else if (results->bits == 8)
        //This is Data
        {
            fullRxMessage += ",D";
            fullRxMessage += String(results->value);
            rxCalculatedCheckSum += results->value;
            Serial.println(fullRxMessage); 
        }
        else if (results->bits == 7)
        //This is a Tag
        {
            irDataIndicator(true, ASTERISK_RX);
            fullRxMessage = "T";
            fullRxMessage += String(results->value);
            fullRxMessage += ",@";
            client.println(fullRxMessage);
            Serial.println(fullRxMessage);
            receivingData = false;
            irDataIndicator(false, ASTERISK_RX);
        }
    }
    else if (results->address == TYPE_LAZERTAG_BEACON)
    // These are Beacons
    {   
        if      (results->bits == 5)
        //LTTO Beacon
        {
            irDataIndicator(true, ASTERISK_RX);
            fullRxMessage = "B";
            fullRxMessage += String(results->value);
            fullRxMessage += ",@";
            client.println(fullRxMessage);
            Serial.println(fullRxMessage);
            irDataIndicator(false, ASTERISK_RX);
        }
        else if (results->bits == 9)
        //LTAR Enhanced Beacon
        {
            irDataIndicator(true, ASTERISK_RX);
            fullRxMessage = "E";
            fullRxMessage += String(results->value);
            fullRxMessage += ",@";
            client.println(fullRxMessage);
            Serial.println(fullRxMessage);
            irDataIndicator(false, ASTERISK_RX);
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
        return;
    }
    
//    if( (results->bits == 9) && (results->value < 256) ) Serial.print("\n");
//    //digitalWrite(13, !digitalRead(13));
//    Serial.print("\tData received = ");
//    Serial.print(results->value, DEC);
//    Serial.print(" ");
//    Serial.print(results->bits, DEC);
//    Serial.print(" ");
//    Serial.print(results->address == TYPE_LAZERTAG_BEACON ? 1 : 0);
//    Serial.println();
//    if( (results->bits == 9) && (results->value > 256) ) Serial.println("\t__________________\n");
//
//    client.print("RCV ");
//    client.print(results->value, HEX);
//    client.print(" ");
//    client.print(results->bits, HEX);
//    client.print(" ");
//    client.print(results->decode_type == TYPE_LAZERTAG_BEACON ? 1 : 0);
//    client.println();

    //digitalWrite(LED_PIN, LOW);
}
