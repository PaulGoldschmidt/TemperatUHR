/*
   TEMPERATUHR NEXT GENERATION (TemperatUHR NG / Version 2.0)
   Software Version: 0.2-PRE
   Copyright Paul Goldschmidt, August 2022 - Heidelberg. https://paul-goldschmidt.de/
   Acknowledgements: https://github.com/PaulGoldschmidt/TemperatUHR/2_DOCUMENTATION/acknogledments.md
*/

void runsensor() {
  sensors.requestTemperatures(); //request temp sensor data
  temperature = sensors.getTempCByIndex(0); //store it in the float "temperature"
  Serial.print("Temperature: ");
  Serial.println(temperature);
  while (temperature == -127) {
    sensors.requestTemperatures(); //request temp sensor data
    temperature = sensors.getTempCByIndex(0); //store it in the float "temperature"
    digitalWrite(RED_LED, LOW);
    delay(250);
    digitalWrite(RED_LED, HIGH);
    delay(250);
  }
  temperatuhrstandbycheck();
}

void calctime() {
  float walkspeed = 1; //walking speed in m/s; 1 m/s standart, change if necessary
  int timetilltarget;
  sensors.requestTemperatures(); //request temp sensor data
  int oldtemperature = sensors.getTempCByIndex(0); //store it in the float "temperature"
  delay(1500); //wait for 1.5 Seconds
  sensors.requestTemperatures(); //request temp sensor data
  temperature = sensors.getTempCByIndex(0); //store it in the float "temperature"
  float degreespersec = (temperature - oldtemperature) / 1.5;
  if (degreespersec >= 0.3) { //cross out devision by zero resulting in notification hysterisis
    timetilltarget = tempdifference() / degreespersec;
    while (timestilltarget[0] == 222) { //checks if array is still filled with impossible values and writes to every element the current time till target
      Serial.print("Initailizing averaging array.");
      for (int i = 0 ; i < 10 ; i++) {
        timestilltarget[i] = timetilltarget;
      }
    }
    timestilltarget[positioninarray] = timetilltarget;
    positioninarray++;
    Serial.println("Postition in array: " + String(positioninarray) + " | Median time till Target: " + String(average(timestilltarget, 10)) + " Seconds");
    if (positioninarray >= 9) {
      positioninarray = 0;
    }
    Blynk.virtualWrite(V2, average(timestilltarget, 10));
  }
  else {
    timetilltarget = 999999;
  }
  int timetosensor = distancetosensor / walkspeed;
  int timetillnotification = timetilltarget - timetosensor;
  if ((timetillnotification <= 0) && (temperatuhrstandby == false)) {
    Blynk.logEvent("temperature_reached", "Quick! Soon TemperatUHR will reach the target temperature."); //TODO: Check Notifications
    Serial.println("Notification send.");
    temperatuhrstandby = true; // set temperatuhr into standby, so that there won't be a second notification
  }
  if (tempdifference() <= 0) {
    Blynk.virtualWrite(V3, 1); //Turn LED on in App
  }
  else {
    Blynk.virtualWrite(V3, 0);
  }
}

void temperatuhrstandbycheck() { //avoiding hysterisis and notifying the user about the current status.
  if (temperatuhrstandby == false) {
    digitalWrite(BLUE_LED, HIGH);
    analogWrite(RED_LED, 1);
  }
  else {
    digitalWrite(RED_LED, HIGH);
    analogWrite(BLUE_LED, 1);
  }
}

int tempdifference() { // calculate the temperature between the current temperature and the goal temperature.
  int tempdifferencecalc;
  if (cooldownmode == false) { 
    tempdifferencecalc = targettemperature - temperature;
  }
  else { // cooldown mode. if implemented for non-negative differences
    tempdifferencecalc = temperature - targettemperature;
  }
  return tempdifferencecalc;
}

BLYNK_WRITE(V4)
{
  distancetosensor = param.asInt(); // assigning incoming value from pin V1 to a variable
  Serial.println("New distance to sensor: " + String(distancetosensor) + " Meters.");
}

BLYNK_WRITE(V1)
{
  targettemperature = param.asInt(); // assigning incoming value from pin V1 to a variable
  Serial.println("New target temperature: " + String(targettemperature) + " Degrees.");
  if (temperature <= targettemperature) { // target temperature higher than current temperature
    cooldownmode = false;
    Serial.println("No cooldown mode!");
  }
  else {
    cooldownmode = true;
    Serial.println("Cooldown mode!");
  }
  temperatuhrstandby = false;
}

BLYNK_CONNECTED() {
    Blynk.syncAll();
    Serial.println("Synced settings: " + String(targettemperature) + " Degrees as target temperature, " + String(distancetosensor) + " Meters as distance to sensor.");
}
