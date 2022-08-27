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
#ifndef APSSID
#define APSSID "TemperatUHR"
#define APPSK  ""
#endif

const char *softAP_ssid = APSSID;

/* hostname for mDNS. Should work at least on windows. Try http://esp8266.local */
const char *myHostname = "temperatuhr";

/* Don't set this wifi credentials. They are configurated at runtime and stored on EEPROM */
char ssid[33] = "";
char password[65] = "";

// DNS server
const byte DNS_PORT = 53;
DNSServer dnsServer;

// Web server
ESP8266WebServer server(80);

/* Soft AP network parameters */
IPAddress apIP(172, 217, 28, 1);
IPAddress netMsk(255, 255, 255, 0);


/** Should I connect to WLAN asap? */
boolean connect;

/** Last time I tried to connect to WLAN */
unsigned long lastConnectTry = 0;

/** Current WLAN status */
unsigned int status = WL_IDLE_STATUS;

String temp;

/* Arduino IOT Cloud Configuration */
const char DEVICE_LOGIN_NAME[]  = "";
const char DEVICE_KEY[]  = "";    // Secret device password

/* MISC Variables */
bool firstinternet = false;
bool internetavailable = false;
const IPAddress remote_ip(8, 8, 8, 8);

void setup()
{
  Serial.begin(115200);
  pinMode(BUILTIN_LED0, OUTPUT); // Initialize the BUILTIN_LED1 pin as an output
  pinMode(RED_LED, OUTPUT); // Initialize the red pin as an output
  pinMode(GREEN_LED, OUTPUT); // Initialize the green pin as an output
  pinMode(BLUE_LED, OUTPUT); // Initialize the blue pin as an output
  digitalWrite(RED_LED, HIGH); // Pull to HIGH: Led OFF
  digitalWrite(GREEN_LED, HIGH); // Pull to HIGH: Led OFF
  digitalWrite(BLUE_LED, HIGH); // Pull to HIGH: Led OFF
  
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(softAP_ssid);
  delay(500); // Without delay I've seen the IP address blank
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  
  /* Setup the DNS server redirecting all the domains to the apIP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);
  /* Setup web pages: root, wifi config pages, SO captive portal detectors and not found. */
  server.on("/", handleRoot);
  server.on("/wifi", handleWifi);
  server.on("/wifisave", handleWifiSave);
  server.on("/generate_204", handleRoot);  //Android captive portal. Maybe not needed. Might be handled by notFound handler.
  server.on("/fwlink", handleRoot);  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
  server.onNotFound(handleNotFound);
  server.begin(); // Web server start
  Serial.println("HTTP server started");
  loadCredentials(); // Load WLAN credentials from network
  connect = strlen(ssid) > 0; // Request WLAN connect if there is a SSID
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
if (connect) {
    Serial.println("Connect requested");
    connect = false;
    connectWifi();
    lastConnectTry = millis();
  }
  {
    unsigned int s = WiFi.status();
    if (s == 0 && millis() > (lastConnectTry + 60000)) {
      /* If WLAN disconnected and idle try to connect */
      /* Don't set retry time too low as retry interfere the softAP operation */
      connect = true;
    }
    if (status != s) { // WLAN status change
      Serial.print("Status: ");
      Serial.println(s);
      status = s;
      if (s == WL_CONNECTED) {
        /* Just connected to WLAN */
        Serial.println("");
        Serial.print("Connected to ");
        Serial.println(ssid);
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());

        // Setup MDNS responder
        if (!MDNS.begin(myHostname)) {
          Serial.println("Error setting up MDNS responder!");
        } else {
          Serial.println("mDNS responder started");
          // Add service to MDNS-SD
          MDNS.addService("http", "tcp", 80);
        }
      } else if (s == WL_NO_SSID_AVAIL) {
        WiFi.disconnect();
      }
    }
    if (s == WL_CONNECTED) {
      MDNS.update();
    }
  }
  
  // Do work:
  //DNS
  dnsServer.processNextRequest();
  //HTTP
  server.handleClient();

  if (internetavailable) {
    //Serial.println("Worker process: Cloud");
  }
  //  ArduinoCloud.update();
}
