
//  July 2020
// For unknown reasons the Combobulator Arduino code cannot be compiled and run - it crashes every 5-10 seconds
// To programme units for sale, use the ESPTOOL as per the instructions below.

// ESPTOOL - Copy and paste the following commands into a terminal window.
//  cd /Users/Richie/Library/Python/2.7/bin
//  python esptool.py --port /dev/tty.SLAB_USBtoUART erase_flash
//  python esptool.py --port /dev/tty.SLAB_USBtoUART write_flash 0x0 CombobV11.bin
//  (There is a copy of ComboV144.bin in Dropbox/Laser Tag/Combobulator)

        //  F.Y.I. - The CombobV114.bin file (as used above) was created by the following (using a known working donor Combobulator).
        //  cd /Users/Richie/Library/Python/2.7/bin
        //  ls /dev/tty* | grep usb
        //  python esptool.py -p /dev/tty.SLAB_USBtoUART -b 115200 read_flash 0 0x400000 CombobV114xxx.bin

#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Update.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <IRremote.h>
#include <esp32_IR_Ltto.h>
#include "EEPROM.h"
#include "Logo.h"

#define         RMT_MODE
#define         RMT_FAST_MODE

#define         BUILD_NUMBER       2106.02     //  1.15

//#define       DEBUG_LOCAL   //N.B. this can cause the LCD screen to go blank due to code in setIrReceivingState()

//OLED
#define         OLED_SDA                4
#define         OLED_SCL                15
#define         OLED_RESET              16
Adafruit_SH1106 display                 (OLED_SDA, OLED_SCL);

//Display constants
#define         LEFT_HOR                11
#define         CENTRE_HOR              12
#define         RIGHT_HOR               13
#define         CENTRE_VER              0
#define         CHAR_WIDTH              6
#define         CHAR_HEIGHT             8
#define         ASTERISK_RX             0
#define         ASTERISK_TX             112
#define         ASTERISK_FLASH_TIME     50
#define         LED_INTENSITY           5

//LTTO data types
const char  PACKET                      = 'P';
const char  DATA                        = 'D';
const char  CHECKSUM                    = 'C';
const char  TAG                         = 'T';
const char  BEACON                      = 'Z';
const char  LTAR_BEACON                 = 'E';

//WiFi
const char      *ssid                   = "Combobulator";
const char      *password               = "Lasertag42";
const int       LED_PIN                 = 25;
WiFiServer      server(8000);
WiFiClient      client;

//Global Variables
String          dataIn                  = "";
String          dataOut                 = "";
int             irTraffic               = false;
unsigned long   txStartTime             = micros();
unsigned long   rxStartTime             = micros();
bool            rxIndicatorOn           = false;
bool            txIndicatorOn           = false;
int             vCursor                 = 0;
int             hCursor                 = 0;
int             analogInput             = 0;
float           batteryVolts            = 0;
bool            isConnected             = false;
unsigned long   lastHostTime            = millis();
unsigned long   batteryTestTimer        = millis();
#define         SERIAL_BUFFER_SIZE      64
String          fullRxMessage           = "";
byte            rxCalculatedCheckSum    = 0;
int             rxErrorCount            = 0;
bool            receivingData           = false;
unsigned long   rxTimer                 = millis();
unsigned long   rxTimeOutInterval       = 2000;     // In case of no end of packet, this times out Rx after the length of longest possible message.
char            serialBuffer            [SERIAL_BUFFER_SIZE];
int             serialBufferPosition    = 0;
unsigned int    ledIntensity            = LED_INTENSITY;
bool            lazerSwarmMode          = false;
#define         LOW_BATT_VOLTS          2.5

//debug
#define         START_DEBUG             1
#define         STOP_DEBUG              0
unsigned long   howLongDidThisTake      = micros();
bool            processingMessage       = false;

//IR Library
#define         TYPE_LAZERTAG_BEACON    6000
#define         TYPE_LAZERTAG_TAG       3000
#define         RX_PIN                  17
#define         TX_PIN                  12   

//ESP32 IR Library
#define         TX_CHAN     0
#define         RX_CHAN     1

#ifdef RMT_MODE
    ESP32_IR        irRx;
    ESP32_IR        irTx;
    unsigned int    IRdataRx[250]; //holding array for IR code in ms
    rmt_item32_t    IRdataTx[250];
    int             codeLength           = 0;
    bool            isSendingActive      = false;
    bool            expectingNonP2packet = false;
    String          txCheckSum           = "";
