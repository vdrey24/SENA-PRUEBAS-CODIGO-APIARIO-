#include <Arduino.h>
#include <ArduinoJson.h>
#include "variables.h"

#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include <ESP32Ping.h>
#include <HTTPClient.h>
#include <DNSServer.h>

#include <SPI.h>
#include <RH_RF95.h>
#include <Wire.h>
#include <RTClib.h>
#include <PubSubClient.h>
#include <EEPROM.h>

// ==== Objetos globales ====
WebServer server(80);
WiFiClient espClient;
PubSubClient client(espClient);
RH_RF95 rf95(RFM95_CS, RFM95_INT);
RTC_DS1307 rtc;

// ==== Variables globales definidas en variables.h ====
int year_, month_, day_, hour_, min_, sec_;
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
const char* SSID_AP = "SENA_AP";

char servidorMqtt[50] = "mqtt.secadodecafe.online";
char usuarioMqtt[50]  = "mosquitto";
char passMqtt[50]     = "mosquitto";
char topic[50]        = "sena/iot/cacao";
int  puertoMqtt       = 1843;

unsigned long millisReset;
unsigned long wifiMillis;
unsigned long statMillis;
unsigned long buttonPressMillis = 0;
bool inWebserverMode = false;

char* msg;
String data_enviar = "";
uint8_t len = 0;
String elId = "";
bool msg_valido = false;
String sMsg = "";
String tiempo_actual = "";
const char* remote_host = "www.google.com";

// ==== otras pestañas ====
void setupWifi();
void verificar_conexion();
bool reconnect();
void enviarMensaje();

bool inicializa_lora();
void parpadeo(byte veces, int duracion);

String parseMensaje();

void grabarEEPROM(int addr, String a, byte cant);
String leerEEPROM(int addr, byte cant);

String consultar_hora_rtc();
void consulta_hora_internet();

// ==== Web handlers (otra pestaña) ====
void webserver_begin_AP();
void web_handle();

//----------------------------------------------------------
//------------------------- SETUP --------------------------
//----------------------------------------------------------
void setup() {
  Serial.begin(9600);
  Serial.print("\nGateway LoRa/Wifi - ESP32 V");
  Serial.println(VERSION);

  EEPROM.begin(512);

  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_LED_AP, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  Wire.begin(PIN_SDA, PIN_SCL);

  if (!rtc.begin()) {
    Serial.println(F("No se encuentra RTC"));
  } else {
    Serial.println(F("RTC ok"));
  }

  if (inicializa_lora()) {
    Serial.println(F("LoRa ok"));
  } else {
    Serial.println(F("\nFallo LoRa General"));
  }

  tiempo_actual = consultar_hora_rtc();
  Serial.print(F("Fecha y hora: "));
  Serial.println(tiempo_actual);

  digitalWrite(PIN_LED, LOW);

  // Arranque en modo normal: intenta con las credenciales guardadas de WiFi/MQTT
  setupWifi();

  millisReset = millis();
  wifiMillis  = millis();
  statMillis  = millis();
}

//----------------------------------------------------------
//-------------------------- LOOP --------------------------
//----------------------------------------------------------
void loop() {
  // MQTT loop
  client.loop();

  // Botón para entrar a modo AP (presionar durante 1s en el pin 33)
  if (digitalRead(PIN_BUTTON) == LOW) {
    if (millis() - buttonPressMillis > 1000 && !inWebserverMode) {
      inWebserverMode = true;
      Serial.println("Boton presionado por 1 segundo. Entrando en modo configuracion.");
      webserver_begin_AP();
      unsigned long webserverMillis = millis();
      while (millis() - webserverMillis < 180000) { // Tiempo de Expiracion de 3 minutos para salir del modo AP
        yield();
        server.handleClient();
      }
      Serial.println("Tiempo de configuracion web expirado. Reiniciando...");
      ESP.restart();
    }
  } else {
    buttonPressMillis = millis();
  }

  // LED Endendido
  if (millis() - statMillis > T_ENCENDIDO) {
    if (WiFi.status() == WL_CONNECTED) parpadeo(1, 50);
    else parpadeo(2, 50);
    statMillis = millis();
  }

  // Verificación periódica
  if (millis() - wifiMillis > T_VERIFICAR) {
    verificar_conexion();
    wifiMillis = millis();
  }

  // Recepción LoRa
  if (rf95.available()) {
    digitalWrite(PIN_LED, HIGH);
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    len = sizeof(buf);
    if (rf95.recv(buf, &len)) {
      msg = (char*)buf;
      sMsg = String(msg);

      if (len > 20 && len < 201) {
        if (msg[0] == '{') {
          elId = parseMensaje();
        }
        byte longId_dec = elId.length() + 1;
        char rtaLoRa[longId_dec];
        elId.toCharArray(rtaLoRa, longId_dec);
        rf95.send((uint8_t*)rtaLoRa, sizeof(rtaLoRa));
        rf95.waitPacketSent();
      } else {
        Serial.println(F("too small"));
      }
    } else {
      Serial.println(F("recv failed"));
    }
    digitalWrite(PIN_LED, LOW);
  }

  // Envío diferido si hubo JSON válido por LoRa
  if (msg_valido) {
    msg_valido = false;
    byte longitud = sMsg.length();
    String fec_data = ",\"f\":\"" + consultar_hora_rtc() + "\"}";
    data_enviar = sMsg.substring(0, longitud - 1) + fec_data;
    Serial.println(data_enviar);
    enviarMensaje();
  }
}