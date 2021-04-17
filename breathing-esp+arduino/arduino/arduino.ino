/**
 * File:        breathing-esp.ino
 * Author:      Francisco Javier Ortín Cervera
 * Created:     17.04.2021
 * Last edit:   17.04.2021
 *
 * Description: Program for representing the breathing of
 *              a living organism using a compressor
 *              controlled by a relay from one hand and  
 *              another compressor and a vacuum pump also 
 *              controlled with relays.
 *              Part of the program that goes to the Arduino 
 *              that receives commands from an ESP-01 in 
 *              this case.
 * 
  */

// Setting pin numbers
int const vacuum = 6;
int const pump = 7;
int const compressor = 5;

unsigned long current = millis();

unsigned long last_vacuum = millis();
unsigned long last_pump = millis();
unsigned long last_compressor = millis();

unsigned long vacuum_time = 1000;
unsigned long pump_time = 1000;
unsigned long pause_time = 1000;
unsigned long compressor_time = 1000;
unsigned long compressor_pause = 1000;

bool print_compressor_pause = true;
bool print_pump = true;
bool print_vacuum = true;

String input = "";

void setup(){
  pinMode(compressor, OUTPUT);
  pinMode(pump, OUTPUT);
  pinMode(vacuum, OUTPUT);
  
  // Serial port for debugging purposes
  Serial.begin(115200);

  digitalWrite(compressor, HIGH);
  digitalWrite(vacuum, HIGH);
  digitalWrite(pump, LOW);
}
  
void loop() {
  current = millis();

  if (current > (last_compressor + compressor_time + compressor_pause)){
    Serial.println("Compressor ON");
    digitalWrite(compressor, HIGH);
    last_compressor = current;
    print_compressor_pause = true;
  } else if (current > (last_compressor + compressor_time)){
    if (print_compressor_pause){
      Serial.println("Compressor OFF");
      digitalWrite(compressor, LOW);
      print_compressor_pause = false;
    }
  }

  if (current > (last_vacuum + vacuum_time + pause_time + pump_time)){
    Serial.println("Pump OFF");
    digitalWrite(pump, LOW);
    Serial.println("Vacuum ON");
    digitalWrite(vacuum, HIGH);
    last_vacuum = current;
    print_vacuum = true;
    print_pump = true;
  } else if (current > (last_vacuum + vacuum_time + pause_time)){
    if (print_pump){
      Serial.println("Pump ON");
      digitalWrite(pump, HIGH);
      print_pump = false;
    }
  } else if (current > (last_vacuum + vacuum_time)){
    if (print_vacuum){
      Serial.println("Vacuum OFF");
      digitalWrite(vacuum, LOW);
      print_vacuum = false;
    }
  } 
  if (Serial.available()){
    input = Serial.readStringUntil('\n');
    if (input[0] == 'p'){
      input.remove(0, 1);
      pump_time = input.toInt();
      Serial.println(pump_time);
    } else if (input[0] == 'v'){
      input.remove(0, 1);
      vacuum_time = input.toInt();
      Serial.println(vacuum_time);
    } else if (input[0] == 'a'){
      input.remove(0, 1);
      pause_time = input.toInt();
      Serial.println(pause_time);
    } else if (input[0] == 'c'){
      input.remove(0, 1);
      compressor_time = input.toInt();
      Serial.println(compressor_time);
    } else if (input[0] == 'r'){
      input.remove(0, 1);
      compressor_pause = input.toInt();
      Serial.println(compressor_pause);
    }
  }
}
