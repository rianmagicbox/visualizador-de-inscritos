
#include <ESP8266WiFi.h>

#include <WiFiClientSecure.h>

#include <SerialDisplay.h>

#include <YoutubeApi.h>

#include <ArduinoJson.h>

const int pinOE = D2;
const uint8_t totalModulos = 4;

SerialDisplay displays(D3,D4,totalModulos); // (data, clock, number of modules)
char ssid[] = "SSID";      
char password[] = "PASSWORD";  
#define API_KEY "your API key"  
#define CHANNEL_ID "Your chanel id" 

WiFiClientSecure client;
YoutubeApi api(API_KEY, client);

unsigned long timeBetweenRequests = 30000;
unsigned long nextRunTime;
unsigned long inscritosAnt = 0;

long subs = 0;


void setup() {

  Serial.begin(115200);

  displays.setBrightnessPin(pinOE);
  
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

 
  api._debug = true;
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

void clear() {
  for (uint8_t i = 1; i <= totalModulos; i++) {
    displays.Set(0x00, i);
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

void loop() {

  if (millis() > nextRunTime)  {
    if(api.getChannelStatistics(CHANNEL_ID))
    {
      unsigned long inscritos = api.channelStats.subscriberCount;
      if (inscritos > inscritosAnt) {
        animacao(32);
        clear();  
        animacao2(32);
        clear();
        inscritosAnt = inscritos;
      }
      displays.Print(inscritos);
      Serial.println(inscritos);
    }
    nextRunTime = millis() + timeBetweenRequests;
  }

  
  
}
