#ifndef CONFIG_H
#define CONFIG_H

// ---- ASIGNACIÓN DE PINES CORRECTA Y LIBRE DE CONFLICTOS ----
#define PIN_DS18B20 4   // Se mueve al GPIO 4 (Línea OneWire limpia)
#define PIN_TRIG    12  // Se mueve al GPIO 12 (Trigger del Ultrasónico)
#define PIN_ECHO    13  // Se mueve al GPIO 13 (Echo del Ultrasónico)
#define PIN_LDR     34  // Entrada analógica del sensor de luz (ADC1)

#define PIN_SD_CS   5   // Chip Select exclusivo para la MicroSD (Bus SPI)
// Los pines MOSI (23), MISO (19) y SCK (18) se conectan directo a sus pines nativos

#define PIN_SERVO   25  // Se mueve al GPIO 25 (Salida PWM limpia para el Comedero)
#define PIN_BOMBA   26  // Se mueve al GPIO 26 (Salida digital para Relé de la Bomba)
#define PIN_LUZ     27  // Se mueve al GPIO 27 (Salida digital para el Relé/LED de Luz)

// ---- PARÁMETROS DE RED Y BROKER MQTT ----
const char* const WIFI_SSID = "Honor200";      
const char* const WIFI_PASSWORD = "Deivigei123";  
const char* const MQTT_BROKER = "broker.hivemq.com"; 
const int MQTT_PORT = 1883;

// Tópicos de comunicación con tu App de React Native
#define TOPIC_TELEMETRIA "pecera/telemetria"
#define TOPIC_CTRL_LUZ   "pecera/control/luz"
#define TOPIC_CTRL_BOMBA "pecera/control/bomba"
#define TOPIC_CTRL_FEED  "pecera/control/comida"

#endif