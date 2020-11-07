#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid     = "SAYANI_WIFI";
const char* password = "00011101";
const char* mqtt_server = "192.168.8.10";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

unsigned long last_msg_time_ms = 0;

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

void callback(char* topic, byte* payload, unsigned int length) {

  last_msg_time_ms = millis();
  
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
  int level_p = 100 - ((distance_int_cm - 15) / 80.0 * 100.0);
  level_p = max(min(level_p, 100),0);
  Serial.print("Water Level: ");
  Serial.print(level_p);
  Serial.print("%");
  Serial.println();
}

void setup() {
  Serial.begin(115200);
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
  delay(1000);
}
