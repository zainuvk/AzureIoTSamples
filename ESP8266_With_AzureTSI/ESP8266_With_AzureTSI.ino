#include <ESP8266WiFi.h>
#include <AzureIoTHub.h>
#include <AzureIoTProtocol_MQTT.h>
#include <AzureIoTUtility.h>
#include <ArduinoJson.h>

const char *ssid = "xxx";
const char *password = "xxxx";

const char *connectionString = "HostName=xxxxxx.azure-devices.net;DeviceId=xxxx;SharedAccessKey=xxxxxxxxxxxxxxxxxxxxxxxxxxxx";
const char *DeviceId = "xxxxxxxx";
const int Messagemaxlen = 256;
const int interval = 2000;

static IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;

static int messageCount = 1;

static bool messagePending = false;
static bool messageSending = true;

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(9600);
    initWiFi();
    initTime();

    //Create AureIOtHubConnectionString
    iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(connectionString, MQTT_Protocol);
    if (iotHubClientHandle == NULL)
    {
        Serial.println("Failed on IoTHubClient_CreateFromConnectionString.");
        while (1)
            ;
    }
    else // added else block
    {
        Serial.println(" IoTHubClient_CreateFromConnectionString Ceated");
    }
}

void loop()
{
    // put your main code here, to run repeatedly:
    delay(2000);

    if (!messagePending && messageSending)
    {
        char messagePayload[Messagemaxlen];
        createMessage(messageCount, messagePayload);
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
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
    }
}

void createMessage(int messageId, char *payload)
{
   
    delay(1000);
   
    // get humidity reading
    float h = 130.0;
    // get temperature reading in Celsius
    float t = 250.0;

    StaticJsonBuffer<Messagemaxlen> jsonBuffer;

    JsonObject &root = jsonBuffer.createObject();
    root["deviceId"] = DeviceId;

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

    root.printTo(payload, Messagemaxlen);
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
