
//static WiFiClientSecure sslClient; // for ESP8266


static void sendCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void *userContextCallback)
{
    if (IOTHUB_CLIENT_CONFIRMATION_OK == result)
    {
             (void)printf("Message sent to Azure IoT Hub.");
    }
    else
    {        
         (void)printf("Failed to send message to Azure IoT Hub.");
    }
    messagePending = false;
}
static void sendMessage(IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle, char *buffer)
{
    IOTHUB_MESSAGE_HANDLE messageHandle = IoTHubMessage_CreateFromByteArray((const unsigned char *)buffer, strlen(buffer));
    if (messageHandle == NULL)
    {
        (void)printf("Unable to create a new IoTHubMessage.");
    }
    else
    {
        (void)printf("Sending message: %s.\r\n", buffer);
        if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, messageHandle, sendCallback, NULL) != IOTHUB_CLIENT_OK)
        {
            (void)printf("Failed to hand over the message to IoTHubClient.");
        }
        else
        {
            messagePending = true;
           (void)printf("IoTHubClient accepted the message for delivery.");
        }

        IoTHubMessage_Destroy(messageHandle);
    }
}


/*void start()
{
    Serial.println("Start sending temperature and humidity data.");
    messageSending = true;
}

void stop()
{
    Serial.println("Stop sending temperature and humidity data.");
    messageSending = false;
}
*/





