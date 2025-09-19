#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>
#include <SPIFFS.h>

// --- Configuración WiFi en modo AP ---
const char* ssid = "ESP32_AP";
const char* password = "12345678";

// --- Servidor Web ---
WebServer server(80);

// Variables de control
int fileCounter = 0;       // Simula el día
int lineCounter = 0;       // Contador de líneas por archivo
const int maxLines = 12;   // 12 lecturas por archivo
String currentFilename;

// ====== Generador de datos de 251 caracteres ======
String generarLectura() {
  int tempAmb = random(20, 35);
  int humAmb = random(40, 80);
  int ruido = random(30, 90);
  int lux = random(100, 1000);
  int lluvia = random(0, 10);
  int tempColmena = random(25, 38);
  float lat = 5.12345 + random(-100, 100) / 10000.0;
  float lon = -72.12345 + random(-100, 100) / 10000.0;

  String data = "T_Amb:" + String(tempAmb) +
                ",H_Amb:" + String(humAmb) +
                ",Ruido:" + String(ruido) +
                ",Lux:" + String(lux) +
                ",Lluvia:" + String(lluvia) +
                ",T_Col:" + String(tempColmena) +
                ",GPS(" + String(lat, 5) + "," + String(lon, 5) + ")";

  while (data.length() < 250) {
    data += "_"; // relleno
  }
  data += "\n"; 
  return data;
}

// ====== Página principal: lista de archivos ======
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  html += "<title>Archivos en ESP32</title></head><body>";
  html += "<h2>Archivos disponibles en SPIFFS</h2><ul>";

  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while (file) {
    String fname = String(file.name());
    html += "<li><a href=\"/download?file=" + fname + "\">" + fname + "</a> (" + String(file.size()) + " bytes)</li>";
    file = root.openNextFile();
  }
  html += "</ul></body></html>";

  server.send(200, "text/html", html);
}

// ====== Descargar archivo ======
void handleDownload() {
  if (!server.hasArg("file")) {
    server.send(400, "text/plain", "Falta argumento 'file'");
    return;
  }
  String fname = server.arg("file");

  // Asegurar que empiece con "/"
  if (!fname.startsWith("/")) {
    fname = "/" + fname;
  }

  File file = SPIFFS.open(fname, "r");
  if (!file) {
    server.send(404, "text/plain", "Archivo no encontrado: " + fname);
    return;
  }

  server.sendHeader("Content-Disposition", "attachment; filename=" + fname);
  server.streamFile(file, "text/plain");
  file.close();
}

// ====== Crear nuevo archivo ======
void crearNuevoArchivo() {
  currentFilename = "/" + String(20250918 + fileCounter) + ".txt"; 
  fileCounter++;
  lineCounter = 0;

  File newFile = SPIFFS.open(currentFilename, FILE_WRITE);
  if (!newFile) {
    Serial.println("Error creando archivo nuevo");
    return;
  }
  newFile.close();

  Serial.println("Nuevo archivo creado: " + currentFilename);
}

void setup() {
  Serial.begin(115200);

  if (!SPIFFS.begin(true)) {
    Serial.println("Error al montar SPIFFS");
    return;
  }

  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Punto de acceso iniciado. IP: ");
  Serial.println(IP);

  server.on("/", handleRoot);
  server.on("/download", handleDownload);
  server.begin();
  Serial.println("Servidor web iniciado");

  crearNuevoArchivo();
}

void loop() {
  server.handleClient();

  static unsigned long lastTime = 0;
  if (millis() - lastTime > 2000) { // cada 2 seg para pruebas
    lastTime = millis();

    if (lineCounter < maxLines) {
      String linea = generarLectura();

      // Opción 1: abrir/cerrar en cada escritura para garantizar 
      //que el archivo no este limpio al descargar antes de cumplir las 12 lecturas diarias
      File f = SPIFFS.open(currentFilename, FILE_APPEND);
      if (f) {
        f.print(linea);
        f.close();
        Serial.print("Escribiendo en ");
        Serial.print(currentFilename);
        Serial.print(": ");
        Serial.println(linea);
      }

      lineCounter++;
    } else {
      crearNuevoArchivo(); 
    }
  }
}