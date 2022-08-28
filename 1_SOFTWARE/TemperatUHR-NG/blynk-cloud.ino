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
  int tempdifference;
  sensors.requestTemperatures(); //request temp sensor data
  int oldtemperature = sensors.getTempCByIndex(0); //store it in the float "temperature"
  delay(2500); //wait for 2.5 Seconds
  sensors.requestTemperatures(); //request temp sensor data
  int newtemperature = sensors.getTempCByIndex(0); //store it in the float "temperature"
  float degreespersec = (newtemperature - oldtemperature) / 2.5;
  if (newtemperature <= targetTemperature) { // target temperature higher than current temperature
    tempdifference = targetTemperature - newtemperature;
  }
  else {
    tempdifference = newtemperature - targetTemperature;
  }
  int timetilltarget = tempdifference / degreespersec;
  Blynk.virtualWrite(V2, timetilltarget);
}

BLYNK_WRITE(V4)
{
  distanceToSensor = param.asInt(); // assigning incoming value from pin V1 to a variable
}

BLYNK_WRITE(V1)
{
  targetTemperature = param.asInt(); // assigning incoming value from pin V1 to a variable
}
