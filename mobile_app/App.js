import React, { useState, useEffect } from 'react';
import { Text, View, StyleSheet, TouchableOpacity, SafeAreaView, ScrollView, StatusBar, Platform } from 'react-native';
import init from 'react_native_mqtt';

// Inicialización del almacenamiento interno requerido por la librería MQTT
import AsyncStorage from '@react-native-async-storage/async-storage';
init({
  size: 10000,
  storageBackend: AsyncStorage,
  defaultExpires: 1000 * 3600 * 24,
  enableCache: true,
  reconnect: true,
});

export default function App() {
  // Estados dinámicos vinculados al ESP32
  const [temperature, setTemperature] = useState(0.0);
  const [waterLevel, setWaterLevel] = useState(0);
  const [lightStatus, setLightStatus] = useState(false); // Ahora se actualiza mediante los sensores
  const [currentTime, setCurrentTime] = useState("Conectando al acuario...");
  const [client, setClient] = useState(null);

  // Estados para controlar los colores e indicadores de los botones activos
  const [pumpStatus, setPumpStatus] = useState(false);

  useEffect(() => {
    const mqttClient = new Paho.MQTT.Client('broker.hivemq.com', 8000, 'App_Pecera_Client_' + Math.random().toString(16).substr(2, 8));

    // Callback 1: Ocurre cuando llega telemetría real desde el ESP32
    mqttClient.onMessageArrived = (message) => {
      if (message.destinationName === 'pecera/telemetria') {
        try {
          const data = JSON.parse(message.payloadString);
          
          // Actualizamos la interfaz con los valores reales del hardware
          setTemperature(data.temperature);
          setWaterLevel(data.waterLevel);
          setLightStatus(data.lightStatus); // Leemos el estado del Relé controlado por el LDR
          
          const now = new Date();
          setCurrentTime(now.toLocaleTimeString() + " - " + now.toLocaleDateString());
        } catch (e) {
          console.log("Error al parsear el JSON del ESP32", e);
        }
      }
    };

    mqttClient.onConnectionLost = (responseObject) => {
      if (responseObject.errorCode !== 0) {
        setCurrentTime("Conexión perdida. Reintentando...");
      }
    };

    mqttClient.connect({
      onSuccess: () => {
        setCurrentTime("Conectado a la Pecera");
        mqttClient.subscribe('pecera/telemetria');
        setClient(mqttClient);
      },
      onFailure: (err) => {
        setCurrentTime("Error de conexión con el Broker");
        console.log(err);
      },
      useSSL: false,
      reconnect: true
    });

    return () => {
      if (mqttClient.isConnected()) {
        mqttClient.disconnect();
      }
    };
  }, []);

  // Función para encender/apagar bomba de agua vía MQTT
  const togglePump = () => {
    if (client && client.isConnected()) {
      const nuevoEstado = !pumpStatus;
      const message = new Paho.MQTT.Message(nuevoEstado ? "1" : "0");
      message.destinationName = 'pecera/control/bomba';
      client.send(message);
      setPumpStatus(nuevoEstado); 
    }
  };

  // Función para activar un pulso del dispensador de comida vía MQTT
  const feedFish = () => {
    if (client && client.isConnected()) {
      const message = new Paho.MQTT.Message("1");
      message.destinationName = 'pecera/control/comida';
      client.send(message);
      alert("¡Comando enviado! El servomotor está girando.");
    } else {
      alert("No hay conexión con la pecera.");
    }
  };

  return (
    <SafeAreaView style={styles.container}>
      <StatusBar barStyle="dark-content" /> 
      
      <ScrollView contentContainerStyle={styles.scrollContainer}>
        <View style={styles.headerContainer}>
          <Text style={styles.title}>Pecera Inteligente IoT</Text>
          <Text style={styles.timeText}>{currentTime}</Text>
        </View>

        {/* SECCIÓN DE MONITOREO (SENSORES Y ESTADOS REALES) */}
        <View style={styles.card}>
          <Text style={styles.cardTitle}>Monitoreo del Sistema</Text>
          
          <View style={styles.dataRow}>
            <Text style={styles.dataLabel}>Temperatura del Agua:</Text>
            <Text style={styles.dataValue}>{temperature.toFixed(1)} °C</Text>
          </View>
          
          <View style={styles.dataRow}>
            <Text style={styles.dataLabel}>Nivel del Tanque:</Text>
            <Text style={styles.dataValue}>{waterLevel} %</Text>
          </View>

          {/* REQUERIMIENTO NUEVO: El estado de la luz pasa a ser Monitoreo */}
          <View style={styles.dataRow}>
            <Text style={styles.dataLabel}>Sistema de Iluminación:</Text>
            <Text style={[styles.statusText, lightStatus ? styles.textOn : styles.textOff]}>
              {lightStatus ? "ENCENDIDO" : "APAGADO"}
            </Text>
          </View>
        </View>

        {/* SECCIÓN DE PANEL DE CONTROL (SÓLO ACTUADORES QUE CONTROLA EL USUARIO) */}
        <View style={styles.card}>
          <Text style={styles.cardTitle}>Panel de Control</Text>

          <View style={styles.controlRow}>
            <Text style={styles.controlLabel}>Bomba de Agua:</Text>
            <TouchableOpacity 
              style={[styles.button, pumpStatus ? styles.btnOn : styles.btnOff]} 
              onPress={togglePump}
            >
              <Text style={styles.btnText}>{pumpStatus ? "ACTIVA" : "INACTIVA"}</Text>
            </TouchableOpacity>
          </View>

          <View style={styles.controlRow}>
            <Text style={styles.controlLabel}>Dispensador de Comida:</Text>
            <TouchableOpacity style={[styles.button, styles.btnAction]} onPress={feedFish}>
              <Text style={styles.btnText}>DAR COMIDA</Text>
            </TouchableOpacity>
          </View>
        </View>
      </ScrollView>
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  container: { 
    flex: 1, 
    backgroundColor: '#f4f6f9',
    paddingTop: Platform.OS === 'android' ? StatusBar.currentHeight : 0 
  },
  scrollContainer: { paddingHorizontal: 20, paddingBottom: 30 },
  headerContainer: { marginTop: 30, marginBottom: 20, alignItems: 'center' },
  title: { fontSize: 26, fontWeight: 'bold', color: '#1a365d', textAlign: 'center' },
  timeText: { fontSize: 14, color: '#718096', textAlign: 'center', marginTop: 5 },
  card: { backgroundColor: '#ffffff', borderRadius: 12, padding: 16, marginBottom: 20, shadowColor: '#000', shadowOffset: { width: 0, height: 2 }, shadowOpacity: 0.1, shadowRadius: 4, elevation: 3 },
  cardTitle: { fontSize: 18, fontWeight: 'bold', color: '#2b6cb0', marginBottom: 15, borderBottomWidth: 1, borderBottomColor: '#edf2f7', paddingBottom: 5 },
  dataRow: { flexDirection: 'row', justifyContent: 'space-between', paddingVertical: 10, alignItems: 'center' },
  dataLabel: { fontSize: 16, color: '#4a5568' },
  dataValue: { fontSize: 16, fontWeight: 'bold', color: '#2d3748' },
  statusText: { fontSize: 16, fontWeight: 'bold' },
  textOn: { color: '#48bb78' },
  textOff: { color: '#e53e3e' },
  controlRow: { flexDirection: 'row', justifyContent: 'space-between', alignItems: 'center', paddingVertical: 12 },
  controlLabel: { fontSize: 15, color: '#4a5568', flex: 1 },
  button: { paddingVertical: 10, paddingHorizontal: 16, borderRadius: 8, minWidth: 120, alignItems: 'center' },
  btnOn: { backgroundColor: '#48bb78' },
  btnOff: { backgroundColor: '#e53e3e' },
  btnAction: { backgroundColor: '#3182ce' },
  btnText: { color: '#ffffff', fontWeight: 'bold', fontSize: 14 }
});