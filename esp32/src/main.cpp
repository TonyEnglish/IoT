#include <WiFi.h>
#include "AzureIotHub.h"
#include "Esp32MQTTClient.h"
#include "time.h"
#include "configs.h"

#define INTERVAL 5000 //wait time after start before stop can be executed
#define DEVICE_ID "Esp32Device"
#define MESSAGE_MAX_LEN 256

// Please input the SSID and password of WiFi
const char *ssid = CONFIG_WIFI_SSID;
const char *password = CONFIG_WIFI_PASSWORD;

/*String containing Hostname, Device Id & Device Key in the format:                         */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessKey=<device_key>"                */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessSignature=<device_sas_token>"    */
static const char *connectionString = DEVICE_CONNECTION_STRING;

//const char *messageData = "{\"deviceId\":\"%s\", \"messageId\":%d, \"Temperature\":%f, \"Humidity\":%f, \"Time\":%ld}";
const char *messageData = "{\"deviceId\":\"%s\", \"messageId\":%d, \"Time\":%ld, \"Race time ms\":%ld}";

int messageCount = 1;
static bool hasWifi = false;
static bool messageSending = false; //change to true to initially start sendind messages
static uint64_t send_interval_ms;
static bool race_started = false;
static uint64_t race_time_ms;

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Utilities
static void InitWifi()
{
  Serial.println("Connecting...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  hasWifi = true;
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

static void SendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result)
{
  if (result == IOTHUB_CLIENT_CONFIRMATION_OK)
  {
    Serial.println("Send Confirmation Callback finished.");
  }
}

static void MessageCallback(const char *payLoad, int size)
{
  Serial.println("Message callback:");
  Serial.println(payLoad);
}

static void DeviceTwinCallback(DEVICE_TWIN_UPDATE_STATE updateState, const unsigned char *payLoad, int size)
{
  char *temp = (char *)malloc(size + 1);
  if (temp == NULL)
  {
    return;
  }
  memcpy(temp, payLoad, size);
  temp[size] = '\0';
  // Display Twin message.
  Serial.println(temp);
  free(temp);
}

static int DeviceMethodCallback(const char *methodName, const unsigned char *payload, int size, unsigned char **response, int *response_size)
{
  //to use to to Direct Method and put in Method Name stop or start and then click invoke method within Azure for the device
  LogInfo("Try to invoke method %s", methodName);
  const char *responseMessage = "\"Successfully invoke device method\"";
  int result = 200;

  if (strcmp(methodName, "start") == 0)
  {
    //LogInfo("Start sending temperature and humidity data");
    LogInfo("Start new run");
    messageSending = true;
  }
  else if (strcmp(methodName, "stop") == 0)
  {
    //LogInfo("Stop sending temperature and humidity data");
    LogInfo("Stop run");
    messageSending = false;
    race_started = false;
  }
  else
  {
    LogInfo("No method %s found", methodName);
    responseMessage = "\"No method found\"";
    result = 404;
  }

  *response_size = strlen(responseMessage) + 1;
  *response = (unsigned char *)strdup(responseMessage);

  return result;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Arduino sketch
void setup()
{
  Serial.begin(115200);
  Serial.println("ESP32 Device");
  Serial.println("Initializing...");
  //pin 2 is gpio 2, called pin 4 on drawing
  pinMode(2, INPUT_PULLDOWN);
  // Initialize the WiFi module
  Serial.println(" > WiFi");
  hasWifi = false;
  InitWifi();
  if (!hasWifi)
  {
    return;
  }
  randomSeed(analogRead(0));

  Serial.println(" > IoT Hub");
  Esp32MQTTClient_SetOption(OPTION_MINI_SOLUTION_NAME, "GetStarted");
  Esp32MQTTClient_Init((const uint8_t *)connectionString, true);

  Esp32MQTTClient_SetSendConfirmationCallback(SendConfirmationCallback);
  Esp32MQTTClient_SetMessageCallback(MessageCallback);
  Esp32MQTTClient_SetDeviceTwinCallback(DeviceTwinCallback);
  Esp32MQTTClient_SetDeviceMethodCallback(DeviceMethodCallback);

  send_interval_ms = millis();
}

void loop()
{
  if (hasWifi)
  {
    if (messageSending &&
        (int)(millis() - send_interval_ms) >= INTERVAL &&
        digitalRead(2))
    {
      if (!race_started)
      {
        Serial.println("Race Started!");
        race_time_ms = millis();
        race_started = true;
      }
      else
      {
        Serial.println("Race Stopped!");
        race_time_ms = millis() - race_time_ms;
        //Serial.println((byte)(race_time_ms % 10));
        //Serial.println(time(nullptr));
        //Serial.println(digitalRead(2));
        char messagePayload[MESSAGE_MAX_LEN];
        //float temperature = (float)random(0,50);
        //float humidity = (float)random(0, 1000)/10;
        //snprintf(messagePayload,MESSAGE_MAX_LEN, messageData, DEVICE_ID, messageCount++, temperature,humidity, time(nullptr));
        snprintf(messagePayload, MESSAGE_MAX_LEN, messageData, DEVICE_ID, messageCount++, time(nullptr), race_time_ms);
        //Serial.println(messagePayload);
        EVENT_INSTANCE *message = Esp32MQTTClient_Event_Generate(messagePayload, MESSAGE);
        //Esp32MQTTClient_Event_AddProp(message, "temperatureAlert", "true");
        Esp32MQTTClient_Event_AddProp(message, "robot_Start_Stop", "true");
        Esp32MQTTClient_SendEventInstance(message);
        race_started = false;
      }
      send_interval_ms = millis();
    }
    else
    {
      Esp32MQTTClient_Check();
    }
  }
  delay(10);
}