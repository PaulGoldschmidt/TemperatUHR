![TemperatUHR Logo](https://raw.githubusercontent.com/PaulGoldschmidt/TemperatUHR/b37c3481bf3908ecba21f54f1f9f180c2d73fb7e/3_RESOURCES/Logo/TemperatUHR-Logo.svg)
# Programming TemperatUHR: Development setup and installing the firmware on-device
In this tutorial, you will learn how to flash the firmware of TemperatUHR to the Microcontroller. The setup in this tutorial also covers the development environment needed for continue writing the firmware of TemperatUHR further.
## Prerequisites
Here's what you'll need for this tutorial:
 1. TemperatUHR NG (Hardware Rev. 2.0 or higher)
 2. A computer (Linux, MacOS or Windows in modern versions) **with administrative rights**
 3. Micro-USB Cable for firmware flashing
 4. Working internet connection
## Installing the required Software
First of all, the good news: We only need to install one new program to our computer. Some of you maybe already have the "Arduino Integrated Development Environment (IDE)" (or short: "Arduino IDE") installed - for those who havn't, just head to https://www.arduino.cc/en/software and download the IDE for your operating system. Next up the bad news: We need to add a lot of things to the development environment.
After installing, start the program and open the settings by clicking on "File" -> "Preferences". Next up, add the following link in the field for "additional board manager URLs":
"http://arduino.esp8266.com/stable/package_esp8266com_index.json" (without quotes)
Now, click "ok" and open the Board Manager by clicking on "Tools" -> "Board" -> "Board Manager". In the newly opened window type "esp2866" and install the package (at the time of writing Version 3.0.2 was the most recent one). After installing, close the window and check that the Arduino IDE is now able to communicate with the connected ESP-Microcontroller. For TemperatUHR, we use the ESP2866 in the package form of an "WEMOS D1 Mini", so select this microcontroller type in "Tools" -> "Board" -> "ESP2866" -> "LOLIN (WEMOS) D1 R2 & MINI". Now, open one of the build-in examples of the Arduino IDE and try to upload it to the connected board. If it doesn't work right away, be sure that the port on which the Microcontroller is connected checks out. You can do this by clicking on "Tools" -> "Port" and than take a look at what ports are offered in the section. Maybe you have to try out the different options till you find the correct port of the Microcontroller. If everything worked and no error message appared while compiling and flashing, you can move on to the next step
## Installing the required liberies
TemperatUHR makes use of various liberies to simplify the whole development. Those liberies have to be installed manually. Here's an overview of the utilized liberies:
| Nr. | Libery Name | What it does | Where to find it | Tested Version
|--|--|--|--|--|
| 1. | DallasTemperature | Sensor profile | Libary Manager | 3.9.0
| 2. | OneWire | Communication protocol to sensor | Libary Manager | 2.3.5
| 3. | Blynk | Communication handling | Libary Manager | 1.1.0
For the record, the libary manager is located under "Sketch" -> "Include Libery" -> "Manage Libraries".
With that, you can start writing the firmware onto the microcontroller.
## Flashing the firmware
With everything prepared, goto GitHub, download the latest package of TemperatUHR published (see https://github.com/PaulGoldschmidt/TemperatUHR/releases) and unzip the download. Jump in the folder "1_SOFTWARE", than into "TemperatUHR-NG" and open "TemperatUHR-NG.ino". Flash the software by clicking the arrow to the right (on the left upper corner, just under "Edit") and take a look at TemperatUHR - if everything has gone to plan, the **LED should now be illuminated on TemperatUHR**. Congratulations, you now flashed the firmware of TemperatUHR to the board. To check the firmware version or debug the whole thing, open the serial console (Ctrl + Shift + M) and take a look at what happens under the hood of TemperatUHR.
## Developing TemperatUHR further
With all of that, you are ready to take the next steps for TemperatUHR into your own hands - use GitHub with its [CLI](https://cli.github.com/) or the [desktop application](https://desktop.github.com/) to create pull requests and help by making additions to the firmware or fixing bugs. [Here you can find a cheet sheet](https://education.github.com/git-cheat-sheet-education.pdf) about how to contribute with GitHub.
