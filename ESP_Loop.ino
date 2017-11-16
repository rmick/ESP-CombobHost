
void loop()
{
    //ArduinoOTA.handle();

    //Debug Test button
    if (digitalRead(0) == LOW)
    {
        IRcontrol('T', 0);
        delay(250);
        return;
    }
    
    //Listen for client messages
    client = server.available(); 
    if (client) 
    {
        Serial.println("Connected");
        writeDisplay("Online", 2, CENTRE_HOR, CENTRE_VER, true);

        //TCP connection established           
        while (client.connected()) 
        {          
            //Check for any IR messages received and action them
            if (lazerTagReceive.decode(&results))
            {
                processIR(&results);
                lazerTagReceive.resume();
            }

            //Check for any WiFi messages received and action them
            if (receivingData == false && client.available())
            {
                processWiFi();
            }
        }
        
        //TCP connection has been terminated
        digitalWrite(LED_PIN, LOW);
        Serial.println("\n\tDisconnected");
        writeDisplay("Offline", 2, CENTRE_HOR, CENTRE_VER, true);  
    }
}
