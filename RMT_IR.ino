void processRmtIr()
{
    #ifdef RMT_MODE
        char _incomingMessageType = irRx.readMessageType();
        int  _incomingMessageData = irRx.readRawDataPacket();

        //Set flags to skip sending hosting packets if receiving data
        if(_incomingMessageType == PACKET)
        {
            
            
            
            
            
            if( !isSendingActive) setIrReceivingState(true);






            
            switch(_incomingMessageData)
            {
                case 16:        //0x10 Request to Join (reply to Px hosting packet)
                    Serial.println("Received P16 reply packet");
                    expectingNonP2packet = true;
                    break;
                case 17:        //0x11 Ack Player Assign (Final Ack from Tagger - player has joined)
                    Serial.println("Received P17 reply packet");
                    expectingNonP2packet = false;
                    break;
                case 130:        //0x82 LTAR Request to Join
                    Serial.println("Received P130 reply packet");
                    expectingNonP2packet = true;
                    break;
                case 132:       //0x84 LTAR Ack Player Assign
                    Serial.println("Received P130 reply packet");
                    expectingNonP2packet = true;
                    break;
                case 135:       //0x87 Host LTAR Release message
                    expectingNonP2packet = false;
                
            }
            
        }
        
        //Clear message if a new command
        switch(_incomingMessageType)
        {
            case PACKET:
            case BEACON:
            case LTAR_BEACON:
            case TAG:
                fullRxMessage = "";
                break;
        }
    
        switch(_incomingMessageType)
        {
            case PACKET:
            case DATA:
            case CHECKSUM:
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
                            Serial.print("CheckSum match with last sent message - Dumping message");Serial.println(_txCheckSumCompare);
                            txCheckSum = "";
                            isSendingActive = false;
                        }
                        else
                        {
                            //Terminate the packet and send it to WiFi
                            fullRxMessage += ",@";
                            client.println(fullRxMessage);
                            Serial.print("processRmtIr:: RxMessage = ");Serial.println(fullRxMessage);
                        }
                    #else
                        //Terminate the packet and send it to WiFi
                        fullRxMessage += ",@";
                        //Serial.print("processRmtIr:: RxMessage = ");Serial.println(fullRxMessage);
                        client.println(fullRxMessage);
                    #endif
                    fullRxMessage = "";
                    setIrReceivingState(false);
                }
                break;
    
            case TAG:
                rgbLED(0,1,0);
                fullRxMessage += _incomingMessageType;
                fullRxMessage += _incomingMessageData;
                fullRxMessage += ",@";
                //Serial.print("processRmtIr:: RxMessage = ");Serial.println(fullRxMessage);
                client.println(fullRxMessage);
                fullRxMessage = "";
                setIrReceivingState(false);
                break;
            
            case BEACON:
            case LTAR_BEACON:
                fullRxMessage += _incomingMessageType;
                fullRxMessage += _incomingMessageData;
                fullRxMessage += ",@";
                //Serial.print("processRmtIr:: RxMessage = ");Serial.println(fullRxMessage);

                //Dont send to Host app, as it floods the network and blocks incoming comms (rehost)!
                //client.println(fullRxMessage);
                //server.write(client.read());
        //client.write(fullRxMessage, fullRxMessage.length() );
                
                
                fullRxMessage = "";
                setIrReceivingState(false);
                break;
        }
    #endif
}
