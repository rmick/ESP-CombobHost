 void IRcontrol(char type, uint16_t message)
{   
    int         _msgLength      = 0;
    int         _interDelay     = 25;
    static int  _checkSumCalc   = 0;
    
    //Send Header
    switch (type)
    {
    // if Type = B then Beacon,   Header is 366, length is 5 bits
    // if Type = T then Tag,      Header is 363, length is 7 bits
    // if Type = D the data byte, Header is 363, length is 8 bits
    // if Type = P then Packet,   Header is 363, length is 9 bits, first bit must be 0
    // if Type = C then CheckSum, Header is 363, length is 9 bits, first bit must be 1

    // Special Format Pause - Inactive
    // 56ms - Tag signature
    // 80ms - Checksum signature
    // 25ms - All others

    case 'P':
        _msgLength = 9;
        _interDelay = 25;
        _checkSumCalc = message;
        PulseIR(3);
        delayMicroseconds (6000);
        PulseIR(3);
        break;

    case 'D':
        _msgLength = 8;
        _interDelay = 25;
        _checkSumCalc = _checkSumCalc + message;
        PulseIR(3);
        delayMicroseconds (6000);
        PulseIR(3);
        break;

    case 'C':
        _msgLength = 9;
        _interDelay = 0;
        message = _checkSumCalc;           // Overwrite the message with the calculated checksum
        message = message | 256;          //  Set the required 9th MSB bit to 1 to indicate it is a checksum
        PulseIR(3);
        delayMicroseconds (6000);
        PulseIR(3);
        break;

    case 'T':
        _msgLength = 7;
        _interDelay = 56;
        PulseIR(3);
        delayMicroseconds (6000);
        PulseIR(3);
        break;

    case 'B':
        _msgLength = 5;
        _interDelay = 25;
        PulseIR(3);
        delayMicroseconds (6000);
        PulseIR(6);
        break;
    }

    //Send message
    for (int bitCount = _msgLength-1; bitCount >=0; bitCount--)
    {
        delayMicroseconds (2000);
        PulseIR(bitRead(message, bitCount)+1);        // the +1 is to convert 0/1 data into 1/2mS pulses.
    }

    lazerTagReceive.enableIRIn();
    lazerTagReceive.resume();

    delay(_interDelay);

    Serial.print(F("\tIR- "));
    Serial.print(type);
    Serial.print(F(" - "));
    Serial.print(message, DEC);
    Serial.print(" -\t");
    long unsigned CurrentTime = millis();
    Serial.print(CurrentTime-TimeSinceLast);
    Serial.println("mS");
    TimeSinceLast = millis();
    if (type == 'C') Serial.println("\n--------------------------");
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

int timeOutCount = 0;

void PulseIR(byte _mSec)
{
    unsigned long _pulseStartTime = micros();
    unsigned long _pulseLength = _mSec*1000;
    unsigned long _pulseEndTime = _pulseStartTime + _pulseLength - 24;

    ledcWrite(1, 50);
    while (_pulseEndTime >micros() )
    {
        delayMicroseconds(1);
    }
    ledcWrite(1,0);

    //Interrupt driven version
    timeOutCount = _mSec;
    //ledcWrite(1,50);
    //enableInterrupt;
}

void Interruptor()
{
    if (--timeOutCount = 0) ledcWrite (1,0);
}

//void PulseIR(byte _mSec)
//{
//    unsigned long _pulseStartTime = micros();
//    unsigned long _pulseLength = _mSec*1000;
//    unsigned long _pulseEndTime = _pulseStartTime + _pulseLength - 24;
//
//    while (_pulseEndTime >micros() )
//    {
//        digitalWrite(TX_PIN, HIGH);
//        delayMicroseconds(12);
//        digitalWrite(TX_PIN, LOW);
//        delayMicroseconds(12);
//    }
//}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

