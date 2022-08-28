/*
   TEMPERATUHR NEXT GENERATION (TemperatUHR NG / Version 2.0)
   Software Version: 0.1-PRE
   Copyright Paul Goldschmidt, August 2022 - Heidelberg. https://paul-goldschmidt.de/
   Acknowledgements: https://github.com/PaulGoldschmidt/TemperatUHR/2_DOCUMENTATION/acknogledments.md
*/

void runsensor() {
  sensors.requestTemperatures(); //request temp sensor data
  float temperature = sensors.getTempCByIndex(0); //store it in the float "temperature"
  Serial.print("Temperature:");
  Serial.println(temperature);
  Blynk.virtualWrite(V0, temperature);
}
