/*
   TEMPERATUHR NEXT GENERATION (TemperatUHR NG / Version 2.0)
   Software Version: 0.1-PRE
   Copyright Paul Goldschmidt, August 2022 - Heidelberg. https://paul-goldschmidt.de/
   Acknowledgements: https://github.com/PaulGoldschmidt/TemperatUHR/blob/main/2_DOCUMENTATION/acknowledgements.md
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

/* Hardware Configuration */
static const short int BUILTIN_LED0 = D0; //GPIO0
static const short int SENSOR = D5; // Dallas DS18B20 Data Pin
static const short int RED_LED = D6; // Red LED Pin
static const short int GREEN_LED = D8; // Green LED Pin
static const short int BLUE_LED = D7; // Blue LED Pin

#define ONE_WIRE_BUS 14 //the sensor is connected to pin #14
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

/* WIFI Configuration */
static const byte WiFiPwdLen = 25;
static const byte APSTANameLen = 20;

struct WiFiEEPromData
{
  bool APSTA = true; // Access Point or Sation Mode - true AP Mode
  bool PwDReq = false; // PasswordRequired
  bool CapPortal = true ; //CaptivePortal on in AP Mode
  char APSTAName[APSTANameLen]; // STATION /AP Point Name TO cONNECT, if definded
  char WiFiPwd[WiFiPwdLen]; // WiFiPAssword, if definded
  char ConfigValid[3]; //If Config is Vaild, Tag "TK" is required"
};

/* hostname for mDNS. Should work at least on windows. Try http://esp8266.local */
const char *ESPHostname = "ESP";

// DNS server
const byte DNS_PORT = 53;
DNSServer dnsServer;

//Conmmon Paramenters
bool SoftAccOK = false;

// Web server
ESP8266WebServer server(80);

/* Soft AP network parameters */
IPAddress apIP(172, 20, 0, 1);
IPAddress netMsk(255, 255, 255, 0);


/** Should I connect to WLAN asap? */
boolean connect;

/** Last time I tried to connect to WLAN */
//long lastConnectTry = 0;
unsigned long currentMillis = 0;
unsigned long startMillis;
const short period = 10; // Sleep after this Minutes of inactivity

/** Current WLAN status */
short status = WL_IDLE_STATUS;

WiFiEEPromData MyWiFiConfig;
String temp = "";

/* Arduino IOT Cloud Configuration */
const char DEVICE_LOGIN_NAME[]  = "";
const char DEVICE_KEY[]  = "";    // Secret device password

/* MISC Variables */
bool firstinternet = false;
bool internetavailable = false;
const IPAddress remote_ip(8, 8, 8, 8);

void setup()
{
  bool ConnectSuccess = false;
  bool CreateSoftAPSucc = false;
  bool CInitFSSystem = false;
  bool CInitHTTPServer = false;
  byte len;
  pinMode(BUILTIN_LED0, OUTPUT); // Initialize the BUILTIN_LED1 pin as an output
  pinMode(RED_LED, OUTPUT); // Initialize the red pin as an output
  pinMode(GREEN_LED, OUTPUT); // Initialize the green pin as an output
  pinMode(BLUE_LED, OUTPUT); // Initialize the blue pin as an output
  digitalWrite(RED_LED, HIGH); // Pull to HIGH: Led OFF
  digitalWrite(GREEN_LED, HIGH); // Pull to HIGH: Led OFF
  digitalWrite(BLUE_LED, HIGH); // Pull to HIGH: Led OFF
  Serial.begin(115200);
  Serial.println("TemperatUHR Starting...");
  WiFi.hostname(ESPHostname); // Set the DHCP hostname assigned to ESP station.
  if (loadCredentials()) // Load WLAN credentials for WiFi Settings
  {
    // Valid Credentials found.
    if (MyWiFiConfig.APSTA == true) // AP Mode
    {
      Serial.println("STA Mode - loaded credentials.");
      len = strlen(MyWiFiConfig.APSTAName);
      MyWiFiConfig.APSTAName[len + 1] = '\0';
      len = strlen(MyWiFiConfig.WiFiPwd);
      MyWiFiConfig.WiFiPwd[len + 1] = '\0';
      len = ConnectWifiAP();
      if ( len == 3 ) {
        ConnectSuccess = true;
      } else {
        ConnectSuccess = false;
        Serial.print("Connection success:");
        Serial.println(ConnectSuccess);
      }
    }
  } 
  else { //Set default Config - Create AP
    Serial.println("DefaultWiFi Cnf");
    SetDefaultWiFiConfig ();
    CreateSoftAPSucc = CreateWifiSoftAP();
    saveCredentials();
    // Blink
    digitalWrite(D0, LOW); // Pull to LOW _Led ON
    delay(500);
    digitalWrite(D0, HIGH);
    delay(500);
    digitalWrite(D0, LOW); // Pull to LOW _Led ON
    delay(500);
    digitalWrite(D0, HIGH);
    delay(500);
    digitalWrite(D0, LOW); // Pull to LOW _Led ON
  }
  if ((ConnectSuccess or CreateSoftAPSucc) and CInitFSSystem)
  {
    InitalizeHTTPServer();
    digitalWrite(D0, LOW); // Pull to LOW _Led ON
    Serial.println("OK");
  }
  else
  {
    Serial.setDebugOutput(true); //Debug Output for WLAN on Serial Interface.
    Serial.print("Err");
    SetDefaultWiFiConfig ();
    CreateSoftAPSucc = CreateWifiSoftAP();
    saveCredentials();
    InitalizeHTTPServer();
  }
  startMillis = millis(); //initial start time
}

void loop()
{
  // Serial.println("Loop exec"); //For debugging stuck loops
  if (!internetavailable) {
    if (WiFi.status() == WL_CONNECTED) { //is internet available? 
      internetavailable = true;
      firstinternet = true;
    }
  }
  if (SoftAccOK)
  {
    dnsServer.processNextRequest(); //DNS
  }
  //HTTP
  server.handleClient();
  yield();

  if (firstinternet) {
    Serial.println("Internet first connected, setting up connections");
    initProperties();
    firstinternet = false;
  }

  if (internetavailable) {
    //Serial.println("Worker process: Cloud");
  }
  //  ArduinoCloud.update();
}
