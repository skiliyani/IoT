#include <WiFi.h>
#include <PubSubClient.h>
#include <U8g2lib.h>

const char* ssid     = "SAYANI_IOT";
const char* password = "00001111";
const char* mqtt_server = "192.168.8.10";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

//U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

char msg[50];

void setup_wifi() {

  delay(100);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

#define WATER  0

void drawSymbol(u8g2_uint_t x, u8g2_uint_t y, uint8_t symbol)
{
  switch(symbol)
  {
    case WATER:
      u8g2.setFont(u8g2_font_open_iconic_thing_4x_t);
      u8g2.drawGlyph(x, y, 72); 
      break;
  }
}

void drawWater(uint8_t symbol, int degree)
{
  drawSymbol(8, 48, symbol);
  u8g2.setFont(u8g2_font_logisoso24_tf);
  u8g2.setCursor(48, 44);
  u8g2.print(degree);
  u8g2.print("%");    // requires enableUTF8Print()
}

void draw(uint8_t symbol, int value)
{
  u8g2.clearBuffer();         // clear the internal memory
  drawWater(symbol, value);   // draw the icon and degree only once
  u8g2.sendBuffer();  
}

void callback(char* topic, byte* payload, unsigned int length) {

  String distance_str = "";
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
    distance_str += (char)payload[i];
  }
  Serial.println();


  int distance_int_cm = distance_str.toInt();
  int levelPercentage = 100 - ((distance_int_cm - 15) / 67.0 * 100.0);
  Serial.print("Water Level: ");
  Serial.print(levelPercentage);
  Serial.print("%");
  Serial.println();

  //snprintf (msg, 50, "%ld%s", levelPercentage,"%");
  //u8x8.drawString(0, 1, msg);
  draw(WATER,levelPercentage);
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  u8g2.begin();
  //u8g2.enableUTF8Print();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("waterLevelTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
