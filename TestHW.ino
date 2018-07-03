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

const char* ssid = "SkySynth V2";
const char* password = "sunaanus";

ESP8266WebServer server(80);
WebSocketsServer webSocket(81);
IPAddress apIP(42, 42, 42, 42);

const String getCanvasDrawing() {
  return String("<body onload='javascript:start();'><script>function drawCanvas() {")
         + "       var ctx = document.getElementById('canvas').getContext('2d'); "
         + "     var canvas = document.getElementById('canvas'); "
         + "     ctx.clearRect(0, 0, canvas.width, canvas.height); "
         + "     ctx.fillStyle = 'rgb(0,0,0)'; "
         + "     ctx.font = '12px Arial'; "
         + "     ctx.beginPath(); "
         + "     ctx.moveTo(40, 30); "
         + "     ctx.lineTo(40, 730); "
         + "     ctx.lineTo(1040, 730); "
         + "     ctx.strokeStyle = 'rgb(0,0,0)'; "
         + "     ctx.lineWidth = 3; "
         + "     ctx.stroke(); "
         + "     ctx.beginPath(); "
         + "     ctx.strokeStyle = 'rgb(224,224,224)'; "
         + "     var i = 40; "
         + "     while (i < 1040) {"
         + "           i += 100; "
         + "           ctx.moveTo(i, 30); "
         + "           ctx.lineTo(i, 727); "
         + "      }"
         + "       ctx.moveTo(0, 65); "
         + "        i = 30; "
         + "        while (i < 730) {  "
         + "            ctx.moveTo(42, i); "
         + "            ctx.lineTo(1040, i); "
         + "            i += 35; "
         + "        }"
         + "       ctx.stroke(); "
         + "       ctx.fillText('100', 8, 40); "
         + "       ctx.fillText('50', 15, 382); "
         + "       ctx.fillText('50', 520, 750); "
         + "       ctx.fillText('100', 1000, 750); "
         + "       ctx.fillText('0', 15, 730); "
         + "       ctx.fillText('0', 40, 757); "
         + "       ctx.font = 'bold 18px Arial'; "
         + "       ctx.fillText('y', 10, 80); "
         + "       ctx.fillText('x', 950, 750); "
         + "       ctx.beginPath(); "
         + "       ctx.strokeStyle = 'rgb(0,0,0)'; "
         + "       ctx.lineWidth = 4; "
         + "        var j = 40; "
         + "        while (j <= 1040) {  "
         + "          ctx.moveTo(j, 725); "
         + "          ctx.lineTo(j, 735); "
         + "          j += 100; "
         + "        }"
         + "        j = 30; "
         + "        while (j < 730) {"
         + "          ctx.moveTo(35, j); "
         + "          ctx.lineTo(45, j); "
         + "          j += 70; "
         + "        }"
         + "        ctx.stroke(); "
         + "        if (gate == 1) {  "
         + "          ctx.beginPath(); "
         + "          ctx.arc(40 + x_value * 10, 30 + Number(100 - y_value) * 7 , Math.max(Number(100 - z_value) * 0.4, 5), 0, 2 * Math.PI); "
         + "          ctx.fillStyle = 'rgb(238,130,238)'; "
         + "          ctx.fill(); "
         + "        }"
         + "    }";
}

const String getJavaScript() {
  return String("var Socket;\n")
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
         + "   var splitData = evt.data.split(\":\");\n"
         + "   x_value = splitData[0];\n"
         + "   y_value = splitData[1];\n"
         + "   z_value = splitData[2];\n"
         + "   gate = splitData[3];\n"
         + "   document.getElementById(\"rxConsole\").value = 'X: ' + x_value + ' Y: ' + y_value + ' Z: ' + z_value;\n"
         + "   send.send('gate#' + gate);\n"
         + "   modulate();\n"
         + " drawCanvas();\n"
         + " }\n"
         + "}\n"
         + "function clamp(num, min, max) {\n"
         + "  return num <= min ? min : num >= max ? max : num;\n"
         + "}</script>";
}

