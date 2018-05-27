void sendLazerSwarmIR(char line[])
{
    const char *delimiters = " ";

    //check that line starts with CMD
    char *token = strtok (line, delimiters);
    if (token == NULL || strcasecmp(token, "cmd") != 0)
    {
        Serial.print("ERROR Invalid command: ");
        Serial.println(token);
        return;
    }
    
    //check the Command byte != NULL
    token = strtok (NULL, delimiters);
    if (token == NULL)
    {
        Serial.println("ERROR Missing command number.");
        return;    
    }

    //grab the Command byte;
    int command = strtol(token, NULL, 16);
    
    switch (command)
    {
        case 0x10:
        {
            //grab the data byte
            token = strtok (NULL, delimiters);
            if (token == NULL)
            {
                Serial.println("ERROR Missing data parameter. !?!?!?!");
                return;    
            }
            short data = strtol(token, NULL, 16);

            //grab the bitCount byte
            token = strtok (NULL, delimiters);
            if (token == NULL)
            {
                Serial.println("ERROR Missing bit count parameter.");
                return;    
            }
            int bitCount = strtol(token, NULL, 16);

            //grab the beacon byte
            token = strtok (NULL, delimiters);
            if (token == NULL)
            {
                Serial.println("ERROR Missing beacon parameter.");
            return;    
            }
            bool isBeacon = (strtol(token, NULL, 16) != 0);

  //IRSend not implemented on ESP32 yet !!!!!    
  //lazerTagSend.sendLTTO(data, bitCount, isBeacon);

            //Convert to LTTO format and send via IRcontrol;
            char packetType;
            if      (bitCount ==9 && data < 256)    packetType = 'P';
            else if (bitCount ==9 && data > 256)    packetType = 'C';
            else if (bitCount ==8 && !isBeacon)     packetType = 'D';
            else if (bitCount ==5 &&  isBeacon)     packetType = 'Z';
            else if (bitCount ==7 && !isBeacon)     packetType = 'T';
            else                                    packetType = '_';     // Indicates an error.
            
            Serial.print(F("\nLZ Data sent =\t"));
            Serial.print(data);
//            Serial.print(",");
//            Serial.print(bitCount);
//            Serial.print(",");
//            Serial.print(isBeacon);
//            if(data < 10) Serial.print("\t");
            Serial.print(" -\t");
//            long unsigned CurrentTime = millis();
//            Serial.print(CurrentTime-TimeSinceLast);
//            Serial.print("mS");
//            //if (bitCount == 9 && data > 256) Serial.print("\n");
//            TimeSinceLast = millis();

    //////IRcontrol(packetType, data);
            //lazerTagReceive.enableIRIn(true);
            //lazerTagReceive.resume();
            
          
            break;
        }
        default:
        {
            Serial.print("ERROR Invalid command number: ");
            Serial.println(command, HEX);
        }
    }
}

