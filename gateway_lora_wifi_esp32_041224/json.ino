#include <ArduinoJson.h>
#include "variables.h"

String parseMensaje() {
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, msg);
  Serial.println(error.c_str());

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    msg_valido = false;
    return "XXXXXXXXX";
  } else {
    if (len < 256 && len > 20) {
      const char* elId_in = doc["id"];
      msg_valido = true;
      return String(elId_in);
    } else {
      msg_valido = false;
      return "XXXXXXXXX";
    }
  }
}