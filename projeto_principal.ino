
#include <ESP8266WiFi.h>

#include <WiFiClientSecure.h>

#include <SerialDisplay.h>

#include <YoutubeApi.h>

#include <ArduinoJson.h>


SerialDisplay displays(D3,D4,4); // (data, clock, number of modules)
char ssid[] = "📶";      
char password[] = "🔒";  
#define API_KEY "🔒"  
#define CHANNEL_ID "UC17rGqWCWZCaYqCSdwNYZeQ" 

WiFiClientSecure client;
YoutubeApi api(API_KEY, client);

unsigned long timeBetweenRequests = 30000;
unsigned long nextRunTime;

long subs = 0;

void setup() {

  Serial.begin(115200);

  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
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

void loop() {

  if (millis() > nextRunTime)  {
    if(api.getChannelStatistics(CHANNEL_ID))
    {
      unsigned long inscritos = api.channelStats.subscriberCount;
      displays.Print(inscritos);
      Serial.println(inscritos);
    }
    nextRunTime = millis() + timeBetweenRequests;
  }
}