const String getCss() {
  return String("<style>\n")
         + "p {"
         + "  display: inline; "
         + " }"
         + ".group {"
         + "   width: 300px; "
         + "   padding: 20px; "
         + "   background-color: green; "
         + "}"
         + ".parameter {"
         + "   padding: 5px; "
         + "   background-color: lightgrey; "
         + "   text-align: center; "
         + "   display: block; "
         + "}"
         + " .parameter.p: before {"
         + "  display: block; "
         + "}"
         + ".parameter p {"
         + "  color: black; "
         + "  font-style: oblique; "
         + "  font-size: 20pt; "
         + "  display: block; "
         + "  margin: -4px; "
         + "}"
         + ".osc {"
         + "background-color: lightcoral; "
         + "}"
         + ".filter {"
         + "background-color: lightpink; "
         + "}"
         + ".volume {"
         + "background-color: lightsalmon; "
         + "}"
         + ".effects {"
         + "background-color: cadetblue; "
         + "}"
         + "#params {"
         + "padding-left: -40%;"
         + "display: inline-flex;"
         + "}"
         //MC Stuff
         + ".content {"
         + "  margin-top: 5px; "
         + "  padding-top: 6px; "
         + "  border-style: solid; "
         + "  display: none; "
         + "  background-color: lightsteelblue; "
         + "  text-align: center; "
         + "}"
         + ".content p {"
         + "padding-top: 6px; "
         + "font-size: 14pt; "
         + "font-style: normal; "
         + "}"
         + "#canvas {"
         + "display:block;"
         + "padding-left: 10%;"
         + "}"
         + ".header div {"
         + "  border-width: 20px; "
         + "  border-style: outset; "
         + "}body {background-color: lightslategray;}"
         + ".flexcontainer{ padding-left: 30%; }"
         + " .header:hover {  border-style: ridge;border-radius: 40px; }"
         + "#rxConsole {font-size: 25pt; color: #DA78D7; text-align: center; resize: none; background-color: grey; border: none; padding-top: 50px font-style: bold; }"
         + ".top { padding-bottom: 30px; background-color: grey; display: block-inline; width: 1360px; text-align: center;"
         + "}";
}

const String getGeneral() {
  return String("<body><div class='flexcontainer'><div class='top'><textarea id = \"rxConsole\" readonly></textarea>\n")
         + "<canvas id='canvas' width='1150' height='800'></canvas><a href='/tutorial'>How To Play!</a></div>\n"
         + "<div id='params'>"
         + "<div class='volume group'><div class='parameter'><h2>Volume</h2>"
         + "<input type='range' min='0' max='5' value='1.0' step='0.1' class='slider param' id='master_vol'>"
         + "<p id='master_vol_label'>1.0</p><br>"
         + "<div id='master_vol_mc' class='header' onclick=showHelp('master_vol_mc_content')>Macro Controls...</div>"
         + "<div id='master_vol_mc_content' class='content'>"
         + "<p>X Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='master_vol_x' class='mod_value'><p id='master_vol_x_label'>0</p><br>"
         + "<p>Y Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='master_vol_y' class='mod_value'><p id='master_vol_y_label'>0</p><br>"
         + "<p>Z Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='master_vol_z' class='mod_value'><p id='master_vol_z_label'>0</p><br>"
         + "</div>"+ addHelpBox("volume", "Gesamtlautstaerke / Volume - Lautstaerke des Instruments (Wenn Ausfaelle bei Audioausgabe auftreten diesen Parameter niedriger stellen!)") + "</div></div>"
         + "<br>";

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
         + "<p>X Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='osc_square_amp_x' class='mod_value'><p id='osc_square_amp_x_label'>0</p><br>"
         + "<p>Y Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='osc_square_amp_y' class='mod_value'><p id='osc_square_amp_y_label'>0</p><br>"
         + "<p>Z Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='osc_square_amp_z' class='mod_value'><p id='osc_square_amp_z_label'>0</p><br>"
         + "</div>"+ addHelpBox("osc1amp", "Oszillator Amplitiude - Lautstaerke des Oszillators (In Prozent 0-100).") + "</div>"
         + "<br>"
         + "<div class='parameter'>"
         + "<h2>OSC 1 Pitch</h2>"
         + "<input type='range' min='0' max='127' value='0' step='0.01' class='slider param' id='osc_square_pitch'>"
         + "<p id='osc_square_pitch_label'>0</p><br>"
         + "<div id='osc_square_pitch_mc' class='header' onclick=showHelp('osc_square_pitch_mc_content')>Macro Controls...</div>"
         + "<div id='osc_square_pitch_mc_content' class='content'>"
         + "<p>X Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='osc_square_pitch_x' class='mod_value'><p id='osc_square_pitch_x_label'>0</p><br>"
         + "<p>Y Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='osc_square_pitch_y' class='mod_value'><p id='osc_square_pitch_y_label'>0</p><br>"
         + "<p>Z Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='osc_square_pitch_z' class='mod_value'><p id='osc_square_pitch_z_label'>0</p><br>"
         + "</div>"+ addHelpBox("osc1pitch", "Oszillator Pitch - Frequenz des Oszillators (In MIDI-Noten 0-127).") + "</div>"
         + "<br>"
         + "</div>";
}

