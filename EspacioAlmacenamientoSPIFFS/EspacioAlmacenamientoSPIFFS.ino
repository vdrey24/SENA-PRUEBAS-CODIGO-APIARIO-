#include <SPIFFS.h>

void setup() {
  Serial.begin(115200);
  if(!SPIFFS.begin(true)){
    Serial.println("Error montando SPIFFS");
    return;
  }

  size_t total = SPIFFS.totalBytes();
  size_t used  = SPIFFS.usedBytes();

  Serial.printf("Total SPIFFS: %d bytes\n", total);
  Serial.printf("Usado: %d bytes (%.2f %%)\n", used, (used * 100.0) / total);
  Serial.printf("Disponible: %d bytes\n", total - used);
}

void loop() {}