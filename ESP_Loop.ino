
void loop()
{
    //WARNING............
    // Dont put any code here, as it will not run if connected to Wifi.

    if(!digitalRead(BUTTON))
    {
    #ifndef RMT_MODE    
        lazerTagReceive.enableIRIn(false);
    #endif    
        if(EEPROM.readByte(OTA_MODE_OFFSET) == false)
        {
            Serial.print("Button Pressed - set Update mode");
            writeDisplay("U/G mode",             2, CENTRE_HOR, 1,  true, false);
            writeDisplay("selected",             2, CENTRE_HOR, 2, false, false);
            writeDisplay("Press Reset to start", 1, CENTRE_HOR, 7, false,  true);
        
            EEPROM.writeByte(OTA_MODE_OFFSET, true);
            EEPROM.commit();
            delay(1000);
        }
        else
        {
            Serial.print("Button Pressed - cancel Update mode");
            writeDisplay("U/G mode",             2, CENTRE_HOR, 1,  true, false);
            writeDisplay("cancelled",            2, CENTRE_HOR, 2, false, false);
            writeDisplay("Press Reset to start", 1, CENTRE_HOR, 7, false,  true);
        
            EEPROM.writeByte(OTA_MODE_OFFSET, false);
            EEPROM.commit();
            delay(1000);
        }  
    }

    checkBattery(true);

    listenForLazerswarm();

    if (lazerSwarmMode)
    {
        //Process IR via ESP32_RMT_IR_LTTO library
        codeLength = irRx.readIR(IRdataRx, sizeof(IRdataRx));
        if (codeLength > 3)  //ignore any short codes
        {
            rgbLED(1, 1, 1);
            //if( !isSendingActive)   setIrReceivingState(true);
            processRmtIr();
        }
    }
   

    //Listen for WiFi client messages
    client = server.available(); 
    if (client) 
    {
        static bool firstTime = true;
        if (firstTime)
        {
            Serial.println("Connected");
            writeDisplay("Online", 2, CENTRE_HOR, CENTRE_VER, true, true);
            firstTime = false;
            dataIn = "";
            //rgbLED(0,1,0);
        }
        
        //TCP connection established     
        while (client.connected())
        {
//THIS IS THE REAL MAIN LOOP

            unsigned long        currentTime     = micros();
            static unsigned long lastWiFiMessage = millis();
            
            //Check for any IR messages received and action them  
#ifdef  RMT_MODE
            //Process IR via ESP32_RMT_IR_LTTO library
            codeLength = irRx.readIR(IRdataRx,sizeof(IRdataRx));
            if (codeLength > 3)  //ignore any short codes
            {
                //if( !isSendingActive)   setIrReceivingState(true);
                processRmtIr();   
            }   
#else
            //Process IR via IR_LIB library
            if (lazerTagReceive.decode(&results))
            {
                setIrReceivingState(true);
                processIR(&results);
                lazerTagReceive.resume();
            }
#endif

            checkBattery(false);

            //Check for any WiFi messages received and action them
            if (receivingData == false && client.available())
            {
                processWiFi();
                lastWiFiMessage = millis();
            }
            
            else if (receivingData == true)
            {
               lastWiFiMessage = millis();
               if ((millis() - rxTimer) > rxTimeOutInterval)
               {
                    receivingData = false;
                    Serial.println("RxTimer reset");
                    Serial.println(fullRxMessage);
                    digitalWrite(LED_PIN, LOW);
               }
            }
            
            if ((millis() - lastWiFiMessage) > 10000)
            {
                Serial.println("TCP comms timeout");
                writeDisplay("Comms Lost", 2, CENTRE_HOR, CENTRE_VER, true, true);
                lastWiFiMessage = millis();
                delay(500);
                client.stop();
            }   
        }
        //TCP connection has been terminated
        Serial.println("\n\tDisconnected");
        writeDisplay("Offline", 2, CENTRE_HOR, CENTRE_VER, true, true);
        rgbLED(0,0,1);
    }

    //TODO: Set a timer, if nothing happens for 2 minutes, power down.
    
    #ifdef DEBUG_LOCAL
        #ifdef RMT_MODE
            codeLength = irRx.readIR(IRdataRx,sizeof(IRdataRx));
            if (codeLength > 0) processRmtIr();      //ignore any short codes
        #elif
            if (lazerTagReceive.decode(&results))
            {
                processIR(&results);
                lazerTagReceive.resume();
            }
        #endif    
        display.clearDisplay();
        display.display();
    #endif
}
