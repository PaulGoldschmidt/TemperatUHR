/*
   TEMPERATUHR NEXT GENERATION (TemperatUHR NG / Version 2.0)
   Software Version: 0.2-PRE
   Copyright Paul Goldschmidt, August 2022 - Heidelberg. https://paul-goldschmidt.de/
   Acknowledgements: https://github.com/PaulGoldschmidt/TemperatUHR/2_DOCUMENTATION/acknogledments.md
*/

/** Load WLAN credentials from EEPROM */
void loadCredentials() {
  EEPROM.begin(512);
  EEPROM.get(0, ssid);
  EEPROM.get(0 + sizeof(ssid), password);
  EEPROM.get(0 + sizeof(ssid) + sizeof(password), auth);
  char ok[2 + 1];
  EEPROM.get(0 + sizeof(ssid) + sizeof(password) + sizeof(auth), ok);
  EEPROM.end();
  if (String(ok) != String("OK")) {
    ssid[0] = 0;
    password[0] = 0;
    auth[0] = 0;
  }
  Serial.println("Recovered credentials:");
  Serial.println(ssid);
  Serial.println(strlen(auth) > 10 ? "********" : "<no auth>");
  Serial.println(strlen(password) > 0 ? "********" : "<no password>");
}

/** Store WLAN credentials to EEPROM */
void saveCredentials() {
  EEPROM.begin(512);
  EEPROM.put(0, ssid);
  EEPROM.put(0 + sizeof(ssid), password);
  EEPROM.put(0 + sizeof(ssid) + sizeof(password), auth);
  char ok[2 + 1] = "OK";
  EEPROM.put(0 + sizeof(ssid) + sizeof(password) + sizeof(auth), ok);
  EEPROM.commit();
  EEPROM.end();
}

