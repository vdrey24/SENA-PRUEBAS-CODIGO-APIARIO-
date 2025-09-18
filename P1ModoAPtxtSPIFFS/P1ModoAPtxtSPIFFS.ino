#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>
#include <SPIFFS.h>

// Configuración de red WiFi en modo AP
const char* ssid = "ESP32_AP";
const char* password = "12345678"; // mínimo 8 caracteres

// Servidor web en el puerto 80
WebServer server(80);

// Nombre del archivo en SPIFFS
const char* filename = "/datos.txt";

// Simulación de un sensor en pin analógico
const int sensorPin = 34;

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  html += "<title>Servidor ESP32</title></head><body>";
  html += "<h2>Servidor Web ESP32 (AP Mode)</h2>";
  html += "<p><a href=\"/download\">Descargar archivo datos.txt</a></p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleDownload() {
  File file = SPIFFS.open(filename, "r");
  if (!file) {
    server.send(500, "text/plain", "No se pudo abrir el archivo");
    return;
  }
  server.streamFile(file, "text/plain");
  file.close();
}

void setup() {
  Serial.begin(115200);

  // Inicializar SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("Error al montar SPIFFS");
    return;
  }

  // Crear archivo si no existe
  if (!SPIFFS.exists(filename)) {
    File file = SPIFFS.open(filename, FILE_WRITE);
    if (file) {
      file.println("Archivo creado - Datos de sensor");
      file.close();
    }
  }

  // Configurar ESP32 como punto de acceso
  WiFi.softAP(ssid, password);

  // Obtener IP del AP
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Punto de acceso iniciado. IP: ");
  Serial.println(IP);

  // Rutas del servidor
  server.on("/", handleRoot);
  server.on("/download", handleDownload);

  server.begin();
  Serial.println("Servidor web iniciado");
}

void loop() {
  server.handleClient();

  // Guardar lecturas en SPIFFS cada 5 seg
  static unsigned long lastTime = 0;
  if (millis() - lastTime > 5000) {
    lastTime = millis();
    int valor = analogRead(sensorPin);

    File file = SPIFFS.open(filename, FILE_APPEND);
    if (file) {
      file.printf("Lectura: %d\n", valor);
      file.close();
      Serial.printf("Dato guardado: %d\n", valor);
    }
  }
}