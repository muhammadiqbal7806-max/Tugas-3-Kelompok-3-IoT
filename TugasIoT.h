// ======================
// BLYNK CONFIG
// ======================
#define BLYNK_TEMPLATE_ID "TMPL6l915BAma"
#define BLYNK_TEMPLATE_NAME "monitoring soil moisture"
#define BLYNK_AUTH_TOKEN "15gqzzif8QhAXYntfIl9wgcgIpzVXawA"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// ======================
// BMP280
// ======================
#include <Wire.h>
#include <Adafruit_BMP280.h>

// ======================
// ML
// ======================
#include <tflm_esp32.h>
#include <eloquent_tinyml.h>
#include <eloquent_tinyml/tf.h>
#include "model fixbanget.h"

// ======================
// WIFI
// ======================
char ssid[] = "Le Minerale";
char pass[] = "gosong12";

// ======================
// SENSOR
// ======================
#define SOIL_PIN 4

Adafruit_BMP280 bmp;

// ======================
// RING BUFFER
// ======================
#define BUFFER_SIZE 5

float soil_buffer[BUFFER_SIZE] = {0};
float temp_buffer[BUFFER_SIZE] = {0};

int buffer_index = 0;
bool buffer_full = false;

// ======================
// ML CONFIG
// ======================
Eloquent::TF::Sequential<64, 32*1024> tf;

// ======================
void setup() {
  Serial.begin(115200);
  delay(1000);

  // INIT BMP280
  Wire.begin(8, 9);
  if (!bmp.begin(0x76)) {
    if (!bmp.begin(0x77)) {
      Serial.println("ERROR: BMP280 tidak ditemukan!");
      while (1);
    }
  }
  Serial.println("BMP280 OK");

  // INIT BLYNK
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // INIT MODEL
  tf.setNumInputs(10);
  tf.setNumOutputs(1);

  tf.resolver.AddFullyConnected();
  tf.resolver.AddSoftmax();
  tf.resolver.AddRelu();
  tf.resolver.AddLogistic();
  tf.resolver.AddQuantize();
  tf.resolver.AddDequantize();

  if (!tf.begin(model_tflite).isOk()) {
    Serial.println("ERROR: Model gagal dimuat!");
    Serial.println(tf.exception.toString());
    while (1);
  }

  Serial.println("SYSTEM READY");
}

// ======================
void addToBuffer(float soil, float temp) {
  soil_buffer[buffer_index] = soil;
  temp_buffer[buffer_index] = temp;

  buffer_index++;

  if (buffer_index >= BUFFER_SIZE) {
    buffer_index = 0;
    buffer_full  = true;
  }
}

// ======================
void loop() {
  Blynk.run();

  // BACA SENSOR
  float soil = (float)analogRead(SOIL_PIN);
  float temp = bmp.readTemperature();

  Serial.print("Soil RAW: ");
  Serial.println(soil);
  Serial.print("Suhu: ");
  Serial.print(temp);
  Serial.println(" C");

  // KIRIM SENSOR KE BLYNK
  Blynk.virtualWrite(V2, soil);
  Blynk.virtualWrite(V3, temp);

  addToBuffer(soil, temp);

  if (buffer_full) {

    // SUSUN INPUT MODEL
    float input[10];
    for (int i = 0; i < BUFFER_SIZE; i++) {
      input[i * 2]     = soil_buffer[i] / 4095.0f;
      input[i * 2 + 1] = temp_buffer[i] / 50.0f;
    }

    // INFERENSI
    if (!tf.predict(input).isOk()) {
      Serial.println("ERROR: Prediksi gagal!");
      Serial.println(tf.exception.toString());
      return;
    }

    float prob   = tf.output(0);
    float persen = prob * 100.0f;

    Serial.print("Probabilitas kering: ");
    Serial.print(persen);
    Serial.println(" %");
    Serial.println("--------------------");

    // KIRIM HASIL KE BLYNK
    Blynk.virtualWrite(V0, persen);

    if (prob > 0.5f) {
      Serial.println("STATUS: KERING");
      Blynk.virtualWrite(V1, "KERING");
    } else {
      Serial.println("STATUS: AMAN");
      Blynk.virtualWrite(V1, "AMAN");
    }
  }

  delay(5000);
}