const String getOSC3() {
  return String("<div class='osc3'><div class='parameter'><h2>OSC 2 AMP</h2>")
         + "<input type='range' min='0.0' max='1.0' value='0.0' step='0.01' class='slider param' id='osc_3_amp'><p id='osc_3_amp_label'>0.0</p><br>"
         + "<div id='osc_3_amp_mc' class='header' onclick=showHelp('osc_3_amp_mc_content')>Macro Controls...</div>"
         + "<div id='osc_3_amp_mc_content' class='content'>"
         + "<p>X Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='osc_3_amp_x' class='mod_value'><p id='osc_3_amp_x_label'>0</p><br>"
         + "<p>Y Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='osc_3_amp_y' class='mod_value'><p id='osc_3_amp_y_label'>0</p><br>"
         + "<p>Z Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='osc_3_amp_z' class='mod_value'><p id='osc_3_amp_z_label'>0</p><br>"
         + "</div>"+ addHelpBox("osc3amp", "Oszillator Amplitiude - Lautstaerke des Oszillators (In Prozent 0-100).") + "</div><br><div class='parameter'>"
         + "<h2>OSC 2 Pitch</h2>"
         + "<input type='range' min='0' max='127' value='0' step='0.01' class='slider param' id='osc_3_pitch'>"
         + "<p id='osc_3_pitch_label'>0</p><br>"
         + "<div id='osc_3_pitch_mc' class='header' onclick=showHelp('osc_3_pitch_mc_content')>Macro Controls...</div>"
         + "<div id='osc_3_pitch_mc_content' class='content'>"
         + "<p>X Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='osc_3_pitch_x' class='mod_value'><p id='osc_3_pitch_x_label'>0</p><br>"
         + "<p>Y Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='osc_3_pitch_y' class='mod_value'><p id='osc_3_pitch_y_label'>0</p><br>"
         + "<p>Z Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='osc_3_pitch_z' class='mod_value'><p id='osc_3_pitch_z_label'>0</p><br>"
         + "</div>"+ addHelpBox("osc3pitch", "Oszillator Pitch - Frequenz des Oszillators (In MIDI-Noten 0-127).") + "</div></div></br>";
}

