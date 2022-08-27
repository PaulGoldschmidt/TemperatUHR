/*
 * TEMPERATUHR NEXT GENERATION (TemperatUHR NG / Version 2.0)
 * Software Version: 0.1-PRE
 * Copyright Paul Goldschmidt, August 2022 - Heidelberg. https://paul-goldschmidt.de/
 * Acknowledgements: https://github.com/PaulGoldschmidt/TemperatUHR/2_DOCUMENTATION/acknogledments.md
 */

const char* addfooter = "<footer><p>Ein Projekt von <a href='https://paul-goldschmidt.de/TemperatUHR'>Paul Goldschmidt</a>. <p>Lizenzen und rechtliche Hinweise: <a href='https://github.com/PaulGoldschmidt/TemperatUHR'>GitHub</a>. Software Version 0.1-PRE</p></footer>";
const char* addstyle = "<style>body { font-family: Verdana; }</style>";

void connectWifi() {
  Serial.println("Connecting as wifi client...");
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  int connRes = WiFi.waitForConnectResult();
  Serial.print("connRes: ");
  Serial.println(connRes);
}

void loadCredentials() {
  EEPROM.begin(512);
  EEPROM.get(0, ssid);
  EEPROM.get(0 + sizeof(ssid), password);
  char ok[2 + 1];
  EEPROM.get(0 + sizeof(ssid) + sizeof(password), ok);
  EEPROM.end();
  if (String(ok) != String("OK")) {
    ssid[0] = 0;
    password[0] = 0;
  }
  Serial.println("Recovered credentials:");
  Serial.println(ssid);
  Serial.println(strlen(password) > 0 ? "********" : "<no password>");
}


/** Store WLAN credentials to EEPROM */
void saveCredentials() {
  EEPROM.begin(512);
  EEPROM.put(0, ssid);
  EEPROM.put(0 + sizeof(ssid), password);
  char ok[2 + 1] = "OK";
  EEPROM.put(0 + sizeof(ssid) + sizeof(password), ok);
  EEPROM.commit();
  EEPROM.end();
}

