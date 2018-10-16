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

#define         BUILD_NUMBER        1810.16    // 1810.10     1.11


//#define       DEBUG_LOCAL   //N.B. this can cause the LCD screen to go blank due to code in setIrReceivingState()

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
unsigned long   lastHostTime            = millis();
unsigned long   batteryTestTimer        = millis();
unsigned long   timeSinceLastBattCheck  = millis();
#define         SERIAL_BUFFER_SIZE      64
String          fullRxMessage           = "";
byte            rxCalculatedCheckSum    = 0;
int             rxErrorCount            = 0;
bool            receivingData           = false;
unsigned long   rxTimer                 = millis();
unsigned long   rxTimeOutInterval       = 1200;
char            serialBuffer            [SERIAL_BUFFER_SIZE];
unsigned int    ledIntensity            = 5;

//debug
#define         START_DEBUG             1
#define         STOP_DEBUG              0
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
#define         irFreq                  38000
#define         ledFreq                 5000
#define         ledChannel              1
#define         redChannel              2
#define         greenChannel            3
#define         blueChannel             4
#define         resolution              8

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
    runSetup;
}

/////////////////////////////////////////////////////////////////////////
