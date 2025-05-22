//  Blynk
#define BLYNK_TEMPLATE_ID "TMPL6-UMMO4k4"
#define BLYNK_TEMPLATE_NAME "GasLeakSmokeDetection"
#define BLYNK_AUTH_TOKEN "ii3qw-JuyMzDZ-PussbZIbXWNSwSkUo8"

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <BlynkSimpleEsp32.h>


// WiFi Credentials
char ssid[] = "Wokwi-GUEST";
char pass[] = "";
char auth[] = BLYNK_AUTH_TOKEN;

// Cấu hình cảm biến DHT22
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Cảm biến khí gas
#define GAS_PIN 33 //MQ2

// Cảm biến ánh sáng
#define LDR1_PIN 34 // LDR
#define FLAME_PIN 32 //On realtime i change ldr2 to ky026

// Buzzer
#define BUZZER_PIN 27

// Khởi tạo LCD I2C (địa chỉ 0x27)
#define SDA 25
#define SCL 26


  
#define BOTtoken "8034300471:AAEZn8Fn6AJKrLGxKwikcKpS4Mvu_upMb14"
#define CHAT_ID "6673024895"
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);


LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(9600);
  // Kết nối WiFi
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  // Kết nối đến Telegram
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  // Khởi tạo cảm biến
  dht.begin();
  
  // Cấu hình chân vào/ra
  pinMode(GAS_PIN, INPUT);
  pinMode(LDR1_PIN, INPUT);
  pinMode(FLAME_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Khởi động LCD
  Wire.begin(SDA, SCL);
  lcd.init();
  lcd.backlight();

  // Hiển thị khởi động
  lcd.setCursor(0, 0);
  lcd.print("ESP32 Sensor Hub");
  delay(2000);
  lcd.clear();

  bot.sendMessage(CHAT_ID, "System Online! Monitoring Fire & Gas Hazards");

}

void loop() {
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    int gasLevel = analogRead(GAS_PIN);
    int ldr1Value = analogRead(LDR1_PIN);
    int flameValue = analogRead(FLAME_PIN);
    
    // These constants should match the photoresistor's "gamma" and "rl10" attributes
  const float GAMMA = 0.7;
  const float RL10 = 50;
  float voltage = ldr1Value / 4095.0 * 3.3;
  float resistance = 2000 * voltage / (1 - voltage / 5);
  float lux = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1 / GAMMA));


    // Hiển thị lên Serial
    Serial.print("Nhiet do: "); Serial.print(temperature); Serial.println(" *C");
    Serial.print("Do am: "); Serial.print(humidity); Serial.println(" %");
    Serial.print("Gas: "); Serial.println(gasLevel); Serial.println(" PPM");
    Serial.print("Lux: "); Serial.println(lux); Serial.println(" LX");
    Serial.print("Flame level: "); Serial.println(flameValue); Serial.println(" NM");
    Serial.println("--------------------");

    // Hiển thị LCD
    lcd.setCursor(0, 0);
    lcd.print("T:"); lcd.print(temperature, 1); lcd.print("C ");
    lcd.print("H:"); lcd.print(humidity, 0); lcd.print("%");

    Blynk.run();


    Serial.print("T: "); Serial.print(temperature, 1);
    Serial.print("°C | H: "); Serial.print(humidity, 1);
    Serial.print(" % | Gas: "); Serial.print(gasLevel);
    Serial.print(" | LUX: "); Serial.print(lux);
    Serial.print(" | Flame: "); Serial.println(flameValue);

    Blynk.virtualWrite(V0, temperature);
    Blynk.virtualWrite(V1, humidity);
    Blynk.virtualWrite(V2, gasLevel);
    Blynk.virtualWrite(V3, lux);
    Blynk.virtualWrite(V4, flameValue);

    bool isFlameDetected = flameValue < 1000;  // giá trị thấp = phát hiện lửa
    bool isGasDetected = gasLevel > 1500;      // ngưỡng khí gas cao
    bool isBrightSuddenly = lux > 12000 && gasLevel > 1000;  // ánh sáng mạnh bất thường + có gas

    if (isFlameDetected || isGasDetected || isBrightSuddenly) {
        Serial.println("WARNING: FIRE/GAS DETECTED!");

        for (int i = 0; i < 3; i++) {
            digitalWrite(BUZZER_PIN, HIGH);
            delay(300);
            digitalWrite(BUZZER_PIN, LOW);
            delay(300);
        }

        String alertMessage = "WARNING! FIRE/GAS DETECTED!\n";
        alertMessage += "Temperature: " + String(temperature, 1) + "°C\n";
        alertMessage += "Humidity: " + String(humidity, 1) + "%\n";
        alertMessage += "Gas Level: " + String(gasLevel) + "\n";
        alertMessage += "Flame Intensity: " + String(flameValue) + "\n";
        bot.sendMessage(CHAT_ID, alertMessage);

        Blynk.virtualWrite(V5, HIGH);
    } else {
        Blynk.virtualWrite(V5, LOW);
    }
        

    delay(2000);
}

