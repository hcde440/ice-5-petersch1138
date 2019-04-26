
#include "config.h";

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <SPI.h>
#include <ArduinoJson.h>    //
char mac[18]; //A MAC address is a 'truly' unique ID for each device, lets use that as our 'truly' unique user ID!!!

#include <ESP8266WiFi.h>    //Requisite Libraries . . .
#include "Wire.h"           
#include <PubSubClient.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
WiFiClient espClient;             // espClient object
PubSubClient mqtt(espClient);  // puubsubclient object
#define OLED_RESET     -1 // Reset pin # (-1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // OLED display object

String temp;
String hum; // variables for incoming data
String pres;

// Peter Schultz 4/25/19
// This code gets values for temperature humidity and pressure from an MQTT server, the data is sent from a publisher using 2 sensors.
// It also displays the values on a local OLED display.

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(100);

  Serial.print("This board is running: ");
  Serial.println(F(__FILE__));
  Serial.print("Compiled: ");
  Serial.println(F(__DATE__ " " __TIME__));
  
  setup_wifi(); // call this to set up wifi
  mqtt.setServer(mqtt_server, 1883); // sets mqtt object server address
  mqtt.setCallback(callback); //register the callback function
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();
}

void loop() {
  // put your main code here, to run repeatedly:

    if (!mqtt.connected()) {
      reconnect(); // run reconnect method if connection stops
    }
    mqtt.loop(); //this keeps the mqtt connection 'active'
  
    
}

void callback(char* topic, byte* payload, unsigned int length) { // this method is called when an incoming message from a topic you are subscribed to is detected
  Serial.println();
  Serial.print("Message arrived [");
  Serial.print(topic); //'topic' refers to the incoming topic name, the 1st argument of the callback function
  Serial.println("] ");

  DynamicJsonBuffer  jsonBuffer; //creates json buffer
  JsonObject& root = jsonBuffer.parseObject(payload); //parse it!

  if (!root.success()) { 
    Serial.println("parseObject() failed, are you sure this message is JSON formatted.");
    return;
  }

  /////
  //We can use strcmp() -- string compare -- to check the incoming topic in case we need to do something
  //special based upon the incoming topic, like move a servo or turn on a light . . .
  //strcmp(firstString, secondString) == 0 <-- '0' means NO differences, they are ==
  /////

  if (strcmp(topic, "Rodrigo/ICE5") == 0) {
    Serial.println("A message from Rodrigo . . .");
  }

  root.printTo(Serial); //print out the parsed message
  Serial.println(); //give us some space on the serial monitor read out
  pres = root["pressure"].as<String>(); // these lines get the data from the json format payload
  
  temp = root["temperature"].as<String>();
  
  hum = root["humidity"].as<String>();
  Serial.print("hum: ");
  Serial.println(hum);
  Serial.print("temp: ");
  Serial.println(temp);
  Serial.print("pres: ");
  Serial.println(pres);

  display.clearDisplay(); // clears display
  display.setCursor(0, 0);
  display.setTextSize(1); // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.print("Temperature: ");
  display.println(temp); // writes the values for temp and humidity and pressure to the OLED display
  display.print("Humidity: ");
  display.println(hum);
  display.print("Pressure: ");
  display.println(pres);
  display.display();
  delay(3000);
}

////CONNECT/RECONNECT/////Monitor the connection to MQTT server, if down, reconnect, usees credentials in config.h
void reconnect() {
  // Loop until we're reconnected
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqtt.connect(mac, mqtt_user, mqtt_password)) { //<<---using MAC as client ID, always unique!!!
      Serial.println("connected");
      mqtt.subscribe("Rodrigo/+"); //we are subscribing to 'fromPeter' and all subtopics below that topic
    } else {                        //please change 'fromPeter' to reflect your topic you are subscribing to
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void setup_wifi() { // this method connects to the wifi using the variables contained in config.h
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");  //get the unique MAC address to use as MQTT client ID, a 'truly' unique ID.
  WiFi.macAddress().toCharArray(mac, 18);
  Serial.println(mac);  //.macAddress returns a byte array 6 bytes representing the MAC address
}           
