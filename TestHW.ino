#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>

#include <Wire.h>
#include <skywriter.h>

#define PIN_TRFR  13
#define PIN_RESET 14

long x_value = 0;
long y_value = 0;
long z_value = 0;
long gate_value = 0;

const char* ssid = "BioMuell23";
const char* password = "sunaanus";

ESP8266WebServer server(80);
WebSocketsServer webSocket(81);
IPAddress apIP(42, 42, 42, 42);

String page = "<body onload=\"javascript:start();\">"
+ String("<script>\n")
+ "var Socket;\n"
+ "var send;\n"
+ "var x_value;\n"
+ "var y_value;\n"
+ "var z_value;\n"
+ "var gate;\n"
+ "send = new WebSocket('ws://localhost:3334');\n"
+ "function getIP() {\n"
+ " return document.getElementById('iptext').value;\n"
+ "}\n"

+ "var updateLabel = function(slider) {\n"
+ "  var id = slider.id;\n"
+ "  var out = document.getElementById(id+'_label');\n"
+ "  out.innerHTML = slider.value;\n"
+ "}\n"
//Modulation!
+ "var modulate = function() {\n"
+ " var X = x_value;\n"
+ " var Y = y_value;\n"
+ " var Z = z_value;\n"
+ " document.querySelectorAll('.param').forEach(function(param){\n"
+ "    var min = param.min;\n"
+ "    var max = param.max;\n"
+ "    var initValue = param.value;\n"
+ "    var modWheightX = document.getElementById(param.id + '_x').value;\n"
+ "    var modWheightY = document.getElementById(param.id + '_y').value;\n"
+ "    var modWheightZ = document.getElementById(param.id + '_z').value;\n"
+ "    var step = param.max / 100;\n"
+ "    var modX = Number(step) * Number(modWheightX) * Number(X);\n"
+ "    var modY = Number(step) * Number(modWheightY) * Number(Y);\n"
+ "    var modZ = Number(step) * Number(modWheightZ) * Number(Z);\n"
+ "    var modValue = clamp(Number(initValue) + modX + modY + modZ, min, max);\n"
+ "    document.getElementById(param.id + '_label').innerHTML = modValue;\n"
+ "    send.send(param.id + '#' + modValue);\n"
+ " });\n"
+ "}\n"
+ "function start() {\n"
+ "  document.querySelectorAll('.slider').forEach(function(slider) {\n"
+ "   var id = slider.id;\n"
+ "   slider.oninput = function() {\n"
+ "     console.log(id+'_label');\n"
+ "     modulate();\n"
//+ "     //var val = document.getElementById(param.id + '_label').innerHTML;\n"
+ "   }"
+ "  });"
+ "  document.querySelectorAll('.mod_value').forEach(function(modRange) {\n"
+ "    modRange.oninput = function() {\n"
+ "      updateLabel(modRange);\n"
+ "      modulate();\n"
+ "    }\n"
+ "  });\n"
+ " Socket = new WebSocket('ws://' + window.location.hostname + ':81/');\n"
+ " Socket.onmessage = function(evt) {\n"
+ "   document.getElementById(\"rxConsole\").value = \"\";\n"
+ "   document.getElementById(\"rxConsole\").value += evt.data;\n"
+ "   var splitData = evt.data.split(\":\");\n"
+ "   x_value = splitData[0];\n"
+ "   y_value = splitData[1];\n"
+ "   z_value = splitData[2];\n"
+ "   gate = splitData[3];\n"
+ "   send.send('gate#' + gate);\n"
+ "   modulate();\n"
+ " }\n"
+ "}\n"
+ "function clamp(num, min, max) {\n"
+ "  return num <= min ? min : num >= max ? max : num;\n"
+ "}\n"
+ "</script>\n"
+ "<style>"
+ " p {"
+ "  display: inline;"
+ " }"
+ "</style>"
+ "<textarea id=\"rxConsole\" readonly></textarea>\n"
+ "        <body>"
+ "                <div id='params'>"
+ "                        <div id='osc-group'>"
+ "                               <p>OSC 1 AMP</p>"
+ "                               <input type='range' min='0.0' max='1.0' value='0.0' step='0.01' class='slider param' id='osc_square_amp'>"
+ "                               <p id='osc_square_amp_label'>0.0</p><br>"
+ "                               <p>X Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='osc_square_amp_x' class='mod_value'><p id='osc_square_amp_x_label'>0</p><br>"
+ "                               <p>Y Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='osc_square_amp_y' class='mod_value'><p id='osc_square_amp_y_label'>0</p><br>"
+ "                               <p>Z Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='osc_square_amp_z' class='mod_value'><p id='osc_square_amp_z_label'>0</p><br>"
+ "                                <br>"
+ "                                <p>OSC 1 Pitch</p>"
+ "                                <input type='range' min='0' max='127' value='0' step='0.01' class='slider param' id='osc_square_pitch'>"
+ "                                <p id='osc_square_pitch_label'>0</p><br>"
+ "                               <p>X Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='osc_square_pitch_x' class='mod_value'><p id='osc_square_pitch_x_label'>0</p><br>"
+ "                               <p>Y Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='osc_square_pitch_y' class='mod_value'><p id='osc_square_pitch_y_label'>0</p><br>"
+ "                               <p>Z Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='osc_square_pitch_z' class='mod_value'><p id='osc_square_pitch_z_label'>0</p><br>"
+ "                                <br>"
+ "                                <p>OSC 2 AMP</p>"
+ "                                <p id='osc_sine_amp_label'>0.0</p><br>"
+ "                                <input type='range' min='0.0' max='1.0' value='0.0' step='0.01' class='slider param' id='osc_sine_amp'>"
+ "                               <p>X Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='osc_sine_amp_x' class='mod_value'><p id='osc_sine_amp_x_label'>0</p><br>"
+ "                               <p>Y Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='osc_sine_amp_y' class='mod_value'><p id='osc_sine_amp_y_label'>0</p><br>"
+ "                               <p>Z Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='osc_sine_amp_z' class='mod_value'><p id='osc_sine_amp_z_label'>0</p><br>"
+ "                                <br>"
+ "                                <p>OSC 2 Pitch</p>"
+ "                                <input type='range' min='0' max='127' value='0' step='0.01' class='slider param' id='osc_sine_pitch'>"
+ "                                <p id='osc_sine_pitch_label'>0</p><br>"
+ "                               <p>X Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='osc_sine_pitch_x' class='mod_value'><p id='osc_sine_pitch_x_label'>0</p><br>"
+ "                               <p>Y Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='osc_sine_pitch_y' class='mod_value'><p id='osc_sine_pitch_y_label'>0</p><br>"
+ "                               <p>Z Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='osc_sine_pitch_z' class='mod_value'><p id='osc_sine_pitch_z_label'>0</p><br>"

