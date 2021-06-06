//  Courtesy of Riley McArdle
//  https://github.com/astrospark/LazerTagHost/blob/master/Arduino/sketches/Relay/Relay.ino


void listenForLazerswarm()
{
    while (Serial.available())
    {
        char previousChar = '\0';
        if (serialBufferPosition > 0)
        {
            previousChar = serialBuffer[serialBufferPosition - 1];
        }

        char thisChar = Serial.read();
        serialBuffer[serialBufferPosition] = thisChar;

        if (previousChar == '\r' && thisChar == '\n')
        {
            serialBuffer[serialBufferPosition - 1] = '\0';
            lazerSwarmMode = true;
            static bool firstTime = true;
            if (firstTime)
            {
                writeDisplay("LazerSwarm",  2, CENTRE_HOR, 2, true, false);
                writeDisplay("Mode",        2, CENTRE_HOR, 3, false, true);
                firstTime = false;
                Serial.println("LazerSwarm Mode is now active:");
            }
            processLaserswarmCmd(serialBuffer);
            serialBufferPosition = 0;
        }
        else
        {
            serialBufferPosition++;
            if (serialBufferPosition >= SERIAL_BUFFER_SIZE) serialBufferPosition = 0;
        }
    }
}

void processLaserswarmCmd(char line[])
{
    const char* delimiters = " ";

    char* token = strtok(line, delimiters);
    if (token == NULL || strcasecmp(token, "cmd") != 0)
    {
        Serial.print("ERROR Invalid command: ");
        Serial.println(token);
        return;
    }

    token = strtok(NULL, delimiters);
    if (token == NULL)
    {
        Serial.println("ERROR Missing command number.");
        return;
    }
    int command = strtol(token, NULL, 16);

    switch (command)
    {
    case 0x10:
    {
        token = strtok(NULL, delimiters);
        if (token == NULL)
        {
            Serial.println("ERROR Missing data parameter.");
            return;
        }
        short data = strtol(token, NULL, 16);

        token = strtok(NULL, delimiters);
        if (token == NULL)
        {
            Serial.println("ERROR Missing bit count parameter.");
            return;
        }
        int bitCount = strtol(token, NULL, 16);

        token = strtok(NULL, delimiters);
        if (token == NULL)
        {
            Serial.println("ERROR Missing beacon parameter.");
            return;
        }
        bool isBeacon = (strtol(token, NULL, 16) != 0);
        
        char dataType = '_';
        if (isBeacon)           dataType = BEACON;
        else if (data > 255)    dataType = CHECKSUM;                   // a 9 bit value must be greater than 255
        else if (bitCount == 9) dataType = PACKET;
        else if (bitCount == 8) dataType = DATA;
        else if (bitCount == 7) dataType = TAG;
                   // a 9 bit value must be greater than 255

        //IRcontrol(dataType, data);
        isSendingActive = true;
        irTx.sendLttoIR(dataType, data);
        if (dataType == CHECKSUM)
        {
            delay(50);
            isSendingActive = false;
        }
        break;
    }
    default:
        Serial.print("ERROR Invalid command number: ");
        Serial.println(command, HEX);
    }
}

void replyLazerSwarm(char _incomingMessageType, int _incomingMessageData)
{
    int _numberOfBits = 0;
    if          (_incomingMessageType == PACKET)    _numberOfBits = 9;
    else if     (_incomingMessageType == DATA)      _numberOfBits = 8;
    else if     (_incomingMessageType == TAG)       _numberOfBits = 7;
    else if     (_incomingMessageType == BEACON)    _numberOfBits = 5;
    else if     (_incomingMessageType == CHECKSUM)  _numberOfBits = 9;
    
    Serial.print("RCV ");
    Serial.print(_incomingMessageData, HEX);
    Serial.print(" ");
    Serial.print(_numberOfBits, HEX);
    Serial.print(" ");
    Serial.print(_incomingMessageType == BEACON ? 1 : 0);
    Serial.println();
}