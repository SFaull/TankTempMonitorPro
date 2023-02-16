

#include "wireless.h"
#include "config.h"
#include "structs.h"
#include "temperature.h"
#include "display.h"
#include "version.h"

#define MQTT_MAX_PACKET_SIZE 512

#ifdef ENABLE_WIRELESS

  #if defined(ESP8266)
    #include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
  #else
    #include <WiFi.h>          //https://github.com/esp8266/Arduino
  #endif
  #include <DNSServer.h>
  #if defined(ESP8266)
    #include <ESP8266WebServer.h>
  #else
    #include <WebServer.h>
  #endif
  #include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
  #include <PubSubClient.h>
  #include <ArduinoOTA.h>
  #include <WifiUDP.h>

#ifdef MQTT_JSON
  #include <ArduinoJson.h>
#endif

// import the headers
#include <ESPmDNS.h>

#define JSON_PROP_TEMP_TANK_TOP     "temperature_tank_top"
#define JSON_PROP_TEMP_TANK_MID_HI  "temperature_tank_mid_hi"
#define JSON_PROP_TEMP_TANK_MID_LO  "temperature_tank_mid_lo"
#define JSON_PROP_TEMP_TANK_BOTTOM  "temperature_tank_bottom"
#define JSON_PROP_TEMP_TANK_PUMP    "temperature_pump"
#define JSON_PROP_WIFI_RSSI         "wifi_rssi"
#define JSON_PROP_LIGHT_LEVEL       "light_level"


  // General variable declarations
  WiFiClient espClient;
  PubSubClient client(espClient);

  static bool discoveryRequested = false;
  
  /* local prototypes */
  static void wifi_init(void);
  static void wifi_process(void);
  static void mqtt_init(void);
  static void mqtt_process(void);
  static void mqtt_discovery_sensor(String sensorName, String sensorFriendlyName, String sensorClass, String sensorUnit);
  static void mqtt_discovery_all_sensors(void);
  static void mqtt_callback(char* topic, byte* payload, unsigned int length);
  static void OTA_init(void);
  static void OTA_process(void);
  static double roundNumber(double value, unsigned int dp);
  static IPAddress findMDNS(String mDnsHost);
  
  void wireless_init(void)
  {    
    wifi_init();
    OTA_init();
    mqtt_init();
  }
  
  void wireless_process(void)
  {  
    wifi_process();
    OTA_process();
  }

  void wireless_mqtt_discovery(void)
  {
    discoveryRequested = true;
  }

  void wireless_mqtt_publish(void)
  {
    mqtt_process();
  }

  void wireless_info(wifiInfo_t * info)
  {
    WiFiManager wifiManager;
    
    info->connected = (WiFi.status() == WL_CONNECTED);
    // if we are connected to a netowk, populate with that info, otherwise populate with the portal info
    if(info->connected)
    {
      info->ssid = String(WiFi.SSID().c_str());
      info->ip = WiFi.localIP().toString();
    }
    else
    {
      info->ssid = HOSTNAME;  // TODO: find better way to get the ap ssid
      info->ip = WiFi.softAPIP().toString();
    }
  }
  
  static void wifi_init(void)
  {
	  WiFi.setHostname(HOSTNAME);
    WiFi.mode(WIFI_STA); // Force to station mode because if device was switched off while in access point mode it will start up next time
    WiFiManager wifiManager;
    wifiManager.setTimeout(WIFI_TIMEOUT);
    bool connectionSuccess = wifiManager.autoConnect(HOSTNAME);  
    if (!connectionSuccess) 
    {
      Serial.println("Connection failed");
      //ESP.reset(); // This is a bit crude. For some unknown reason webserver can only be started once per boot up 
      // so resetting the device allows to go back into config mode again when it reboots.
      //delay(5000);
    } 
  }
  
  static void wifi_process(void)
  {
      // try connecting to the last access point
    if(WiFi.status() != WL_CONNECTED)
      WiFi.mode(WIFI_STA);
  
    int seconds = 0;
    // dont give up unless no connection for 60 seconds.
    while(WiFi.status() != WL_CONNECTED)
    {
      seconds++;
      delay(1000);
  
      // its been 1 minute, probably not gonna reconnect, lets reset.
      if(seconds > 60)
      {
          ESP.restart(); // This is a bit crude. For some unknown reason webserver can only be started once per boot up 
          // so resetting the device allows to go back into config mode again when it reboots.
          delay(5000);
      }
    }
  }

  String wireless_get_connection_strength()
  {
    int32_t rssi = WiFi.RSSI();
#if 0
    Serial.print("Signal strength");
    Serial.print(rssi);
    Serial.println("dBm");
#endif
    if(rssi < -70)
      return "Weak";
    if(rssi < -60)
      return "Fair";
    if(rssi < -50)
      return "Good";

    return "Excellent";
  }
  
  static void mqtt_init(void)
  {
    IPAddress ip = findMDNS(MQTT_SERVER);
    client.setServer(ip, MQTT_PORT);
    client.setCallback(mqtt_callback);
    client.setBufferSize(512);
  }

  void mqtt_discovery_all_sensors(void)
  {
    mqtt_discovery_sensor(JSON_PROP_TEMP_TANK_TOP, "Tank Top", "temperature", "°C");
    mqtt_discovery_sensor(JSON_PROP_TEMP_TANK_MID_HI, "Tank Mid Upper", "temperature", "°C");
    mqtt_discovery_sensor(JSON_PROP_TEMP_TANK_MID_LO, "Tank Mid Lower", "temperature", "°C");
    mqtt_discovery_sensor(JSON_PROP_TEMP_TANK_BOTTOM, "Tank Bottom", "temperature", "°C");
    mqtt_discovery_sensor(JSON_PROP_TEMP_TANK_PUMP, "Pump", "temperature", "°C");
    mqtt_discovery_sensor(JSON_PROP_WIFI_RSSI, "WiFi RSSI", "signal_strength", "dBm");
    mqtt_discovery_sensor(JSON_PROP_LIGHT_LEVEL, "Light Level", "illuminance", "ADC");
  }

  
  void mqtt_discovery_sensor(String sensorName, String sensorFriendlyName, String sensorClass, String sensorUnit) 
  {
    // This is the discovery topic for this specific sensor
    String discoveryTopic = String(MQTT_DISCOVERY_TOPIC) + "/" + sensorName + "/config";

    StaticJsonDocument<2048> doc; // create a JSON document
    char buffer[512];

    doc["name"] = sensorFriendlyName;
    doc["unique_id"] = String(HOSTNAME) + "_" + sensorName;
    doc["state_topic"] = MQTT_TOPIC("data");
    doc["state_class"] = "measurement";
    doc["unit_of_measurement"] = sensorUnit;
    doc["device_class"] = sensorClass;
    doc["force_update"] = true;
    doc["value_template"] = "{{ value_json." + sensorName + " }}";
    doc["device"]["identifiers"] = HOSTNAME; 
    doc["device"]["name"] = String(DEVICE_MODEL) + "_" + String(HOSTNAME); 
    doc["device"]["manufacturer"] = "SamFaull";
    doc["device"]["sw_version"] = VERSION_STRING; 
    doc["device"]["model"] = DEVICE_MODEL; 

    size_t n = serializeJson(doc, buffer);

    bool result = client.publish(discoveryTopic.c_str(), buffer, n);

    // debug
    //Serial.print("Bytes: ");
    //Serial.println(n);
    Serial.print("Topic: ");
    Serial.println(discoveryTopic.c_str());
    Serial.print("JSON: ");
    serializeJsonPretty(doc, Serial);
    Serial.println();
    Serial.println(result ? "Send: Success" : "Send: Failed");
  }

  
  static void mqtt_process(void)
  {
    /* Check wifi connection */
    if(WiFi.status() != WL_CONNECTED)
    {
      Serial.println("MQTT publish failed, no wifi connection");
      return;
    } 
    
    /* check mqtt connection */
    if (!client.connected())
    {
        Serial.println("No MQTT connection");
        Serial.print("Attempting MQTT connection... ");
        // Attempt to connect
        if (client.connect(HOSTNAME, MQTT_USERNAME, MQTT_PASSWORD))
        {
          Serial.println("Connected");
          client.subscribe(MQTT_TOPIC("cmd"));

          
#ifdef MQTT_JSON
          wifiInfo_t info;
          wireless_info(&info);
          StaticJsonDocument<1024> doc; // create a JSON document
          doc["name"] = DEVICE_NAME;
          doc["description"] = "Water Tank Temperature Monitor";
          doc["version"] = VERSION_STRING;
          doc["core"] = "ESP32";
          doc["ssid"] = info.ssid;
          doc["ip"] = info.ip;
          doc["sensor_type"] = "DS18B20";
          char buffer[512]; // create a character buffer for the JSON serialised stream
          size_t n = serializeJson(doc, buffer);  // serialise the JSON doc
          client.publish(MQTT_TOPIC("meta"), buffer, n);  // pulish the stream
          serializeJsonPretty(doc, Serial);
          Serial.println();
#else
          client.publish(MQTT_TOPIC("meta"), "Connected");
#endif
        }
        else
        {
          Serial.print("failed: ");
          Serial.println(client.state());
          return;
        }
    }

    /* Post MQTT Message */
    if(client.connected())
    {
      if(discoveryRequested)
      {
        mqtt_discovery_all_sensors();
        discoveryRequested = false;
      }

      /* fetch the temperature data */
      bool validReadings = true;
      double temp[5];
      for(int i=0; i<5; i++)
      {
        temp[i] = roundNumber(temperature_get((position_t)i), 2);
        
        if(temp[i] < -54.0 || temp[i] > 126.0)
        {
          validReadings = false;
        }
        
      }

      /* if the data is good, publish it */
      if(validReadings)
      {
#ifdef MQTT_JSON
        StaticJsonDocument<1024> doc; // create a JSON document
        // copy the temparure readings into the JSON object as strings
        doc[JSON_PROP_TEMP_TANK_TOP] = temp[0];
        doc[JSON_PROP_TEMP_TANK_MID_HI] = temp[1];
        doc[JSON_PROP_TEMP_TANK_MID_LO] = temp[2];
        doc[JSON_PROP_TEMP_TANK_BOTTOM] = temp[3];
        doc[JSON_PROP_TEMP_TANK_PUMP] = temp[4];
        doc[JSON_PROP_WIFI_RSSI] = WiFi.RSSI();
        doc[JSON_PROP_LIGHT_LEVEL] = analogRead(LDR_PIN);
        char buffer[512]; // create a character buffer for the JSON serialised stream
        size_t n = serializeJson(doc, buffer);  // serialise the JSON doc
        client.publish(MQTT_TOPIC("data"), buffer, n);  // pulish the stream
        serializeJsonPretty(doc, Serial);
        Serial.println();
#else
        String msg = String(temp[0]) + "," + String(temp[1]) + "," + String(temp[2]) + "," + String(temp[3]) + "," + String(temp[4]);
        
        client.publish(MQTT_TOPIC("RawData"), msg.c_str());
        
        client.publish(MQTT_TOPIC("Sensor1"), String(temp[0]).c_str());
        client.publish(MQTT_TOPIC("Sensor2"), String(temp[1]).c_str());
        client.publish(MQTT_TOPIC("Sensor3"), String(temp[2]).c_str());
        client.publish(MQTT_TOPIC("Sensor4"), String(temp[3]).c_str());
        client.publish(MQTT_TOPIC("Pump"),    String(temp[4]).c_str());
        
        Serial.print("Published: ");
        Serial.println(msg);
#endif
      }
    }
  
    client.loop();
  }
  
  static void mqtt_callback(char* topic, byte* payload, unsigned int length)
  {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
  
    /* --------------- Print incoming message to serial ------------------ */
    char input[length + 1];
    for (int i = 0; i < length; i++)
      input[i] = (char)payload[i];  // store payload as char array
    input[length] = '\0'; // dont forget to add a termination character
  
    Serial.print("MQTT message received: ");
    Serial.println(input);
  
    if (strcmp(topic, MQTT_TOPIC("cmd"))==0)
    {    
      if(strcmp(input,"*IDN?")==0)
      {
        //TODO
        //client.publish(MQTTtopic_comms, "Shed -> Current Monitor");
      }
      if(strcmp(input,"*RST")==0)
      {
        wireless_forget_network();
      }
    }  
  }

  bool upgradeInProgress = false;
  unsigned int upgradeProgress = 0;
  

  bool wirelss_upgrade_in_progress(unsigned int* percent)
  {
    *percent = upgradeProgress;
    return upgradeInProgress; 
  }

  void wireless_OTA_callback_init()
  {
    ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else { // U_SPIFFS
        type = "filesystem";
      }
  
      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
      display_upgrade_start();
      upgradeInProgress = true;
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
      display_upgrade_complete();
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      upgradeProgress = progress / (total / 100);
      Serial.printf("Progress: %u%%\r", upgradeProgress);
      display_upgrade_progress(upgradeProgress);
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      upgradeProgress = 0;
      upgradeInProgress = false;
      display_upgrade_error();
      if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
      }
    });
  }

  void wireless_forget_network(void)
  {
    esp_wifi_restore();
    delay(1000);
    ESP.restart();
  }


  static void OTA_init(void)
  {
    ArduinoOTA.setHostname(HOSTNAME);
    
    ArduinoOTA.begin();
    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  
  static void OTA_process(void)
  {
    ArduinoOTA.handle();
  }

  // rounds a number to x decimal places
  // example: round(3.14159, 2) -> 3.14
double roundNumber(double value, unsigned int dp) {
   return (int)(value * (10*dp) + 0.5) / (10.0 * (float)dp) ;
}


// on my laptop (Ubuntu) the equivalent command is: `avahi-resolve-host-name -4 mqtt-broker.local`
IPAddress findMDNS(String mDnsHost) { 
  // the input mDnsHost is e.g. "mqtt-broker" from "mqtt-broker.local"
  Serial.println("Finding the mDNS details...");
  // Need to make sure that we're connected to the wifi first
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  if (!MDNS.begin(HOSTNAME)) {
    Serial.println("Error setting up MDNS responder!");
  } else {
    Serial.println("Finished intitializing the MDNS client...");
  }

  Serial.println("mDNS responder started");
  IPAddress serverIp = MDNS.queryHost(mDnsHost);
  while (serverIp.toString() == "0.0.0.0") {
    Serial.println("Trying again to resolve mDNS");
    delay(250);
    serverIp = MDNS.queryHost(mDnsHost);
  }
  Serial.print("IP address of server: ");
  Serial.println(serverIp.toString());
  Serial.println("Done finding the mDNS details...");
  return serverIp;
}

#endif
