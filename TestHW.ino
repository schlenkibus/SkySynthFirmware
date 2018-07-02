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

const String getJavaScript() {
  return String("<body onload='javascript:start();'>")
         + "<script>\n"
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
         + "function showHelp(divId) {"
         + "    var div = document.getElementById(divId);"
         + "    if (div.style.display == 'none') {"
         + "        div.style.display = 'block';"
         + "    }"
         + "    else"
         + "        div.style.display = 'none';"
         + "}"
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
         + " drawCanvas();\n"
         + " }\n"
         + "}\n"
         + "function clamp(num, min, max) {\n"
         + "  return num <= min ? min : num >= max ? max : num;\n"
         + "}\n"
         + "  function drawCanvas() {\n"
         + "    var ctx = document.getElementById('canvas').getContext('2d');\n"
         + "    ctx.clearRect(0, 0, canvas.width, canvas.height);\n"
         + "    ctx.fillStyle = 'rgb(0,0,0)';\n"
         + "    ctx.font = '12px Arial';\n"
         + "    ctx.fillText('100', 30, 40);\n"
         + "    ctx.fillText('50', 30, 270);\n"
         + "    ctx.font = '14px Arial';\n"
         + "    ctx.beginPath();\n"
         + "    ctx.moveTo(40,0);\n"
         + "    ctx.lineTo(40, 500);\n"
         + "    ctx.lineTo(740, 500);\n"
         + "    ctx.strokeStyle = 'rgb(0, 0, 0)';\n"
         + "    ctx.stroke();\n"
         + "    ctx.fillStyle = 'rgb(' + Number(130 + 100 - z_value) + ',133,' + Number(130 + z_value) + ')';\n"
         + "    ctx.beginPath();\n"
         + "    var ypos =  500 - Number(y_value * 5);\n"
         + "    if(gate == 1) {\n"
         + "     ctx.arc(40 + x_value * 7, ypos, 20, 0, 2 * Math.PI);\n"
         + "     ctx.fill();\n"
         + "    }\n"
         + "}\n"
         + "</script>\n";
}

const String getCss() {
  return String(" <style>\n")
         + "p {"
         + "  display: inline;"
         + " }"
         + ".group {"
         + "   width: 300px;"
         + "   padding: 20px;"
         + "   background-color: green;"
         + "}"
         + ".parameter {"
         + "   padding: 5px;"
         + "   background-color: lightgrey;"
         + "   text-align: center;"
         + "   display: block;"
         + "}"
         + " .parameter.p:before {"
         + "  display: block;"
         + "}"

         + ".parameter p {"
         + "  color: black;"
         + "  font-style: oblique;"
         + "  font-size: 20pt;"
         + "  display: block;"
         + "  margin: -4px;"
         + "}"

         + ".osc {"
         + "background-color: lightcoral;"
         + "}"
         + ".filter {"
         + "background-color: lightpink;"
         + "}"
         + ".volume {"
         + "background-color: lightsalmon;"
         + "}"
         + ".effects {"
         + "background-color: chartreuse;"
         + "}"

         + ".content {"
         + "margin: 4px -5px -5px -5px;"
         + "  padding-top: 6px;"
         + "  width: 100%;"
         + "  display: none;"
         + "  background-color: lightsteelblue;"
         + "  text-align: center;"
         + "}"
         + ".content p {"
         + "padding-top: 6px;"
         + "font-size: 14pt;"
         + "font-style: normal;"
         + "}"
         + ".header div {"
         + "  border-width: 20px;"
         + "  border-style: outset;"
         + "}"
         + ".header:hover {"
         + "  color: blue;"
         + "}"
         + "</style>";
}

