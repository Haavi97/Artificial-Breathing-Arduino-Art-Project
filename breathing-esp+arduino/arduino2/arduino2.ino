/**
 * File:        breathing-esp.ino
 * Author:      Francisco Javier Ortín Cervera
 * Created:     18.04.2021
 * Last edit:   18.04.2021
 *
 * Description: Axuliary Arduino board for 
 *              controlling servo motor.
 * 
  */
  
#include <Servo.h>

Servo myservo;

int const servoPin = 4;
int const sense = 7;

int closed_angle = 90;
// initial angle at which the valve is closed

int open_angle = 10;
// angle at which the valve is open

int state = LOW;


void setup(){

  myservo.attach(servoPin);

  pinMode(sense, INPUT);
  

  myservo.write(closed_angle);
}
  
void loop() {
  if (digitalRead(sense)) {
    if (!state){
        myservo.write(closed_angle);
        state = HIGH;
      }
    } else {
      if (state){
        myservo.write(open_angle);
        state = LOW;
      }
    }
}
