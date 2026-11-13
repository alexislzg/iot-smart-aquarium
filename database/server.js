// Instalar primero con: npm install mqtt mongodb
const mqtt = require('mqtt');
const { MongoClient } = require('mongodb');

// 1. Configuración de conexiones (Mismas credenciales que el ESP32)
const MQTT_BROKER = 'mqtt://broker.hivemq.com';
const MONGO_URL = 'mongodb://localhost:27017'; // Tu servidor local de MongoDB
const DB_NAME = 'pecera_iot';
const COLLECTION_NAME = 'historial_sensores';

// 2. Conectar a la Base de Datos de MongoDB
MongoClient.connect(MONGO_URL)
  .then((client) => {
    console.log('=> Conectado exitosamente a MongoDB local.');
    const db = client.db(DB_NAME);
    const sensoresCollection = db.collection(COLLECTION_NAME);

    // 3. Conectar al Broker MQTT por Internet
    const mqttClient = mqtt.connect(MQTT_BROKER);

    mqttClient.on('connect', () => {
      console.log('=> Conectado al Broker MQTT. Escuchando al ESP32...');
      // Nos suscribimos al canal de telemetría de la pecera
      mqttClient.subscribe('pecera/telemetria');
    });

    // 4. Cachar los datos del ESP32 y meterlos a MongoDB
    mqttClient.on('message', async (topic, message) => {
      if (topic === 'pecera/telemetria') {
        try {
          // Parseamos el JSON que mandó el ESP32
          const datosSensores = JSON.parse(message.toString());
          
          // Le inyectamos la fecha y hora del servidor para el historial de MongoDB
          datosSensores.fecha_registro = new Date();

          // Guardar el documento de forma real y permanente en MongoDB
          await sensoresCollection.insertOne(datosSensores);
          console.log('Datos guardados en MongoDB:', datosSensores);
          
        } catch (error) {
          console.error('Error al procesar los datos del MQTT:', error);
        }
      }
    });
  })
  .catch((err) => {
    console.error('Error crítico al conectar a MongoDB:', err);
  });