/*
    License:
    Copyright (C) 2013  Ian W. Kephart (KD8SSF@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    
    Purpose:
    This code enables the use of any standard ATX 2.0 power supply with a Raspberry Pi. 
    Gerber files for board production are avilable at: <https://github.com/KD8SSF/RPI_ATX_breakout>
    Inspired by the "ATX Raspi" from Low Power Lab. Please see <http://lowpowerlab.com/atxraspi/#installation> 
    for details regarding installation of the accompanying script on your Raspberry Pi. Please note with 
    this implementation connections to pins 7 and 8 on the RPi GPIO are the only ones required, 
    the power connection will be handled by the USB-A port on the board. 
*/


#define holdTime 4000       //button-hold interval for shutdown

// Pin assignments
const int CaseSwitch = 2;   //PC case power switch
const int CasePowerLED = 3; //PC case Power LED
const int CaseHDDLED = 4;   //PC case HDD LED
const int STDBYLED = 13;    //On-board standby indicator
const int PWRONLED = 12;    //On-board PWRON indicator
const int PWROFFLED = 11;   //On-board PWROFF indicator
const int PSON = 8;         //output to ATX power supply pin 16
const int RPISD = 10;       //output to RPi to signal shutdown
const int HALTOK = 9;       //input from RPi on good shutdown



// Variables
int STATUS = 1;             //track STDBY/PWRON/PWROFF modes
int buttonState = 0;        //state of the Case switch
long int btnDwnTime;        //when the switch was pressed

void setup () {
  pinMode(CaseSwitch, INPUT);
  pinMode(CasePowerLED, OUTPUT);
  pinMode(CaseHDDLED, OUTPUT);
  pinMode(STDBYLED, OUTPUT);
  pinMode(PWRONLED, OUTPUT);
  pinMode(PWROFFLED, OUTPUT);
  pinMode(PSON, OUTPUT);
  pinMode(RPISD, OUTPUT);
  pinMode(HALTOK, INPUT);
}

void loop () {
  buttonState = digitalRead(CaseSwitch);    //read the switch at the start of the loop
  
  if (buttonState == HIGH && STATUS == 2) { //store the button press time if power supply is on and button pressed 
        static long btnDwnTime = millis();
  }
  
  switch (STATUS) {
                                            //Standby mode, watching for button press to turn on the PSU
    case 1:                
      if (buttonState == LOW) {
        digitalWrite(STDBYLED, LOW);
        digitalWrite(PWRONLED, HIGH);
        digitalWrite(CasePowerLED, HIGH);
        digitalWrite(PSON, LOW);
        STATUS = 2;
        }
      else {
        if (buttonState == HIGH) {
           digitalWrite(STDBYLED, HIGH);
           STATUS = 1;
          } 
        }
        break;
                                            //PSU-ON, watching for long press (4+ seconds) on the case switch to turn the PSU off
    case 2:
      if (buttonState == LOW && (millis() - btnDwnTime) > long(holdTime)) {
        digitalWrite(PWRONLED, LOW);
        digitalWrite(PWROFFLED, HIGH);
        digitalWrite(RPISD, HIGH);  
        STATUS = 3;
        long btnDwnTime = 0;
        }
      else {
        if (buttonState == HIGH) {
          digitalWrite(PWRONLED, HIGH);
          STATUS = 2;
          }
        }
        break;
                                            //PSU-OFF, waiting for the RPi to signal that it has halted
    case 3:                
      if (digitalRead(HALTOK) == LOW) {
        delay(5000);                        //wait five seconds for the RPi to finish shutdown
        digitalWrite(PWROFFLED, LOW);
        digitalWrite(CasePowerLED, LOW);
        digitalWrite(PSON, HIGH);
        digitalWrite(STDBYLED, HIGH);
        long btnDwnTime = 0;
        STATUS = 1;
      }
      else {
        if (digitalRead(HALTOK) == HIGH) {   //blink the case "HDD activity" LED while waiting on the RPi HALT-OK signal
          digitalWrite(CaseHDDLED, HIGH);
          delay(50);
          digitalWrite(CaseHDDLED, LOW);
          STATUS = 3;
        }  
      }
        break;
  } //switch end
  
}
