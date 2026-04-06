import pandas as pd
import numpy as np
import tensorflow as tf
from sklearn.model_selection import train_test_split

# ======================
# LOAD
# ======================
df = pd.read_csv("dataset fix.csv")

X = df.drop("label", axis=1).values
y = df["label"].values

# ======================
# NORMALISASI SAMA SEPERTI DI ESP32
# m1,m2,m3,m4,m5 → dibagi 4095
# t1,t2,t3,t4,t5 → dibagi 50
# ======================
for i in range(10):
    if i % 2 == 0:  # kolom soil (m1,m2,m3,m4,m5)
        X[:, i] = X[:, i] / 4095.0
    else:           # kolom suhu (t1,t2,t3,t4,t5)
        X[:, i] = X[:, i] / 50.0

# ======================
# SPLIT
# ======================
X_train, X_test, y_train, y_test = train_test_split(
    X, y, test_size=0.2, random_state=42
)

# ======================
# MODEL
# ======================
model = tf.keras.Sequential([
    tf.keras.layers.Dense(16, activation='relu', input_shape=(10,)),
    tf.keras.layers.Dense(8, activation='relu'),
    tf.keras.layers.Dense(1, activation='sigmoid')
])

# ======================
# COMPILE
# ======================
model.compile(
    optimizer='adam',
    loss='binary_crossentropy',
    metrics=['accuracy']
)

# ======================
# TRAIN
# ======================
model.fit(
    X_train, y_train,
    epochs=50,
    validation_data=(X_test, y_test)
)

# ======================
# EVALUASI
# ======================
loss, acc = model.evaluate(X_test, y_test)
print(f"\nAkurasi: {acc*100:.2f}%")

# ======================
# CONVERT KE TFLITE
# ======================
converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.convert()

# ======================
# CONVERT KE C ARRAY
# ======================
with open("model_fix.h", "w") as f:
    f.write("alignas(8) unsigned char model_tflite[] = {\n  ")
    f.write(", ".join([f"0x{b:02x}" for b in tflite_model]))
    f.write("\n};\n")
    f.write(f"unsigned int model_tflite_len = {len(tflite_model)};\n")

print("model_fix.h berhasil dibuat!")