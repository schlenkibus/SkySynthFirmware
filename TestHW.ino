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
+ "function getIP() {\n"
+ " return document.getElementById('iptext').value;\n"
+ "}\n"
//FUNCTION reconnecting to PD!
+ "function reconnectToWebsocket() {\n"
+ " send = new WebSocket('ws://' + getIP() + ':3334');\n"
+ "}\n"
+ "function start() {\n"
//FUNCTION label updating!
+ "  document.querySelectorAll('.slider').forEach(function(slider) {\n"
+ "   var id = slider.id;\n"
+ "   slider.oninput = function() {\n"
+ "     console.log(id+'_label');\n"
+ "     var out = document.getElementById(id+'_label');"
+ "     out.innerHTML = this.value;"
+ "     var val = slider.value;"
+ "     send.send(id + '#' + slider.value);"
+ "   }"
+ "  });"
+ " reconnectToWebsocket();\n"
+ " Socket = new WebSocket('ws://' + window.location.hostname + ':81/');\n"
+ " Socket.onmessage = function(evt) {\n"
+ "   document.getElementById(\"rxConsole\").value = \"\";\n"
+ "   document.getElementById(\"rxConsole\").value += evt.data;\n"
+ " }\n"
+ "}\n"
+ "</script>\n"
+ "<style>"
+ " p {"
+ "  display: inline;"
+ " }"
+ "</style>"
+ "<textarea id=\"rxConsole\" readonly></textarea>\n"
+ "<textarea id='iptext'>localhost</textarea>\n"
+ "<button id='reconnect' onclick='reconnectToWebsocket'>reconnect</button>\n"
+ "        <body>"
+ "                <div id='params'>"
+ "                        <div id='osc-group'>"
+ "                                <p>OSC 1 AMP</p>"
+ "                                <input type='range' min='0.0' max='1.0' value='0.0' step='0.01' class='slider' id='osc_square_amp'>"
+ "                                <p id='osc_square_amp_label'>0.0</p>"
+ "                                <br>"
+ "                                <p>OSC 1 Pitch</p>"
+ "                                <input type='range' min='0' max='127' value='0' step='0.01' class='slider' id='osc_square_pitch'>"
+ "                                <p id='osc_square_pitch_label'>0</p>"
+ "                                <br>"
+ "                                <p>OSC 2 AMP</p>"
+ "                                <input type='range' min='0.0' max='1.0' value='0.0' step='0.01' class='slider' id='osc_sine_amp'>"
+ "                                <p id='osc_sine_amp_label'>0.0</p>"
+ "                                <br>"
+ "                                <p>OSC 2 Pitch</p>"
+ "                                <input type='range' min='0' max='127' value='0' step='0.01' class='slider' id='osc_sine_pitch'>"
+ "                                <p id='osc_sine_pitch_label'>0</p>"
+ "                        </div>"
+ "                        <br>"
+ "                        <div id='filter-group'>"
+ "                                <p>HighPass Cutoff</p>"
+ "                                <input type='range' min='0.0' max='2000.0' value='0.0' step='0.5' class='slider' id='hp_co'>"
+ "                                <p id='hp_co_label'>0.0</p>"
+ "                                <br>"
+ "                                <p>HighPass Filter Mix</p>"
+ "                                <input type='range' min='0.0' max='1.0' value='0.0' step='0.01' class='slider' id='hp_mix'>"
+ "                                <p id='hp_mix_label'>0.0</p>"
+ "                                <br>"
+ "                                <p>LowPass Cutoff</p>"
+ "                                <input type='range' min='0.0' max='2000.0' value='0.0' step='0.5' class='slider' id='lp_co'>"
+ "                                <p id='lp_co_label'>0.0</p>"
+ "                                <br>"
+ "                                <p>LowPass Filter Mix</p>"
+ "                                <input type='range' min='0.0' max='1.0' value='0.0' step='0.01' class='slider' id='lp_mix'>"
+ "                                <p id='lp_mix_label'>0.0</p>"
+ "                                <br>"
+ "                                <p>BandPass Band Position</p>"
+ "                                <input type='range' min='0.0' max='2000.0' value='0.0' step='0.5' class='slider' id='bp_pos'>"
+ "                                <p id='bp_pos_label'>0.0</p>"
+ "                                <br>"
+ "                                <p>BandPass Band Width</p>"
+ "                                <input type='range' min='1' max='127' value='0' step='0.1' class='slider' id='bp_width'>"
+ "                                <p id='bp_width_label'>0</p>"
+ "                                <br>"
+ "                                <p>BandPass Filter Mix</p>"
+ "                                <input type='range' min='0.0' max='1.0' value='0.0' step='0.01' class='slider' id='bp_mix'>"
+ "                                <p id='bp_mix_label'>0.0</p>"
+ "                        </div>"
+ "                        <br>"
+ "                        <div id='general-group'>"
+ "                                <p>Volume</p>"
+ "                                <input type='range' min='0' max='5' value='1.0' step='0.1' class='slider' id='master_vol'>"
+ "                                <p id='master_vol_label'>1.0</p>"
+ "                                <br>"
+ "                                <p>Envelope Ramp Time</p>"
+ "                                <input type='range' min='0' max='500' value='0' step='1' class='slider' id='ar_time'>"
+ "                                <p id='ar_time_label'>0</p>ms"
+ "                        </div>"
+ "                        <br>"
+ "                        <div id='mc-group'>"
+ "                           <p>MC X:</p><select>"
+ "                             <option value='osc_square_amp'>OSC 1 AMP</option>"
+ "                             <option value='osc_square_pitch'>OSC 1 Pitch</option>"
+ "                             <option value='osc_sine_amp'>OSC 2 AMP</option>"
+ "                             <option value='osc_sine_pitch'>OSC 2 Pitch</option>"
+ "                             <option value='hp_co'>Highpass Cutoff</option>"
+ "                             <option value='hp_mix'>Highpass Mix</option>"
+ "                             <option value='lp_co'>LowPass Cutoff</option>"
+ "                             <option value='lp_mix'>LowPass Mix</option>"
+ "                             <option value='bp_pos'>BandPass Position</option>"
+ "                             <option value='bp_width'>BandPass Band Width</option>"
+ "                             <option value='bp_mix'>BandPass Mix</option>"
+ "                             <option value='master_vol'>Master Volume</option>"
+ "                           </select>"
+ "                        </div>"
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
 char buf[27];
 sprintf(buf, "X:%05uY:%05uZ:%05uGATE:%01u", x_value, y_value, z_value, gate_value);
 Serial.println(buf);
 webSocket.broadcastTXT(buf, sizeof(buf)); 
}

void handle_xyz(unsigned int x, unsigned int y, unsigned int z){
 x_value = x;
 y_value = y;
 z_value = z;
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


