
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
        rgbLED(0,1,0);

        //TCP connection established           
        while (client.connected())
        {
            //THIS IS THE REAL MAIN LOOP
            if(!digitalRead(BUTTON)) Serial.println("HELLO Mr Button");
            
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
            }
        }
        //TCP connection has been terminated
        static bool firstTimeThru = true;
        if (firstTimeThru)
        {
            client.stop();
            Serial.println("\n\tDisconnected");
            writeDisplay("Offline", 2, CENTRE_HOR, CENTRE_VER, true);
            rgbLED(0,0,1);
            firstTimeThru = false;
        }
    }

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

