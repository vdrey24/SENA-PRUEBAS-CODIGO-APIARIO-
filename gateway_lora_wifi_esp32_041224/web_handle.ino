#include <WebServer.h>
#include <WiFi.h>
#include "variables.h"

// Objetos globales en gateway.ino
extern WebServer server;

// ==== Utilidades HTML ====
String html_head =
  "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
  "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
  "<title>Configuración ESP32</title>"
  "<style>"
  "body{font-family:verdana;margin:0;padding:0;background:#f7f7f7;color:#222}"
  ".wrap{max-width:480px;margin:30px auto;background:#fff;padding:20px;border-radius:12px;box-shadow:0 10px 25px rgba(0,0,0,.08)}"
  "h1{font-size:20px;margin:0 0 12px}"
  "a.btn,button.btn,input.btn{display:inline-block;padding:10px 14px;border-radius:8px;border:0;background:#39a900;color:#fff;font-weight:700;text-decoration:none;margin:8px 4px}"
  ".input{width:100%;padding:10px;border:1px solid #39a900;border-radius:8px;margin:8px 0}"
  ".ssid{display:block;margin:6px 0}"
  ".muted{color:#666;font-size:12px}"
  "</style>"
  "<script>"
  "function fillSSID(el){document.getElementById('s').value=el.dataset.ssid;document.getElementById('p').value='';}"
  "function togglePass(id){var x=document.getElementById(id);x.type=(x.type==='password')?'text':'password';}"
  "</script>"
  "</head><body>";

String html_tail = "</body></html>";

// ==== Páginas ====

String page_menu(String mensaje) {
  String w = leerEEPROM(EE_SSID_50, 50);
  String h = leerEEPROM(EE_HOST_MQTT, 50);
  String prt = leerEEPROM(EE_PORT_MQTT, 50);
  String usr = leerEEPROM(EE_USER_MQTT, 50);
  String tpc = leerEEPROM(EE_TOPIC_50, 50);

  String s = html_head;
  s += "<div class='wrap'>";
  s += "<h1>Configuración del dispositivo</h1>";
  if (mensaje.length()) { s += "<p class='muted'>" + mensaje + "</p>"; }
  s += "<p><strong>WiFi guardado:</strong> " + (w.length()? w : "(no definido)") + "</p>";
  s += "<p><strong>MQTT:</strong> " + (h.length()? h : "(no definido)") + (prt.length()? (":" + prt) : "") + "</p>";
  if (tpc.length()) s += "<p><strong>Topic:</strong> " + tpc + "</p>";
  if (usr.length()) s += "<p><strong>Usuario MQTT:</strong> " + usr + "</p>";
  s += "<p><a class='btn' href='/wifi'>Configurar WiFi</a>";
  s += "<a class='btn' href='/mqtt'>Configurar MQTT</a></p>";
  s += "<p><a class='btn' href='/reboot'>Guardar configuración y reiniciar</a></p>";
  s += "<p class='muted'>Para entrar a este modo nuevamente, mantén presionado el botón 1s.</p>";
  s += "</div>";
  s += html_tail;
  return s;
}

String page_wifi(String msg) {
  String w = leerEEPROM(EE_SSID_50, 50);
  String s = html_head;
  s += "<div class='wrap'><h1>WiFi</h1>";
  if (msg.length()) s += "<p class='muted'>" + msg + "</p>";
  
  // Formulario WiFi
  s += "<form action='/save_wifi' method='get'>";
  s += "SSID:<input class='input' id='s' name='s' type='text' value='" + w + "' maxlength='45'>";
  s += "Password:<input class='input' id='p' name='p' type='password' maxlength='45'>";
  s += "<label class='muted'><input type='checkbox' onclick=\"togglePass('p')\"> Mostrar contraseña</label>";
  s += "<input class='btn' type='submit' value='Guardar WiFi'>";
  s += "<a class='btn' href='/'>Volver</a>";
  s += "</form>";

  // Escaneo de redes directamente aquí
  WiFi.mode(WIFI_AP_STA);
  delay(100);
  int n = WiFi.scanNetworks();
  s += "<h2>Redes encontradas</h2>";
  if (n <= 0) {
    s += "<p>No se encontraron redes. <a class='btn' href='/wifi'>Reintentar</a></p>";
  } else {
    int maxn = n > 15 ? 15 : n;
    for (int i = 0; i < maxn; i++) {
      String ssid = WiFi.SSID(i);
      int rssi = WiFi.RSSI(i);
      s += "<a href='#' class='ssid' data-ssid='" + ssid + "' onclick='fillSSID(this);return false;'>" + ssid + " (" + String(rssi) + " dBm)</a><br>";
    }
  }

  s += "</div>" + html_tail;
  return s;
}

