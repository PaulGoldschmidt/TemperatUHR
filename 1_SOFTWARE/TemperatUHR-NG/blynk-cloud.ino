/*
   TEMPERATUHR NEXT GENERATION (TemperatUHR NG / Version 2.0)
   Software Version: 0.2-PRE
   Copyright Paul Goldschmidt, August 2022 - Heidelberg. https://paul-goldschmidt.de/
   Acknowledgements: https://github.com/PaulGoldschmidt/TemperatUHR/2_DOCUMENTATION/acknogledments.md
*/

void runsensor() {
  sensors.requestTemperatures(); //request temp sensor data
  temperature = sensors.getTempCByIndex(0); //store it in the float "temperature"
  Serial.print("Temperature:");
  Serial.println(temperature);
  while (temperature == -127) {
    sensors.requestTemperatures(); //request temp sensor data
    temperature = sensors.getTempCByIndex(0); //store it in the float "temperature"
    digitalWrite(RED_LED, LOW);
    delay(250);
    digitalWrite(RED_LED, HIGH);
    delay(250);
  }
}

void calctime() {
  float walkspeed = 1; //walking speed in m/s; 1 m/s standart, change if necessary
  sensors.requestTemperatures(); //request temp sensor data
  int oldtemperature = sensors.getTempCByIndex(0); //store it in the float "temperature"
  delay(2500); //wait for 2.5 Seconds
  sensors.requestTemperatures(); //request temp sensor data
  temperature = sensors.getTempCByIndex(0); //store it in the float "temperature"
  float degreespersec = (temperature - oldtemperature) / 2.5;
  int timetilltarget = tempdifference() / degreespersec;
  Blynk.virtualWrite(V2, timetilltarget);
  int timetosensor = distancetosensor / walkspeed;
  int timetillnotification = timetilltarget - timetosensor;
  if (timetillnotification <= 0) {
    Blynk.notify("Quick! Soon TemperatUHR will reach the target temperature.");
    temperatuhrstandby = true; // set temperatuhr into standby, so that there won't be a second notification
  }
  else {
    Blynk.virtualWrite(V3, 0);
    }
}

void temperatuhrstandbycheck() {
  if (tempdifference() > 5) {
    temperatuhrstandby = false;
    digitalWrite(BLUE_LED, HIGH);
    analogWrite(RED_LED, 155);
  }

  if (temperatuhrstandby == true) {
    digitalWrite(RED_LED, HIGH);
    analogWrite(BLUE_LED, 155);
  }
}

int tempdifference() {
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
  }
  else {
    cooldownmode = true;
  }
}
