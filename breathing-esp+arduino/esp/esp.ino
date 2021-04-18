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
 *              Part of the program that goes for the ESP-01 
 *              (ESP8266) and works as a server communicating
 *              the needed commands to an Arduino
 * 
  */


#ifdef ESP32
  #include <WiFi.h>
  #include <AsyncTCP.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#endif
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
unsigned long pvp_time = 10000;
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
String slider6Value = "1000";

const char* PARAM_INPUT = "value";
const char* PARAM_INPUT2 = "id";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Breathing control panel</title>
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 2.3rem;}
    p {font-size: 1.9rem;}
    body {max-width: 400px; margin:0px auto; padding-bottom: 25px;}
    .button1 {border-radius: 12px; padding: 20px; font-size: 16px;}
    .slider { -webkit-appearance: none; margin: 14px; width: 360px; height: 25px; background: #FFD65C;
      outline: none; -webkit-transition: .2s; transition: opacity .2s;}
    .slider::-webkit-slider-thumb {-webkit-appearance: none; appearance: none; width: 35px; height: 35px; background: #003249; cursor: pointer;}
    .slider::-moz-range-thumb { width: 35px; height: 35px; background: #003249; cursor: pointer; } 
  </style>
</head>
<body>
  <h2>Breathing control panel</h2>
  <button onclick="switchButton2()" class="button button1">Pause/Play pump</button>
  <br>
  <button onclick="switchButton()" class="button button1">Pause/Play compressor</button>
  
  <p>Vacuum in: <span id="vacuumSliderValue">%SLIDERVALUE1%</span></p>
  <p><input type="range" onchange="updateSliderPWM(this, 1)" id="vacuumSlider" min="0" max="10000" value="%SLIDERVALUE1%" step="1" class="slider"></p>
  
  <p>Pause vacuum-pump: <span id="pvpSliderValue">%SLIDERVALUE6%</span></p>
  <p><input type="range" onchange="updateSliderPWM(this, 6)" id="pvpSlider" min="0" max="10000" value="%SLIDERVALUE6%" step="1" class="slider"></p>
  
  <p>Pump: <span id="pumpSliderValue">%SLIDERVALUE2%</span></p>
  <p><input type="range" onchange="updateSliderPWM(this, 2)" id="pumpSlider" min="0" max="10000" value="%SLIDERVALUE2%" step="1" class="slider"></p>
  
  <p>Pause: <span id="Pause">%SLIDERVALUE3%</span></p>
  <p><input type="range" onchange="updateSliderPWM(this, 3)" id="PauseSlider" min="0" max="10000" value="%SLIDERVALUE3%" step="1" class="slider"></p>
  
  <p>Compressor: <span id="compressorSliderValue">%SLIDERVALUE4%</span></p>
  <p><input type="range" onchange="updateSliderPWM(this, 4)" id="compressorSlider" min="0" max="10000" value="%SLIDERVALUE4%" step="1" class="slider"></p>
  
  <p>Compressor pause: <span id="compressorPause">%SLIDERVALUE5%</span></p>
  <p><input type="range" onchange="updateSliderPWM(this, 5)" id="compressorPauseSlider" min="0" max="10000" value="%SLIDERVALUE5%" step="1" class="slider"></p>
<script>
function switchButton() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/stop", true); 
  xhr.send();
}
function switchButton2() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/switch2", true); 
  xhr.send();
}
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
    case 6:
      var id = "pvpSliderValue";
      var sliderValue = document.getElementById("pvpSlider").value;
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
  } else if (var == "SLIDERVALUE6"){
    return slider6Value;
  }
  return String();
}

void setup(){
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }

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
        Serial.print("v");
        Serial.println(slider1Value);
      } else if (inputId == "2"){
        slider2Value = inputMessage;
        pump_time = slider2Value.toInt();
        Serial.print("p");
        Serial.println(slider2Value);
      } else if (inputId == "3"){
        slider3Value = inputMessage;
        pause_time = slider3Value.toInt();
        Serial.print("a");
        Serial.println(slider3Value);
      } else if (inputId == "4"){
        slider4Value = inputMessage;
        compressor_time = slider4Value.toInt();
        Serial.print("c");
        Serial.println(slider4Value);
      } else if (inputId == "5"){
        slider5Value = inputMessage;
        compressor_pause = slider5Value.toInt();
        Serial.print("r");
        Serial.println(slider5Value);
      }  else if (inputId == "6"){
        slider6Value = inputMessage;
        pvp_time = slider6Value.toInt();
        Serial.print("s");
        Serial.println(slider6Value);
      } 
    }
    else {
      inputMessage = "No message sent";
    }
    request->send(200, "text/plain", "OK");
  });
  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("k");
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/switch2", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("q");
    request->send_P(200, "text/html", index_html, processor);
  });
  
  // Start server
  server.begin();

  digitalWrite(compressor, HIGH);
  digitalWrite(vacuum, HIGH);
  digitalWrite(pump, LOW);
}
  
void loop() {
}