const String getGeneral() {
  return String("<textarea id=\"rxConsole\" readonly></textarea>\n")
         + "<body>"
         + "<canvas id='canvas' width='750' height='554'></canvas>\n"
         + "<div class='volume group'><div class='parameter'><h2>Volume</h2>"
         + "<input type='range' min='0' max='5' value='1.0' step='0.1' class='slider param' id='master_vol'>"
         + "<p id='master_vol_label'>1.0</p><br>"
         + "<div id='master_vol_mc' class='header' onclick=showHelp('master_vol_mc_content')>Macro Controls...</div>"
         + "<div id='master_vol_mc_content' class='content'>"
         + "<p>X Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='master_vol_x' class='mod_value'><p id='master_vol_x_label'>0</p><br>"
         + "<p>Y Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='master_vol_y' class='mod_value'><p id='master_vol_y_label'>0</p><br>"
         + "<p>Z Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='master_vol_z' class='mod_value'><p id='master_vol_z_label'>0</p><br>"
         + "</div></div></div>"
         + "<br>"
         + "<div id='params'>";
}

const String getOSC1() {
  return String("<div class='osc group'>")
         + "<div class='osc1'>"
         + "<div class='parameter'>"
         + "<h2>OSC 1 AMP</h2>"
         + "<input type='range' min='0.0' max='1.0' value='0.0' step='0.01' class='slider param' id='osc_square_amp'>"
         + "<p id='osc_square_amp_label'>0.0</p><br>"
         + "<div id='osc_square_amp_mc' class='header' onclick=showHelp('osc_square_amp_mc_content')>Macro Controls...</div>"
         + "<div id='osc_square_amp_mc_content' class='content'>"
         + "<p>X Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='osc_square_amp_x' class='mod_value'><p id='osc_square_amp_x_label'>0</p><br>"
         + "<p>Y Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='osc_square_amp_y' class='mod_value'><p id='osc_square_amp_y_label'>0</p><br>"
         + "<p>Z Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='osc_square_amp_z' class='mod_value'><p id='osc_square_amp_z_label'>0</p><br>"
         + "</div></div>"
         + "<br>"
         + "<div class='parameter'>"
         + "<h2>OSC 1 Pitch</h2>"
         + "<input type='range' min='0' max='127' value='0' step='0.01' class='slider param' id='osc_square_pitch'>"
         + "<p id='osc_square_pitch_label'>0</p><br>"
         + "<div id='osc_square_pitch_mc' class='header' onclick=showHelp('osc_square_pitch_mc_content')>Macro Controls...</div>"
         + "<div id='osc_square_pitch_mc_content' class='content'>"
         + "<p>X Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='osc_square_pitch_x' class='mod_value'><p id='osc_square_pitch_x_label'>0</p><br>"
         + "<p>Y Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='osc_square_pitch_y' class='mod_value'><p id='osc_square_pitch_y_label'>0</p><br>"
         + "<p>Z Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='osc_square_pitch_z' class='mod_value'><p id='osc_square_pitch_z_label'>0</p><br>"
         + "</div></div>"
         + "<br>"
         + "</div>";
}

const String getOSC2() {
  return String("<div class='osc2'><div class='parameter'><h2>OSC 2 AMP</h2>")
         + "<input type='range' min='0.0' max='1.0' value='0.0' step='0.01' class='slider param' id='osc_sine_amp'><p id='osc_sine_amp_label'>0.0</p><br>"
         + "<div id='osc_sine_amp_mc' class='header' onclick=showHelp('osc_sine_amp_mc_content')>Macro Controls...</div>"
         + "<div id='osc_sine_amp_mc_content' class='content'>"
         + "<p>X Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='osc_sine_amp_x' class='mod_value'><p id='osc_sine_amp_x_label'>0</p><br>"
         + "<p>Y Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='osc_sine_amp_y' class='mod_value'><p id='osc_sine_amp_y_label'>0</p><br>"
         + "<p>Z Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='osc_sine_amp_z' class='mod_value'><p id='osc_sine_amp_z_label'>0</p><br>"
         + "</div></div><br><div class='parameter'>"
         + "<h2>OSC 2 Pitch</h2>"
         + "<input type='range' min='0' max='127' value='0' step='0.01' class='slider param' id='osc_sine_pitch'>"
         + "<p id='osc_sine_pitch_label'>0</p><br>"
         + "<div id='osc_sine_pitch_mc' class='header' onclick=showHelp('osc_sine_pitch_mc_content')>Macro Controls...</div>"
         + "<div id='osc_sine_pitch_mc_content' class='content'>"
         + "<p>X Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='osc_sine_pitch_x' class='mod_value'><p id='osc_sine_pitch_x_label'>0</p><br>"
         + "<p>Y Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='osc_sine_pitch_y' class='mod_value'><p id='osc_sine_pitch_y_label'>0</p><br>"
         + "<p>Z Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='osc_sine_pitch_z' class='mod_value'><p id='osc_sine_pitch_z_label'>0</p><br>"
         + "</div></div></div></div>";
}

