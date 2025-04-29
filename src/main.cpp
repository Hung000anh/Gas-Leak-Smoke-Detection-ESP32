#define BLYNK_TEMPLATE_ID "TMPL6-UMMO4k4"
#define BLYNK_TEMPLATE_NAME "GasLeakSmokeDetection"
#define BLYNK_AUTH_TOKEN "AJ8dti2HSmJs-akLFPOIM_AeXi6JMyZ6"

#define DHTPIN 12       // Chân kết nối cảm biến DHT22
#define DHTTYPE DHT22
#define LDR_PIN 34      // Chân analog output của LDR
#define RELAY_PIN 22    // Chân điều khiển relay

#include <DHT.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

// WiFi credentials
char ssid[] = "Wokwi-GUEST";  // Hoặc WiFi nhà bạn
char pass[] = "";             // Mật khẩu WiFi

void sendSensorData() {
  float temp = dht.readTemperature();
  float humi = dht.readHumidity();

  // Tính lux từ LDR
  const float GAMMA = 0.7;
  const float RL10 = 50;
  int analogValue = analogRead(LDR_PIN);
  float voltage = analogValue / 4095.0 * 3.3;
  float resistance = 2000 * voltage / (1 - voltage / 5);
  float lux = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1 / GAMMA));

  // In ra Serial
  Serial.print("Nhiet do: "); Serial.print(temp);
  Serial.print(" *C, Do am: "); Serial.print(humi);
  Serial.print(" %, Anh sang: "); Serial.println(lux);

  // Gửi lên Blynk
  Blynk.virtualWrite(V0, humi); // Humidity
  Blynk.virtualWrite(V1, temp); // Temperature
  Blynk.virtualWrite(V2, lux);  // Lux

  // Điều kiện phát hiện cháy: nhiệt độ cao và ánh sáng mạnh
  if (temp > 70 && lux > 10000) {
    digitalWrite(RELAY_PIN, HIGH);
    Serial.println("🔥 Cảnh báo cháy! Buzzer ON");
  } else {
    digitalWrite(RELAY_PIN, LOW);
    Serial.println("✅ Bình thường. Buzzer OFF");
  }
}

void setup() {
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  dht.begin();
  pinMode(LDR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Tắt buzzer ban đầu

  timer.setInterval(1000L, sendSensorData);
}

void loop() {
  Blynk.run();
  timer.run();
}
