#include "wireless.h"
#ifdef ENABLE_WIRELESS
  #include <WiFiManager.h>
  #include <ESP8266WiFi.h>
  #include <PubSubClient.h>
  #include <ArduinoOTA.h>
  #include <WifiUDP.h>
  
  // General variable declarations
  WiFiClient espClient;
  PubSubClient client(espClient);
  
  /* local prototypes */
  static void wifi_init(void);
  static void wifi_process(void);
  static void mqtt_init(void);
  static void mqtt_process(void);
  static void mqtt_callback(char* topic, byte* payload, unsigned int length);
  static void OTA_init(void);
  static void OTA_process(void);
  
  void wireless_init(void)
  {
    OTA_init();
    wifi_init();
    mqtt_init();
  }
  
  void wireless_process(void)
  {  
    wifi_process();
    mqtt_process();
    OTA_process();
  }
  
  static void wifi_init(void)
  {
    WiFi.mode(WIFI_STA); // Force to station mode because if device was switched off while in access point mode it will start up next time
    WiFiManager wifiManager;
    wifiManager.setTimeout(WIFI_TIMEOUT);
    bool connectionSuccess = wifiManager.autoConnect(DEVICE_NAME);  
    if (!connectionSuccess) 
    {
      Serial.println("Connection failed... restarting");
      ESP.reset(); // This is a bit crude. For some unknown reason webserver can only be started once per boot up 
      // so resetting the device allows to go back into config mode again when it reboots.
      delay(5000);
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
          ESP.reset(); // This is a bit crude. For some unknown reason webserver can only be started once per boot up 
          // so resetting the device allows to go back into config mode again when it reboots.
          delay(5000);
      }
    }
  }
  
  static void mqtt_init(void)
  {
    client.setServer(MQTT_SERVER, MQTT_PORT);
    client.setCallback(mqtt_callback);
  }
  
  static void mqtt_process(void)
  {
    
    // check mqtt connection (TODO should have some timout on this.)
    if (!client.connected())
    {
      // Loop until we're reconnected
      while (!client.connected())
      {
        Serial.print("Attempting MQTT connection... ");
        // Attempt to connect
        if (client.connect(DEVICE_NAME, MQTT_USERNAME, MQTT_PASSWORD))
        {
          Serial.println("Connected");
          client.publish(MQTT_ROOM, "Connected");
          client.subscribe(MQTT_COMMS);
        }
        else
        {
          Serial.print("failed, rc=");
          Serial.print(client.state());
          Serial.print(" try again in 5 seconds");
          ledController.setColour(WS2812_BRIGHTNESS,0,0);
          // Wait 5 seconds before retrying and flash LED red
          delay(3000);
          ledController.setColour(0,0,0);
          delay(2000);
        }
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
  
    if (strcmp(topic, MQTT_COMMS)==0)
    {    
      if(strcmp(input,"*IDN?")==0)
      {
        //TODO
        //client.publish(MQTTtopic_comms, "Shed -> Current Monitor");
      }
      if(strcmp(input,"*RST")==0)
      {
        // give info
        ledController.setColour(WS2812_BRIGHTNESS,0,0);
        client.disconnect();
        WiFiManager wifiManager;
        wifiManager.resetSettings();
        wifiManager.autoConnect(DEVICE_NAME);
        //ESP.restart();
      }
    }  
  }
  
  static void OTA_init(void)
  {
    ArduinoOTA.onStart([]() {
      Serial.println("OTA Update Started");
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nOTA Update Complete");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();
  }
  
  static void OTA_process(void)
  {
    ArduinoOTA.handle();
  }
#endif