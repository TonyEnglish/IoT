# IoT
In PowerShell make sure you have extension azure-iot and have removed azure-cli-iot-ext (deprecated)  
  az extension add --name azure-iot  
  az extension list  
  az extension remove --name azure-cli-iot-ext  
To show events (if they are not being pushed some where else (like with message routing)  
  az iot hub monitor-events -n {iothub_name}  
This project is setup for ESP 8266 with WIFI access to Azure IoT Hub with NTP and scheduler  
HSM: https://www.infineon.com/cms/en/product/security-smart-card-solutions/optiga-embedded-security-solutions/optiga-trust/optiga-trust-m-sls32aia/  
HSM purchase: https://www.adafruit.com/product/4351  
GitHub for HSM lib https://github.com/Infineon/optiga-trust-m  
Pinout for NodeMCU ESP-32s https://cyberblogspot.com/nodemcu-esp-32s-pin-configuration/  
Datasheet for NodeMCU ESP32 https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf  
![alt text](https://github.com/TonyEnglish/IoT/blob/main/images/NodeMCU%2032s.png)
