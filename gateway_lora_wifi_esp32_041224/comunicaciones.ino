#include <WiFi.h>
#include <ESP32Ping.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include "variables.h"

// Objetos globales definidos en gateway.ino
extern WiFiClient espClient;
extern PubSubClient client;

// Prototipos
void diagnostico_wifi(int value);
void diagnostico_mqtt(int value);
void consulta_hora_internet();

void setupWifi() {
  // Leer credenciales
  String ssid_alm = leerEEPROM(EE_SSID_50, 50);
  String pass_alm = leerEEPROM(EE_PASS_50, 50);

  // Leer MQTT
  String host_mqtt_alm = leerEEPROM(EE_HOST_MQTT, 50);
  String user_mqtt_alm = leerEEPROM(EE_USER_MQTT, 50);
  String pass_mqtt_alm = leerEEPROM(EE_PASS_MQTT, 50);
  String port_mqtt_alm = leerEEPROM(EE_PORT_MQTT, 50);
  String topic_alm     = leerEEPROM(EE_TOPIC_50, 50);

  if (host_mqtt_alm.length() > 0) host_mqtt_alm.toCharArray(servidorMqtt, host_mqtt_alm.length() + 1);
  if (user_mqtt_alm.length() > 0) user_mqtt_alm.toCharArray(usuarioMqtt, user_mqtt_alm.length() + 1);
  if (pass_mqtt_alm.length() > 0) pass_mqtt_alm.toCharArray(passMqtt, pass_mqtt_alm.length() + 1);
  if (topic_alm.length() > 0)     topic_alm.toCharArray(topic, topic_alm.length() + 1);
  if (port_mqtt_alm.length() > 0) puertoMqtt = port_mqtt_alm.toInt();

  client.setServer(servidorMqtt, puertoMqtt);

  // Escanear y conectar a SSID guardado
  int n = WiFi.scanNetworks();
  byte coincide = 0;

  if (n > 0) {
    for (int i = 0; i < n; i++) {
      String ssid_actual = WiFi.SSID(i);
      if (ssid_actual.equals(ssid_alm)) {
        coincide++;
        WiFi.mode(WIFI_STA);
        delay(10);
        unsigned long inicio = millis();
        WiFi.begin(ssid_alm.c_str(), pass_alm.c_str());
        while ((WiFi.status() != WL_CONNECTED) && (millis() - inicio < TIEMPO_CONEXION_WIFI)) {
          Serial.print(F("."));
          parpadeo(1, 75);
          yield();
        }
        if (WiFi.status() == WL_CONNECTED) {
          Serial.println(F("\nConexión WiFi exitosa"));
          if (Ping.ping(remote_host)) {
            Serial.println("Hay conexión a internet");
            consulta_hora_internet();
            if (reconnect()) {
              Serial.println("MQTT Ok");
            } else {
              Serial.println("Falla MQTT");
            }
          }
        } else {
          Serial.println(F("Fallo de conexión Wifi"));
          diagnostico_wifi(WiFi.status());
        }
      }
    }
  } else {
    Serial.println(F("No se encontraron redes"));
  }
  if (coincide == 0) {
    Serial.println("No coincidencias con SSID guardado");
  }
}

void diagnostico_wifi(int value) {
  switch (value) {
    case WL_IDLE_STATUS:    Serial.println(F("WL_IDLE_STATUS")); break;
    case WL_NO_SSID_AVAIL:  Serial.println(F("WL_NO_SSID_AVAIL - SSID no disponible")); break;
    case WL_CONNECT_FAILED: Serial.println(F("WL_CONNECT_FAILED - Password incorrecta")); break;
    case WL_DISCONNECTED:   Serial.println(F("WL_DISCONNECTED - No en modo STA")); break;
    default:                Serial.print(F("WiFi status: ")); Serial.println(value); break;
  }
}

bool reconnect() {
  Serial.println(F("Reconnect MQTT"));
  byte retries = 2;
  for (int i = 0; i < retries; i++) {
    yield();
    String clientId = "SENA-";
    clientId += String(random(0xffff), HEX);
    Serial.print(F("clientId: "));
    Serial.println(clientId);
    if (client.connect(clientId.c_str(), usuarioMqtt, passMqtt)) {
      client.subscribe(topic); // suscripción al mismo topic si lo deseas
      return true;
    } else {
      int valueMqtt = client.state();
      diagnostico_mqtt(valueMqtt);
      Serial.println(F("Intentando nuevamente..."));
      delay(2000);
      yield();
    }
  }
  return false;
}

void diagnostico_mqtt(int value) {
  switch (value) {
    case -4: Serial.println(F("MQTT_CONNECTION_TIMEOUT")); break;
    case -3: Serial.println(F("MQTT_CONNECTION_LOST")); break;
    case -2: Serial.println(F("MQTT_CONNECT_FAILED")); break;
    case -1: Serial.println(F("MQTT_DISCONNECTED")); break;
    case  0: Serial.println(F("MQTT_CONNECTED")); break;
    case  1: Serial.println(F("MQTT_CONNECT_BAD_PROTOCOL")); break;
    case  2: Serial.println(F("MQTT_CONNECT_BAD_CLIENT_ID")); break;
    case  3: Serial.println(F("MQTT_CONNECT_UNAVAILABLE")); break;
    case  4: Serial.println(F("MQTT_CONNECT_BAD_CREDENTIALS")); break;
    case  5: Serial.println(F("MQTT_CONNECT_UNAUTHORIZED")); break;
    default: Serial.print(F("MQTT code ")); Serial.println(value); break;
  }
}

void enviarMensaje() {
  client.loop();
  bool wifiOk = (WiFi.status() == WL_CONNECTED);
  bool internetOk = wifiOk && Ping.ping(remote_host);
  bool mqttOk = client.connected();

  if (wifiOk && internetOk && mqttOk) {
    Serial.println(F("Enviando..."));
    client.publish(topic, data_enviar.c_str());
  } else {
    Serial.println(F("Mensaje no enviado"));
    Serial.print(F("wifi ok: "));    Serial.println(wifiOk);
    Serial.print(F("internet ok: ")); Serial.println(internetOk);
    Serial.print(F("mqtt ok: "));     Serial.println(mqttOk);
  }
}

void verificar_conexion() {
  Serial.println(F("Verificando conexión"));
  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      reconnect();
    }
  } else {
    Serial.println(F("-- Desconectado --"));
    setupWifi();
  }
}