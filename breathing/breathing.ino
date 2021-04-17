/**
 * File:        breathing.ino
 * Author:      Francisco Javier Ortín Cervera
 * Created:     21.01.2021
 * Last edit:   04.02.2021
 *
 * Description: Program for representing the breathing of
 *              a living organism using a compressor 
 *              controlled by a relay. 
 *              There are also a button and a potentiometer
 *              to control the frequencies of the breathing
 *              times.
 * 
  */

#include <Servo.h>

int const potentiometer = A1;
int const compressor = 4;
int const button = 5;
int const servo = 6;

Servo myservo;

int control = 0;
// 0 controls input time
// 1 controls rest  time

int after_control = 200;  
// Time (ms) to way after pressing control button

int time_read = 1000;
int factor = 8; 
// Factor to multiply the analog read time so
// if factor is 4 then the maximum time is 4*1023

int filling_time = 20000;
// Initial time to fill before the breathing
int input_time = 5000; 
// Default time the compressor is turned on
int rest_time = 5000;
// Default time the compressor is turned off

int closed_angle = 90;
// initial angle at which the valve is closed

int open_angle = 10;
// angle at which the valve is open

void setup() {
  // put your setup code here, to run once:
  pinMode(compressor, OUTPUT);
  pinMode(potentiometer,INPUT);
  pinMode(button, INPUT);

  myservo.attach(servo);
  
  digitalWrite(compressor, LOW);
  
  myservo.write(closed_angle);

  Serial.begin(9600);
  // Serial com for debugging
}

void loop() {
  if (digitalRead(button) == HIGH){
    control = !control;
    delay(after_control);
    Serial.print("Control input: ");
    Serial.println(control);
  }
  
  time_read = analogRead(potentiometer)*factor;
  Serial.print("Time read: ");
  Serial.println(time_read);
  
  if (control)rest_time  = time_read;
  else        input_time = time_read;
  
  digitalWrite(compressor, HIGH);
  myservo.write(open_angle);
  delay(input_time);
  digitalWrite(compressor, LOW);
  myservo.write(closed_angle);
  delay(rest_time);
}
