#pragma once

// ====== Información del Firmware ======
#define VERSION "1.0.3"

// ====== Configuración de tiempos ======
#define TIEMPO_CONEXION_WIFI 15000
#define T_VERIFICAR 60000   // Verificación de red WiFi
#define T_ENCENDIDO 2000         // Parpadeo LED de Encendido
#define INTENTOS_LORA_INI 3
#define RF95_FREQ 915.0

// ====== Distribución de pines ======
#define RFM95_RST 14
#define RFM95_CS 5
#define RFM95_INT 2
#define PIN_LED 26
#define PIN_LED_AP 25
#define PIN_SDA 21
#define PIN_SCL 22
#define PIN_BUTTON 33

// ====== Direcciones EEPROM (50 bytes c/u salvo puerto) ======
#define EE_SSID_50     0
#define EE_PASS_50     50
#define EE_HOST_MQTT   100
#define EE_USER_MQTT   150
#define EE_PASS_MQTT   200
#define EE_PORT_MQTT   250   // guardamos como texto para simplicidad
#define EE_TOPIC_50    300   // opcional: topic

// ====== Variables de tiempo (RTC/NTP) ======
extern int year_;
extern int month_;
extern int day_;
extern int hour_;
extern int min_;
extern int sec_;

// ====== Configuración de red AP ======
#include <IPAddress.h>
extern IPAddress local_ip;
extern IPAddress gateway;
extern IPAddress subnet;
extern const char* SSID_AP;

// ====== MQTT (valores por defecto si EEPROM vacía) ======
extern char servidorMqtt[50];
extern char usuarioMqtt[50];
extern char passMqtt[50];
extern char topic[50];
extern int  puertoMqtt;

// ====== Variables generales ======
extern unsigned long millisReset;
extern unsigned long wifiMillis;
extern unsigned long statMillis;
extern unsigned long buttonPressMillis;
extern bool inWebserverMode;

// ====== Variables de comunicación ======
extern char* msg;            // Puntero a mensaje recibido
extern String data_enviar;   // Datos para enviar por MQTT o LoRa
extern uint8_t len;          // Longitud de mensaje recibido
extern String elId;          // ID extraído de JSON
extern bool msg_valido;      // Flag de validación de mensaje
extern String sMsg;          // Mensaje convertido a String
extern String tiempo_actual; // Hora y fecha actual en texto
extern const char* remote_host; // Host remoto para NTP o test