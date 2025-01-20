#include <WiFi.h>
#include "WiFiClientSecure.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#define WLAN_SSID "L&S"
#define WLAN_PASS "lucianabeatriz"
#define AIO_USERNAME "LuisMiguel_"
#define AIO_KEY "aio_CntD81zvKvMCY5tXQgGukT1wTHld"
#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883

#define TOUCH_PIN 14
#define LED_PIN 2

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish decodedWord = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/decoded_word_feed");

unsigned long lastTime = 0;
String morseSequence = "";
String decodedWordText = "";
String validMorseCode = ".."; 

void setup() {
  Serial.begin(115200);
  pinMode(TOUCH_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.println("Iniciando detecção de código Morse com sensor touch");

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  Serial.print("Conectando ao WiFi...");
  
  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nErro: WiFi não conectou. Reiniciando ESP32...");
    delay(5000);
    ESP.restart();
  }

  Serial.println("\nWiFi Conectado!");
}

void MQTT_connect() {
  if (mqtt.connected()) return;
  Serial.print("Conectando ao MQTT...");
  
  while (mqtt.connect() != 0) {
    Serial.println("\nFalha na conexão MQTT. Tentando novamente...");
    delay(5000);
  }
  
  Serial.println("MQTT Conectado!");
}

void loop() {
  MQTT_connect();

  int touchState = digitalRead(TOUCH_PIN);

  if (touchState == LOW) { 
    unsigned long currentTime = millis();

    if (currentTime - lastTime > 1000) {
      morseSequence += "-";
      Serial.println("Traço detectado");
    } else if (currentTime - lastTime > 200) {
      morseSequence += ".";
      Serial.println("Ponto detectado");
    }
    lastTime = currentTime;
  }

  if (morseSequence.length() > 8) {
    Serial.println("Sequência de Morse detectada: " + morseSequence);

    if (morseSequence == "...---...") {
      decodedWordText = "SOS";  
    } else {
      decodedWordText = "Desconhecido";
    }

    Serial.print("Palavra Decodificada: ");
    Serial.println(decodedWordText);

    if (!decodedWord.publish(decodedWordText.c_str())) {
      Serial.println("Erro no envio MQTT!");
    } else {
      Serial.println("Palavra enviada via MQTT!");
    }

    if (decodedWordText != "Desconhecido") {
      Serial.println("Código correto! Acendendo o LED...");
      digitalWrite(LED_PIN, HIGH);
      delay(5000);
      digitalWrite(LED_PIN, LOW);
    } else {
      Serial.println("Código incorreto! Reiniciando...");
    }

    morseSequence = "";
  }

  delay(50);
}