const String getOSC2() {
  return String("<div class='osc2'><div class='parameter'><h2>OSC 3 AMP</h2>")
         + "<input type='range' min='0.0' max='1.0' value='0.0' step='0.01' class='slider param' id='osc_sine_amp'><p id='osc_sine_amp_label'>0.0</p><br>"
         + "<div id='osc_sine_amp_mc' class='header' onclick=showHelp('osc_sine_amp_mc_content')>Macro Controls...</div>"
         + "<div id='osc_sine_amp_mc_content' class='content'>"
         + "<p>X Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='osc_sine_amp_x' class='mod_value'><p id='osc_sine_amp_x_label'>0</p><br>"
         + "<p>Y Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='osc_sine_amp_y' class='mod_value'><p id='osc_sine_amp_y_label'>0</p><br>"
         + "<p>Z Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='osc_sine_amp_z' class='mod_value'><p id='osc_sine_amp_z_label'>0</p><br>"
         + "</div>"+ addHelpBox("osc2amp", "Oszillator Amplitiude - Lautstaerke des Oszillators (In Prozent 0-100).") + "</div><br><div class='parameter'>"
         + "<h2>OSC 3 Pitch</h2>"
         + "<input type='range' min='0' max='127' value='0' step='0.01' class='slider param' id='osc_sine_pitch'>"
         + "<p id='osc_sine_pitch_label'>0</p><br>"
         + "<div id='osc_sine_pitch_mc' class='header' onclick=showHelp('osc_sine_pitch_mc_content')>Macro Controls...</div>"
         + "<div id='osc_sine_pitch_mc_content' class='content'>"
         + "<p>X Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='osc_sine_pitch_x' class='mod_value'><p id='osc_sine_pitch_x_label'>0</p><br>"
         + "<p>Y Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='osc_sine_pitch_y' class='mod_value'><p id='osc_sine_pitch_y_label'>0</p><br>"
         + "<p>Z Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='osc_sine_pitch_z' class='mod_value'><p id='osc_sine_pitch_z_label'>0</p><br>"
         + "</div>"+ addHelpBox("osc2pitch", "Oszillator Pitch - Frequenz des Oszillators (In MIDI-Noten 0-127).") + "</div></div></div></div>";
}

const String getFilterLP() {
  return String("<br>")
         + "<div class='filter group'><div class='filterlp'>"
         + "<div class='parameter'><h2>LowPass Cutoff</h2>"
         + "<input type='range' min='0.0' max='127.0' value='0.0' step='0.5' class='slider param' id='lp_co'>"
         + "<p id='lp_co_label'>0.0</p><br>"
         + "<div id='lp_co_mc' class='header' onclick=showHelp('lp_co_content')>Macro Controls...</div>"
         + "<div id='lp_co_content' class='content'>"
         + "<p>X Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='lp_co_x' class='mod_value'><p id='lp_co_x_label'>0</p><br>"
         + "<p>Y Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='lp_co_y' class='mod_value'><p id='lp_co_y_label'>0</p><br>"
         + "<p>Z Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='lp_co_z' class='mod_value'><p id='lp_co_z_label'>0</p><br>"
         + "</div>"+ addHelpBox("lpco", "LowPass Cutoff - An welcher Frequenz soll der LowPass-Filter den Sound beschneiden. (In MIDI-Noten 0-127)") + "</div><br>"
         + "<div class='parameter'><h2>LowPass Filter Mix</h2>"
         + "<input type='range' min='0.0' max='1.0' value='0.0' step='0.01' class='slider param' id='lp_mix'>"
         + "<p id='lp_mix_label'>0.0</p><br>"
         + "<div id='lp_mix_mc' class='header' onclick=showHelp('lp_mix_content')>Macro Controls...</div>"
         + "<div id='lp_mix_content' class='content'>"
         + "<p>X Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='lp_mix_x' class='mod_value'><p id='lp_mix_x_label'>0</p><br>"
         + "<p>Y Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='lp_mix_y' class='mod_value'><p id='lp_mix_y_label'>0</p><br>"
         + "<p>Z Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='lp_mix_z' class='mod_value'><p id='lp_mix_z_label'>0</p><br>"
         + "</div>"+ addHelpBox("lpmix", "LowPass Mix - Wie viel des durch Lowpass gefilterten Sounds soll zu hoeren sein (0 - 100%)") + "</div></div><br>";
}

