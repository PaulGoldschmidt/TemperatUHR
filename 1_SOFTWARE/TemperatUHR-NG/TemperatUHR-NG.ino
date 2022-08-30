/*
   TEMPERATUHR NEXT GENERATION (TemperatUHR NG / Hardware Version 2.0+)
   Copyright Paul Goldschmidt, August 2022 - Heidelberg. https://paul-goldschmidt.de/
   Acknowledgements: https://github.com/PaulGoldschmidt/TemperatUHR/2_DOCUMENTATION/acknogledments.md
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <BlynkSimpleEsp8266.h>

/* Software Version */
#define SoftwareVer "1.0-STABLE"

/* Set these to your desired softAP credentials. They are not configurable at runtime */

const char *softAP_ssid = "TemperatUHR";

/* hostname for mDNS. Should work at least on windows. Try http://esp8266.local */
const char *myHostname = "temperatUHR";

/* Don't set this wifi credentials. They are configurated at runtime and stored on EEPROM */
char ssid[33] = "";
char password[65] = "";
char auth[110] = "";

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

/* Hardware Configuration */
static const short int BUILTIN_LED0 = D0; //GPIO0: Build-In LED
static const short int SENSOR = D5; // Dallas DS18B20 Data Pin
static const short int RED_LED = D6; // Red LED Pin
static const short int GREEN_LED = D8; // Green LED Pin
static const short int BLUE_LED = D7; // Blue LED Pin

#define ONE_WIRE_BUS 14 //the sensor is connected to pin #14
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float temperature;

/* Blynk Configuration */
#define BLYNK_PRINT Serial //if needed: Debug console
WidgetLED led1(V2); //an virtual LED is in the app "connected" to V2.

unsigned int distancetosensor;
int targettemperature;
bool internetavailable = false;
bool firstinternet = false;
bool temperatuhrstandby = false;
bool cooldownmode = false;
int timestilltarget[5] = {222}; // initalize array with impossible values so that we can firstly check if array has been written to
int positioninarray = 0;
bool blynkconnectionstatus = false;

void setup() {
  delay(100);
  Serial.begin(115200);
  Serial.println("Hallo, world! TemperatUHR starting...");
  pinMode(BUILTIN_LED0, OUTPUT); // Initialize the BUILTIN_LED1 pin as an output
  pinMode(RED_LED, OUTPUT); // Initialize the red pin as an output
  pinMode(GREEN_LED, OUTPUT); // Initialize the green pin as an output
  pinMode(BLUE_LED, OUTPUT); // Initialize the blue pin as an output
  digitalWrite(RED_LED, LOW); // Pull to LOW: Led ON
  digitalWrite(GREEN_LED, LOW); // Pull to LOW: Led ON
  digitalWrite(BLUE_LED, LOW); // Pull to LOW: Led ON
  initCaptive();
  digitalWrite(RED_LED, HIGH); // Pull to HIGH: Led OFF
  digitalWrite(GREEN_LED, HIGH); // Pull to HIGH: Led OFF
  digitalWrite(BLUE_LED, HIGH); // Pull to HIGH: Led OFF
}

void connectWifi() {
  Serial.println("Connecting as wifi client...");
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  int connRes = WiFi.waitForConnectResult();
  Serial.print("connRes: ");
  Serial.println(connRes);
}

void loop() {
  if (!internetavailable) {
    digitalWrite(BLUE_LED, LOW); // Pull to LOW: Led ON
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

  if (firstinternet) {
    digitalWrite(BLUE_LED, HIGH); //Now with internet, we can turn off the blue LED
    Serial.println("Internet first connected, setting up connections.");
    delay(500);
    Blynk.config(auth, "blynk.cloud", 80);
    firstinternet = false;
  }

  if (internetavailable) {
    Serial.println("\nWorker process: Cloud");
    runsensor();
    Blynk.virtualWrite(V0, temperature);
    calctime(); 
    if (strlen(auth) > 30) {
      digitalWrite(GREEN_LED, LOW);
      Blynk.run();
      delay(250);
      digitalWrite(GREEN_LED, HIGH);
    }
    else { //error state 1: no blynk token provided
      digitalWrite(RED_LED, LOW);
      digitalWrite(GREEN_LED, LOW);
      delay(250);
      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(RED_LED, LOW);
    }
  }
  blynkconnectionstatus = Blynk.connected();
  if (blynkconnectionstatus == true) {
    WiFi.softAPdisconnect(true);
  }
  else {
      if (WiFi.softAP(softAP_ssid) == false) {
        WiFi.softAP(softAP_ssid);
      }
  }
  delay(500);
}
