#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
//#include <ArduinoOTA.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <IRremote.h>
#include "Logo.h"

//#define BUILD_NUMBER 1711.25
#define BUILD_NUMBER 42.42

//OLED
#define OLED_SDA 4
#define OLED_SCL 15
Adafruit_SH1106 display(OLED_SDA, OLED_SCL);

//WiFi
const char  *ssid = "Combobulator";
const char  *password = "Lasertag42";
const int   LED_PIN = 25;
WiFiServer  server(8000);
WiFiClient  client;

//Global Variables
String          dataIn              = "";
String          dataOut             = "";
int             irTraffic           = false;
unsigned long   txStartTime         = micros();
unsigned long   rxStartTime         = micros();
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
#define     ASTERISK_FLASH_TIME     50

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
bool            receivingData = false;
unsigned long   rxTimer = millis();
unsigned long   rxTimeOutInterval = 1200;

/////////////////////////////////////////////////////////////////////////

void setup()
{
    //Setup
    Serial.begin(250000);
    delay(100);
    Serial.println("\n");
    Serial.println("The Combobluator is now running........\n");
    Serial.print("Build:");
    Serial.println(BUILD_NUMBER);
    Serial.println("\n");
    pinMode(LED_PIN, OUTPUT);

    //Start the access point
    WiFi.softAP(ssid, password);
//    WiFi.softAPConfig(IPAddress(192, 168 , 4 ,1));
    server.begin();

    //IR stuff
    lazerTagReceive.enableIRIn();
    lazerTagReceive.resume();
    pinMode(TX_PIN, OUTPUT);
    void processSignature(decode_results *results);

    //Setup Button for testing
    pinMode(0, INPUT);


    //LED cWrite Setup
    int freq = 38000;
    int ledChannel = 1;
    int resolution = 8;
    ledcAttachPin(TX_PIN, ledChannel);
    ledcSetup(ledChannel, freq, resolution);
    
//    //OTA configuration
//        // Port defaults to 3232
//        ArduinoOTA.setPort(3232);
//        // Hostname defaults to esp3232-[MAC]
//        ArduinoOTA.setHostname("TheCombobulator");
//        // No authentication by default
//        ArduinoOTA.setPassword("zaphod");
//        // Password can be set with it's md5 value as well
//        // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
//        // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");
//
//        ArduinoOTA.onStart([]()
//        {
//            String type;
//            if (ArduinoOTA.getCommand() == U_FLASH)
//              type = "sketch";
//            else // U_SPIFFS
//              type = "filesystem";
//            Serial.println("Start OTA updating " + type);
//        });
//        ArduinoOTA.onEnd([]()
//        {
//            Serial.println("\nEnd");
//        });
//        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
//        {
//            Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
//        });
//        ArduinoOTA.onError([](ota_error_t error) 
//        {
//            Serial.printf("Error[%u]: ", error);
//            if      (error == OTA_AUTH_ERROR)    Serial.println("Auth Failed");
//            else if (error == OTA_BEGIN_ERROR)   Serial.println("Begin Failed");
//            else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
//            else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
//            else if (error == OTA_END_ERROR)     Serial.println("End Failed");
//        });
//
//    
//    ArduinoOTA.begin();
//      Serial.println("OTA Ready");
//      Serial.print("IP address: ");
//      Serial.println(WiFi.localIP());
    
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
    writeDisplay("v" + String(BUILD_NUMBER), 2, CENTRE_HOR, CENTRE_VER, true);
}

/////////////////////////////////////////////////////////////////////////


