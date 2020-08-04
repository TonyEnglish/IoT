# IoT
In PowerShell make sure you have extension azure-iot and have removed azure-cli-iot-ext (deprecated)  
  az extension add --name azure-iot  
  az extension list  
  az extension remove --name azure-cli-iot-ext  
To show events (if they are not being pushed some where else (like with message routing)  
  az iot hub monitor-events -n {iothub_name}  
