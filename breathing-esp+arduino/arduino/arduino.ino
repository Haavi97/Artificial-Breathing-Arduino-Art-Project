/**
 * File:        breathing-esp.ino
 * Author:      Francisco Javier Ortín Cervera
 * Created:     17.04.2021
 * Last edit:   18.04.2021
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
  
#include <EEPROM.h>

unsigned long EEPROM_readlong(int address);
void EEPROM_writelong(int address, unsigned long value);
void EEPROM_writeint(int address, int value);
unsigned int EEPROM_readint(int address);

// Setting pin numbers
int const vacuum = 6;
int const pump = 7;
int const compressor = 12;
int const servoPin = 8;

unsigned long current = millis();

unsigned long last_vacuum = millis();
unsigned long last_pump = millis();
unsigned long last_compressor = millis();

unsigned long vacuum_time = EEPROM_readlong(0);
unsigned long pvp_time = EEPROM_readlong(5);
unsigned long pump_time = EEPROM_readlong(10);
unsigned long pause_time = EEPROM_readlong(15);
unsigned long compressor_time = EEPROM_readlong(20);
unsigned long compressor_pause = EEPROM_readlong(25);

int closed_angle = 90;
// initial angle at which the valve is closed

int open_angle = 10;
// angle at which the valve is open

bool print_compressor_pause = true;
bool print_pump = true;
bool print_vacuum = true;
bool to_update = true;

bool activeCompressor = true;
bool activePump = true;

String input = "";

void setup(){  
  pinMode(compressor, OUTPUT);
  pinMode(pump, OUTPUT);
  pinMode(vacuum, OUTPUT);

  pinMode(servoPin, OUTPUT);
  
  // Serial port for debugging purposes
  Serial.begin(115200);

  digitalWrite(compressor, HIGH);
  digitalWrite(servoPin, HIGH);
  digitalWrite(vacuum, HIGH);
  digitalWrite(pump, LOW);
}
  
void loop() {
  current = millis();
  if (activeCompressor) {
    if (current > (last_compressor + compressor_time + compressor_pause)){
      if (to_update){
        Serial.println("Compressor ON");
        digitalWrite(servoPin, HIGH);
        digitalWrite(compressor, HIGH);
        last_compressor = current;
        print_compressor_pause = true;
        to_update = false;
      }
    } else if (current > (last_compressor + compressor_time)){
      if (print_compressor_pause){
        Serial.println("Compressor OFF");
        digitalWrite(servoPin, LOW);
        digitalWrite(compressor, LOW);
        print_compressor_pause = false;
        to_update = true;
      }
    }
  } else{
      digitalWrite(servoPin, LOW);
      digitalWrite(compressor, LOW);
  }
    
  if (activePump){
    if (current > (last_vacuum + vacuum_time + pause_time + pump_time + pvp_time)){
      Serial.println("Pump OFF");
      digitalWrite(pump, LOW);
      Serial.println("Vacuum ON");
      digitalWrite(vacuum, HIGH);
      last_vacuum = current;
      print_vacuum = true;
      print_pump = true;
    } else if (current > (last_vacuum + vacuum_time + pause_time + pump_time)){
      digitalWrite(pump, LOW);
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
  } else {
    digitalWrite(vacuum, LOW);
    digitalWrite(pump, LOW);
  }
  
  if (Serial.available()){
    input = Serial.readStringUntil('\n');
    if (input[0] == 'p'){
      input.remove(0, 1);
      pump_time = input.toInt();
      Serial.println(pump_time);
      last_compressor = current;
      last_vacuum = current;
    } else if (input[0] == 'v'){
      input.remove(0, 1);
      vacuum_time = input.toInt();
      Serial.println(vacuum_time);
      last_compressor = current;
      last_vacuum = current;
    } else if (input[0] == 'a'){
      input.remove(0, 1);
      pause_time = input.toInt();
      Serial.println(pause_time);
      last_compressor = current;
      last_vacuum = current;
    } else if (input[0] == 'c'){
      input.remove(0, 1);
      compressor_time = input.toInt();
      Serial.println(compressor_time);
      last_compressor = current;
      last_vacuum = current;
    } else if (input[0] == 'r'){
      input.remove(0, 1);
      compressor_pause = input.toInt();
      Serial.println(compressor_pause);
      last_compressor = current;
      last_vacuum = current;
    } else if (input[0] == 's'){
      input.remove(0, 1);
      pvp_time = input.toInt();
      Serial.println(compressor_pause);
      last_compressor = current;
      last_vacuum = current;
    } else if (input[0] == 'k'){
      activeCompressor = !activeCompressor;
      EEPROM_writelong(0, vacuum_time);
      EEPROM_writelong(5, pvp_time);
      EEPROM_writelong(10, pump_time);
      EEPROM_writelong(15, pause_time);
      EEPROM_writelong(20, compressor_time);
      EEPROM_writelong(25, compressor_pause);
      Serial.println(vacuum_time);
      Serial.println(pvp_time);
      Serial.println(pump_time);
      Serial.println(pause_time);
      Serial.println(compressor_time);
      Serial.println(compressor_pause);
    } else if (input[0] == 'q'){
      activePump = !activePump;
    }
  }
}


// from: https://forum.arduino.cc/t/saving-an-unsigned-long-int-to-internal-eeprom/487205
// read double word from EEPROM, give starting address
 unsigned long EEPROM_readlong(int address)
 {
 //use word read function for reading upper part
 unsigned long dword = EEPROM_readint(address);
 //shift read word up
 dword = dword << 16;
 // read lower word from EEPROM and OR it into double word
 dword = dword | EEPROM_readint(address+2);
 return dword;
}

//write word to EEPROM
 void EEPROM_writeint(int address, int value) 
{
 EEPROM.write(address,highByte(value));
 EEPROM.write(address+1 ,lowByte(value));
}
 
 //write long integer into EEPROM
 void EEPROM_writelong(int address, unsigned long value) 
{
 //truncate upper part and write lower part into EEPROM
 EEPROM_writeint(address+2, word(value));
 //shift upper part down
 value = value >> 16;
 //truncate and write
 EEPROM_writeint(address, word(value));
}

unsigned int EEPROM_readint(int address) 
{
 unsigned int word = word(EEPROM.read(address), EEPROM.read(address+1));
 return word;
}
