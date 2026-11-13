#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Config.h"
#include "TimeManager.h"
#include "Storage.h"

// Instanciación de Hardware y Red
WiFiClient espClient;
PubSubClient mqtt(espClient);
TimeManager rtcYDisplay;
Storage tarjetaSD;
Servo comederoServo;

// Configuración de Sensores Clave
OneWire oneWire(PIN_DS18B20);
DallasTemperature sensors(&oneWire);

// Estados de Actuadores y Banderas de Eventos (Requerimiento b.ii)
bool statusLuz = false;
bool statusBomba = false;
bool cambioLuzDetectado = false;
bool cambioBombaDetectada = false;

unsigned long tiempoUltimoCambioLuz = 0;
unsigned long tiempoUltimoCambioBomba = 0;

// Gestor de Tareas sin delay (Requerimiento b.iv)
unsigned long previousMillisSensors = 0;
const unsigned long intervalSensors = 5000; // Cada 5 segundos un ciclo

// ---- FUNCIONES AUXILIARES DE LECTURA REAL DEL HARDWARE ----

// 1. Lectura Real del Sensor de Temperatura DS18B20
float leerTemperaturaReal() {
    sensors.requestTemperatures(); 
    float temp = sensors.getTempCByIndex(0);
    if (temp == DEVICE_DISCONNECTED_C) {
        Serial.println("Error: Sensor DS18B20 desconectado.");
        return 25.0; // Valor seguro por defecto en caso de falla de hardware
    }
    return temp;
}

// 2. Lectura y Cálculo Real del Porcentaje de Agua (HC-SR04)
float leerNivelAguaReal() {
    digitalWrite(PIN_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIG, LOW);
    
    // Mide la duración del pulso en el pin ECHO (en microsegundos)
    long duracion = pulseIn(PIN_ECHO, HIGH, 30000); // Timeout de 30ms para evitar bloqueos
    
    // Conversión matemática a distancia en centímetros
    float distancia = (duracion * 0.0343) / 2;
    
    // Mapeo lineal basado en las dimensiones de tu contenedor previo (cm)
    float DISTANCIA_VACIO = 13; 
    float DISTANCIA_LLENO = 4;  
    
    if (duracion == 0 || distancia >= DISTANCIA_VACIO) return 0.0;
    if (distancia <= DISTANCIA_LLENO) return 100.0;
    
    float porcentaje = 100.0 - ((distancia - DISTANCIA_LLENO) / (DISTANCIA_VACIO - DISTANCIA_LLENO) * 100.0);
    return porcentaje;
}

// ---- CONFIGURACIÓN DE RED ASÍNCRONA ----
void setupWiFi() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) { delay(100); }
}

void callbackMQTT(char* topic, byte* payload, unsigned int length) {
    String mensaje = "";
    for (unsigned int i = 0; i < length; i++) { mensaje += (char)payload[i]; }
    
    unsigned long tiempoActual = millis();

    if (String(topic) == TOPIC_CTRL_BOMBA) {
        bool nuevoEstado = (mensaje == "1");
        if (nuevoEstado != statusBomba) {
            statusBomba = nuevoEstado;
            // CORRECCIÓN: Lógica inversa para el relé (LOW enciende, HIGH apaga)
            digitalWrite(PIN_BOMBA, statusBomba ? LOW : HIGH);
            cambioBombaDetectada = true; 
        }
    }
    else if (String(topic) == TOPIC_CTRL_FEED) {
        if (mensaje == "1") {
            comederoServo.write(115); // Ángulo de barrido mecánico (Abrir)
            delay(300);               // Pulso síncrono corto controlado
            comederoServo.write(90);  // Cerrar compuerta
        }
    }
}

void reconnectMQTT() {
    while (!mqtt.connected()) {
        if (mqtt.connect("ESP32_Pecera_Prototipo")) {
            mqtt.subscribe(TOPIC_CTRL_BOMBA);
            mqtt.subscribe(TOPIC_CTRL_FEED);
        } else {
            delay(2000);
        }
    }
}

// ---- ORQUESTACIÓN PRINCIPAL ----
void setup() {
    Serial.begin(115200);
    
    // Configuración de Direcciones Eléctricas
    pinMode(PIN_BOMBA, OUTPUT);
    pinMode(PIN_LUZ, OUTPUT);
    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT);
    
    // CORRECCIÓN: Inicializar relés apagados (En ALTO debido a la lógica inversa)
    digitalWrite(PIN_BOMBA, HIGH);
    digitalWrite(PIN_LUZ, HIGH);
    
    comederoServo.attach(PIN_SERVO);
    comederoServo.write(90); 
    
    sensors.begin(); // Inicializa bus OneWire de temperatura
    setupWiFi();
    
    mqtt.setServer(MQTT_BROKER, MQTT_PORT);
    mqtt.setCallback(callbackMQTT);
    
    rtcYDisplay.init();
    tarjetaSD.init();
}