+ "                        </div>"
+ "                        <br>"
+ "                        <div id='filter-group'>"
+ "                                <p>LowPass Cutoff</p>"
+ "                                <input type='range' min='0.0' max='127.0' value='0.0' step='0.5' class='slider param' id='lp_co'>"
+ "                                <p id='lp_co_label'>0.0</p><br>"
+ "                               <p>X Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='lp_co_x' class='mod_value'><p id='lp_co_x_label'>0</p><br>"
+ "                               <p>Y Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='lp_co_y' class='mod_value'><p id='lp_co_y_label'>0</p><br>"
+ "                               <p>Z Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='lp_co_z' class='mod_value'><p id='lp_co_z_label'>0</p><br>"
+ "                                <br>"
+ "                                <p>LowPass Filter Mix</p>"
+ "                                <input type='range' min='0.0' max='1.0' value='0.0' step='0.01' class='slider param' id='lp_mix'>"
+ "                                <p id='lp_mix_label'>0.0</p><br>"
+ "                               <p>X Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='lp_mix_x' class='mod_value'><p id='lp_mix_x_label'>0</p><br>"
+ "                               <p>Y Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='lp_mix_y' class='mod_value'><p id='lp_mix_y_label'>0</p><br>"
+ "                               <p>Z Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='lp_mix_z' class='mod_value'><p id='lp_mix_z_label'>0</p><br>"
+ "                                <br>"
+ "                                <p>BandPass Band Position</p>"
+ "                                <input type='range' min='0.0' max='127.0' value='0.0' step='0.5' class='slider param' id='bp_pos'>"
+ "                                <p id='bp_pos_label'>0.0</p><br>"
+ "                               <p>X Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='bp_pos_x' class='mod_value'><p id='bp_pos_x_label'>0</p><br>"
+ "                               <p>Y Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='bp_pos_y' class='mod_value'><p id='bp_pos_y_label'>0</p><br>"
+ "                               <p>Z Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='bp_pos_z' class='mod_value'><p id='bp_pos_z_label'>0</p><br>"
+ "                                <br>"
+ "                                <p>BandPass Band Width</p>"
+ "                                <input type='range' min='1' max='10' value='0' step='0.1' class='slider param' id='bp_width'>"
+ "                                <p id='bp_width_label'>0</p><br>"
+ "                               <p>X Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='bp_width_x' class='mod_value'><p id='bp_width_x_label'>0</p><br>"
+ "                               <p>Y Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='bp_width_y' class='mod_value'><p id='bp_width_y_label'>0</p><br>"
+ "                               <p>Z Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='bp_width_z' class='mod_value'><p id='bp_width_z_label'>0</p><br>"
+ "                                <br>"
+ "                                <p>BandPass Filter Mix</p>"
+ "                                <input type='range' min='0.0' max='1.0' value='0.0' step='0.01' class='slider param' id='bp_mix'>"
+ "                                <p id='bp_mix_label'>0.0</p><br>"
+ "                               <p>X Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='bp_mix_x' class='mod_value'><p id='bp_mix_x_label'>0</p><br>"
+ "                               <p>Y Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='bp_mix_y' class='mod_value'><p id='bp_mix_y_label'>0</p><br>"
+ "                               <p>Z Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='bp_mix_z' class='mod_value'><p id='bp_mix_z_label'>0</p><br>"
+ "                        </div>"
+ "                        <br>"
+ "                        <div id='general-group'>"
+ "                                <p>Volume</p>"
+ "                                <input type='range' min='0' max='5' value='1.0' step='0.1' class='slider param' id='master_vol'>"
+ "                                <p id='master_vol_label'>1.0</p><br>"
+ "                               <p>X Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='master_vol_x' class='mod_value'><p id='master_vol_x_label'>0</p><br>"
+ "                               <p>Y Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='master_vol_y' class='mod_value'><p id='master_vol_y_label'>0</p><br>"
+ "                               <p>Z Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='master_vol_z' class='mod_value'><p id='master_vol_z_label'>0</p><br>"
+ "                                <br>"
+ "                                <p>Envelope Ramp Time</p>"
+ "                                <input type='range' min='0' max='500' value='0' step='1' class='slider param' id='ar_time'>"
+ "                                <p id='ar_time_label'>0</p>ms<br>"
+ "                               <p>X Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='ar_time_x' class='mod_value'><p id='ar_time_x_label'>0</p><br>"
+ "                               <p>Y Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='ar_time_y' class='mod_value'><p id='ar_time_y_label'>0</p><br>"
+ "                               <p>Z Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='ar_time_z' class='mod_value'><p id='ar_time_z_label'>0</p><br>"
+ "                                <br>"
+ "                                <p>Delay Amp</p>"
+ "                                <input type='range' min='0' max='0.5' value='0' step='0.01' class='slider param' id='delay_amp'>"
+ "                                <p id='delay_amp_label'>0</p><br>"   
+ "                               <p>X Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='delay_amp_x' class='mod_value'><p id='delay_amp_x_label'>0</p><br>"
+ "                               <p>Y Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='delay_amp_y' class='mod_value'><p id='delay_amp_y_label'>0</p><br>"
+ "                               <p>Z Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.1' id='delay_amp_z' class='mod_value'><p id='delay_amp_z_label'>0</p><br>"
+ "                        </div>"
+ "                        <br>"
+ "                </div>"
+ "           </body>";

void setup() {
  Serial.begin(9600);
  while(!Serial){
    delay(500);
  };
  
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));   // subnet FF FF FF 00  
 
  WiFi.softAP(ssid);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  
  startWebSocket();

  server.on("/", [](){
    server.send(200, "text/html", page);
  });

  server.begin();

  Skywriter.begin(PIN_TRFR, PIN_RESET);
  Skywriter.onXYZ(handle_xyz);

}

void loop() {
  gate_value = 0;
  Skywriter.poll();
  webSocket.loop();
  server.handleClient();
  sendValues();
}

void startWebSocket() {
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void sendValues() {
 char buf[13];
 sprintf(buf, "%03u:%03u:%03u:%01u", x_value, y_value, z_value, gate_value);
 Serial.println(buf);
 webSocket.broadcastTXT(buf, sizeof(buf)); 
}

void handle_xyz(unsigned int x, unsigned int y, unsigned int z){
 x_value = map(x, 0, 65536, 0, 100);
 y_value = map(y, 0, 65536, 0, 100);
 z_value = map(z, 0, 65536, 0, 100);
 gate_value = 1;
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\n", num, payload);
      break;
  }
}