const String getFilterBP() {
  return String("<div class='filterbp'><div class='parameter'><h2>BandPass Band Position</h2>")
         + "<input type='range' min='0.0' max='127.0' value='0.0' step='0.5' class='slider param' id='bp_pos'>"
         + "<p id='bp_pos_label'>0.0</p><br>"
         + "<div id='bp_pos_mc' class='header' onclick=showHelp('bp_pos_content')>Macro Controls...</div>"
         + "<div id='bp_pos_content' class='content'>"
         + "<p>X Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='bp_pos_x' class='mod_value'><p id='bp_pos_x_label'>0</p><br>"
         + "<p>Y Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='bp_pos_y' class='mod_value'><p id='bp_pos_y_label'>0</p><br>"
         + "<p>Z Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='bp_pos_z' class='mod_value'><p id='bp_pos_z_label'>0</p><br>"
         + "</div>"+ addHelpBox("bppos", "BandPass Position - An welcher Frequenz-Position soll der BandPass greifen. (In MIDI-Noten 0-127)") + "</div><br>"
         + "<div class='parameter'><h2>BandPass Band Width</h2>"
         + "<input type='range' min='1' max='10' value='0' step='0.1' class='slider param' id='bp_width'>"
         + "<p id='bp_width_label'>0</p><br>"
         + "<div id='bp_width_mc' class='header' onclick=showHelp('bp_width_content')>Macro Controls...</div>"
         + "<div id='bp_width_content' class='content'>";
}
const String getFilterBP2() {         
  return String("<p>X Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='bp_width_x' class='mod_value'><p id='bp_width_x_label'>0</p><br>")
         + "<p>Y Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='bp_width_y' class='mod_value'><p id='bp_width_y_label'>0</p><br>"
         + "<p>Z Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='bp_width_z' class='mod_value'><p id='bp_width_z_label'>0</p><br>"
         + "</div>"+ addHelpBox("bpwidth", "BandPass Bandbreite - Wie weit um die BandPass - Position soll durchgelassen werden. (0-10)") + "</div><br>"
         + "<div class='parameter'><h2>BandPass Filter Mix</h2>"
         + "<input type='range' min='0.0' max='1.0' value='0.0' step='0.01' class='slider param' id='bp_mix'>"
         + "<p id='bp_mix_label'>0.0</p><br>"
         + "<div id='bp_mix_mc' class='header' onclick=showHelp('bp_mix_content')>Macro Controls...</div>"
         + "<div id='bp_mix_content' class='content'>"
         + "<p>X Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='bp_mix_x' class='mod_value'><p id='bp_mix_x_label'>0</p><br>"
         + "<p>Y Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='bp_mix_y' class='mod_value'><p id='bp_mix_y_label'>0</p><br>"
         + "<p>Z Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='bp_mix_z' class='mod_value'><p id='bp_mix_z_label'>0</p><br>"
         + "</div>"+ addHelpBox("bpmix", "BandPass Mix - Wie viel des durch Bandpass gefilterten Sounds soll zu hoeren sein (0 - 100%)") + "</div></div></div><br>";
}