const String getFilterLP() {
  return String("<br>")
         + "<div class='filter group'><div class='filterlp'>"
         + "<div class='parameter'><h2>LowPass Cutoff</h2>"
         + "<input type='range' min='0.0' max='127.0' value='0.0' step='0.5' class='slider param' id='lp_co'>"
         + "<p id='lp_co_label'>0.0</p><br>"
         + "<div id='lp_co_mc' class='header' onclick=showHelp('lp_co_content')>Macro Controls...</div>"
         + "<div id='lp_co_content' class='content'>"
         + "<p>X Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='lp_co_x' class='mod_value'><p id='lp_co_x_label'>0</p><br>"
         + "<p>Y Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='lp_co_y' class='mod_value'><p id='lp_co_y_label'>0</p><br>"
         + "<p>Z Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='lp_co_z' class='mod_value'><p id='lp_co_z_label'>0</p><br>"
         + "</div></div><br>"
         + "<div class='parameter'><h2>LowPass Filter Mix</h2>"
         + "<input type='range' min='0.0' max='1.0' value='0.0' step='0.01' class='slider param' id='lp_mix'>"
         + "<p id='lp_mix_label'>0.0</p><br>"
         + "<div id='lp_mix_mc' class='header' onclick=showHelp('lp_mix_content')>Macro Controls...</div>"
         + "<div id='lp_mix_content' class='content'>"
         + "<p>X Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='lp_mix_x' class='mod_value'><p id='lp_mix_x_label'>0</p><br>"
         + "<p>Y Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='lp_mix_y' class='mod_value'><p id='lp_mix_y_label'>0</p><br>"
         + "<p>Z Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='lp_mix_z' class='mod_value'><p id='lp_mix_z_label'>0</p><br>"
         + "</div></div></div><br>";
}

const String getFilterBP() {
  return String("<div class='filterbp'><div class='parameter'><h2>BandPass Band Position</h2>")
         + "<input type='range' min='0.0' max='127.0' value='0.0' step='0.5' class='slider param' id='bp_pos'>"
         + "<p id='bp_pos_label'>0.0</p><br>"
         + "<div id='bp_pos_mc' class='header' onclick=showHelp('bp_pos_content')>Macro Controls...</div>"
         + "<div id='bp_pos_content' class='content'>"
         + "<p>X Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='bp_pos_x' class='mod_value'><p id='bp_pos_x_label'>0</p><br>"
         + "<p>Y Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='bp_pos_y' class='mod_value'><p id='bp_pos_y_label'>0</p><br>"
         + "<p>Z Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='bp_pos_z' class='mod_value'><p id='bp_pos_z_label'>0</p><br>"
         + "</div></div><br>"
         + "<div class='parameter'><h2>BandPass Band Width</h2>"
         + "<input type='range' min='1' max='10' value='0' step='0.1' class='slider param' id='bp_width'>"
         + "<p id='bp_width_label'>0</p><br>"
         + "<div id='bp_width_mc' class='header' onclick=showHelp('bp_width_content')>Macro Controls...</div>"
         + "<div id='bp_width_content' class='content'>"
         + "<p>X Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='bp_width_x' class='mod_value'><p id='bp_width_x_label'>0</p><br>"
         + "<p>Y Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='bp_width_y' class='mod_value'><p id='bp_width_y_label'>0</p><br>"
         + "<p>Z Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='bp_width_z' class='mod_value'><p id='bp_width_z_label'>0</p><br>"
         + "</div></div><br>"
         + "<div class='parameter'><h2>BandPass Filter Mix</h2>"
         + "<input type='range' min='0.0' max='1.0' value='0.0' step='0.01' class='slider param' id='bp_mix'>"
         + "<p id='bp_mix_label'>0.0</p><br>"
         + "<div id='bp_mix_mc' class='header' onclick=showHelp('bp_mix_content')>Macro Controls...</div>"
         + "<div id='bp_mix_content' class='content'>"
         + "<p>X Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='bp_mix_x' class='mod_value'><p id='bp_mix_x_label'>0</p><br>"
         + "<p>Y Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='bp_mix_y' class='mod_value'><p id='bp_mix_y_label'>0</p><br>"
         + "<p>Z Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='bp_mix_z' class='mod_value'><p id='bp_mix_z_label'>0</p><br>"
         + "</div></div></div></div><br>";
}