/** Is this an IP? */
boolean isIp(String str) {
  for (int i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

String GetEncryptionType(byte thisType) {
  String Output = "";
  // read the encryption type and print out the name:
  switch (thisType) {
    case ENC_TYPE_WEP:
      Output = "WEP";
      return Output;
      break;
    case ENC_TYPE_TKIP:
      Output = "WPA";
      return Output;
      break;
    case ENC_TYPE_CCMP:
      Output = "WPA2";
      return Output;
      break;
    case ENC_TYPE_NONE:
      Output = "None";
      return Output;
      break;
    case ENC_TYPE_AUTO:
      Output = "Auto";
      return Output;
      break;
  }
}

/** IP to String? */
String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}


String formatBytes(size_t bytes) { // lesbare Anzeige der Speichergrößen
  if (bytes < 1024) {
    return String(bytes) + " Byte";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + " KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + " MB";
  }
}

void handleRoot() {
  // Main Page:
  // FSInfo fs_info;
  temp = "";
  short PicCount = 0;
  byte ServArgs = 0;

  //Building Page
  // HTML Header
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  // HTML Content
  server.send ( 200, "text/html", temp ); // Speichersparen - Schon mal dem Cleint senden
  temp = "";
  temp += "<!DOCTYPE HTML><html lang='de'><head><meta charset='UTF-8'><meta name= viewport content='width=device-width, initial-scale=1.0,'>";
  temp += addstyle;
  server.sendContent(temp);
  temp = "";
  temp += "<style type='text/css'><!-- DIV.container { min-height: 10em; display: table-cell; vertical-align: middle }.button {height:35px; width:90px; font-size:16px}";
  server.sendContent(temp);
  temp = "";
  temp += "body {background-color: powderblue;}</style>";
  temp += "<head><title>Captive Portal</title></head>";
  temp += "<h2>Captive Portal</h2>";
  temp += "<body>";
  server.sendContent(temp);
  temp = "";
  temp += "<br><table border=2 bgcolor = white width = 500 cellpadding =5 ><caption><p><h3>Sytemlinks:</h2></p></caption>";
  temp += "<tr><th><br>";
  temp += "<a href='/wifi'>WIFI Einstellungen</a><br><br>";
  temp += "</th></tr></table><br><br>";
  temp += "<footer><p>Ein Projekt von <a href='https://paul-goldschmidt.de/TemperatUHR'>Paul Goldschmidt</a>. <p>Lizenzen und rechtliche Hinweise: <a href='https://github.com/PaulGoldschmidt/TemperatUHR'>GitHub</a>. Software Version 0.1-PRE</p></footer>";
  temp += "</body></html>";
  server.sendContent(temp);
  temp = "";
  server.client().stop(); // Stop is needed because we sent no content length
}



void handleNotFound() {
  if (captivePortal())
  { // If caprive portal redirect instead of displaying the error page.
    return;
  }

  temp = "";
  // HTML Header
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  // HTML Content
  temp += "<!DOCTYPE HTML><html lang='de'><head><meta charset='UTF-8'><meta name= viewport content='width=device-width, initial-scale=1.0,'>";
  temp += "<style type='text/css'><!-- DIV.container { min-height: 10em; display: table-cell; vertical-align: middle }.button {height:35px; width:90px; font-size:16px}";
  temp += "body {background-color: powderblue;}</style>";
  temp += "<head><title>File not found</title></head>";
  temp += "<h2> 404 File Not Found</h2><br>";
  temp += "<h4>Debug Information:</h4><br>";
  temp += "<body>";
  temp += "URI: ";
  temp += server.uri();
  temp += "\nMethod: ";
  temp += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  temp += "<br>Arguments: ";
  temp += server.args();
  temp += "\n";
  for ( uint8_t i = 0; i < server.args(); i++ ) {
    temp += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }
  temp += "<br>Server Hostheader: " + server.hostHeader();
  for ( uint8_t i = 0; i < server.headers(); i++ ) {
    temp += " " + server.headerName ( i ) + ": " + server.header ( i ) + "\n<br>";
  }
  temp += "</table></form><br><br><table border=2 bgcolor = white width = 500 cellpadding =5 ><caption><p><h2>You may want to browse to:</h2></p></caption>";
  temp += "<tr><th>";
  temp += "<a href='/'>Main Page</a><br>";
  temp += "<a href='/wifi'>WIFI Settings</a><br>";
  temp += "</th></tr></table><br><br>";
  temp += addfooter;
  temp += "</body></html>";
  server.send ( 404, "", temp );
  server.client().stop(); // Stop is needed because we sent no content length
  temp = "";

}

/** Redirect to captive portal if we got a request for another domain. Return true in that case so the page handler do not try to handle the request again. */
boolean captivePortal() {
  if (!isIp(server.hostHeader()) && server.hostHeader() != (String(myHostname) + ".local")) {
    Serial.println("Request redirected to captive portal");
    server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
    server.send(302, "text/plain", "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
    server.client().stop(); // Stop is needed because we sent no content length
    return true;
  }
  return false;
}



/** Wifi config page handler */
void handleWifi()
{
  // Page: /wifi
  byte i;
  byte len ;
  temp = "";
  // Check for Site Parameters
  if (server.hasArg("Reboot") ) // Reboot System
  {
    temp = "Rebooting System in 5 Seconds..";
    server.send ( 200, "text/html", temp );
    delay(5000);
    server.client().stop();
    WiFi.disconnect();
    delay(1000);
    pinMode(D6, OUTPUT);
    digitalWrite(D6, LOW);
  }
  // HTML Header
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  // HTML Content
  temp += addstyle;
  temp += "<!DOCTYPE HTML><html lang='de'><head><meta charset='UTF-8'><meta name= viewport content='width=device-width, initial-scale=1.0,'>";
  server.send ( 200, "text/html", temp );
  temp = "";
  temp += "<style type='text/css'><!-- DIV.container { min-height: 10em; display: table-cell; vertical-align: middle }.button {height:35px; width:90px; font-size:16px}";
  temp += "body {background-color: powderblue;}</style><head><title>TemperatUHR - WLAN Configuration</title></head>";
  server.sendContent(temp);
  temp = "";
  temp += "<h2>TemperatUHR - WLAN Configuration</h2><body><left>";
  temp += "<table border=2 bgcolor = white width = 500 ><td><h4>Current WiFi Settings: </h4>";
  if (server.client().localIP() == apIP) {
    temp += "Mode: Soft Access Point (AP)<br>";
    temp += "SSID: " + String (softAP_ssid) + "<br><br>";
  } else {
    temp += "Mode: Station (STA) <br>";
    temp += "SSID: " + String (ssid) + "<br>";
    temp += "BSSID: " + WiFi.BSSIDstr() + "<br><br>";
  }
  temp += "</td></table><br>";
  server.sendContent(temp);
  temp = "";
  temp += "<form action='/wifi' method='post'>";
  temp += "<table border=2 bgcolor = white width = 500><tr><th><br>";
  temp += "Available WiFi Networks:<table border=2 bgcolor = white ></tr></th><td>Number </td><td>SSID </td><td>Encryption </td><td>WiFi Strength </td>";
  server.sendContent(temp);
  temp = "";
  WiFi.scanDelete();
  int n = WiFi.scanNetworks(false, false); //WiFi.scanNetworks(async, show_hidden)
  if (n > 0) {
    for (int i = 0; i < n; i++) {
      temp += "</tr></th>";
      String Nrb = String(i);
      temp += "<td>" + Nrb + "</td>";
      temp += "<td>" + WiFi.SSID(i) + "</td>";

      Nrb = GetEncryptionType(WiFi.encryptionType(i));
      temp += "<td>" + Nrb + "</td>";
      temp += "<td>" + String(WiFi.RSSI(i)) + "</td>";
    }
  } else {
    temp += "</tr></th>";
    temp += "<td>1 </td>";
    temp += "<td>No WLAN found</td>";
    temp += "<td> --- </td>";
    temp += "<td> --- </td>";
  }
  temp += "</table><table border=2 bgcolor = white ></tr></th><td>Connect to WiFi SSID: </td><td><select name='n'>";
  if (n > 0) {
    for (int i = 0; i < n; i++) {
      temp += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + "</option>";
    }
  } else {
    temp += "<option value='No_WiFi_Network'>No WiFi Network found!/option>";
  }
  server.sendContent(temp);
  temp = "";
  temp += "</select></td></tr></th></tr></th><td>WiFi Password: </td><td>";
  temp += "<input type='text' name='p' maxlength='40' size='40'>";
  temp += "</td></tr></th><br></th></tr></table></table>";
  server.sendContent(temp);
  temp = "";
  temp += "<br><button type='submit' name='Settings' value='1' style='height: 50px; width: 140px' autofocus>Set WiFi Settings</button>";
  temp += "<button type='submit' name='Reboot' value='1' style='height: 50px; width: 200px' >Reboot System</button>";
  server.sendContent(temp);
  temp = "";
  temp += "<button type='reset' name='action' value='1' style='height: 50px; width: 100px' >Reset</button></form>";
  temp += "<table border=2 bgcolor = white width = 500 cellpadding =5 ><caption><p><h3>Sytemlinks:</h2></p></caption><tr><th><br>";
  server.sendContent(temp);
  temp = "";
  temp += "<a href='/'>Main Page</a><br><br></th></tr></table><br><br>";
  temp += addfooter;
  temp += "</body></html>";
  server.sendContent(temp);
  server.client().stop(); // Stop is needed because we sent no content length
  temp = "";
}

/** Handle the WLAN save form and redirect to WLAN config page again */
void handleWifiSave() {
  Serial.println("wifi save");
  server.arg("n").toCharArray(ssid, sizeof(ssid) - 1);
  server.arg("p").toCharArray(password, sizeof(password) - 1);
  server.sendHeader("Location", "wifi", true);
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(302, "text/plain", "");    // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.client().stop(); // Stop is needed because we sent no content length
  saveCredentials();
  connect = strlen(ssid) > 0; // Request WLAN connect with new credentials if there is a SSID
}

#define SD_BUFFER_PIXELS 20
