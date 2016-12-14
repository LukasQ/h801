//
// Alternative firmware for H801 5 channel LED dimmer
// based on https://github.com/mertenats/open-home-automation/blob/master/ha_mqtt_rgb_light/ha_mqtt_rgb_light.ino
// converted do 5 single channel controller to use independent white bulbs
//
#include <string>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>   // Local WebServer used to serve the configuration portal
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiManager.h>        // WiFi Configuration Magic
#include <PubSubClient.h>       // MQTT client
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define DEVELOPMENT

WiFiManager wifiManager;
WiFiClient wifiClient;
PubSubClient client(wifiClient);
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

const char* mqtt_server = "172.24.166.136";
// Password for update server
const char* username = "";
const char* password = "";

// MQTT topics
// state, brightness
const char* MQTT_UP = "active";

char* MQTT_LIGHT_W1_STATE_TOPIC = "XXXXXXXX/w1/light/status";
char* MQTT_LIGHT_W1_COMMAND_TOPIC = "XXXXXXXX/w1/light/switch";
char* MQTT_LIGHT_W1_BRIGHTNESS_STATE_TOPIC = "XXXXXXXX/w1/brightness/status";
char* MQTT_LIGHT_W1_BRIGHTNESS_COMMAND_TOPIC = "XXXXXXXX/w1/brightness/set";

char* MQTT_LIGHT_W2_STATE_TOPIC = "XXXXXXXX/w2/light/status";
char* MQTT_LIGHT_W2_COMMAND_TOPIC = "XXXXXXXX/w2/light/switch";
char* MQTT_LIGHT_W2_BRIGHTNESS_STATE_TOPIC = "XXXXXXXX/w2/brightness/status";
char* MQTT_LIGHT_W2_BRIGHTNESS_COMMAND_TOPIC = "XXXXXXXX/w2/brightness/set";

char* MQTT_LIGHT_W3_STATE_TOPIC = "XXXXXXXX/w3/light/status";
char* MQTT_LIGHT_W3_COMMAND_TOPIC = "XXXXXXXX/w3/light/switch";
char* MQTT_LIGHT_W3_BRIGHTNESS_STATE_TOPIC = "XXXXXXXX/w3/brightness/status";
char* MQTT_LIGHT_W3_BRIGHTNESS_COMMAND_TOPIC = "XXXXXXXX/w3/brightness/set";

char* MQTT_LIGHT_W4_STATE_TOPIC = "XXXXXXXX/w4/light/status";
char* MQTT_LIGHT_W4_COMMAND_TOPIC = "XXXXXXXX/w4/light/switch";
char* MQTT_LIGHT_W4_BRIGHTNESS_STATE_TOPIC = "XXXXXXXX/w4/brightness/status";
char* MQTT_LIGHT_W4_BRIGHTNESS_COMMAND_TOPIC = "XXXXXXXX/w4/brightness/set";

char* MQTT_LIGHT_W5_STATE_TOPIC = "XXXXXXXX/w5/light/status";
char* MQTT_LIGHT_W5_COMMAND_TOPIC = "XXXXXXXX/w5/light/switch";
char* MQTT_LIGHT_W5_BRIGHTNESS_STATE_TOPIC = "XXXXXXXX/w5/brightness/status";
char* MQTT_LIGHT_W5_BRIGHTNESS_COMMAND_TOPIC = "XXXXXXXX/w5/brightness/set";


char* chip_id = "00000000";
char* myhostname = "esp00000000";

// buffer used to send/receive data with MQTT
const uint8_t MSG_BUFFER_SIZE = 20;
char m_msg_buffer[MSG_BUFFER_SIZE];


// Light
// the payload that represents enabled/disabled state, by default
const char* LIGHT_ON = "ON";
const char* LIGHT_OFF = "OFF";


#define W3_PIN  15 //RED
#define W4_PIN  13 //GREEN
#define W5_PIN  12 //BLUE
#define W1_PIN  14
#define W2_PIN  4

#define GREEN_PIN    1
#define RED_PIN      5

// store the state of the rgb light (colors, brightness, ...)
boolean m_w1_state = false;
uint8_t m_w1_brightness = 255;

boolean m_w2_state = false;
uint8_t m_w2_brightness = 255;

boolean m_w3_state = false;
uint8_t m_w3_brightness = 255;

boolean m_w4_state = false;
uint8_t m_w4_brightness = 255;

boolean m_w5_state = false;
uint8_t m_w5_brightness = 255;

