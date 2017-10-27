#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <IRremote.h>
#include "Logo.h"

//OLED
#define OLED_SDA 4
#define OLED_SCL 15
Adafruit_SH1106 display(OLED_SDA, OLED_SCL);

//WiFi
const char  *ssid = "CombobulatorMkII";
const char  *password = "Lasertag42";
const int   LED_PIN = 25;
WiFiServer  server(8000);
WiFiClient  client;

//Global Variables
String          dataIn              = "";
String          dataOut             = "";
int             irTraffic           = false;
//unsigned long   txStartTime         = micros();
//unsigned long   rxStartTime         = micros();
bool            rxIndicatorOn       = false;
bool            txIndicatorOn       = false;
int             vCursor             = 0;
int             hCursor             = 0;

//Display constants
#define     LEFT_HOR                11
#define     CENTRE_HOR              12
#define     RIGHT_HOR               13
#define     CENTRE_VER              0
#define     CHAR_WIDTH              6
#define     CHAR_HEIGHT             8
#define     ASTERISK_RX             0
#define     ASTERISK_TX             112
//#define     ASTERISK_FLASH_TIME     50

//IR Library
#define     TYPE_LAZERTAG_BEACON    6000
#define     TYPE_LAZERTAG_TAG       3000
#define     RECEIVE_PIN             17
#define     TX_PIN                  5


IRrecv          lazerTagReceive(RECEIVE_PIN);
IRsend          lazerTagSend;
#define         SERIAL_BUFFER_SIZE 64
char            serialBuffer[SERIAL_BUFFER_SIZE];
long unsigned   TimeSinceLast;
decode_results  results;
String          fullRxMessage = "";
byte            rxCalculatedCheckSum = 0;
int             rxErrorCount = 0;
/////////////////////////////////////////////////////////////////////////

// First Commit to GitHub.
// Definitely a WIP and probalby not even working

void setup()
{
    //Setup
    Serial.begin(115200);
    Serial.println("The Combobluator is now running........\n");
    pinMode(LED_PIN, OUTPUT);

    //Start the access point
//  WiFi.config(IPAddress(192,168,0,1));
    WiFi.softAP(ssid, password);
    server.begin();

    //IR stuff
    lazerTagReceive.enableIRIn();
    pinMode(TX_PIN, OUTPUT);

    //OTA configuration
    // Port defaults to 3232
 // ArduinoOTA.setPort(3232);
    // Hostname defaults to esp3232-[MAC]
 // ArduinoOTA.setHostname("myesp32");
    // No authentication by default
 // ArduinoOTA.setPassword("admin");
    // Password can be set with it's md5 value as well
 // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
 // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

    ArduinoOTA.onStart([]()
    {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
          type = "sketch";
        else // U_SPIFFS
          type = "filesystem";
        Serial.println("Start OTA updating " + type);
    });
    ArduinoOTA.onEnd([]()
    {
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
    {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) 
    {
        Serial.printf("Error[%u]: ", error);
        if      (error == OTA_AUTH_ERROR)    Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR)   Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR)     Serial.println("End Failed");
    });
    ArduinoOTA.begin();
      Serial.println("OTA Ready");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
    
    //Initialise the OLED screen
    pinMode(16, OUTPUT); 
    digitalWrite(16, LOW);                      // set GPIO16 low to reset OLED 
    delay(50); 
    digitalWrite(16, HIGH);                     // while OLED is running, must set GPIO16 to high 
    display.begin(SH1106_SWITCHCAPVCC, 0x3C);   // initialize with the I2C addr 0x3C
    display.setTextColor(WHITE, BLACK);

    // Splash the Logo
    display.clearDisplay();
    display.drawXBitmap(0, 0, CombobulatorLogo, 128, 64, WHITE);
    display.display();
    delay (3500);
    display.clearDisplay();
    writeDisplay("Offline", 2, CENTRE_HOR, CENTRE_VER, true);
}

/////////////////////////////////////////////////////////////////////////