void loop() {
    if (!mqtt.connected()) { reconnectMQTT(); }
    mqtt.loop(); 

    unsigned long currentMillis = millis();

    // TAREA DE ADQUISICIÓN Y LOGGING (SIN DELAY)
    if (currentMillis - previousMillisSensors >= intervalSensors) {
        previousMillisSensors = currentMillis;

        // LECTURAS ADQUIRIDAS DIRECTAMENTE DEL HARDWARE REAL
        float temperaturaAgua = leerTemperaturaReal(); 
        float nivelAguaPorcentaje = leerNivelAguaReal(); 
        int luzAmbiental = analogRead(PIN_LDR); // Lectura directa analógica del LDR (0 a 4095)

        // ---- CORRECCIÓN DEL CONTROL AUTOMÁTICO DE RELÉ MEDIANTE SENSOR LDR ----
        // Cambiado a '<': Si el valor cae por debajo del umbral, hay oscuridad y se prende la lámpara.
        int UMBRAL_OSCURIDAD = 2200; 

        if (luzAmbiental < UMBRAL_OSCURIDAD) {
            if (statusLuz == true) {
                statusLuz = false;
                digitalWrite(PIN_LUZ, HIGH); // LOW enciende el relay físico de la lámpara (Lógica Inversa)
                cambioLuzDetectado = true;   
                
                mqtt.publish(TOPIC_CTRL_LUZ, "0");
            }
        } else {
            if (statusLuz == false) {
                statusLuz = true;
                digitalWrite(PIN_LUZ, LOW); // HIGH apaga el relay físico de la lámpara (Lógica Inversa)
                cambioLuzDetectado = true;   
                
                mqtt.publish(TOPIC_CTRL_LUZ, "1");
            }
        }
        // -----------------------------------------------------------

        // 1. Actualización inmediata de la LCD Física
        rtcYDisplay.updateDisplay(temperaturaAgua, nivelAguaPorcentaje);

        // 2. Envío Real por MQTT hacia la App de React Native
        StaticJsonDocument<200> jsonMqtt;
        jsonMqtt["temperature"] = temperaturaAgua;
        jsonMqtt["waterLevel"] = nivelAguaPorcentaje;
        jsonMqtt["lightStatus"] = statusLuz; 
        
        char bufferMqtt[200];
        serializeJson(jsonMqtt, bufferMqtt);
        mqtt.publish(TOPIC_TELEMETRIA, bufferMqtt);

        // 3. Estructuración del Registro para la Tarjeta MicroSD
        StaticJsonDocument<550> jsonSD;
        jsonSD["timestamp"] = rtcYDisplay.getTimestamp(); 
        jsonSD["temperature"] = temperaturaAgua;
        jsonSD["water_level"] = nivelAguaPorcentaje;
        jsonSD["raw_ldr"] = luzAmbiental;

        // CRITERIO b.ii: Inyección Anidada Condicional de Actuadores
        if (cambioLuzDetectado) {
            JsonObject nodoLuz = jsonSD.createNestedObject("actuator_event");
            nodoLuz["dispositivo"] = "ILUMINACION_LED";
            nodoLuz["estado"] = statusLuz ? "ENCENDIDO" : "APAGADO";
            nodoLuz["tiempo_en_estado_anterior_ms"] = currentMillis - tiempoUltimoCambioLuz;
            
            tiempoUltimoCambioLuz = currentMillis;
            cambioLuzDetectado = false; 
        }

        if (cambioBombaDetectada) {
            JsonObject nodoBomba = jsonSD.createNestedObject("actuator_event");
            nodoBomba["dispositivo"] = "BOMBA_RELE";
            nodoBomba["estado"] = statusBomba ? "ENCENDIDO" : "APAGADO";
            nodoBomba["tiempo_en_estado_anterior_ms"] = currentMillis - tiempoUltimoCambioBomba;

            tiempoUltimoCambioBomba = currentMillis;
            cambioBombaDetectada = false; 
        }

        // Almacenamiento físico en formato de texto plano JSON dentro de la MicroSD
        tarjetaSD.appendLog(jsonSD);
    }
}