const String getEffects() {
  return String("<div class='effects group'>")
         + "<div class='parameter'><h2>Envelope Ramp Time</h2>"
         + "<input type='range' min='0' max='500' value='0' step='1' class='slider param' id='ar_time'>"
         + "<p id='ar_time_label'>0</p>ms<br>"
         + "<div id='ar_time_mc' class='header' onclick=showHelp('ar_time_content')>Macro Controls...</div>"
         + "<div id='ar_time_content' class='content'>"
         + "<p>X Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='ar_time_x' class='mod_value'><p id='ar_time_x_label'>0</p><br>"
         + "<p>Y Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='ar_time_y' class='mod_value'><p id='ar_time_y_label'>0</p><br>"
         + "<p>Z Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='ar_time_z' class='mod_value'><p id='ar_time_z_label'>0</p><br>"
         + "</div>"+ addHelpBox("ar_time", "Envelope - Attack und Release: Zeit bis Audio-Rampe auf dem Maximalen oder Minimalen Wert ist. In Millisekunden.") + "</div><br>"
         + "<div class='parameter'><h2>Delay Amp</h2>"
         + "<input type='range' min='0' max='0.5' value='0' step='0.01' class='slider param' id='delay_amp'>"
         + "<p id='delay_amp_label'>0</p><br>"
         + "<div id='delay_amp_mc' class='header' onclick=showHelp('delay_amp_content')>Macro Controls...</div>"
         + "<div id='delay_amp_content' class='content'>"
         + "<p>X Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='delay_amp_x' class='mod_value'><p id='delay_amp_x_label'>0</p><br>"
         + "<p>Y Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='delay_amp_y' class='mod_value'><p id='delay_amp_y_label'>0</p><br>"
         + "<p>Z Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='delay_amp_z' class='mod_value'><p id='delay_amp_z_label'>0</p><br>"
         + "</div>"+ addHelpBox("delay_amp", "Delay - Wiederholt vorhergehende Audio-Signale mit entsprechenem Faktor. (0% - 50%)") + "</div></br>"
         + "<div class='parameter'><h2>Chorus Amp</h2>"
         + "<input type='range' min='0' max='1' value='0' step='0.01' class='slider param' id='chrous_wet'>"
         + "<p id='chrous_wet_label'>0</p><br>"
         + "<div id='chrous_wet_mc' class='header' onclick=showHelp('chrous_wet_content')>Macro Controls...</div>"
         + "<div id='chrous_wet_content' class='content'>"
         + "<p>X Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='chrous_wet_x' class='mod_value'><p id='chrous_wet_x_label'>0</p><br>"
         + "<p>Y Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='chrous_wet_y' class='mod_value'><p id='chrous_wet_y_label'>0</p><br>"
         + "<p>Z Mod Amount</p><input type='range' min='-1' max='1' value='0' step='0.01' id='chrous_wet_z' class='mod_value'><p id='chrous_wet_z_label'>0</p><br>"
         + "</div>"+ addHelpBox("chrous_wet", "Chorus Amplitude - Ein sehr kurzer Delay mit Pitch Aenderung -> Vollerer Sound. (0% - 100%)") + "</div><br>"
         + "</div></div></div></body>";
}

const String addHelpBox(String id, String text) {
  return String("<div id='"+ id +"_help_btn' class='header' onclick=showHelp('"+id+"_help')>?</div>")
         + "<div id='"+ id +"_help' class='content help'>"
         +  text
         + "</div>";
}

const String getSliderCss() {
  return String("input[type=range] {")
  + "height: 34px;-webkit-appearance: none;margin: 10px 0;width: 100%;}"
  + "input[type=range]:focus {  outline: none;}"
  + "input[type=range]::-webkit-slider-runnable-track {  width: 100%;height: 20px;cursor: pointer;animate: 0.2s;box-shadow: 0px 0px 0px #000000;background: #B6B1B1;border-radius: 50px;border: 1px solid #8A8A8A;}"
  + "input[type=range]::-webkit-slider-thumb {box-shadow: 0px 0px 10px #828282;border: 4px solid #8A8A8A;height: 24px;width: 24px;border-radius: 50px;background: #DA78D7;cursor: pointer;-webkit-appearance: none;margin-top: -4.5px;}"
  + "input[type=range]:focus::-webkit-slider-runnable-track {background: #B6B1B1;}input[type=range]::-moz-range-track {width: 100%;height: 20px;cursor: pointer;animate: 0.2s;box-shadow: 0px 0px 0px #000000;background: #B6B1B1;border-radius: 50px;border: 1px solid #8A8A8A;}"
  + "input[type=range]::-moz-range-thumb {box-shadow: 0px 0px 10px #828282;border: 4px solid #8A8A8A;height: 24px;width: 24px;border-radius: 50px;background: #DA78D7;cursor: pointer;}"
  + "input[type=range]::-ms-track {width: 100%;height: 20px;cursor: pointer;animate: 0.2s;background: transparent;border-color: transparent;color: transparent;}"
  + "input[type=range]::-ms-fill-lower {background: #B6B1B1;border: 1px solid #8A8A8A;border-radius: 100px;box-shadow: 0px 0px 0px #000000;}"
  + "input[type=range]::-ms-fill-upper {background: #B6B1B1;border: 1px solid #8A8A8A;border-radius: 100px;box-shadow: 0px 0px 0px #000000;}"
  + "input[type=range]::-ms-thumb {margin-top: 1px;box-shadow: 0px 0px 10px #828282;border: 4px solid #8A8A8A;height: 24px;width: 24px;border-radius: 50px;background: #DA78D7;cursor: pointer;}"
  + "input[type=range]:focus::-ms-fill-lower {background: #B6B1B1;}"
  + "input[type=range]:focus::-ms-fill-upper {background: #B6B1B1;}</style>";
}