void loop()
{
    ArduinoOTA.handle();
    
    /* listen for client */
    client = server.available(); 
    if (client) 
    {                   
        //Show Connected state
        digitalWrite(LED_PIN, HIGH);
        Serial.println("Connected");
        writeDisplay("Online", 2, CENTRE_HOR, CENTRE_VER, true);

        /* check client is connected */           
        while (client.connected()) 
        {          
            if (lazerTagReceive.decode(&results))
            {
                processIR(&results);
                lazerTagReceive.resume();
            }
            
            if (client.available())
            {
                //turn off * indicators (no delay required as it takes about 100uS to refresh the display).
                // if (micros() - startTime > delay)
                if (txIndicatorOn == true) irDataIndicator(false, ASTERISK_TX);   // turn off Tx indicator
                if (rxIndicatorOn == true) irDataIndicator(false, ASTERISK_RX);    // turn off Rx indicator
                
                char tcpChar = client.read();
                if (tcpChar == '\n')
                {
                    if(dataIn.startsWith("CMD 10", 0))
                    {
                        char irMessageIn[SERIAL_BUFFER_SIZE];
                        dataIn.toCharArray(irMessageIn, SERIAL_BUFFER_SIZE);
                        sendLazerSwarmIR(irMessageIn);      //Send to IR
                        irDataIndicator(true, ASTERISK_TX);
                        //Serial.print("TX: " + dataIn);
                        dataIn = "";
                    }
                    else if(dataIn.startsWith("ltto", 0))
                    {
                        irDataIndicator(true, ASTERISK_TX);
                        char irMessageIn[SERIAL_BUFFER_SIZE];
                        dataIn.toCharArray(irMessageIn, SERIAL_BUFFER_SIZE);
                        sendLttoIR(irMessageIn);      //Send to IR
                        irDataIndicator(false, ASTERISK_TX);
                        //Serial.print("LTTO_TX: " + dataIn);
                        dataIn = "";
                    }
                    
                    else if(dataIn.startsWith("TXT", 0))
                    {
                        int lineNumber = (dataIn.charAt(3)-48);                     //grab the 4th character and convert from ASCII to Decimal number
                        dataIn.remove(0,5);                                         //remove the TXT#: characters
                        bool shallWeClearDisplay = false;
                        if(lineNumber == 1) shallWeClearDisplay = true;             //Clear screen if Line# = 1, else don't
                        writeDisplay(dataIn, 2, CENTRE_HOR, lineNumber, shallWeClearDisplay);
                        //Serial.print("LCD: " + dataIn);
                        dataIn = "";
                    }
                    else if(dataIn.startsWith("DSP",0))
                    {
                        //full direct access to pixels
                        //DSP(xCursor, yCursor, text, fontSize, colour clearDisp)
                        int xCurDelim = dataIn.indexOf(',') + 1;
                        int yCurDelim = dataIn.indexOf(',', xCurDelim + 1) + 1;
                        int textDelim = dataIn.indexOf(',', yCurDelim + 1) + 1;
                        int fontDelim = dataIn.indexOf(',', textDelim + 1) + 1;
                        int colrDelim = dataIn.indexOf(',', fontDelim + 1) + 1;
                        int cDspDelim = dataIn.indexOf(',', colrDelim + 1) + 1;

                        Serial.print("Strings are : ");
                        Serial.print(dataIn.substring(xCurDelim, yCurDelim-1)+" ");
                        Serial.print(dataIn.substring(yCurDelim, textDelim-1)+" ");
                        Serial.print(dataIn.substring(textDelim, fontDelim-1)+" ");
                        Serial.print(dataIn.substring(fontDelim, colrDelim-1)+" ");
                        Serial.print(dataIn.substring(colrDelim, cDspDelim-1)+" ");
                        Serial.println(dataIn.substring(cDspDelim));
                       
                        hCursor         = (dataIn.substring(xCurDelim, yCurDelim-1)).toInt();
                        vCursor         = (dataIn.substring(yCurDelim, textDelim-1)).toInt();
                        String text     = (dataIn.substring(textDelim, fontDelim-1));
                        int fontSize    = (dataIn.substring(fontDelim, colrDelim-1)).toInt();
                        int colour      = (dataIn.substring(colrDelim, cDspDelim-1)).toInt();
                        bool clearDisp  = (dataIn.substring(colrDelim)).toInt();
                        writeDSPdisplay(hCursor, vCursor, text, fontSize, colour, clearDisp);

                        dataIn = "";
                    }
                }
                else
                {
                    dataIn += tcpChar;
                }
            }
//            if (Serial.available() )
//            {
//                char serialChar = Serial.read();
//                if (serialChar == '\n')
//                {
//                    //TODO: send from IR
//                    writeDisplay(dataOut, 1, LEFT_HOR, 3, true);
//                    client.print(dataOut);
//                    Serial.println("Serial Message Out = " + dataOut);
//                    dataOut = "";
//                }
//                else
//                {
//                    dataOut += serialChar;
//                }
//            }
        }
        digitalWrite(LED_PIN, LOW);
        Serial.println("\tDisconnected");
        writeDisplay("Offline", 2, CENTRE_HOR, CENTRE_VER, true);  
    }
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

void writeDisplay(String text, int fontSize, int alignment, int lineNumber, bool erase)
{
    if (erase) display.clearDisplay();
    display.setTextSize(fontSize);
    display.setTextColor(WHITE, BLACK);
    vCursor = ((lineNumber-1)*(fontSize * CHAR_HEIGHT));
    if(lineNumber > 1) vCursor += 1;   // add a gap between lines
    if (lineNumber == CENTRE_VER) vCursor = 32 - ((fontSize * CHAR_HEIGHT)/2);
    int hCursor;
    int numChars = text.length();
    switch (alignment)
    {
        case LEFT_HOR:
            hCursor = 0;
            break;
        case CENTRE_HOR:
            hCursor = 64 - ((numChars)*((fontSize * CHAR_WIDTH)/2));
            break;
        case RIGHT_HOR:
            hCursor = 128 - (numChars*(fontSize * CHAR_WIDTH));
            break;
    }
    
    display.setCursor(hCursor, vCursor);
    display.println(text);
    display.display();
}

void writeDSPdisplay(int hCursor, int vCursor, String text, int fontSize, int colour, bool clearDsp)
{
    if (clearDsp) display.clearDisplay();
    display.display();
    display.setCursor(hCursor, vCursor);
    display.setTextSize(fontSize);
    display.setTextColor(WHITE, BLACK);
    display.println(text);
    display.display();
}

void irDataIndicator(int state, int location)
{    
    return;
    if (state == true)
    {
        display.setTextSize(2);
        display.setCursor(location, 48);
        display.setTextColor(WHITE, BLACK);
        display.println("*");
        display.display();
        if (location == ASTERISK_TX)
        {
            //txStartTime = micros();
            txIndicatorOn = true;
        }
        if (location == ASTERISK_RX)
        {
            //rxStartTime = millis();
            rxIndicatorOn = true;
        }
    }
    else if (state == false)
    {
        display.setTextSize(2);
        display.setCursor(location, 48);
        display.setTextColor(BLACK, BLACK);
        display.println("*");
        display.display();

        if (location == ASTERISK_TX) txIndicatorOn = false;
        if (location == ASTERISK_RX) rxIndicatorOn = false;
    }
}

//----------------------------------------------------------------

void sendLttoIR(char line[])
{
    //Pseudo code
         //dump the ltto
         // while(there is still data to send)
         // {
         //     extract data up to :
         //     send data
         //     wait 20mS
         // } 
}

//----------------------------------------------------------------

void sendLazerSwarmIR(char line[])
{
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    const char *delimiters = " ";

    //check that line starts with CMD
    char *token = strtok (line, delimiters);
    if (token == NULL || strcasecmp(token, "cmd") != 0)
    {
        Serial.print("ERROR Invalid command: ");
        Serial.println(token);
        return;
    }
    
    //check the Command byte != NULL
    token = strtok (NULL, delimiters);
    if (token == NULL)
    {
        Serial.println("ERROR Missing command number.");
        return;    
    }

    //grab the Command byte;
    int command = strtol(token, NULL, 16);
    
    switch (command)
    {
        case 0x10:
        {
            //grab the data byte
            token = strtok (NULL, delimiters);
            if (token == NULL)
            {
                Serial.println("ERROR Missing data parameter.");
                return;    
            }
            short data = strtol(token, NULL, 16);

            //grab the bitCount byte
            token = strtok (NULL, delimiters);
            if (token == NULL)
            {
                Serial.println("ERROR Missing bit count parameter.");
                return;    
            }
            int bitCount = strtol(token, NULL, 16);

            //grab the beacon byte
            token = strtok (NULL, delimiters);
            if (token == NULL)
            {
                Serial.println("ERROR Missing beacon parameter.");
            return;    
            }
            bool isBeacon = (strtol(token, NULL, 16) != 0);

  //IRSend not implemented on ESP32 yet !!!!!    
  //lazerTagSend.sendLTTO(data, bitCount, isBeacon);

            //Convert to LTTO format and send via IRcontrol;
            char packetType;
            if      (bitCount ==9 && data < 256)    packetType = 'P';
            else if (bitCount ==9 && data > 256)    packetType = 'C';
            else if (bitCount ==8 && !isBeacon)     packetType = 'D';
            else if (bitCount ==5 &&  isBeacon)     packetType = 'B';
            else if (bitCount ==7 && !isBeacon)     packetType = 'T';
            else                                    packetType = '_';     // Indicates an error.
            
            Serial.print(F("\nLZ Data sent =\t"));
            Serial.print(data);
//            Serial.print(",");
//            Serial.print(bitCount);
//            Serial.print(",");
//            Serial.print(isBeacon);
//            if(data < 10) Serial.print("\t");
            Serial.print(" -\t");
//            long unsigned CurrentTime = millis();
//            Serial.print(CurrentTime-TimeSinceLast);
//            Serial.print("mS");
//            //if (bitCount == 9 && data > 256) Serial.print("\n");
//            TimeSinceLast = millis();

            lazerTagSend.enableIROut(38);
            
            IRcontrol(packetType, data);
        
            lazerTagReceive.enableIRIn();
            lazerTagReceive.resume();
          
            break;
        }
        default:
        {
            Serial.print("ERROR Invalid command number: ");
            Serial.println(command, HEX);
        }
    }
}

void processIR(decode_results *results)
{
    if (results->address == TYPE_LAZERTAG_TAG)
    // These are Tags, Packets, Data and Checksums
    {
        if      ( (results->bits == 9) && (results->value < 256) )
        // This is a Packet header
        {
            fullRxMessage = "P";
            fullRxMessage += String(results->value);
            rxCalculatedCheckSum = 0;
            digitalWrite(LED_PIN, HIGH);
        }
        else if ( (results->bits == 9) && (results->value > 256) )
        // This is a Checksum
        {
            fullRxMessage += ":C";
            fullRxMessage += String(results->value);
            if (rxCalculatedCheckSum == results->value)    Serial.print("Good Checksum :-)");
            client.println(fullRxMessage);
            Serial.println(fullRxMessage);
            digitalWrite(LED_PIN, LOW);
        }
        else if (results->bits == 8)
        //This is Data
        {
            fullRxMessage += ":D";
            fullRxMessage += String(results->value);
            rxCalculatedCheckSum += results->value; 
        }
        else if (results->bits == 7)
        //This is a Tag
        {
            fullRxMessage = "T";
            fullRxMessage += String(results->value);
            client.println(fullRxMessage);
            Serial.println(fullRxMessage);
        }
    }
    else if (results->address == TYPE_LAZERTAG_BEACON)
    // These are Beacons
    {   
        if      (results->bits == 5)
        //LTTO Beacon
        {
            fullRxMessage = "B";
            fullRxMessage += String(results->value);
            client.println(fullRxMessage);
            Serial.println(fullRxMessage);
        }
        else if (results->bits == 9)
        //LTAR ENhanced Beacon
        {
            fullRxMessage = "E";
            fullRxMessage += String(results->value);
            client.println(fullRxMessage);
            Serial.println(fullRxMessage);
        }
    }
    else
    // Must be an error
    {
        Serial.print(results->address);
        Serial.print("-RAW ");
        for (int i = 0; i < results->rawlen; i++)
        {
            Serial.print(results->rawbuf[i] * USECPERTICK, DEC);
            if (i < (results->rawlen - 1)) Serial.print(" ");
        }
        Serial.println();
        client.println("X" + String(++rxErrorCount));
        return;
    }
    
//    if( (results->bits == 9) && (results->value < 256) ) Serial.print("\n");
//    digitalWrite(13, !digitalRead(13));
//    Serial.print("\tData received = ");
//    Serial.print(results->value, DEC);
//    Serial.print(" ");
//    Serial.print(results->bits, DEC);
//    Serial.print(" ");
//    Serial.print(results->address == TYPE_LAZERTAG_BEACON ? 1 : 0);
//    Serial.println();
//    if( (results->bits == 9) && (results->value > 256) ) Serial.println("\t__________________\n");
//
//    client.print("RCV ");
//    client.print(results->value, HEX);
//    client.print(" ");
//    client.print(results->bits, HEX);
//    client.print(" ");
//    client.print(results->decode_type == TYPE_LAZERTAG_BEACON ? 1 : 0);
//    client.println();
//    digitalWrite(LED_PIN, LOW);
}



