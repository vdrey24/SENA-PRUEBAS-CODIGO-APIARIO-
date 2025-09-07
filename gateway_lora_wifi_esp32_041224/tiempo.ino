#include <RTClib.h>
#include <HTTPClient.h>
#include "variables.h"

// Objetos en gateway.ino
extern RTC_DS1307 rtc;
extern WiFiClient espClient;

String ceros_fun(int a) {
  String b = String(a);
  if (a < 10) b = "0" + b;
  return b;
}

String consultar_hora_rtc() {
  DateTime now = rtc.now();
  year_ = now.year();
  month_ = now.month();
  String sMonth_ = ceros_fun(month_);
  day_ = now.day();
  String sDay_ = ceros_fun(day_);
  hour_ = now.hour();
  String sHour_ = ceros_fun(hour_);
  min_ = now.minute();
  String sMin_ = ceros_fun(min_);
  sec_ = now.second();
  String sSec_ = ceros_fun(sec_);
  String fecha_ = String(year_) + "-" + sMonth_ + "-" + sDay_ + "T" + sHour_ + ":" + sMin_ + ":" + sSec_ + "-0500";
  return fecha_;
}

void consulta_hora_internet() {
  Serial.println(F("Consultando hora internet"));
  HTTPClient http;
  http.begin(espClient, "http://daimob.co/hora_daimob/get_hora_colombia.php");
  http.addHeader("Content-Type", "text/plain");
  int httpResponseCode = http.GET();
  if (httpResponseCode == 200) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
    DateTime now = rtc.now();
    int year_i = payload.substring(0, 4).toInt();
    int hora_i = payload.substring(11, 13).toInt();
    int min_i = payload.substring(14, 16).toInt();
    if ((year_ != year_i) || (hour_ != hora_i) || (min_ > min_i + 2) || (min_ < min_i - 2)) {
      int mes_i = payload.substring(5, 7).toInt();
      int dia_i = payload.substring(8, 10).toInt();
      int seg_i = payload.substring(17).toInt();
      Serial.println(F("Actualizando RTC"));
      rtc.adjust(DateTime(year_i, mes_i, dia_i, hora_i, min_i, seg_i));
    } else {
      Serial.println(F("No se requiere actualización"));
    }
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}