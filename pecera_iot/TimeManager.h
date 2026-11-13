#ifndef TIMEMANAGER_H
#define TIMEMANAGER_H

#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>

class TimeManager {
private:
    RTC_DS1307 rtc;
    LiquidCrystal_I2C lcd;

public:
    // Ajustado a la dirección 0x27, con 20 columnas y 4 filas
    TimeManager() : lcd(0x27, 20, 4) {}

    void init() {
        Wire.begin(21, 22); // Inicializa bus I2C (SDA=GPIO 21, SCL=GPIO 22)
        lcd.init();
        lcd.backlight();
        lcd.clear();
        
        if (!rtc.begin()) {
            Serial.println("Error crítico: No se detectó el módulo RTC DS1307");
        }
        
        if (!rtc.isrunning()) {
            Serial.println("RTC no está corriendo. Sincronizando con hora de compilación...");
            rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); 
        }
    }

    String getTimestamp() {
        DateTime now = rtc.now();
        char buffer[25];
        sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d", 
                now.year(), now.month(), now.day(), 
                now.hour(), now.minute(), now.second());
        return String(buffer);
    }

    // Actualiza la LCD de 20x4 de forma fija y limpia (Sin parpadeos ni saturar el bus)
    void updateDisplay(float temp, float nivel) {
        DateTime now = rtc.now();
        
        // Fila 1 (Renglón 0): Fecha (Columna 0 al 19)
        lcd.setCursor(0, 0);
        char filaFecha[21];
        sprintf(filaFecha, "FECHA: %02d/%02d/%04d   ", now.day(), now.month(), now.year());
        lcd.print(filaFecha);
        Serial.println(filaFecha);

        // Fila 2 (Renglón 1): Hora Completa
        lcd.setCursor(0, 1);
        char filaHora[21];
        sprintf(filaHora, "HORA:  %02d:%02d:%02d    ", now.hour(), now.minute(), now.second());
        lcd.print(filaHora);
        Serial.println(filaHora);
        
        // Fila 3 (Renglón 2): Temperatura del Agua
        lcd.setCursor(0, 2);
        char filaTemp[21];
        sprintf(filaTemp, "TEMP AGUA: %2.1f C    ", temp);
        lcd.print(filaTemp);
        Serial.println(filaTemp);

        // Fila 4 (Renglón 3): Nivel del Tanque del Ultrasónico
        lcd.setCursor(0, 3);
        char filaNivel[21];
        sprintf(filaNivel, "NIVEL AGUA: %3.0f %%   ", nivel);
        lcd.print(filaNivel);
        Serial.println(filaNivel);
    }
};

#endif