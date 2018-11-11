#include <WiFi.h>
#include <AzureIoTHub.h>
#include <AzureIoTProtocol_MQTT.h>
#include <AzureIoTUtility.h>
#include <ArduinoJson.h>
#include "DHT.h"
#define DEVICE_ID "NodeMCUDevice"
#define MESSAGE_MAX_LEN 256
#define DHTPIN 2 // what digital pin we're connected to
#define DHTTYPE DHT11 // DHT 11
DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "XXXX";
const char* password= "XXXXXX";
const char* connectionString="HostName=XXX.azure-devices.net;DeviceId=XXXX;SharedAccessKey=XXXX";
const char* DeviceId="XXXX";
const int Messagemaxlen= 256;
const int interval= 2000;


static IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;

static int messageCount = 1;

static bool messagePending = false;
static bool messageSending = true;

const char *onSuccess = "\"Successfully invoke device method\"";
const char *notFound = "\"No method found\"";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  dht.begin();
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

     //Testing for direct method 
   IoTHubClient_LL_SetDeviceMethodCallback(iotHubClientHandle, deviceMethodCallback, NULL);

}

void loop() {
  // put your main code here, to run repeatedly:

   // put your main code here, to run repeatedly:
   delay(2000);
    
 if (!messagePending && messageSending)
  {
     char messagePayload[Messagemaxlen];
    // createMessage(messageCount, messagePayload);
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
  if (std::isnan(h) || std::isnan(t))
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



static void sendCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void *userContextCallback)
{
    if (IOTHUB_CLIENT_CONFIRMATION_OK == result)
    {
      
        Serial.println("Message sent to Azure IoT Hub.");
        
       }
    else
    {
        
         Serial.println("Failed to send message to Azure IoT Hub.");
    }
    messagePending = false;
}
static void sendMessage(IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle, char *buffer)
{
    IOTHUB_MESSAGE_HANDLE messageHandle = IoTHubMessage_CreateFromByteArray((const unsigned char *)buffer, strlen(buffer));
    if (messageHandle == NULL)
    {
        Serial.println("Unable to create a new IoTHubMessage.");
    }
    else
    {
        
        Serial.printf("Sending message: %s.\r\n", buffer);
        if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, messageHandle, sendCallback, NULL) != IOTHUB_CLIENT_OK)
        {
            Serial.println("Failed to hand over the message to IoTHubClient.");
        }
        else
        {
            messagePending = true;
           Serial.println("IoTHubClient accepted the message for delivery.");
        }

        IoTHubMessage_Destroy(messageHandle);
    }
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



int deviceMethodCallback(const char *methodName, const unsigned char *payload, size_t size, unsigned char **response, size_t *response_size, void *userContextCallback)
{
    Serial.printf("Try to invoke method %s.\r\n", methodName);
    const char *responseMessage = onSuccess;
    int result = 200;

    if (strcmp(methodName, "start") == 0)
    {
        start();
    }
    else if (strcmp(methodName, "stop") == 0)
    {
        stop();
    }
    else
    {
        Serial.printf("No method %s found.\r\n", methodName);
        responseMessage = notFound;
        result = 404;
    }

    *response_size = strlen(responseMessage);
    *response = (unsigned char *)malloc(*response_size);
    strncpy((char *)(*response), responseMessage, *response_size);

    return result;
}
void start()
{
    Serial.println("Start sending temperature and humidity data.");
    messageSending = true;
}

void stop()
{
    Serial.println("Stop sending temperature and humidity data.");
    messageSending = false;
}



