// wifi
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Update these with values suitable for your network.
const char *ssid = "Wokwi-GUEST";
const char *password = "";
const char *mqtt_server = "broker.emqx.io";

// wifi
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

// dot matrix
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

// Uncomment according to your hardware type
#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW

// Defining size, and output pins
#define MAX_DEVICES 4
#define CS_PIN 5
#define LDR_PIN 36

// init led matrix
MD_Parola Display = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// init message
String receivedMessage;
String message;

void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  // setting wifi chip sebagai station/client
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  // ?
  randomSeed(micros());

  // logging
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setMessageOnLed()
{

  // config
  StaticJsonDocument<200> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, receivedMessage);
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
  }

  // Extract values
  message = doc["message"].as<String>();
}


// subscribe
void callback(char *topic, byte *payload, unsigned int length)
{

  // print message
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  receivedMessage = "";
  for (int i = 0; i < length; i++)
  {
    receivedMessage += (char)payload[i];
  }
  Serial.println(receivedMessage);

  setMessageOnLed();
}


// reconnect to mqtt
void reconnect()
{

  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection... ");
    Serial.println(mqtt_server);
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str()))
    {
      Serial.println("Connected");
      client.subscribe("tesarptm");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      Serial.println("try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup()
{

  // config
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  Serial.begin(9600);

  // dot led matrix
  pinMode(LDR_PIN, INPUT);

  // begin display
  Display.begin();
  Display.setIntensity(0); // Set the intensity (brightness) of the display (0-15):
  Display.displayClear();
  Display.displayText("Hello world!", PA_CENTER, 70, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
}

void loop()
{

  // reconnect if not connect
  if (!client.connected())
  {
    reconnect();
  }

  // listener
  client.loop();

  // publish every 2000 ms
  // unsigned long now = millis();
  // if (now - lastMsg > 2000)
  // {
  //   lastMsg = now;
  //   ++value;
  //   snprintf(msg, MSG_BUFFER_SIZE, "Indobot #%ld", value);
  //   Serial.print("Publish message: ");
  //   Serial.println(msg);
  //   client.publish("/indobot/p/mqtt", msg);
  // }

  if (Display.displayAnimate() && message != "")
  {
    Display.displayText(message.c_str(), PA_CENTER, 70, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    delay(500);
    Display.displayReset();
  }
}
