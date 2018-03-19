#include <ESP8266WiFi.h>
#include <gpio.h>

#define PHOTORESISTOR A0
#define LED 5 // D1
#define REDLED 4 // D2

unsigned long lastMillis = 0;
bool notificationSent = false;

// WiFi network info.
const char* ssid = "YOUR_WIFI_SSID";
const char* wifiPassword = "YOUR_WIFI_PASSWORD";

// IFTTT Maker webhook info.
const char* host = "YOUR_WEBHOOK_HOST";
const char* url = "YOUR_WEBHOOK_URL_WITHOUT_HOST";
const int httpPort = 80;

void setup() {
  Serial.begin(9600);
  delay(10);

  pinMode(LED, OUTPUT);
  pinMode(REDLED, OUTPUT);

  connectToWifi();
}

void loop() {
  //Publish data every 60 seconds (60000 milliseconds).
  if (millis() - lastMillis > 60000) {
    lastMillis = millis();

    int lux = analogRead(PHOTORESISTOR);

    bool finished = false;

    if (lux == 1024) {
      finished = true;
    }

    Serial.println();
    Serial.print(lastMillis);
    Serial.print(" - ");
    Serial.print(lux);
    Serial.print(" - ");

    if (finished == true) {
      Serial.print("finished");
      digitalWrite(LED, HIGH);

      if (notificationSent == false) {
        notificationSent = true;

        sendNotification();
      } else {
        Serial.print(" - Notification already sent in this wash");
      }
    } else {
      Serial.println("not finished");
      digitalWrite(LED, LOW);

      notificationSent = false;
    }
  }
}

void connectToWifi() {
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, wifiPassword);

  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(REDLED, HIGH);
    delay(500);
    digitalWrite(REDLED, LOW);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void sendNotification() {
  Serial.println();
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client; 
  
  if (!client.connect(host, httpPort)) {
    digitalWrite(REDLED, HIGH);
    Serial.println("connection failed");
    return;
  }

  Serial.print("Requesting URL: ");
  Serial.println(url);

  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  unsigned long timeout = millis();
  
  while (client.available() == 0) {
    digitalWrite(REDLED, LOW);
    if (millis() - timeout > 5000) {
      digitalWrite(REDLED, HIGH);
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");
  Serial.println();
}

