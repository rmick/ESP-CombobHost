int loopCount = 0;

void HowLong(bool state)
{
    if (state == START)
    {
        howLongDidThisTake = micros();
        processingMessage = true;
    }
    else
    {
        //if(loopCount == 0) Serial.print("\n\t");
        loopCount++; 
        if ( (micros() - howLongDidThisTake) > 500)
        {
            Serial.print    ("\n\t");
            Serial.println  ( (micros() - howLongDidThisTake) / 1000.0 );
        }
        
        processingMessage = false;
        if(loopCount > 20)
        {
            loopCount = 0;
        }
    }
}