String page_mqtt(String msg) {
  String h   = leerEEPROM(EE_HOST_MQTT, 50);
  String prt = leerEEPROM(EE_PORT_MQTT, 50);
  String usr = leerEEPROM(EE_USER_MQTT, 50);
  String pwd = leerEEPROM(EE_PASS_MQTT, 50);
  String tpc = leerEEPROM(EE_TOPIC_50, 50);

  String s = html_head;
  s += "<div class='wrap'><h1>MQTT</h1>";
  if (msg.length()) s += "<p class='muted'>" + msg + "</p>";
  s += "<form action='/save_mqtt' method='get'>";
  s += "Host:<input class='input' name='h' type='text' value='" + h + "' maxlength='45'>";
  s += "Puerto:<input class='input' name='u' type='number' value='" + (prt.length()? prt : "1883") + "'>";
  s += "Usuario:<input class='input' name='m' type='text' value='" + usr + "' maxlength='45'>";
  s += "Contraseña:<input class='input' id='o' name='o' type='password' value='" + pwd + "' maxlength='45'>";
  s += "<label class='muted'><input type='checkbox' onclick=\"togglePass('o')\"> Mostrar contraseña</label>";
  s += "Topic:<input class='input' name='t' type='text' value='" + tpc + "' maxlength='45'>";
  s += "<input class='btn' type='submit' value='Guardar MQTT'>";
  s += "<a class='btn' href='/'>Volver</a>";
  s += "</form></div>" + html_tail;
  return s;
}

// ==== Handlers ====
void webserver_begin_AP() {
  digitalWrite(PIN_LED_AP, HIGH);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(SSID_AP);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);

  server.on("/", []() {
    server.send(200, "text/html", page_menu(""));
  });

  server.on("/wifi", []() {
    server.send(200, "text/html", page_wifi(""));
  });

  server.on("/save_wifi", []() {
    String ssid = server.arg("s");
    String pass = server.arg("p");
    grabarEEPROM(EE_SSID_50, ssid, 50);
    grabarEEPROM(EE_PASS_50, pass, 50);
    server.send(200, "text/html", page_wifi("WiFi guardado"));
  });

  server.on("/mqtt", []() {
    server.send(200, "text/html", page_mqtt(""));
  });

  server.on("/save_mqtt", []() {
    String host = server.arg("h");
    String port = server.arg("u");
    String user = server.arg("m");
    String pwd  = server.arg("o");
    String tpc  = server.arg("t");

    grabarEEPROM(EE_HOST_MQTT, host, 50);
    grabarEEPROM(EE_PORT_MQTT, port, 50);
    grabarEEPROM(EE_USER_MQTT, user, 50);
    grabarEEPROM(EE_PASS_MQTT, pwd, 50);
    if (tpc.length()) grabarEEPROM(EE_TOPIC_50, tpc, 50);

    server.send(200, "text/html", page_mqtt("MQTT guardado"));
  });

  server.on("/reboot", []() {
    String s = html_head;
    s += "<div class='wrap'><h1>Reiniciando...</h1><p>El dispositivo se reiniciará para aplicar la configuración.</p></div>";
    s += html_tail;
    server.send(200, "text/html", s);
    delay(1000);
    ESP.restart();
  });

  server.onNotFound([]() {
    server.send(404, "text/plain", "404: Recurso no encontrado");
  });

  server.begin();
  Serial.println("HTTP server started (AP mode).");
}