const String getEffects() {
  return String("<div class='effects group'>")
         + "<div class='parameter'><h2>Envelope Ramp Time</h2>"
         + "<input type='range' min='0' max='500' value='0' step='1' class='slider param' id='ar_time'>"
         + "<p id='ar_time_label'>0</p>ms<br>"
         + "<div id='ar_time_mc' class='header' onclick=showHelp('ar_time_content')>Macro Controls...</div>"
         + "<div id='ar_time_content' class='content'>"
         + "<p>X Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='ar_time_x' class='mod_value'><p id='ar_time_x_label'>0</p><br>"
         + "<p>Y Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='ar_time_y' class='mod_value'><p id='ar_time_y_label'>0</p><br>"
         + "<p>Z Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='ar_time_z' class='mod_value'><p id='ar_time_z_label'>0</p><br>"
         + "</div></div><br>"
         + "<div class='parameter'><h2>Delay Amp</h2>"
         + "<input type='range' min='0' max='0.5' value='0' step='0.01' class='slider param' id='delay_amp'>"
         + "<p id='delay_amp_label'>0</p><br>"
         + "<div id='delay_amp_mc' class='header' onclick=showHelp('delay_amp_content')>Macro Controls...</div>"
         + "<div id='delay_amp_content' class='content'>"
         + "<p>X Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='delay_amp_x' class='mod_value'><p id='delay_amp_x_label'>0</p><br>"
         + "<p>Y Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='delay_amp_y' class='mod_value'><p id='delay_amp_y_label'>0</p><br>"
         + "<p>Z Mod Ammount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='delay_amp_z' class='mod_value'><p id='delay_amp_z_label'>0</p><br>"
         + "</div></div></div></div>"
         + "</body>";
}

String tutorial = "Tutorial WIP\n";

char buf[13];
int normalizedX;
int normalizedY;
int normalizedZ;

const auto lenght = getJavaScript().length() + getCss().length() + getGeneral().length() + getOSC1().length() + getOSC2().length() + getFilterLP().length() + getFilterBP().length() + getEffects().length();

void setup() {
  ESP.wdtDisable();
  Serial.begin(19200);
  while (!Serial) {
    delay(500);
  };

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid);

  Serial.println("Starting!");
  ESP.wdtEnable(15000);
  startWebSocket();


  server.on("/", []() {
    Serial.println("Requested / ");
    ESP.wdtDisable();
    server.setContentLength(lenght);
    server.send(200, "text/html", getJavaScript());
    server.sendContent(getCss());
    server.sendContent(getGeneral());
    server.sendContent(getOSC1());
    server.sendContent(getOSC2());
    server.sendContent(getFilterLP());
    server.sendContent(getFilterBP());
    server.sendContent(getEffects());
    ESP.wdtEnable(15000);
  });

  server.begin();

  Skywriter.begin(PIN_TRFR, PIN_RESET);
  Skywriter.onXYZ(handle_xyz);
}

void loop() {
  gate_value = 0;
  Skywriter.poll();
  server.handleClient();
  webSocket.loop();
  sendValues();
}

void startWebSocket() {
  webSocket.begin();
}

void sendValues() {
  sprintf(buf, "%03u:%03u:%03u:%01u", x_value, y_value, z_value, gate_value);
  webSocket.broadcastTXT(buf, sizeof(buf));
}

void handle_xyz(unsigned int x, unsigned int y, unsigned int z) {
  gate_value = 1;
  x_value = map(x, 0, 65536, 0, 100);
  y_value = map(y, 0, 65536, 0, 100);
  z_value = map(z, 0, 65536, 0, 100);
}



