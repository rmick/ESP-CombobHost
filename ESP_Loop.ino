
void loop()
{
    //WARNING............
    // Dont put any code here, as it will not run if connected to Wifi.

    if(!digitalRead(BUTTON))
        {
            lazerTagReceive.enableIRIn(false);
            Serial.print("Button Pressed - set Update mode");
            writeDisplay("U/G mode",             2, CENTRE_HOR, 1, true);
            writeDisplay("selected",             2, CENTRE_HOR, 2, false);
            writeDisplay("Press Reset to start", 1, CENTRE_HOR, 7, false);
            
            EEPROM.writeByte(OTA_MODE_OFFSET, true);
            EEPROM.commit();
            delay(1000);
        }

    //Listen for client messages
    client = server.available(); 
    if (client) 
    {
        Serial.println("Connected");

        writeDisplay("Online", 2, CENTRE_HOR, CENTRE_VER, true);

        //TCP connection established           
        while (client.connected()) 
      
//THIS IS THE REAL MAIN LOOP
        {
            //Check for any IR messages received and action them              
            if (lazerTagReceive.decode(&results))
            {
                setIrReceivingState(true);
                processIR(&results);
                lazerTagReceive.resume();
            }

            //Check for any WiFi messages received and action them
            if (receivingData == false && client.available())
            {
                processWiFi();
            }
            
            else if (receivingData == true)
            {
               if ((millis() - rxTimer) > rxTimeOutInterval)
               {
                    receivingData = false;
                    Serial.println("RxTimer reset");
                    Serial.println(fullRxMessage);
                    digitalWrite(LED_PIN, LOW);
               }
               //else Serial.print("Rx.");
            }

            //Show if battery is flat.
            if(BatteryVoltage() < 5.2) rgbLED(1,0,0); 

        }
        //TCP connection has been terminated
        digitalWrite(LED_PIN, LOW);
        Serial.println("\n\tDisconnected");
        writeDisplay("Offline", 2, CENTRE_HOR, CENTRE_VER, true);

       
    }
    //We are now in the Offline loop


    //TODO: Set a timer, if nothing happens for 2 minutes, power down.


    
    #ifdef DEBUG_LOCAL
        if (lazerTagReceive.decode(&results))
        {
            processIR(&results);
            lazerTagReceive.resume();
        }
        display.clearDisplay();
        display.display();
    #endif
//    }

}