void setup()
{
  pinMode(W1_PIN, OUTPUT);
  setW1(0);
  pinMode(W2_PIN, OUTPUT);
  setW2(0);
  pinMode(W3_PIN, OUTPUT);
  setW3(0);
  pinMode(W4_PIN, OUTPUT);
  setW4(0);
  pinMode(W5_PIN, OUTPUT);
  setW5(0);

  pinMode(GREEN_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  digitalWrite(RED_PIN, 0);
  digitalWrite(GREEN_PIN, 1);

  sprintf(chip_id, "%08X", ESP.getChipId());
  sprintf(myhostname, "esp%08X", ESP.getChipId());

  // Setup console
  Serial1.begin(115200);
  delay(10);
  Serial1.println();
  Serial1.println();

  // reset if necessary
  // wifiManager.resetSettings();

  wifiManager.setTimeout(3600);
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  wifiManager.addParameter(&custom_mqtt_server);
  WiFiManagerParameter custom_password("password", "password for updates", password, 40);
  wifiManager.addParameter(&custom_password);
  wifiManager.setCustomHeadElement(chip_id);
  wifiManager.autoConnect();

  mqtt_server = custom_mqtt_server.getValue();
  password = custom_password.getValue();

  Serial1.println("");

  Serial1.println("WiFi connected");
  Serial1.println("IP address: ");
  Serial1.println(WiFi.localIP());

  Serial1.println("");

  // init the MQTT connection
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // replace chip ID in channel names
  memcpy(MQTT_LIGHT_W1_STATE_TOPIC, chip_id, 8);
  memcpy(MQTT_LIGHT_W1_COMMAND_TOPIC, chip_id, 8);
  memcpy(MQTT_LIGHT_W1_BRIGHTNESS_STATE_TOPIC, chip_id, 8);
  memcpy(MQTT_LIGHT_W1_BRIGHTNESS_COMMAND_TOPIC, chip_id, 8);

  memcpy(MQTT_LIGHT_W2_STATE_TOPIC, chip_id, 8);
  memcpy(MQTT_LIGHT_W2_COMMAND_TOPIC, chip_id, 8);
  memcpy(MQTT_LIGHT_W2_BRIGHTNESS_STATE_TOPIC, chip_id, 8);
  memcpy(MQTT_LIGHT_W2_BRIGHTNESS_COMMAND_TOPIC, chip_id, 8);

  memcpy(MQTT_LIGHT_W3_STATE_TOPIC, chip_id, 8);
  memcpy(MQTT_LIGHT_W3_COMMAND_TOPIC, chip_id, 8);
  memcpy(MQTT_LIGHT_W3_BRIGHTNESS_STATE_TOPIC, chip_id, 8);
  memcpy(MQTT_LIGHT_W3_BRIGHTNESS_COMMAND_TOPIC, chip_id, 8);

  memcpy(MQTT_LIGHT_W4_STATE_TOPIC, chip_id, 8);
  memcpy(MQTT_LIGHT_W4_COMMAND_TOPIC, chip_id, 8);
  memcpy(MQTT_LIGHT_W4_BRIGHTNESS_STATE_TOPIC, chip_id, 8);
  memcpy(MQTT_LIGHT_W4_BRIGHTNESS_COMMAND_TOPIC, chip_id, 8);

  memcpy(MQTT_LIGHT_W5_STATE_TOPIC, chip_id, 8);
  memcpy(MQTT_LIGHT_W5_COMMAND_TOPIC, chip_id, 8);
  memcpy(MQTT_LIGHT_W5_BRIGHTNESS_STATE_TOPIC, chip_id, 8);
  memcpy(MQTT_LIGHT_W5_BRIGHTNESS_COMMAND_TOPIC, chip_id, 8);

  digitalWrite(RED_PIN, 1);

  // OTA
  // do not start OTA server if no password has been set
  if (password != "") {
    MDNS.begin(myhostname);
    httpUpdater.setup(&httpServer, username, password);
    httpServer.begin();
    MDNS.addService("http", "tcp", 80);
  }
}

void setW1(uint8_t brightness) {
  // convert the brightness in % (0-100%) into a digital value (0-255)
  analogWrite(W1_PIN, brightness);
}

void setW2(uint8_t brightness) {
  // convert the brightness in % (0-100%) into a digital value (0-255)
  analogWrite(W2_PIN, brightness);
}

void setW3(uint8_t brightness) {
  // convert the brightness in % (0-100%) into a digital value (0-255)
  analogWrite(W3_PIN, brightness);
}

void setW4(uint8_t brightness) {
  // convert the brightness in % (0-100%) into a digital value (0-255)
  analogWrite(W4_PIN, brightness);
}

void setW5(uint8_t brightness) {
  // convert the brightness in % (0-100%) into a digital value (0-255)
  analogWrite(W5_PIN, brightness);
}

void publishW1State() {
  if (m_w1_state) {
    client.publish(MQTT_LIGHT_W1_STATE_TOPIC, LIGHT_ON, true);
  } else {
    client.publish(MQTT_LIGHT_W1_STATE_TOPIC, LIGHT_OFF, true);
  }
}

void publishW2State() {
  if (m_w2_state) {
    client.publish(MQTT_LIGHT_W2_STATE_TOPIC, LIGHT_ON, true);
  } else {
    client.publish(MQTT_LIGHT_W2_STATE_TOPIC, LIGHT_OFF, true);
  }
}

void publishW3State() {
  if (m_w3_state) {
    client.publish(MQTT_LIGHT_W3_STATE_TOPIC, LIGHT_ON, true);
  } else {
    client.publish(MQTT_LIGHT_W3_STATE_TOPIC, LIGHT_OFF, true);
  }
}

void publishW4State() {
  if (m_w4_state) {
    client.publish(MQTT_LIGHT_W4_STATE_TOPIC, LIGHT_ON, true);
  } else {
    client.publish(MQTT_LIGHT_W4_STATE_TOPIC, LIGHT_OFF, true);
  }
}

void publishW5State() {
  if (m_w5_state) {
    client.publish(MQTT_LIGHT_W5_STATE_TOPIC, LIGHT_ON, true);
  } else {
    client.publish(MQTT_LIGHT_W5_STATE_TOPIC, LIGHT_OFF, true);
  }
}

void publishW1Brightness() {
  snprintf(m_msg_buffer, MSG_BUFFER_SIZE, "%d", m_w1_brightness);
  client.publish(MQTT_LIGHT_W1_BRIGHTNESS_STATE_TOPIC, m_msg_buffer, true);
}

void publishW2Brightness() {
  snprintf(m_msg_buffer, MSG_BUFFER_SIZE, "%d", m_w2_brightness);
  client.publish(MQTT_LIGHT_W2_BRIGHTNESS_STATE_TOPIC, m_msg_buffer, true);
}

void publishW3Brightness() {
  snprintf(m_msg_buffer, MSG_BUFFER_SIZE, "%d", m_w3_brightness);
  client.publish(MQTT_LIGHT_W3_BRIGHTNESS_STATE_TOPIC, m_msg_buffer, true);
}

void publishW4Brightness() {
  snprintf(m_msg_buffer, MSG_BUFFER_SIZE, "%d", m_w4_brightness);
  client.publish(MQTT_LIGHT_W4_BRIGHTNESS_STATE_TOPIC, m_msg_buffer, true);
}

void publishW5Brightness() {
  snprintf(m_msg_buffer, MSG_BUFFER_SIZE, "%d", m_w5_brightness);
  client.publish(MQTT_LIGHT_W5_BRIGHTNESS_STATE_TOPIC, m_msg_buffer, true);
}


// function called when a MQTT message arrived
void callback(char* p_topic, byte* p_payload, unsigned int p_length) {
  // concat the payload into a string
  String payload;
  for (uint8_t i = 0; i < p_length; i++) {
    payload.concat((char)p_payload[i]);
  }

  /* hier weitern machen */


  // handle message topic
  if (String(MQTT_LIGHT_W1_COMMAND_TOPIC).equals(p_topic)) {
    // test if the payload is equal to "ON" or "OFF"
    if (payload.equals(String(LIGHT_ON))) {
      m_w1_state = true;
      setW1(m_w1_brightness);
      publishW1State();
    } else if (payload.equals(String(LIGHT_OFF))) {
      m_w1_state = false;
      setW1(0);
      publishW1State();
    }
  }  else if (String(MQTT_LIGHT_W2_COMMAND_TOPIC).equals(p_topic)) {
    // test if the payload is equal to "ON" or "OFF"
    if (payload.equals(String(LIGHT_ON))) {
      m_w2_state = true;
      setW2(m_w2_brightness);
      publishW2State();
    } else if (payload.equals(String(LIGHT_OFF))) {
      m_w2_state = false;
      setW2(0);
      publishW2State();
    }
  }  else if (String(MQTT_LIGHT_W3_COMMAND_TOPIC).equals(p_topic)) {
    // test if the payload is equal to "ON" or "OFF"
    if (payload.equals(String(LIGHT_ON))) {
      m_w3_state = true;
      setW3(m_w3_brightness);
      publishW3State();
    } else if (payload.equals(String(LIGHT_OFF))) {
      m_w3_state = false;
      setW3(0);
      publishW3State();
    }
  }  else if (String(MQTT_LIGHT_W4_COMMAND_TOPIC).equals(p_topic)) {
    // test if the payload is equal to "ON" or "OFF"
    if (payload.equals(String(LIGHT_ON))) {
      m_w4_state = true;
      setW4(m_w4_brightness);
      publishW4State();
    } else if (payload.equals(String(LIGHT_OFF))) {
      m_w4_state = false;
      setW4(0);
      publishW4State();
    }
  }  else if (String(MQTT_LIGHT_W5_COMMAND_TOPIC).equals(p_topic)) {
    // test if the payload is equal to "ON" or "OFF"
    if (payload.equals(String(LIGHT_ON))) {
      m_w5_state = true;
      setW5(m_w5_brightness);
      publishW5State();
    } else if (payload.equals(String(LIGHT_OFF))) {
      m_w5_state = false;
      setW5(0);
      publishW5State();
    }
  } else if (String(MQTT_LIGHT_W1_BRIGHTNESS_COMMAND_TOPIC).equals(p_topic)) {
    uint8_t brightness = payload.toInt();
    if (brightness < 0 || brightness > 255) {
      // do nothing...
      return;
    } else {
      m_w1_brightness = brightness;
      setW1(m_w1_brightness);
      publishW1Brightness();
    }
  } else if (String(MQTT_LIGHT_W2_BRIGHTNESS_COMMAND_TOPIC).equals(p_topic)) {
    uint8_t brightness = payload.toInt();
    if (brightness < 0 || brightness > 255) {
      // do nothing...
      return;
    } else {
      m_w2_brightness = brightness;
      setW2(m_w2_brightness);
      publishW2Brightness();
    }
  } else if (String(MQTT_LIGHT_W3_BRIGHTNESS_COMMAND_TOPIC).equals(p_topic)) {
    uint8_t brightness = payload.toInt();
    if (brightness < 0 || brightness > 255) {
      // do nothing...
      return;
    } else {
      m_w3_brightness = brightness;
      setW3(m_w3_brightness);
      publishW3Brightness();
    }
  } else if (String(MQTT_LIGHT_W4_BRIGHTNESS_COMMAND_TOPIC).equals(p_topic)) {
    uint8_t brightness = payload.toInt();
    if (brightness < 0 || brightness > 255) {
      // do nothing...
      return;
    } else {
      m_w4_brightness = brightness;
      setW4(m_w4_brightness);
      publishW4Brightness();
    }
  } else if (String(MQTT_LIGHT_W5_BRIGHTNESS_COMMAND_TOPIC).equals(p_topic)) {
    uint8_t brightness = payload.toInt();
    if (brightness < 0 || brightness > 255) {
      // do nothing...
      return;
    } else {
      m_w5_brightness = brightness;
      setW5(m_w5_brightness);
      publishW5Brightness();
    }
  }


  digitalWrite(GREEN_PIN, 0);
  delay(1);
  digitalWrite(GREEN_PIN, 1);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(chip_id)) {
      Serial.println("connected");

      client.publish(MQTT_UP, chip_id);

      // Once connected, publish an announcement...
      // publish the initial values
      publishW1State();
      publishW1Brightness();

      publishW2State();
      publishW2Brightness();

      publishW3State();
      publishW3Brightness();

      publishW4State();
      publishW4Brightness();

      publishW5State();
      publishW5Brightness();


      // ... and resubscribe
      client.subscribe(MQTT_LIGHT_W1_COMMAND_TOPIC);
      client.subscribe(MQTT_LIGHT_W1_BRIGHTNESS_COMMAND_TOPIC);

      client.subscribe(MQTT_LIGHT_W2_COMMAND_TOPIC);
      client.subscribe(MQTT_LIGHT_W2_BRIGHTNESS_COMMAND_TOPIC);

      client.subscribe(MQTT_LIGHT_W3_COMMAND_TOPIC);
      client.subscribe(MQTT_LIGHT_W3_BRIGHTNESS_COMMAND_TOPIC);

      client.subscribe(MQTT_LIGHT_W3_COMMAND_TOPIC);
      client.subscribe(MQTT_LIGHT_W3_BRIGHTNESS_COMMAND_TOPIC);

      client.subscribe(MQTT_LIGHT_W4_COMMAND_TOPIC);
      client.subscribe(MQTT_LIGHT_W4_BRIGHTNESS_COMMAND_TOPIC);


    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

uint16_t i = 0;

void loop()
{
  // process OTA updates
  httpServer.handleClient();

  i++;
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Post the full status to MQTT every 65535 cycles. This is roughly once a minute
  // this isn't exact, but it doesn't have to be. Usually, clients will store the value
  // internally. This is only used if a client starts up again and did not receive
  // previous messages
  delay(1);
  if (i == 0) {
    publishW1State();
    publishW1Brightness();

    publishW2State();
    publishW2Brightness();

    publishW3State();
    publishW3Brightness();

    publishW4State();
    publishW4Brightness();

    publishW5State();
    publishW5Brightness();
  }
}
