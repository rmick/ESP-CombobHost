void processRmtIr()
{
    #ifdef RMT_MODE
        char _incomingMessageType = irRx.readMessageType();
        int  _incomingMessageData = irRx.readRawDataPacket();

        if(_incomingMessageType == 'P')
        {
            if(_incomingMessageData == 16)  //reply to Px hosting packet
            {
                Serial.println("Receiving P1 reply packet");
                expectingNonP2packet = true;
            }

            if(_incomingMessageData == 1)   //Final Ack from Tagger - player has joined.
            {
                Serial.println("Received P17 reply packet");
                expectingNonP2packet = false;
            }
        }
        
        //Clear message if a new command
        switch(_incomingMessageType)
        {
            case 'P':
            case 'B':
            case 'T':
            case 'E':
                fullRxMessage = "";
                break;
        }
    
        switch(_incomingMessageType)
        {
            case 'P':
            case 'D':
            case 'C':
                //Add current Type + Data to message
                fullRxMessage += _incomingMessageType;
                fullRxMessage += _incomingMessageData;
        
                //add a delimiter comma after each incoming message, unless it is a Checksum
                if(_incomingMessageType != 'C')
                {
                    fullRxMessage += ",";
                }
                else        // it is a checksum message
                {
                    //If the incoming packet has the same checksum as the last Tx packet, then dump it
                    #ifdef RMT_MODE 
                        String _txCheckSumCompare = _incomingMessageType + (String)_incomingMessageData;
                        if(_txCheckSumCompare == txCheckSum)
                        {
                            Serial.print("CheckSum match - Dumping message");Serial.println(_txCheckSumCompare);
                            txCheckSum = "";
                            isSendingActive = false;
                        }
                        else
                        {
                            //Terminate the packet and send it to WiFi
                            fullRxMessage += ",@";
                            Serial.print("processRmtIr:: RxMessage = ");
                            Serial.println(fullRxMessage);
                            client.println(fullRxMessage);
                        }
                    #else
                        //Terminate the packet and send it to WiFi
                        fullRxMessage += ",@";
                        Serial.print("processRmtIr:: RxMessage = ");
                        Serial.println(fullRxMessage);
                        client.println(fullRxMessage);
                    #endif
                    fullRxMessage = "";
                    setIrReceivingState(false);
                }
                break;
    
            case 'T':
            case 'B':
            case 'E':
                fullRxMessage += _incomingMessageType;
                fullRxMessage += _incomingMessageData;
                fullRxMessage += ",@";
                Serial.print("processRmtIr:: RxMessage = ");
                Serial.println(fullRxMessage);
                client.println(fullRxMessage);
                fullRxMessage = "";
                setIrReceivingState(false);
        }
    #endif
}
