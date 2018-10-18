// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef IOT_CONFIGS_H
#define IOT_CONFIGS_H

#define DEVICE_ID "<Give your Deviceid here>"
// Interval time(ms) for sending message to IoT Hub
#define INTERVAL 2000

#define MESSAGE_MAX_LEN 256

/**
 * WiFi setup
 */
#define IOT_CONFIG_WIFI_SSID            "<Give your WIFI SSID here>"
#define IOT_CONFIG_WIFI_PASSWORD        "<Give your WIFI password here"

/**
 * Find under Microsoft Azure IoT Suite -> DEVICES -> <your device> -> Device Details and Authentication Keys
 * String containing Hostname, Device Id & Device Key in the format:
 *  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessKey=<device_key>"    
 */

#define IOT_CONFIG_CONNECTION_STRING  "HostName=XXXX.azure-devices.net;DeviceId=XXXXX;SharedAccessKey=XXXXXXX"


#endif /* IOT_CONFIGS_H */
