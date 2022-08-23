/*
 * TEMPERATUHR NEXT GENERATION (TemperatUHR NG / Version 2.0)
 * Software Version: 0.1-PRE
 * Copyright Paul Goldschmidt, August 2022 - Heidelberg. https://paul-goldschmidt.de/
 * Acknowledgements: https://github.com/PaulGoldschmidt/TemperatUHR/blob/main/2_DOCUMENTATION/acknowledgements.md
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <EEPROM.h>

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

static const short int BUILTIN_LED0 = D0; //GPIO0
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

void setup()
{

  bool ConnectSuccess = false;
  bool CreateSoftAPSucc = false;
  bool CInitFSSystem = false;
  bool CInitHTTPServer = false;
  byte len;
  pinMode(D0, OUTPUT); // Initialize the BUILTIN_LED1 pin as an output
  Serial.begin(9600);
  Serial.println("TemperatUHR Starting...");
  WiFi.hostname(ESPHostname); // Set the DHCP hostname assigned to ESP station.
  if (loadCredentials()) // Load WLAN credentials for WiFi Settings
  {
    // Valid Credentials found.
    if (MyWiFiConfig.APSTA == true) // AP Mode
    {
      //Serial.println("AP Mode");
      //Serial.println(MyWiFiConfig.APSTA);
      len = strlen(MyWiFiConfig.APSTAName);
      MyWiFiConfig.APSTAName[len + 1] = '\0';
      len = strlen(MyWiFiConfig.WiFiPwd);
      MyWiFiConfig.WiFiPwd[len + 1] = '\0';
      CreateSoftAPSucc = CreateWifiSoftAP();
    } else
    {
      //Serial.println("STA Mode");
      len = strlen(MyWiFiConfig.APSTAName);
      MyWiFiConfig.APSTAName[len + 1] = '\0';
      len = strlen(MyWiFiConfig.WiFiPwd);
      MyWiFiConfig.WiFiPwd[len + 1] = '\0';
      len = ConnectWifiAP();
      if ( len == 3 ) {
        ConnectSuccess = true;
      } else {
        ConnectSuccess = false;
      }
    }
  } else
  { //Set default Config - Create AP
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
  if (SoftAccOK)
  {
    dnsServer.processNextRequest(); //DNS
  }
  //HTTP
  server.handleClient();
  yield();
}
