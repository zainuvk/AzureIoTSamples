#include <ESP8266WiFi.h>
#include <AzureIoTHub.h>
#include <AzureIoTProtocol_MQTT.h>
#include <AzureIoTUtility.h>
#include <ArduinoJson.h>
#include "dht_temperature_AzureIotHub_config.h"
#include "DHT.h"


#define DHTPIN D1 // what digital pin we're connected to
#define DHTTYPE DHT11 // DHT 11
DHT dht(DHTPIN, DHTTYPE);

const char* ssid = IOT_CONFIG_WIFI_SSID;
const char* password= IOT_CONFIG_WIFI_PASSWORD;

static bool messagePending = false;
static bool messageSending = true;


static int interval = INTERVAL;
static IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;
static const char* connectionString = IOT_CONFIG_CONNECTION_STRING;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  dht.begin();
  
 //Connect wifi
  initWiFi();
  initTime();

  //Create AureIOtHubConnectionString
  iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(connectionString, MQTT_Protocol);
  if (iotHubClientHandle == NULL)
  {
        Serial.println("Failed on IoTHubClient_CreateFromConnectionString.");
        while (1);
   }
   else // added else block
   {       
      Serial.println(" IoTHubClient_CreateFromConnectionString Ceated");    
   }  
   
}

static int messageCount = 1;

void loop() {
  // put your main code here, to run repeatedly:
  delay(2000);
    
  if (!messagePending && messageSending)
  {
      char messagePayload[MESSAGE_MAX_LEN];
      readMessage(messageCount, messagePayload);
      sendMessage(iotHubClientHandle, messagePayload);
      messageCount++;
      delay(interval);
    }
    IoTHubClient_LL_DoWork(iotHubClientHandle);
    delay(10);   
}

void initWiFi()
{
   Serial.print("Connecting to ");
   Serial.println(ssid);
   WiFi.begin(ssid,password);
   while (WiFi.status() != WL_CONNECTED)
   {
    delay(500);
    Serial.print(".");
   } 

    if(WiFi.status() == WL_CONNECTED)
    {
     Serial.println("");
     Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    }
}

void readMessage(int messageId, char *payload)
{
  // get humidity reading
  float h = dht.readHumidity();
  // get temperature reading in Celsius
  float t = dht.readTemperature();
  // get temperature reading in Fahrenheit
  //float f = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t))
  {
      Serial.println("Failed to read from DHT sensor!");
  return;
  }
 
   StaticJsonBuffer<MESSAGE_MAX_LEN> jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["deviceId"] = DEVICE_ID;
    root["messageId"] = messageId;
   // NAN is not the valid json, change it to NULL
    if (std::isnan(t))
    {
        root["temperatureC"] = NULL;
    }
    else
    {
        root["temperatureC"] = t;
      
    }

    if (std::isnan(h))
    {
        root["humidity"] = NULL;
    }
    else
    {
        root["humidity"] = h;
    }
     
    root.printTo(payload, MESSAGE_MAX_LEN);
   
}

void initTime()
{
    time_t epochTime;
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    while (true)
    {
        epochTime = time(NULL);

        if (epochTime == 0)
        {
            Serial.println("Fetching NTP epoch time failed! Waiting 2 seconds to retry.");
            delay(2000);
        }
        else
        {
            Serial.printf("Fetched NTP epoch time is: %lu.\r\n", epochTime);
            break;
        }
    }
}

