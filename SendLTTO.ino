
void sendLttoIR(String fullDataString)
{
    int delimiterPosition = 0;
    const char *delimiter = ":";

    //check that line starts with 'ltto
    if (fullDataString.startsWith("ltto"))
    {
        delimiterPosition = fullDataString.indexOf(":");
        fullDataString.remove(0, delimiterPosition + 1);
    }
    else
    {
        Serial.print("ERROR Invalid command header: ");
        delimiterPosition = fullDataString.indexOf(":");
        Serial.print(fullDataString.substring(0, delimiterPosition));
        return;
    }

    while(fullDataString.length() > 0)
    {
        char packetType = fullDataString.charAt(0);
        fullDataString.remove(0, 1);
        
        delimiterPosition = fullDataString.indexOf(":");
        uint16_t data = fullDataString.substring(0,delimiterPosition).toInt();
        fullDataString.remove(0, delimiterPosition + 1);
        
        //lazerTagSend.enableIROut(38);    
        IRcontrol(packetType, data);
    }
    
}

