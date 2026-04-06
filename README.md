
# Predictive Plant Watering (TinyML - ESP32)

## Deskripsi
Proyek ini merupakan implementasi Edge AI / TinyML untuk memprediksi kondisi kekeringan tanah menggunakan data time-series dari sensor soil moisture dan suhu.

Sistem menggunakan 5 data terakhir untuk memprediksi apakah tanah akan menjadi kering atau tetap aman, sehingga lebih cerdas dibandingkan metode threshold biasa (If-Else).

---

## Tujuan
- Memprediksi kondisi tanah secara real-time
- Menggunakan Machine Learning langsung di ESP32 (offline)
- Mengimplementasikan Ring Buffer
- Monitoring data melalui Blynk

---

## Hardware
- ESP32  
- Sensor Soil Moisture (Analog)  
- Sensor BMP280 (Suhu)  

---

## Machine Learning
Model menggunakan TensorFlow Lite (TinyML):

- Input: 10 fitur (5 soil + 5 suhu)
- Output: 1 nilai probabilitas

### Format Input:
[soil1, temp1, soil2, temp2, ..., soil5, temp5]

### Normalisasi:
- Soil → dibagi 4095  
- Suhu → dibagi 50  

### Output:
- > 0.5 → KERING  
- ≤ 0.5 → AMAN  

---

## Ring Buffer
Digunakan untuk menyimpan 5 data terakhir:

#define BUFFER_SIZE 5

Fungsi:
- Menyimpan data historis
- Menggeser data otomatis
- Menjadi input model ML

---

## Alur Sistem
1. Sensor membaca soil & suhu  
2. Data masuk ke buffer  
3. Jika buffer penuh:
   - Normalisasi data  
   - Masukkan ke model ML  
4. Model melakukan prediksi  
5. Hasil dikirim ke Blynk  

---

## Integrasi Blynk
- V0 → Persentase kekeringan (%)
- V1 → Status (KERING / AMAN)
- V2 → Soil Moisture
- V3 → Suhu

---

## Perbandingan Metode

If-Else:
- Sederhana
- Tidak melihat pola perubahan data

TinyML:
- Bisa membaca pola data
- Bisa prediksi sebelum kering
- Lebih adaptif

---

## Arsitektur Model
- Input Layer (10 neuron)
- Hidden Layer (Dense + ReLU)
- Output Layer (Sigmoid)

---

## Cara Menjalankan
1. Upload kode ke ESP32  
2. Hubungkan ke WiFi & Blynk  
3. Pastikan sensor aktif  
4. Tunggu buffer penuh (5 data)  
5. Sistem mulai prediksi otomatis  

