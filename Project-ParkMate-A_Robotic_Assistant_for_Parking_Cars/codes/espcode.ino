#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

// ---------- WiFi Credentials ----------
const char* WIFI_SSID = "Demo461";
const char* WIFI_PASS = "12345678";

// ---------- Google Apps Script Web App URL ----------
const char* SCRIPT_URL ="Your sheet link here";


String buffer = "";
unsigned long lastPostTime = 0;
unsigned long lastDebugPrint = 0;

const unsigned long POST_INTERVAL  = 10000;
const unsigned long DEBUG_INTERVAL = 5000;

int avail, occ, s1, s2, s3, ev;


void setup() {
  Serial.begin(9600);

  Serial.println("\nESP8266 starting...");
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected Successfully!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}


void sendToGoogleSheet() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected. Skipping upload.");
    return;
  }

  WiFiClientSecure client;
  client.setInsecure(); 

  HTTPClient http;
  http.begin(client, SCRIPT_URL);
  http.addHeader("Content-Type", "application/json");

  String payload = "{";
  payload += "\"avail\":" + String(avail) + ",";
  payload += "\"occ\":"   + String(occ)   + ",";
  payload += "\"s1\":"    + String(s1)    + ",";
  payload += "\"s2\":"    + String(s2)    + ",";
  payload += "\"s3\":"    + String(s3)    + ",";
  payload += "\"ev\":"    + String(ev);
  payload += "}";

  int httpCode = http.POST(payload);

  Serial.print("Google Sheet POST response: ");
  Serial.println(httpCode);

  http.end();
}


void loop() {

  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      sscanf(buffer.c_str(),
             "PARK,%d,%d,%d,%d,%d,%d",
             &avail, &occ, &s1, &s2, &s3, &ev);
      buffer = "";
    } else {
      buffer += c;
    }
  }


  if (millis() - lastPostTime >= POST_INTERVAL) {
    lastPostTime = millis();
    sendToGoogleSheet();
  }


  if (millis() - lastDebugPrint >= DEBUG_INTERVAL) {
    lastDebugPrint = millis();
    Serial.println("Code working");
  }
}

