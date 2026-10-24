// OOP Version of Auto Fish Feeder with Blynk Integration

#define BLYNK_TEMPLATE_ID "  " // Sesuaikan dengan proyek anda
#define BLYNK_TEMPLATE_NAME "  " // Sesuaikan dengan proyek anda
#define BLYNK_AUTH_TOKEN "  " // Sesuaikan dengan proyek anda

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <TimeLib.h>
#include <WidgetRTC.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP32Servo.h>

class AutoFeeder {
private:
  const char* ssid = "  "; // Sesuaikan dengan proyek anda
  const char* pass = "  "; // Sesuaikan dengan proyek anda
  const int servoPin = 13; // Sesuaikan dengan proyek anda
  const int oneWireBus = 4; // Sesuaikan dengan proyek anda
  const int ledBlynk = V4; // Sesuaikan dengan proyek anda

  OneWire oneWire;
  DallasTemperature sensors;
  Servo servo;
  WidgetRTC rtc;
  BlynkTimer timer;

  bool sudahPagi = false;
  bool sudahSiang = false;
  bool sudahSore = false;

public:
  AutoFeeder() : oneWire(oneWireBus), sensors(&oneWire) {}

  void begin() {
    Serial.begin(115200);
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    sensors.begin();
    servo.attach(servoPin);
    servo.write(0);
    timer.setInterval(1000L, [this]() { sendTimeToBlynk(); });
    timer.setInterval(3000L, [this]() { sendTemperatureToBlynk(); });
    timer.setInterval(1000L, [this]() { cekJadwalPakan(); });
  }

  void update() {
    Blynk.run();
    timer.run();
  }

  void connectRTC() {
    rtc.begin();
  }

private:
  void sendTimeToBlynk() {
    String jam = (hour() < 10 ? "0" : "") + String(hour()) + ":" +
                 (minute() < 10 ? "0" : "") + String(minute()) + ":" +
                 (second() < 10 ? "0" : "") + String(second());

    String tanggal = (day() < 10 ? "0" : "") + String(day()) + "/" +
                     (month() < 10 ? "0" : "") + String(month()) + "/" +
                     String(year());

    Blynk.virtualWrite(V1, jam); // Sesuaikan dengan proyek anda
    Blynk.virtualWrite(V2, tanggal); // Sesuaikan dengan proyek anda
    Serial.println("Jam: " + jam + " | Tanggal: " + tanggal);
  }

  void sendTemperatureToBlynk() {
    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0);
    Blynk.virtualWrite(V3, tempC); // Sesuaikan dengan proyek anda
    Serial.println("Suhu Air: " + String(tempC) + " °C");

    String status;
    if (tempC > 30) {
      status = "Suhu Air Panas";
    } else if (tempC > 28 && tempC < 30) {
      status = "Suhu Air Hangat";
    } else if (tempC >= 25 && tempC <= 28) {
      status = "Suhu Air Normal";
    } else {
      status = "Suhu Air Dingin";
    }
    Serial.println("Update status suhu: " + status);
    Blynk.virtualWrite(V5, status); // Sesuaikan dengan proyek anda
  }

  void beriPakan() {
    Serial.println(">> Pemberian pakan dimulai");
    Blynk.virtualWrite(ledBlynk, 255);
    servo.write(90);
    delay(2000);
    servo.write(0);
    delay(500);
    Blynk.virtualWrite(ledBlynk, 0);
    Serial.println(">> Pakan selesai\n");
  }

  void cekJadwalPakan() {
    int h = hour();
    int m = minute();
    int s = second();

    if (h == 18 && m == 20 && s == 0 && !sudahPagi) {
      beriPakan();
      sudahPagi = true;
    }
    if (h == 18 && m == 23 && s == 0 && !sudahSiang) {
      beriPakan();
      sudahSiang = true;
    }
    if (h == 18 && m == 25 && s == 0 && !sudahSore) {
      beriPakan();
      sudahSore = true;
    }
    if (h == 0 && m == 0 && s == 0) {
      sudahPagi = false;
      sudahSiang = false;
      sudahSore = false;
    }
  }
};

AutoFeeder feeder;

BLYNK_CONNECTED() {
  feeder.connectRTC();
}

void setup() {
  feeder.begin();
  setSyncInterval(10 * 60);
}

void loop() {
  feeder.update();
}