#else
    IRrecv          lazerTagReceive(RX_PIN);
    long unsigned   TimeSinceLast;
    decode_results  results;
#endif

//GameHosting
#define         ANNOUNCE_GAME_INTERVAL  1500
bool            isHostingActive = false;
bool            cancelHosting   = false;

//EEPROM
int             eepromAddress           = 0;
#define         EEPROM_SIZE             256
#define         HARDWARE_VERSION        1
#define         OTA_MODE_OFFSET         201
#define         OTA_COUNT               205
#define         SSID_OFFSET             10
#define         PSWD_OFFSET             50

//Combobulator Hardware  
#define         RED_LED                 14
#define         GREEN_LED               26
#define         BLUE_LED                27
#define         BATT_VOLTS              34
#define         BUTTON                  0
#define         OLED_RESET              16
    
//LED cWrite Setup
#define         IR_FREQ                 38000
#define         LED_FREQ                5000
#define         IR_CHANNEL              1
#define         RED_CHANNEL             2
#define         GREEN_CHANNEL           3
#define         BLUE_CHANNEL            4
#define         FADE_RESOLUTION         8

// S3 Bucket Config
String          otaHost                 = "combobulator.s3.ap-southeast-2.amazonaws.com";
int             otaPort                 = 80;           
String          otaFileName             = "/ESP-CombobHost.ino.bin";
bool            otaMode                 = false;
unsigned long   timeOut                 = millis();
uint8_t         otaCount                = 0;

/////////////////////////////////////////////////////////////////////////////

void setup()
{
    //Setup
    Serial.begin(115200);           // This baud rate is required for Lazerswarm compatibility.
    delay(100);
    Serial.println("\n");
    Serial.println("The Combobulator is now running........\n");
    Serial.print("Build:"); Serial.println(BUILD_NUMBER);
   #ifdef DEBUG_LOCAL
    Serial.println("DeBug Build");
   #endif 
    Serial.println("\n");


    //Initiliase the Combobulator Hardware
    pinMode     (LED_PIN,    OUTPUT);
    digitalWrite(LED_PIN,    LOW   );
    pinMode     (BUTTON,     INPUT );
    pinMode     (TX_PIN,     OUTPUT);
    pinMode     (RED_LED,    OUTPUT);
    pinMode     (GREEN_LED,  OUTPUT);    
    pinMode     (BLUE_LED,   OUTPUT);
    pinMode     (BATT_VOLTS, INPUT );

    //Initialise the OLED screen
    pinMode     (OLED_RESET, OUTPUT); 
    digitalWrite(OLED_RESET, LOW   );                     // set GPIO16 low to reset OLED 
    delay       (50);
    digitalWrite(OLED_RESET, HIGH  );                     // while OLED is running, must set GPIO16 to high 
    display.begin(SH1106_SWITCHCAPVCC, 0x3C);             // initialize with the I2C addr 0x3C
    display.setTextColor(WHITE, BLACK);

    //LED cWrite Setup
    ledcAttachPin(TX_PIN,    IR_CHANNEL     );
    ledcAttachPin(RED_LED,   RED_CHANNEL    );
    ledcAttachPin(GREEN_LED, GREEN_CHANNEL  );
    ledcAttachPin(BLUE_LED,  BLUE_CHANNEL   );
    
    ledcSetup(IR_CHANNEL,     IR_FREQ,    FADE_RESOLUTION);
    ledcSetup(RED_CHANNEL,    LED_FREQ,   FADE_RESOLUTION);
    ledcSetup(GREEN_CHANNEL,  LED_FREQ,   FADE_RESOLUTION);
    ledcSetup(BLUE_CHANNEL,   LED_FREQ,   FADE_RESOLUTION);
    
    //Force the LEDs off
    ledcWrite(IR_CHANNEL,     0); 
    ledcWrite(RED_CHANNEL,    0); 
    ledcWrite(GREEN_CHANNEL,  0); 
    ledcWrite(BLUE_CHANNEL,   0);

    analogReadResolution(10);
    
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

    +   WiFi.softAPConfig(IPAddress(192, 168, 42, 42), IPAddress(192, 168, 42, 42), IPAddress(255, 255, 0, 0));
//        IPAddress local_IP(192, 168, 4, 42);
//        IPAddress gateway(192, 168, 4, 1);
//        IPAddress subnet(255, 255, 0, 0);
//        WiFi.config(local_IP, gateway, subnet);
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

        writeDisplay("Offline", 2, CENTRE_HOR, CENTRE_VER, true, true);

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

/////////////////////////////////////////////////////////////////////////
