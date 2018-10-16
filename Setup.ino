void runSetup()
{
    //Setup
    Serial.begin(250000);
    delay(100);
    Serial.println("\n");
    Serial.println("The Combobulator is now running........\n");
    Serial.print("Build:"); Serial.println(BUILD_NUMBER);
   #ifdef DEBUG_LOCAL
    Serial.println("DeBug Build");
   #endif 
    Serial.println("\n");

   //Initiliase the Combobulator Hardware
    pinMode     (RED_LED,    OUTPUT );
    pinMode     (GREEN_LED,  OUTPUT );    
    pinMode     (BLUE_LED,   OUTPUT );
    pinMode     (BATT_VOLTS, INPUT  );
    pinMode     (LED_PIN,    OUTPUT );
    digitalWrite(LED_PIN,    LOW    );
    pinMode     (BUTTON,     INPUT  );
    pinMode     (TX_PIN,     OUTPUT );
    analogReadResolution(10);

    //Initialise the OLED screen
    pinMode     (OLED_RESET, OUTPUT); 
    digitalWrite(OLED_RESET, LOW);                      // set GPIO16 low to reset OLED 
    delay       (50); 
    digitalWrite(OLED_RESET, HIGH);                     // while OLED is running, must set GPIO16 to high 
    display.begin(SH1106_SWITCHCAPVCC, 0x3C);           // initialize with the I2C addr 0x3C
    display.setTextColor(WHITE, BLACK);

    //LED cWrite Setup
    ledcAttachPin(TX_PIN,    ledChannel);
    ledcAttachPin(RED_LED,   redChannel);
    ledcAttachPin(GREEN_LED, greenChannel);
    ledcAttachPin(BLUE_LED,  blueChannel);
    
    ledcSetup(ledChannel,    irFreq,    resolution);
    ledcSetup(redChannel,    ledFreq,   resolution);
    ledcSetup(greenChannel,  ledFreq,   resolution);
    ledcSetup(blueChannel,   ledFreq,   resolution);
    
    //Force the LEDs off
    ledcWrite(ledChannel,    0); 
    ledcWrite(redChannel,    0); 
    ledcWrite(greenChannel,  0); 
    ledcWrite(blueChannel,   0);
    
    //Set up EEPROM
    if (!EEPROM.begin(EEPROM_SIZE))
    {
        Serial.println("failed to initialise EEPROM");
    }
    else
    {
        Serial.println("EEPROM initiliased");
        Serial.print("\t_____________________________________\n\n\t******* Hardware Revision = ");
        Serial.print((char)EEPROM.readByte(HARDWARE_VERSION));
        Serial.println(" *******\n\t_____________________________________\n");

        otaCount = EEPROM.readByte(OTA_COUNT);
        Serial.print("otaCount = ");
        Serial.println(otaCount);

        if(EEPROM.readByte(OTA_MODE_OFFSET) == true)    otaMode = true;
        else                                            otaMode = false;
    }

    if (otaMode)
    {  
        Serial.println("----------\nOTA Mode\n----------");
        
        //Force the IR LEDs off !
        digitalWrite(TX_PIN, LOW);

        writeDisplay("U/G Mode", 2, CENTRE_HOR, 1, true, true);
        
        //Read Credentials from EEPROM
        String ssidString = EEPROM.readString(SSID_OFFSET);
        ssidString.remove(ssidString.length()-1);
        const char *otaSSID = &ssidString[0u];
        String pswdString = EEPROM.readString(PSWD_OFFSET);
        pswdString.remove(pswdString.length()-2);
        const char *otaPSWD = &pswdString[0u];

        Serial.print("\n---------------------\nSSID = ");
        Serial.print(ssidString);
        Serial.print(":\t");
        Serial.print(ssidString.length());
        Serial.print(":");
        Serial.print(ssidString.charAt(0));
        Serial.print(":");
        Serial.print(ssidString.charAt(ssidString.length()-1)); 
        Serial.println(":END\n");    
        
        Serial.print("\n---------------------\nPassword = ");
        Serial.print(pswdString);
        Serial.print(":\t");
        Serial.print(pswdString.length());
        Serial.print(":");
        Serial.print(pswdString.charAt(0));
        Serial.print(":");
        Serial.print(pswdString.charAt(pswdString.length()-1)); 
        Serial.println(":END\n");    

        //DO NOT TOUCH
        //  This is here to force the ESP32 to reset the WiFi and initialise correctly.
        Serial.print("WIFI status = ");
        Serial.println(WiFi.getMode());
        WiFi.disconnect(true);
        delay(1000);
        WiFi.mode(WIFI_STA);
        delay(1000);
        Serial.print("WIFI status = ");
        Serial.println(WiFi.getMode());
        // End silly stuff !!!
        
        // Connect to provided SSID and PSWD
        WiFi.begin(otaSSID, otaPSWD);
        Serial.println("Connecting to:" + String(otaSSID));
        Serial.println("Credentials:"   + String(otaPSWD));
        
        writeDisplay("Connecting to", 1, CENTRE_HOR, 4, false, false);
        writeDisplay(otaSSID,         1, CENTRE_HOR, 6, false,  true);

        // Wait for connection to establish
        timeOut = millis();
        bool reTry = false;
        bool flashDot = false;
        while (WiFi.status() != WL_CONNECTED)
        {
            if (flashDot) writeDisplay(" ", 2, CENTRE_HOR, 4, false, true);
            else          writeDisplay(".", 2, CENTRE_HOR, 4, false, true);
            flashDot = !flashDot;
            delay(500);

            if ( (millis() - timeOut) > 7500) reTry = true;
            Serial.print("TimeOut = ");
            Serial.print(timeOut);
            Serial.print(" : ");
            Serial.println(millis() - timeOut);

            if(!digitalRead(BUTTON))
            {
                Serial.println("Button Pressed - Cancelling OTA mode");
                writeDisplay("Upgrade",   2, CENTRE_HOR, 1, true,  true);
                writeDisplay("cancelled", 2, CENTRE_HOR, 2, false, true);
                EEPROM.writeByte(OTA_MODE_OFFSET, false);         
                EEPROM.commit();
                delay(1000);
                ESP.restart();
            }

            if (reTry == true)
            {
                if(++otaCount > 2)
                {
                    writeDisplay("Failed",    2, CENTRE_HOR, 2,  true, false);
                    writeDisplay("I give up", 2, CENTRE_HOR, 3, false,  true);
                    EEPROM.writeByte(OTA_MODE_OFFSET, false);
                    EEPROM.writeByte(OTA_COUNT, 0); 
                    EEPROM.commit();
                    delay(1000);
                }
                else
                {
                    writeDisplay("Failed",            2, CENTRE_HOR, 1,  true, false);
                    writeDisplay("Re-trying",         2, CENTRE_HOR, 2, false, false);
                    writeDisplay(String(otaCount),    2, CENTRE_HOR, 4, false,  true); 
                    EEPROM.writeByte(OTA_MODE_OFFSET, true);   
                    EEPROM.writeByte(OTA_COUNT, otaCount);         
                    EEPROM.commit();
                    delay(1000);
                }
                
                ESP.restart();
            }
        }

        // Connection Succeeded
        Serial.println("");
        Serial.println("Connected to " + String(otaSSID));
        writeDisplay("WiFi",      2, CENTRE_HOR, 2,  true, false);
        writeDisplay("Connected", 2, CENTRE_HOR, 3, false,  true);
        delay(1000);
        
        // Execute OTA Update
        execOTA();
        //writeDisplay("Updated", 2, CENTRE_HOR, 2,  true, false);
        //writeDisplay(":-)",     2, CENTRE_HOR, 4, false,  true);
        //delay(1500);
    }
    
    else
    {
        Serial.println("----------\nRun Mode\n----------");

        //DO NOT TOUCH
        //  This is here to force the ESP32 to reset the WiFi and initialise correctly.
        Serial.print("WIFI status = ");
        Serial.println(WiFi.getMode());
        WiFi.disconnect(true);
        delay(1000);
        // End silly stuff !!!
            
        //Start the access point
        WiFi.mode(WIFI_AP_STA);
        Serial.print("WIFI status = ");
        Serial.println(WiFi.getMode());
        WiFi.softAP(ssid, password);
        server.begin();
        WiFi.softAPConfig(IPAddress(192, 168, 42, 42), IPAddress(192, 168, 42, 42), IPAddress(255, 255, 0, 0));
        WiFi.begin();

        // Splash the Logo
        display.clearDisplay();
        display.drawXBitmap(0, 0, CombobulatorLogo, 128, 64, WHITE);
        display.display();
        delay (1250);
        display.clearDisplay();
        writeDisplay("v" + String(BUILD_NUMBER), 2, CENTRE_HOR, CENTRE_VER, true, true);
        delay(750);
       #ifdef DEBUG_LOCAL  
        writeDisplay("DeBug Build", 1, CENTRE_HOR, 7, false, true);
        delay(1500);
       #endif

        //writeDisplay("Offline", 2, CENTRE_HOR, CENTRE_VER, true, false);
        //writeDisplay("Battery =" + String(BatteryVoltage()) + " v", 1, CENTRE_HOR, 8, false, true);
  //rgbLED(0,0,1);

        //IR config
        #ifdef RMT_MODE
            Serial.println("\nConfiguring IR Rx...");
            bool rxPinOk = irRx.ESP32_IRrxPIN(RX_PIN, RX_CHAN);
            Serial.println("Initializing IR Rx...");
            irRx.initReceive();
            if(rxPinOk) Serial.println("Rx Init complete");
            else        Serial.println("Rx Init failed");
            
            Serial.println("\nConfiguring IR Tx...");
            bool txPinOk = irTx.ESP32_IRtxPIN(TX_PIN,TX_CHAN);
            Serial.println("Initializing IR Tx...");
            irTx.initTransmit(); //setup the ESP32 to send IR code
            if(txPinOk) Serial.println("Tx Init complete\n");
            else        Serial.println("Tx Init failed");
        #else
            lazerTagReceive.enableIRIn(true);
            lazerTagReceive.resume();     
            void processSignature(decode_results *results);
        #endif

    }
}
