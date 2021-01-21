#include <TwitchApi.h>

#include <ESP8266WiFi.h>

#include <WiFiClientSecure.h>

#include "CACert.h"

#include <ArduinoJson.h> // This Sketch doesn't technically need this, but the library does so it must be installed.

#include <SerialDisplay.h>

#include <YoutubeApi.h>

//------- Replace the following! ------
const char *ssid = "";
const char *password = " ";

// twitch
#define TWITCH_CLIENT_ID ""
#define TWITCH_ACCESSTOKEN ""

// youtube
#define API_KEY ""
#define CHANNEL_ID ""

const int pinOE = D2;
const uint8_t totalModulos = 4;

SerialDisplay displays(D3, D4, totalModulos);

WiFiClientSecure client;
TwitchApi twitch(client, TWITCH_CLIENT_ID, TWITCH_ACCESSTOKEN);
YoutubeApi api(API_KEY, client);

String twitchName = "rianmagicbox";

unsigned long delayBetweenRequests = 30000; // twitch
unsigned long requestDueTime = 0;               //time when request due

unsigned long nextRunTime;
unsigned long subviewersAnt = 0;




unsigned long delayBetweenTimeSync = 3600000; // Time between requests (1 hour)
unsigned long timesyncDueTime = 0;

//This is needed to verify certificates
void setTime()
{
  // Synchronize time using SNTP. This is necessary to verify that
  // the TLS certificates offered by the server are currently valid.
  Serial.print("Setting time using SNTP");
  configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

// Load root certificate in DER format into WiFiClientSecure object
void setCertificate()
{
  bool res = client.setCACert_P(caCert, caCertLen);
  if (!res) {
    Serial.println("Failed to load root CA certificate!");
    while (true) {
      yield();
    }
  }
}

void setupWifi()
{
  // Set WiFi to station mode and disconnect from an AP if it was Previously
  // connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  animacao(500);
  clear();
  animacao2(500);
  clear();

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);

  client.setInsecure();

  // api._debug = true;
}

void setup()
{

  Serial.begin(115200);

  displays.setBrightnessPin(pinOE);

  //Init Wifi connection
  setupWifi();

  //Set Twitch.tv Root Certificate
  setCertificate();

  //Set client timeout
  client.setTimeout(60000);
}

void printAndWait(byte modulo, byte segmento, unsigned long tempo) {
  displays.Set(segmento, modulo);
  unsigned long previousMillis = millis();
  unsigned long currentMillis = millis();
  while (currentMillis - previousMillis < tempo) {
    currentMillis = millis();
    delay(10);
  }
}

void animacao(unsigned long tempo) {
  // Da esquerda para a direita
  for (uint8_t i = 1; i <= totalModulos; i++) {
    printAndWait(i, SerialDisplay::PIN_A, tempo);
  }
  // Fazendo o contorno no último módulo a direita
  printAndWait(totalModulos, (SerialDisplay::PIN_A | SerialDisplay::PIN_B), tempo);
  printAndWait(totalModulos, (SerialDisplay::PIN_A | SerialDisplay::PIN_B | SerialDisplay::PIN_C), tempo);
  printAndWait(totalModulos, (SerialDisplay::PIN_A | SerialDisplay::PIN_B | SerialDisplay::PIN_C | SerialDisplay::PIN_D), tempo);

  // Da direita para a esquerda
  for (uint8_t i = totalModulos - 1; i >= 1; i--) {
    printAndWait(i, SerialDisplay::PIN_A | SerialDisplay::PIN_D, tempo);
  }

  // Fazendo o contorno no primeiro módulo a esquerda
  printAndWait(1, (SerialDisplay::PIN_A | SerialDisplay::PIN_D | SerialDisplay::PIN_E), tempo);
  printAndWait(1, (SerialDisplay::PIN_A | SerialDisplay::PIN_D | SerialDisplay::PIN_E | SerialDisplay::PIN_F), tempo);
}

void animacao2(unsigned long tempo) {
  for (uint8_t i = totalModulos; i >= 1; i--) {
    printAndWait (i, SerialDisplay::PIN_A, tempo);
  }
  printAndWait(1, (SerialDisplay::PIN_A | SerialDisplay::PIN_F), tempo);
  printAndWait(1, (SerialDisplay::PIN_A | SerialDisplay::PIN_F | SerialDisplay::PIN_E), tempo);
  printAndWait(1, (SerialDisplay::PIN_A | SerialDisplay::PIN_F | SerialDisplay::PIN_E | SerialDisplay::PIN_D), tempo);

  for (uint8_t i = 2; i <= totalModulos; i++) {
    printAndWait (i, SerialDisplay::PIN_A | SerialDisplay::PIN_D, tempo);
  }
  printAndWait(totalModulos, (SerialDisplay::PIN_A | SerialDisplay::PIN_C | SerialDisplay::PIN_D), tempo);
  printAndWait(totalModulos, (SerialDisplay::PIN_A | SerialDisplay::PIN_C | SerialDisplay::PIN_D | SerialDisplay::PIN_B), tempo);
}

void clear() {
  for (uint8_t i = 1; i <= totalModulos; i++) {
    displays.Set(0x00, i);
  }
}

void animacao3() {
  animacao(32);
  clear();
  animacao2(32);
  clear();
}

void loop() {

  // horario do esp
  if (millis() > timesyncDueTime)
  {
    setTime();
    timesyncDueTime = millis() + delayBetweenTimeSync;
  }
  // twitch
  if (millis() > requestDueTime) {
    StreamInfo stream = twitch.getStreamInfo(twitchName.c_str());
    unsigned long subviewers;
    if (!stream.error) {
      Serial.println("twitch");
      subviewers = stream.viewerCount;
    } else {

      if (api.getChannelStatistics(CHANNEL_ID)) {
        Serial.println("youtube");
        subviewers = api.channelStats.subscriberCount;
      }
    }
    if (subviewers > subviewersAnt) {
      animacao3();
      subviewersAnt = subviewers;
    }
    displays.Print(subviewers);
    Serial.println(subviewers);
    requestDueTime = millis() + delayBetweenRequests;
  }
}
