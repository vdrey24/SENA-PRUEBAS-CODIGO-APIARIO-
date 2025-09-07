#include <EEPROM.h>
#include "variables.h"

void grabarEEPROM(int addr, String a, byte cant) {
  Serial.print("Guardando :");
  Serial.println(a);
  int tamano = a.length();
  char inchar[cant];
  a.toCharArray(inchar, tamano + 1);
  if (inchar[0] != 0) {
    for (int i = 0; i < tamano ; i++) {
      EEPROM.write(addr + i, inchar[i]);
    }
    for (int i = tamano; i < cant; i++) {
      EEPROM.write(addr + i, 255);
    }
    EEPROM.commit();
  }
}

String leerEEPROM(int addr, byte cant) {
  byte lectura;
  String strLectura;
  for (int i = addr; i < addr + cant; i++) {
    lectura = EEPROM.read(i);
    if (lectura != 255 && lectura != 0) {
      strLectura += (char)lectura;
    }
  }
  Serial.print("leído :");
  Serial.println(strLectura);
  return strLectura;
}