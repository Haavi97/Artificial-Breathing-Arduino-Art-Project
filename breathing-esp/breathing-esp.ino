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
 * 
  */


#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "breathing-esp.h"

// Setting pin numbers
int const vacuum = 2;
int const pump = 0;
int const compressor = 4;

unsigned long current = millis();

unsigned long last_vacuum = millis();
unsigned long last_pump = millis();
unsigned long last_compressor = millis();

unsigned long vacuum_time = 10000;
unsigned long pump_time = 10000;
unsigned long pause_time = 10000;
unsigned long compressor_time = 10000;
unsigned long compressor_pause = 10000;

bool print_compressor_pause = true;
bool print_pump = true;
bool print_vacuum = true;

// Credentials already defined in header file breathing-esp.h
// const char* ssid = "REPLACE_WITH_YOUR_SSID";
// const char* password = "REPLACE_WITH_YOUR_PASSWORD";

// Set your Static IP address
IPAddress local_IP(192, 168, 1, 8);
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 0, 0);

// 1: Vacuum pump
// 2: Pump
// 3: Pause
// 4: Compressor
// 5: Compressor pause

String slider1Value = "1000";
String slider2Value = "1000";
String slider3Value = "1000";
String slider4Value = "1000";
String slider5Value = "1000";

// setting PWM properties
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;

const char* PARAM_INPUT = "value";
const char* PARAM_INPUT2 = "id";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP Web Server</title>
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 2.3rem;}
    p {font-size: 1.9rem;}
    body {max-width: 400px; margin:0px auto; padding-bottom: 25px;}
    .slider { -webkit-appearance: none; margin: 14px; width: 360px; height: 25px; background: #FFD65C;
      outline: none; -webkit-transition: .2s; transition: opacity .2s;}
    .slider::-webkit-slider-thumb {-webkit-appearance: none; appearance: none; width: 35px; height: 35px; background: #003249; cursor: pointer;}
    .slider::-moz-range-thumb { width: 35px; height: 35px; background: #003249; cursor: pointer; } 
  </style>
</head>
<body>
  <h2>Breathing control panel</h2>
  <p>Vacuum in: <span id="vacuumSliderValue">%SLIDERVALUE1%</span></p>
  <p><input type="range" onchange="updateSliderPWM(this, 1)" id="vacuumSlider" min="0" max="10000" value="%SLIDERVALUE1%" step="1" class="slider"></p>
  
  <p>Pump: <span id="pumpSliderValue">%SLIDERVALUE2%</span></p>
  <p><input type="range" onchange="updateSliderPWM(this, 2)" id="pumpSlider" min="0" max="10000" value="%SLIDERVALUE2%" step="1" class="slider"></p>
  
  <p>Pause: <span id="Pause">%SLIDERVALUE3%</span></p>
  <p><input type="range" onchange="updateSliderPWM(this, 3)" id="PauseSlider" min="0" max="10000" value="%SLIDERVALUE3%" step="1" class="slider"></p>
  
  <p>Compressor: <span id="compressorSliderValue">%SLIDERVALUE4%</span></p>
  <p><input type="range" onchange="updateSliderPWM(this, 4)" id="compressorSlider" min="0" max="10000" value="%SLIDERVALUE4%" step="1" class="slider"></p>
  
  <p>Compressor pause: <span id="compressorPause">%SLIDERVALUE5%</span></p>
  <p><input type="range" onchange="updateSliderPWM(this, 5)" id="compressorPauseSlider" min="0" max="10000" value="%SLIDERVALUE5%" step="1" class="slider"></p>
<script>
function updateSliderPWM(element, idn) {
  switch(idn){
    case 1:
      var id = "vacuumSliderValue";
      var sliderValue = document.getElementById("vacuumSlider").value;
      break;
    case 2:
      var id = "pumpSliderValue";
      var sliderValue = document.getElementById("pumpSlider").value;
      break;
    case 3:
      var id = "Pause";
      var sliderValue = document.getElementById("PauseSlider").value;
      break;
    case 4:
      var id = "compressorSliderValue";
      var sliderValue = document.getElementById("compressorSlider").value;
      break;
    case 5:
      var id = "compressorPause";
      var sliderValue = document.getElementById("compressorPauseSlider").value;
      break;
  }
  document.getElementById(id).innerHTML = sliderValue;
  console.log(sliderValue);
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/slider?value="+sliderValue+"&id="+idn.toString(), true);
  xhr.send();
}
</script>
</body>
</html>
)rawliteral";

// Replaces placeholder with button section in your web page
String processor(const String& var){
  //Serial.println(var);
  if (var == "SLIDERVALUE1"){
    return slider1Value;
  } else if (var == "SLIDERVALUE2"){
    return slider2Value;
  } else if (var == "SLIDERVALUE3"){
    return slider3Value;
  } else if (var == "SLIDERVALUE4"){
    return slider4Value;
  } else if (var == "SLIDERVALUE5"){
    return slider5Value;
  }
  return String();
}

void setup(){
  pinMode(compressor, OUTPUT);
  pinMode(pump, OUTPUT);
  pinMode(vacuum, OUTPUT);
  
  // Serial port for debugging purposes
  Serial.begin(115200);

  // Setting static ip address
  //if (!WiFi.config(local_IP, gateway, subnet)) {
  //  Serial.println("STA Failed to configure");
  //}

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Send a GET request to <ESP_IP>/slider?value=<inputMessage>
  server.on("/slider", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputId;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>&id=<inputId>
    if (request->hasParam(PARAM_INPUT) && request->hasParam(PARAM_INPUT2)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      inputId = request->getParam(PARAM_INPUT2)->value();
      if (inputId == "1"){
        slider1Value = inputMessage;
        vacuum_time = slider1Value.toInt();
        Serial.print("Vacuum time: ");
        Serial.println(slider1Value);
      } else if (inputId == "2"){
        slider2Value = inputMessage;
        pump_time = slider2Value.toInt();
        Serial.print("Pump time: ");
        Serial.println(slider2Value);
      } else if (inputId == "3"){
        slider3Value = inputMessage;
        pause_time = slider3Value.toInt();
        Serial.print("Pause time: ");
        Serial.println(slider3Value);
      } else if (inputId == "4"){
        slider4Value = inputMessage;
        compressor_time = slider4Value.toInt();
        Serial.print("Compressor time: ");
        Serial.println(slider4Value);
      } else if (inputId == "5"){
        slider5Value = inputMessage;
        compressor_pause = slider5Value.toInt();
        Serial.print("Compressor pause time: ");
        Serial.println(slider5Value);
      } 
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK");
  });
  
  // Start server
  server.begin();

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
  } else if (current > (last_vacuum + vacuum_time)){
    if (print_vacuum){
      Serial.println("Vacuum OFF");
      digitalWrite(vacuum, LOW);
      print_vacuum = false;
      print_pump = true;
    }
  } else if (current > last_vacuum + vacuum_time + pause_time){
    if (print_pump){
      Serial.println("Pump ON");
      digitalWrite(pump, HIGH);
      print_pump = false;
    }
  }
}
