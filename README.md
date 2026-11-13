# 🐟 Sistema de Monitoreo IoT para Peceras Inteligentes

Arquitectura IoT de extremo a extremo diseñada para el monitoreo en tiempo real y la gestión automatizada de las condiciones ambientales de un acuario.

## 📐 Arquitectura y Componentes

El sistema se divide en tres capas principales:

1. **Capa de Hardware (Edge):**
   * **Microcontrolador:** ESP32 V1.
   * **Sensores y Módulos:** 
     * **DS18B20:** Temperatura del agua.
     * **HC-SR04:** Nivel de agua del tanque.
     * **LDR:** Intensidad lumínica ambiental.
     * **RTC DS1307:** Sincronización horaria y sellado de tiempo real.
     * **Módulo MicroSD:** Almacenamiento local de registros de datos.
     * **Módulos de Relés (Relevadores):** Control de potencia para actuadores (bomba de agua e iluminación).
     * **Servomotor MG995 (15 kg):** Mecanismo de actuación física para el alimentador automático.
     * **Pantalla LCD I2C:** Interfaz visual local de parámetros.

2. **Capa de Backend y Comunicaciones:**
   * **Protocolo MQTT:** Transmisión de telemetría en tiempo real.
   * **Servidor:** API desarrollada en **Node.js**.
   * **Base de Datos:** Almacenamiento en **MongoDB**.

3. **Capa de Usuario (Frontend Móvil):**
   * Aplicación móvil desarrollada en **React Native** (gestionada con Expo) para visualizar parámetros y controlar actuadores (bomba de agua y alimentador).

## 📂 Estructura del Repositorio

* `/pecera_iot` - Código fuente en C++ para el ESP32.
* `/database` - Servidor Node.js y lógica de conexión a base de datos.
* `/mobile_app` - Aplicación móvil desarrollada en React Native.

## 🛠️ Tecnologías Utilizadas

![ESP32](https://img.shields.io/badge/ESP32-000000?style=for-the-badge&logo=espressif&logoColor=red)
![C++](https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![MQTT](https://img.shields.io/badge/MQTT-3C5280?style=for-the-badge&logo=mqtt&logoColor=white)
![Node.js](https://img.shields.io/badge/Node.js-339933?style=for-the-badge&logo=nodedotjs&logoColor=white)
![MongoDB](https://img.shields.io/badge/MongoDB-4EA94B?style=for-the-badge&logo=mongodb&logoColor=white)
![React Native](https://img.shields.io/badge/React_Native-20232A?style=for-the-badge&logo=react&logoColor=61DAFB)