const String getTutorial() {
  return String("<style>.content {width: 30%;border-style: dotted;margin: 20px;margin-left: 35%;padding: 10px;}.content h1 {text-align: center;}</style>")
  +"<body><div class='content'><h1>Synthese:</h1><p>Bei diesem subtraktiven Synthesizer erzeugen 2 Oszillatoren klangliches Rohmaterial, welches dann durch klangveraendernde Module (Lowpass- und Bandpass Filter) nachbearbeitet wird. Der gewuenschte Klang wird erzielt, indem aus dem obertonreichen"
  +"Spektrum der Oszillatoren die unerwuenschten Frequenzanteile herausgefiltert oder abgesenkt werden (=Subtraktion).</p>"
  +"</div><div class='content osc'><h1>Oszilatoren</h1><p>Das klangliche Rohmaterial wird in dieser Synthese-Engine";
}
const String getTutorial2() {  
  return String("von zwei Square-Wave Oszillatoren erzeugt. Diese koennen unterschiedlichste Pitches und Amplituden haben. Jeder Oszillator besitzt einen <u>Pitsch</u> welcher in MIDI-Noten (0-127) umgerechnet und entbrechende Frequenzen erzeugt. Au&szlig;erdem hat jeder Oszillator eine <u>Amplitude</u> welche das Gewicht des entsprechenden Oszillators angibt, der Parameter wird als Multiplikator angegeben (0 - 1).</br><b>Hinweise:</b><ul><li>Wenn der Pitsch auf 0 gesetzt ist kommt es beim Audio-Rendern zu Fragmenten, daher sollten keine Pitches von 0 abgespielt werden.</li></ul></p>");
}
const String getTutorial3() {  
  return String("</div><div class='content filter'><h1>Filter</h1><p>Das von den Oszillatoren erzeugte Signal, kann durch 2 Filter gefiltert werden. Diese sind ein <u>Low Pass Filter</u> und eine <u>Band Pass Filter</u>. Ihre Funktion entspricht den Namen. Der Lowpass laesst die Frequenzen unter dem Cutoff durch. Hoehere Frequenzen werden abgeschnitten. Der Bandpass legt einen Bereich fest in welchem die Frequenzen passieren duerfen. Andere Bereiche werden auch Hier Abgeschnitten. Der Lowpass hat einen <u>Cutoff</u> welcher wie beim <u>OSC</u> in MIDI-Noten (0-127) angegeben und auch in Frequenzen umgerechnet wird. Der <u>Band Pass</u> besitzt stattdessen eine in MIDI-Noten angegebene <u>Band Pass Position</u> welche die Position des Bandes im Frequenzspektrum angibt. Au&szlig;erdem eine <u>Breite des Bandes</u>. Das Hei&szlig;t wie weit, Frequenzen um die Position, noch passieren duerfen.</br>Beide Filter haben einen <u>Filter Mix</u> dieser muss, um einen Sound hoeren zu koennen != 0 sein.</br><b>Hinweise:</b><ul><li>Das schnelle Bewegen der Filter Einstellungen beim spielen des Instruments kann zu starken Verzerrungen fuehren. (z.B. durch Modulation)</li><li>Die Ausgabe kann bei zu lauter Amplitude clippen, d.h. manche Frequenzen koennen nicht dargestellt werden, das kann z.B. durch 2 Filter die sehr viel durchlassen passieren, bei einem solchen Fall die Gesamtlautstaerke des Geraets runterstellen (~0.3)</li></ul></p>");
}
const String getTutorial4() {  
  return String("</div><div class='content general fx'><h1>Delay & Envelope</h1><p>Wenn das Signal Ton an/Ton aus kommt kann die Zeit bis der Ton ganz Ab- oder Anklingt mit dem <u>Envelope</u> gesteuert werden. Die Einheit sind <u>ms</u> wenn der <u>Gate</u>-Parameter (An/Aus) sich aendert dauert es x ms bis der Ton komplett zu hoeren / verklangen ist.</br>");
}
const String getTutorial5() {  
  return String("Der <u>Delay</u> ist ein einfacher Effekt welcher die vorausgegangen Signale in die Aktuellen reinmischt. Der Parameter <u>Delay Amp</u> beschreibt die Lautstaerke des \"delayten\" Signals.");
}
const String getTutorial6() {  
  return String("</p></div><div class='content modulation'><h1>Modulation</h1><p>Dieses Instrument verfuegt ueber einen Sensor welcher <u>X, Y und Z</u> Position im Bereich <u>0-100</u> eines Elektronisch-Leitenden Objekts anzeigt. z.B. Einem Finger. Diese Positionen koennen dazu genutzt werden die eingestellten Parameter zu veraendern. Die Einstellungen dazu finden sich unter dem Untermenue \"<u>Macro Controls...</u>\". Dort zu Finden sind Slider mit einem Bereich von <u>-1 bis 1</u> diese Werte werden als Prozent des verfuegbaren Wertebereichs des Parameters interpretiert. <b>Beispiel: der Slider X Modulation wird auf 0.1 gesetzt bei OSC 1 AMP, welcher auf 0.5 steht. Wenn der Finger nun ganz Links im Sensorbereich erkannt wird bleibt der Wert des Parameters auf 0.5, doch die Bewegung des Fingers an den rechten Rand fuehrt zu einer Modulation bis maximal: 0.6.</b> Diese Veraenderung der Parameter kann zu sehr Interessanten Klaengen fuehren. <b>Hinweise zum Sensor: </b><ul> <li>Da es sich um einen Prototypen handelt empfehlen wir den Sensor nicht zu beruehren.</li><li>Die aktuelle erkannte Messtelle wird auf der Weboberflaeche in der Matrix angezeigt. Die Groe&szlig;e des Punktes ist dabei die Z Achse, die die Entfernung zum Sensor abbildet.</li><li>Die konkreten Werte werden am oberen Bildschirmrand in Zahlen ausgegeben.</li></ul></p>")
  + "</div></body>";
}

