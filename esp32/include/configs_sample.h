#ifndef CONFIGS_H
#define CONFIGS_H

/**
 * WiFi setup
 */
#define CONFIG_WIFI_SSID            "ssid"
#define CONFIG_WIFI_PASSWORD        "pwd"

/**
 * IoT Hub Device Connection String setup
 * Find your Device Connection String by going to your Azure portal, creating (or navigating to) an IoT Hub, 
 * navigating to IoT Devices tab on the left, and creating (or selecting an existing) IoT Device. 
 * Then click on the named Device ID, and you will have able to copy the Primary or Secondary Device Connection String to this sample.
 */
#define DEVICE_CONNECTION_STRING    "HostName=abc.azure-devices.net;DeviceId=abc;SharedAccessKey=xyz"

#endif /* CONFIGS_H */