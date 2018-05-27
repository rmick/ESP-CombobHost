#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Update.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <IRremote.h>
#include "EEPROM.h"
#include "Logo.h"

#define         BUILD_NUMBER            1805.28
//#define       DEBUG_LOCAL

//OLED
#define         OLED_SDA                4
#define         OLED_SCL                15
Adafruit_SH1106 display                 (OLED_SDA, OLED_SCL);

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

//debug
#define         START                   1
#define         STOP                    0
unsigned long   howLongDidThisTake      = micros();
bool            processingMessage       = false;

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

//IR Library
#define         TYPE_LAZERTAG_BEACON    6000
#define         TYPE_LAZERTAG_TAG       3000
#define         RECEIVE_PIN             17
#define         TX_PIN                  14

IRrecv          lazerTagReceive         (RECEIVE_PIN);
long unsigned   TimeSinceLast;
decode_results  results;

#define         SERIAL_BUFFER_SIZE      64
String          fullRxMessage           = "";
byte            rxCalculatedCheckSum    = 0;
int             rxErrorCount            = 0;
bool            receivingData           = false;
unsigned long   rxTimer                 = millis();
unsigned long   rxTimeOutInterval       = 1200;
char            serialBuffer            [SERIAL_BUFFER_SIZE];

//EEPROM
int             eepromAddress           = 0;
#define         EEPROM_SIZE             256
#define         OTA_MODE_OFFSET         1
#define         PSWD_OFFSET             10
#define         SSID_OFFSET             50

//Combobualtor Hardware
#define         RED_LED                 32
#define         GREEN_LED               33
#define         BLUE_LED                36
#define         BATT_VOLTS              34
#define         BUTTON                  0

// S3 Bucket Config
String          otaHost                 = "combobulator.s3.ap-southeast-2.amazonaws.com";
int             otaPort                 = 80;           
String          otaFileName             = "/ESP-CombobHost.ino.bin";
bool            otaMode                 = false;

/////////////////////////////////////////////////////////////////////////////

void setup()
{
    //Setup
    Serial.begin(115200);
    //Serial.setDebugOutput(true);
    delay(100);
    Serial.println("\n");
    Serial.println("The Combobulator is now running........\n");
    Serial.print("Build:");
    Serial.println(BUILD_NUMBER);
   #ifdef DEBUG_LOCAL
    Serial.println("DeBug Build");
   #endif 
    Serial.println("\n");
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    pinMode(BUTTON, INPUT);
    pinMode(TX_PIN, OUTPUT);

    //Initialise the OLED screen
    pinMode(16, OUTPUT); 
    digitalWrite(16, LOW);                      // set GPIO16 low to reset OLED 
    delay(50); 
    digitalWrite(16, HIGH);                     // while OLED is running, must set GPIO16 to high 
    display.begin(SH1106_SWITCHCAPVCC, 0x3C);   // initialize with the I2C addr 0x3C
    display.setTextColor(WHITE, BLACK);

    //LED cWrite Setup
    int freq = 38000;
    int ledChannel = 1;
    int resolution = 8;
    ledcAttachPin(TX_PIN, ledChannel);
    ledcSetup(ledChannel, freq, resolution);

    //Initiliase the Combobulator Hardware
    pinMode(RED_LED,    INPUT_PULLDOWN);
    pinMode(GREEN_LED,  INPUT_PULLDOWN);    
    pinMode(BLUE_LED,   INPUT_PULLDOWN);
    pinMode(BATT_VOLTS, INPUT);
    
    
    //Set up EEPROM
    if (!EEPROM.begin(EEPROM_SIZE))
    {
        Serial.println("failed to initialise EEPROM");
    }
    else
    {
        Serial.println("EEPROM initiliased");

        if(EEPROM.readByte(OTA_MODE_OFFSET) == true)    otaMode = true;
        else                                            otaMode = false;
    }

    if (otaMode)
    {  
        Serial.println("----------\nOTA Mode\n----------");
        
        //Force the IR LEDs off !
        digitalWrite(TX_PIN, LOW);

        writeDisplay("U/G Mode", 2, CENTRE_HOR, 1, true);
        
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

        //Set default to runMode, in case it fails. (i.e. this is one time routine).
        EEPROM.writeByte(OTA_MODE_OFFSET, false);
        EEPROM.commit();
        delay(1000);

        Serial.print("WIFI status = ");
        Serial.println(WiFi.getMode());

        WiFi.disconnect(true);
        delay(1000);
        WiFi.mode(WIFI_STA);
        delay(100);
        
        // Connect to provided SSID and PSWD
        WiFi.begin(otaSSID, otaPSWD);
        Serial.println("Connecting to" + String(otaSSID));
        Serial.println("Credentials:"  + String(otaPSWD));
        
        writeDisplay("Connecting to", 1, CENTRE_HOR, 4, false);
        writeDisplay(otaSSID,         1, CENTRE_HOR, 6, false);

        // Wait for connection to establish
        bool flashDot = false;
        while (WiFi.status() != WL_CONNECTED)
        {
            if (flashDot) writeDisplay(" ", 2, CENTRE_HOR, 4, false);
            else          writeDisplay(".", 2, CENTRE_HOR, 4, false);
            flashDot = !flashDot;
            delay(500);

            if(!digitalRead(BUTTON))
            {
                Serial.print("Button Pressed - Set OTA mode");
                writeDisplay("OTA mode", 2, CENTRE_HOR, 1, false);
                EEPROM.writeByte(OTA_MODE_OFFSET, true);         
                EEPROM.commit();
                delay(1000);
            }
    
        }

        // Connection Succeeded
        Serial.println("");
        Serial.println("Connected to " + String(otaSSID));
        writeDisplay("WiFi",      2, CENTRE_HOR, 2,  true);
        writeDisplay("Connected", 2, CENTRE_HOR, 3, false);
        delay(1000);
        
        // Execute OTA Update
        execOTA();
        writeDisplay("Updated", 2, CENTRE_HOR, 2,  true);
        writeDisplay(":-)",     2, CENTRE_HOR, 4, false);
        delay(1500);
    }
    
    else
    {
        Serial.println("----------\nRun Mode\n----------");
            
        //Start the access point
        WiFi.mode(WIFI_AP_STA);
        WiFi.softAP(ssid, password);
        server.begin();
    
        IPAddress local_IP(192, 168, 4, 42);
        IPAddress gateway(192, 168, 4, 1);
        IPAddress subnet(255, 255, 0, 0);
        WiFi.config(local_IP, gateway, subnet);
        WiFi.begin();

        // Splash the Logo
        display.clearDisplay();
        display.drawXBitmap(0, 0, CombobulatorLogo, 128, 64, WHITE);
        display.display();
        delay (2000);
        display.clearDisplay();
        writeDisplay("v" + String(BUILD_NUMBER), 2, CENTRE_HOR, CENTRE_VER, true);
       #ifdef DEBUG_LOCAL
        delay(750);  
        writeDisplay("DeBugBuild", 2, CENTRE_HOR, 4, false);
       #endif
        delay(1500);
    
//        //Show Battery Voltage
//        display.clearDisplay();
//        writeDisplay("Battery =", 2, CENTRE_HOR, 2, true);
//        writeDisplay(String(BatteryVoltage()) + " v", 2, CENTRE_HOR, 3, false);
//        delay(500);

        writeDisplay("Offline", 2, CENTRE_HOR, CENTRE_VER, true);
        
        lazerTagReceive.enableIRIn(true);
        lazerTagReceive.resume();
        
        void processSignature(decode_results *results);
    }
}

/////////////////////////////////////////////////////////////////////////


