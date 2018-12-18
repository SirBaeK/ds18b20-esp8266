#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>

#define myPeriodic 4
#define ONE_WIRE_BUS 2
#define MQTT_VERSION MQTT_VERSION_3_1_1
/*sleeping time
const PROGMEM uint16_t SLEEPING_TIME_IN_SECONDS = 600; // 10 minutes x 60 seconds
*/
//IPAddress MQTT_SERVER(192.168.2.14);
const PROGMEM char* MQTT_SERVER = "192.168.11.6"; // mqtt server
const PROGMEM char* MQTT_SERVER_PORT = "1883"; // mqtt server port
const PROGMEM char* MQTT_SENSOR_TOPIC = "wireless/temperature1";
const PROGMEM char* MQTT_CLIENT_ID = "wireless_temperature1";
const PROGMEM char* MQTT_USER = "admin";
const PROGMEM char* MQTT_PASS = "admin";
const char* WIFI_SSID = "artemis-liberec.cz_Kafka 2G"; // Nome SSID Wifi
const char* WIFI_PASSWD = "28749341"; // Password Wifi
int status = WL_IDLE_STATUS;     // the Wifi radio's status
int sent = 1;                   //how many data was sent(count)

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// function called to publish the temperature 
void publishData(float temp, float temp2, float count) {
  // create a JSON object
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  // INFO: the data must be converted into a string; a problem occurs when using floats...
  root["teplota1"] = (String)temp;
  root["teplota2"] = (String)temp2;
  root["pocetMereni"] = (String)count;
  root.prettyPrintTo(Serial);
  char data[200];
  root.printTo(data, root.measureLength() + 1);
  client.publish(MQTT_SENSOR_TOPIC, data, true);
  Serial.println();
  Serial.println("Data odeslana po " + String(sent));
  Serial.println();
}



//try reconnect if drop down mqtt connection 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("INFO: Pokus o MQTT spojeni...");
    // Attempt to connect
    if (client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS)) {
      Serial.println(" pripojeno!");                                       
    } else {
      Serial.print(" ERROR: chyba spojeni, rc=");
      Serial.println(client.state());
      Serial.println();
      Serial.println(" DEBUG: dalsi pokus o pripojeni za 5s ");
      Serial.println(" -~_5_~- ");
      Serial.println(client.state());
      delay(5000);
//      Serial.println(" -~_8_~- ");
//      delay(2000);
//      Serial.println(" -~_6_~- ");
//      delay(2000);
//      Serial.println(" -~_4_~- ");
//      delay(2000);
//      Serial.println(" -~_2_~- ");
//      delay(2000);
//     Serial.println(" .-o_0_o-. ");    
    }
  ;}
}


//START SETUP
void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  Serial.println();
  //set IP address
  Serial.print("INFO: Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);

  // init the WiFi connection
  WiFi.begin(WIFI_SSID, WIFI_PASSWD);
  IPAddress ip(192,168,11,19);  //Node static IP
  IPAddress gateway(192,168,11,254);
  IPAddress subnet(255,255,255,0);
  WiFi.config(ip, gateway, subnet);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("INFO: Pripojeno k WiFi: ");
  Serial.println(WIFI_SSID);
  Serial.print("INFO: IP adresa teplomeru: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  delay(5000);
  // init the MQTT connection
  Serial.print("init the MQTT connection ");
  client.setServer("192.168.11.6",1883);

  client.setCallback(callback);
    delay(5000);  
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

//START LOOP
void loop() {
  if (!client.connected()) {  
    delay(5000);  
    reconnect();
  }
  client.loop();
  Serial.println();
  Serial.println("Sbirani dat pro " + String(sent) + " odeslani");
  float temp;
  DS18B20.requestTemperatures();
  temp = DS18B20.getTempCByIndex(0);
  
  float temp2;
  DS18B20.requestTemperatures();
  temp2= DS18B20.getTempCByIndex(1);

  float count;
  count= float(sent);
  
  if (isnan(temp) || isnan(temp2)) {
    Serial.println("ERROR: Chyba cteni z teplomeru DS18B20 !");
    return;
    } 
   else {
    publishData(temp,temp2,count);
    String(sent++);
    }

  int count2 = myPeriodic;
  while(count2--);
  delay(5000);

  Serial.println("INFO: Odpojuji MQTT spojeni");
  client.disconnect();
     Serial.println();
  Serial.println();

/*Serial.println("INFO: Closing the Wifi connection");
  WiFi.disconnect();

  ESP.deepSleep(SLEEPING_TIME_IN_SECONDS * 1000000, WAKE_RF_DEFAULT);
  delay(500); // wait for deep sleep to happen
*/
    
  }
