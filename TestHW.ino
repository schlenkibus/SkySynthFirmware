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
+ "function reconnectToWebsocket() {\n"
+ " send = new WebSocket('ws://' + getIP() + ':3333');\n"
+ "}\n"
+ "function start() {\n"
+ " reconnectToWebsocket();\n"
+ " send.onclose = function(event) {\n"
+ "   setTimeout(function(){reconnectToWebsocket('ws://localhost:3333')}, 1000);\n"
+ " }\n"
+ " Socket = new WebSocket('ws://' + window.location.hostname + ':81/');\n"
+ " Socket.onmessage = function(evt) {\n"
+ "   document.getElementById(\"rxConsole\").value = \"\";\n"
+ "   document.getElementById(\"rxConsole\").value += evt.data;\n"
+ "   if(send.readyState != send.CLOSED)\n"
+ "     send.send(evt.data);\n"
+ " }\n"
+ "}\n"
+ "</script>\n"
+ "<textarea id=\"rxConsole\" readonly></textarea>\n"
+ "<textarea id='iptext'>localhost</textarea>\n"
+ "<button id='reconnect' onclick='reconnectToWebsocket'>reconnect</button>\n";

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
 char buf[28];
 sprintf(buf, "X:%05uY:%05uZ:%05uGATE:%01u", x_value, y_value, z_value, gate_value);
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


