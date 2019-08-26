#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <U8x8lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif

const char* ssid     = "SAYANI_IOT";
const char* password = "00001111";
const char* mqtt_server = "192.168.8.10";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

U8X8_SSD1306_128X32_UNIVISION_SW_I2C u8x8(/* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE); 

char msg[50];
char msg_time[50];
unsigned long time_ = 0;

void setup_wifi() {

  delay(100);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  u8x8.setFont(u8x8_font_torussansbold8_r);
  u8x8.drawString(0,0, "WiFi...");

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

  u8x8.drawString(0,0, "WiFi connected");
}

void callback(char* topic, byte* payload, unsigned int length) {

  time_ = millis();
  
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
  int levelPercentage = 100 - ((distance_int_cm - 15) / 80.0 * 100.0);
  levelPercentage = max(min(levelPercentage, 100),0);
  Serial.print("Water Level: ");
  Serial.print(levelPercentage);
  Serial.print("%");
  Serial.println();

  snprintf (msg, 50, "%ld%%", levelPercentage);
  
  u8x8.clear();
  u8x8.setFont(u8x8_font_torussansbold8_r);
  u8x8.draw2x2String(0,0, msg);
}

void setup() {
  Serial.begin(115200);
  u8x8.begin();
  u8x8.setPowerSave(0);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
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

      u8x8.clear();
      u8x8.drawString(0,0, "Waiting...");
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
  
  snprintf (msg_time, 50, "%ld sec ago", (millis() - time_) /  1000);
  u8x8.drawString(0,3, msg_time); 
  
  delay(1000);
}
