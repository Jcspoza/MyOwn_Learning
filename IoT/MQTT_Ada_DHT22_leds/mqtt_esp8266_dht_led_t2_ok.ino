/***************************************************
 * Primer intento de MQTT propio con temperatura humedad y un led simulando un rele
 * JCSP
 * 29-3-2017
 * Resultado :
 * Conexion de DHT12, AM2302
 * vista frente con serigrafia identificativa
 * 1ro Izquierda "+"= 3,3 v
 * 2do izquierd "data"  GPIO02 --> D4 + PULL-UP 10K A 3.3
 * 3ro No conectado
 * Derecha "-" = GND
 * 
 * LED Rojo en pin D7 , GPIO13
 * LED yellow en pin D6 GPIO12
 * con resitencia de 120 ohm en circuito con 3.3 v
 ****************************************************/
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <DHT.h>
#include <Wire.h>

/************************* LED PINES *********************************/
int ledRedPin = 13; // GPIO13 = D7, con resistencia de 100 ohm
int ledYelslidPin = 12; // GPIO13 = D6, con resistencia de 100 ohm, sera PMW, con un feed slider

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "belkin.75e"
#define WLAN_PASS       "........."

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "jcspoza"
#define AIO_KEY         "........"

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/***************************** Feeds ******************************
 * Hay que  crear 4 feed : Temperatura, Humedad, ledred y ledyel este ultimo PMW
 * 
 */

// Setup a feed called 'temperatura' y 'humedad' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish temperatura = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperatura");
Adafruit_MQTT_Publish humedad = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humedad");

// Setup a feed called 'ledred' y 'ledyelslid'for subscribing to changes.
Adafruit_MQTT_Subscribe ledred = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/ledred");
Adafruit_MQTT_Subscribe ledyelslid = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/ledyelslid");
/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

/*************************** DHT ************************************/
#define DHTPIN 2     // what digital pin we're connected to.. connect D4 pin of Nodemcu
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22 // DHT 22 (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  pinMode(ledRedPin, OUTPUT);
  pinMode(ledYelslidPin, OUTPUT);
  Serial.begin(115200);
  delay(10);

  Serial.println(F("Adafruit MQTT demo 2 pub + 2 sub"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // Setup MQTT subscription for ledRedPin & ledYelslidPin slider feed.
  mqtt.subscribe(&ledred);
  mqtt.subscribe(&ledyelslid);
}

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
 
  MQTT_connect();

  // LEER Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // Read temperature as Celsius (the default)
  if (isnan(h) || isnan(t)) {  // Check if any reads failed and exit early (to try again).
    Serial.println("Error leer DHT!");
    return;
  }

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here

   Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    // Check if its the ledred button feed
    if (subscription == &ledred) {
      Serial.print(F("Led RED On-Off button: "));
      Serial.println((char *)ledred.lastread);
      
      if (strcmp((char *)ledred.lastread, "ON") == 0) {
        digitalWrite(ledRedPin, HIGH); 
      }
      if (strcmp((char *)ledred.lastread, "OFF") == 0) {
        digitalWrite(ledRedPin, LOW); 
      }
    }
    
    // check if its the slider feed ledyelslid
    if (subscription == &ledyelslid) {
      Serial.print(F("Led yellow Slider: "));
      Serial.println((char *)ledyelslid.lastread);
      uint16_t sliderval = atoi((char *)ledyelslid.lastread);  // convert to a number
      analogWrite(ledYelslidPin, sliderval);
    }
  }
  // Now we can publish stuff!
  Serial.print(F("\nSending Temperature val "));
  Serial.print(t);
  Serial.print("...");
  if (! temperatura.publish(t)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
Serial.print(F("\nSending Humidity val "));
  Serial.print(h);
  Serial.print("...");
  if (! humedad.publish(h)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  /*
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  */
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
