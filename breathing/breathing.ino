/**
 * File:        breathing.ino
 * Author:      Francisco Javier Ortín Cervera
 * Created:     21.01.2021
 * Last edit:   29.12.2021
 *
 * Description: Program for representing the breathing of
 *              a living organism using a compressor 
 *              controlled by a relay. 
 *              There are also a button and a potentiometer
 *              to control the frequencies of the breathing
 *              times.
 * 
  */

int const compressor = 4;
int const potentiometer = A1;
int const button = 5;

int control = 0;
// 0 controls input time
// 1 controls rest  time

int after_control = 200;  
// Time (ms) to way after pressing control button

int time_read = 1000;
int factor = 4; 
// Factor to multiply the analog read time so
// if factor is 4 then the maximum time is 4*1023

int input_time = 500; 
// Default time the compressor is turned on
int rest_time = 1000;
// Default time the compressor is turned off

void setup() {
  // put your setup code here, to run once:
  pinMode(compressor, OUTPUT);
  pinMode(potentiometer,INPUT);
  pinMode(button, INPUT);
  digitalWrite(compressor, LOW);

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
  delay(input_time);
  digitalWrite(compressor, LOW);
  delay(rest_time);
  
}