void initCaptive() {
  Serial.println();
  Serial.println("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
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

/** Handle root or redirect to captive portal */
void handleRoot() {
  if (captivePortal()) { // If caprive portal redirect instead of displaying the page.
    return;
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");

  String Page;
  Page += F(
            "<!DOCTYPE html><html lang='en'><head>"
            "<meta name='viewport' content='width=device-width'>"
            "<title>TemperatUHR Configuration</title>"
            "<style>"
            "p {"
            "text-align: center;"
            "font-family: Arial, Helvetica, sans-serif;"
            "}"
            " h1 {"
            "text-align: center;"
            "font-family: Arial, Helvetica, sans-serif;"
            "}"
            "</style>"
            "</head><body>"
            "<h1>TemperatUHR Configuration Webapp</h1>");
  if (server.client().localIP() == apIP) {
    Page += String(F("<p>You are connected through the soft AP: ")) + softAP_ssid + F("</p>");
  } else {
    Page += String(F("<p>You are connected through the wifi network: ")) + ssid + F("</p>");
  }
  Page += F(
            "<p>You may want to <a href='/wifi'>config the wifi connection and add an Blynk Token</a>.</p>"
            "</body></html>");

  server.send(200, "text/html", Page);
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
void handleWifi() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");

  String Page;
  Page += F(
            "<!DOCTYPE html><html lang='en'><head>"
            "<style>"
            "p {"
            "text-align: center;"
            "font-family: Arial, Helvetica, sans-serif;"
            "}"
            " h1 {"
            "text-align: center;"
            "font-family: Arial, Helvetica, sans-serif;"
            "}"
            " table {"
            "text-align: center;"
            "margin-left:auto;"
            "margin-right:auto;"
            "font-family: Arial, Helvetica, sans-serif;"
            "}"
            " form {"
            "text-align: center;"
            "font-family: Arial, Helvetica, sans-serif;"
            "}"
            " input, textarea, select, button {"
            "padding: 0px;"
            "margin: 3px;"
            "box-sizing: border-box;"
            "}"
            " th, td {"
            "padding: 5px;"
            "}"
            " footer {"
            "font-size: small"
            "}"
            "</style>"
            "<meta name='viewport' content='width=device-width'>"
            "<title>TemperatUHR Credentials Configuration</title></head><body>"
            "<h1>TemperatUHR Credentials Configuration</h1>");
  if (server.client().localIP() == apIP) {
    Page += String(F("<p>You are connected through the soft AP: ")) + softAP_ssid + F("</p>");
  } else {
    Page += String(F("<p>You are connected through the wifi network: ")) + ssid + F("</p>");
  }
  Page +=
    String(F(
             "\r\n<br />"
             "<table><tr><th>SoftAP Information</th></tr>"
             "<tr><td>SSID ")) +
    String(softAP_ssid) +
    F("</td></tr>"
      "<tr><td>IP ") +
    toStringIp(WiFi.softAPIP()) +
    F("</td></tr>"
      "</table>"
      "\r\n<br />"
      "<table><tr><th>WLAN Configuration  </th></tr>"
      "<tr><td>SSID: ") +
    String(ssid) +
    F("</td></tr>"
      "<tr><td>IP ") +
    toStringIp(WiFi.localIP()) +
    F("</td></tr>"
      "</table>"
      "\r\n<br />"
      "<table><tr><th>WLAN list (refresh if any missing)</th></tr>");
  Serial.println("scan start");
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n > 0) {
    for (int i = 0; i < n; i++) {
      Page += String(F("\r\n<tr><td>SSID ")) + WiFi.SSID(i) + ((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? F(" ") : F(" *")) + F(" (") + WiFi.RSSI(i) + F(")</td></tr>");
    }
  } else {
    Page += F("<tr><td>No WLAN found</td></tr>");
  }
  Page += F(
            "</table>"
            "\r\n<br /><form method='POST' action='wifisave'><h4>Connect to network:</h4>"
            "<select name='n'>");
            if (n > 0) {
            for (int i = 0; i < n; i++) {
  Page += String(F("<option value='")) + WiFi.SSID(i) + String(F("'>")) + WiFi.SSID(i) + String(F("</option>"));
            }
            }
  Page += F(
            "</select>"
            "<br /><input type='password' placeholder='Password' name='p'/>"
            "<br /><input type='text' placeholder='Blynk Auth Token' name='b'/>"
            "<br /><br /><input type='submit' value='Connect/Disconnect'/></form>"
            "<p>You may want to <a href='/'>return to the home page</a>.</p>"
            "<footer>"
            "<p>TemperatUHR: A project by Paul Goldschmidt<br>");
  Page += String(F("In cooperation with the <a href=\"https://www.paul-award.de/\">PAUL AWARD</a> / <a href=\"https://www.fed.de/\">FED e.V.</a> | Software Version: <a href=\"https://github.com/PaulGoldschmidt/TemperatUHR/releases/\">")) + String(SoftwareVer) + String(F("</a></p>"));
  Page += F(
            "</footer>"
            "</body></html>");
  server.send(200, "text/html", Page);
  server.client().stop(); // Stop is needed because we sent no content length
}

/** Handle the WLAN save form and redirect to WLAN config page again */
void handleWifiSave() {
  Serial.println("wifi save");
  server.arg("n").toCharArray(ssid, sizeof(ssid) - 1);
  server.arg("p").toCharArray(password, sizeof(password) - 1);
  server.arg("b").toCharArray(auth, sizeof(auth) - 1);
  server.sendHeader("Location", "wifi", true);
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(302, "text/plain", "");    // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.client().stop(); // Stop is needed because we sent no content length
  saveCredentials();
  connect = strlen(ssid) > 0; // Request WLAN connect with new credentials if there is a SSID
}

void handleNotFound() {
  if (captivePortal()) { // If captive portal redirect instead of displaying the error page.
    return;
  }
  String message = F("File Not Found\n\n");
  message += F("URI: ");
  message += server.uri();
  message += F("\nMethod: ");
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += server.args();
  message += F("\n");

  for (uint8_t i = 0; i < server.args(); i++) {
    message += String(F(" ")) + server.argName(i) + F(": ") + server.arg(i) + F("\n");
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(404, "text/plain", message);
}