char buf[13];
int normalizedX;
int normalizedY;
int normalizedZ;

const auto lenght = getCanvasDrawing().length() + getJavaScript().length() + getCss().length() + getSliderCss().length() + getGeneral().length() + getOSC1().length() + getOSC2().length() + getFilterLP().length() + getFilterBP().length() + getFilterBP2().length() + getEffects().length();

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
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", getCanvasDrawing());
    server.sendContent(getJavaScript());
    server.sendContent(getCss());
    server.sendContent(getSliderCss());
    server.sendContent(getGeneral());
    server.sendContent(getOSC1());
    server.sendContent(getOSC3());
    server.sendContent(getOSC2());
    server.sendContent(getFilterLP());
    server.sendContent(getFilterBP());
    server.sendContent(getFilterBP2());
    server.sendContent(getEffects());
    server.sendContent("");
    server.client().stop();
    ESP.wdtEnable(15000);
  });

  server.on("/tutorial", [](){
    server.setContentLength(getTutorial().length() + getTutorial2().length() + getTutorial3().length() + getTutorial4().length() + getTutorial5().length() + getTutorial6().length());
    server.send(200, "text/html", getTutorial());
    server.sendContent(getTutorial2());
    server.sendContent(getTutorial3());
    server.sendContent(getTutorial4());
    server.sendContent(getTutorial5());
    server.sendContent(getTutorial6());
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



