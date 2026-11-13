#ifndef STORAGE_H
#define STORAGE_H

#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include "Config.h"

class Storage {
public:
    bool init() {
        if (!SD.begin(PIN_SD_CS)) {
            Serial.println("Error crítico: Tarjeta o lector MicroSD no detectado.");
            return false;
        }
        Serial.println("Módulo MicroSD inicializado correctamente.");
        return true;
    }

    // Recibe el objeto JSON estructurado dinámicamente y lo añade al archivo de texto
    void appendLog(const JsonDocument& doc) {
        File file = SD.open("/datalogger.json", FILE_APPEND);
        if (!file) {
            Serial.println("Error al intentar abrir o escribir en datalogger.json");
            return;
        }
        // Serializa el JSON y lo escribe de forma compacta en el archivo de texto de la SD
        serializeJson(doc, file);
        file.println(); // Agrega un salto de línea indispensable para separar los registros
        file.close();
    }
};

#endif