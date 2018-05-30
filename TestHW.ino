#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include <WebSockets.h>
#include <WebSocketsClient.h>
#include <WebSocketsServer.h>

#include <Wire.h>
#include <skywriter.h>

#define PIN_TRFR  13    // TRFR Pin of Skywriter
#define PIN_RESET 14    // Reset Pin of Skywriter

#define PIN_SDA 1
#define PIN_SCL 3

long x_value = 0;
long y_value = 0;
long z_value = 0;

const char* ssid = "BioMuell";
const char* password = "88888888";

WebSocketsServer webSocket = WebSocketsServer(81);
ESP8266WebServer server(80);

String page = "<h1>Hello!</h1>";

void setup() {
  
  Serial.begin(9600);
  while(!Serial){
    delay(500);
  };
  //WifiAcessPoint
  IPAddress ip(192, 168, 1, 24);
  IPAddress gateway(192, 168, 1, 1); // set gateway to match your network 
  IPAddress subnet(255, 255, 255, 0); // set subnet mask to match your network
  WiFi.config(ip, gateway, subnet);
  WiFi.mode(WIFI_STA);
  
  WiFi.softAP(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  //Skywriter Setup
  Wire.begin(PIN_SDA, PIN_SCL); 

  Skywriter.begin(PIN_TRFR, PIN_RESET);
  Skywriter.onXYZ(handle_xyz);

  //Websockets Setup
  webSocket.begin(); 
  webSocket.onEvent(webSocketEvent); 


  server.on("/yeet", [](){
    server.send(200, "text/html", page);
  });

  server.begin();

}

void loop() {
  Skywriter.poll();
  webSocket.loop();  
  server.handleClient();
}

void handle_xyz(unsigned int x, unsigned int y, unsigned int z){
 x_value = x;
 y_value = y;
 z_value = z;
 char buf[17];
 sprintf(buf, "%05u:%05u:%05u", x, y, z);
 Serial.println(buf);

 webSocket.broadcastTXT(buf, sizeof(buf));
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length){ 
 /* if (type == WStype_TEXT){ 
   for(int i = 0; i < length; i++) Serial.print((char) payload[i]); 
   Serial.println(); 
  }*/ 
} 



