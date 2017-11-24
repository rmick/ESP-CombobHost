
void loop()
{
    //ArduinoOTA.handle();

    //WARNING............
        // Dont put any code here, as it will not run if connected to Wifi.
    
    //Listen for client messages
    client = server.available(); 
    if (client) 
    {
        Serial.println("Connected");
        writeDisplay("Online", 2, CENTRE_HOR, CENTRE_VER, true);

        //TCP connection established           
        while (client.connected()) 
        {          
//THIS IS THE REAL MAIN LOOP
 
            //Debug Test button
            if (digitalRead(0) == LOW)  receivingData = false;
            
            //Check for any IR messages received and action them
            if (lazerTagReceive.decode(&results))
            {
                processIR(&results);
                lazerTagReceive.resume();
            }

            //Check for any WiFi messages received and action them
            if      (receivingData == false && client.available())
            {
                processWiFi();
            }
            
            else if (receivingData == true)  // && client.available())
            {
                //TODO: Add a timer and reset if taking too long.
               if ((millis() - rxTimer) > rxTimeOutInterval)
               {
                    receivingData = false;
                    Serial.println("RxTimer reset");
                    Serial.println(fullRxMessage);
                    client.println("START");  //required to release the STOP command.
                    digitalWrite(LED_PIN, LOW);
               }
            }
        }
        //TCP connection has been terminated
        digitalWrite(LED_PIN, LOW);
        Serial.println("\n\tDisconnected");
        writeDisplay("Offline", 2, CENTRE_HOR, CENTRE_VER, true);
    }
    //We are now in the Offline loop
        
    if (lazerTagReceive.decode(&results))
    {
        processIR(&results);
        lazerTagReceive.resume();
    }
    display.clearDisplay();
    display.display();
}
