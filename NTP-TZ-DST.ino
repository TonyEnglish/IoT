#include <ESP8266WiFi.h>
#include <TimeLib.h>
#include <NtpClientLib.h>
#include <WiFiClientSecure.h>
#include <AzureIoTHubMQTTClient.h>
#include <TimeAlarms.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h> 
 
//variables for WiFi
const char *ssid2 = ""; //add ssid
const char *password = ""; /add pw
WiFiEventHandler gotIpEventHandler, disconnectedEventHandler, connectingEventHandeler;
int reqConnect = 0; 
int isConnected = 0;
const long reqConnectNum = 15; // number of intervals to wait for connection
bool wifiFirstConnected = false;
unsigned long previousMillis = 0;
const long interval = 500;
//SHA1 finger print of certificate use web browser to view and copy
const char fingerprint[] PROGMEM = "8B 1A 35 97 05 18 8C 55 77 CB 2D CD 9A 06 33 18 07 C0 BB 97";

//Variables for NTP
int8_t timeZone = -7;
int8_t minutesTimeZone = 0;
boolean syncEventTriggered = false; // True if a time even has been triggered
NTPSyncEvent_t ntpEvent; // Last triggered event

//Variables for LCD
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 20 chars and 4 line display

//Variables for Azure IoT Hub
#define IOTHUB_HOSTNAME         "" //add iot hub name
#define DEVICE_ID               "" //add device name
#define DEVICE_KEY              "" //Primary key of the device
WiFiClientSecure tlsClient;
AzureIoTHubMQTTClient client(tlsClient, IOTHUB_HOSTNAME, DEVICE_ID, DEVICE_KEY);
unsigned long lastMillis = 0;

//fuctions for Azure
void connectToIoTHub(); // <- predefine connectToIoTHub() for setup()
void onMessageCallback(const MQTT::Publish& msg);// sample from IoT hub to client [{"Name" : "SetAirResistance","Parameters" : {"Position" : 20 }}]

void onClientEvent(const AzureIoTHubMQTTClient::AzureIoTHubMQTTClientEvent event) {
    if (event == AzureIoTHubMQTTClient::AzureIoTHubMQTTClientEventConnected) {
        Serial.println("Connected to Azure IoT Hub");
        //Add the callback to process cloud-to-device message/command
        client.onMessage(onMessageCallback);
    }
}

void onActivateRelayCommand(String cmdName, JsonVariant jsonValue) {
    //Parse cloud-to-device message JSON. In this example, I send the command message with following format:
    //{"Name":"ActivateRelay","Parameters":{"Activated":0}}
    Serial.println("onActivateRelayCommand Fuction");
    JsonObject& jsonObject = jsonValue.as<JsonObject>();
    if (jsonObject.containsKey("Parameters")) {
        auto params = jsonValue["Parameters"];
        auto isAct = (params["Activated"]);
        if (isAct) {
            Serial.println("Activated true");
        }
        else {
            Serial.println("Activated false");
        }
    }
}

void onActivateZoneCommand(String cmdName, JsonVariant jsonValue) {
    //Parse cloud-to-device message JSON. In this example, I send the command message with following format:
    //{"Name":"ActivateZone","Parameters":{"Zone":4,"Activated":0}}
    Serial.println("onActivateZoneCommand Fuction");
    JsonObject& jsonObject = jsonValue.as<JsonObject>();
    if (jsonObject.containsKey("Parameters")) {
        auto params = jsonValue["Parameters"];
        int Zone = (params["Zone"]);
        auto isAct = (params["Activated"]);
        if (isAct) {//{"Name":"ActivateZone","Parameters":{"Zone":4,"Activated":1}}
            Serial.print("Zone ");
            Serial.print(Zone);
            Serial.println(" turned on");
        }
        else {//{"Name":"ActivateZone","Parameters":{"Zone":4,"Activated":0}}
            Serial.print("Zone ");
            Serial.print(Zone);
            Serial.println(" turned off");
        }
    }
}

void onMessageCallback(const MQTT::Publish& msg) {
    //Handle Cloud to Device message by yourself. sample message to send: [{"Name" : "SetAirResistance","Parameters" : {"Position" : 20 }}]
    Serial.println("Message from IoT hub to device");
    Serial.println(msg.payload_string());
    if (msg.payload_len() == 0) {
        return;
    }
}

void connectToIoTHub() {
    Serial.print("\nBeginning Azure IoT Hub Client... ");
    if (client.begin()) {
        Serial.println("OK");
    } else {
        Serial.println("Could not connect to MQTT");
    }
}

void readSensor(float *temp, float *press) {
    *temp = 20 + (rand() % 10 + 2);
    *press = 90 + (rand() % 8 + 2);
}

//fuctions for wifi connection
void onConnected(const WiFiEventStationModeConnected& event){
  Serial.println ( "Connected to AP." );
  isConnected = 1;    
}
 
//fuctions for wifi connection
void onDisconnect(const WiFiEventStationModeDisconnected& event){
  Serial.println ( "WiFi On Disconnect." );
  isConnected = 0;
}

//fuctions for wifi connection
void onGotIP(const WiFiEventStationModeGotIP& event){
  Serial.print ( "Got Ip: " );
  isConnected = 1.5;
  wifiFirstConnected = true;
  Serial.println(WiFi.localIP());
  connectToIoTHub();
}

//fuctions for NTP connection
void processSyncEvent (NTPSyncEvent_t ntpEvent) {
    if (ntpEvent) {
        Serial.print ("Time Sync error: ");
        if (ntpEvent == noResponse)
            Serial.println ("NTP server not reachable");
        else if (ntpEvent == invalidAddress)
            Serial.println ("Invalid NTP server address");
    } else {
        Serial.print ("Got NTP time: ");
        isConnected = 2;
        Serial.println (NTP.getTimeDateString (NTP.getLastNTPSync ()));
    }
}

// functions to be called when the spriklers come on each evening:
void SprinklerOn() {
  Serial.println("Sprinkers: - turn sprinklers on");
  Serial.println (NTP.getTimeDateString ());
  lcd.setCursor(0,2);
  lcd.print(NTP.getTimeDateString ());
}

// functions to be called when the spriklers come on each evening:
void SprinklerOff() {
  Serial.println("Sprinkers: - turn sprinklers off");
  lcd.setCursor(0,2);
  lcd.print(NTP.getTimeDateString ());
}

//fuction for LCD
void addLCDData(){
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(3,0);
  lcd.print("Hello, Michael!");
  lcd.setCursor(2,3);
  lcd.print("Sprinker Controler");
}

void setup() {
  Serial.begin(115200);
  while(!Serial) {
      yield();
  }

  //WiFi Connection sequence
  WiFi.disconnect();
  WiFi.persistent(false);
  connectingEventHandeler = WiFi.onStationModeConnected(onConnected);
  disconnectedEventHandler = WiFi.onStationModeDisconnected(onDisconnect);
  gotIpEventHandler = WiFi.onStationModeGotIP(onGotIP);

  tlsClient.setFingerprint(fingerprint);
  tlsClient.setTimeout(15000);

  NTP.onNTPSyncEvent ([](NTPSyncEvent_t event) {
    ntpEvent = event;
    syncEventTriggered = true;
  });

  //Configure alarms
  Alarm.timerRepeat(60, SprinklerOff);           // timer for every hour to sync time on Arduino to NTP time
  Alarm.timerRepeat(15, SprinklerOn); // print message every 15 seconds
  //Alarm.alarmRepeat(17,05,0, SprinklerOff);  // 5:05pm every day
  //Alarm.timerRepeat(10, digitalClockDisplay); //display current time every 10 seconds

  //Setup LCD screen
  //Wire.begin(2,0);
  lcd.init(); //use sda->d2 scl->d1
  addLCDData();

  //MQTT setup events
  //Handle client events
  client.onEvent(onClientEvent);
  //Add command to handle and its handler
  //Command format is assumed like this: {"Name":"[COMMAND_NAME]","Parameters":[PARAMETERS_JSON_ARRAY]}
  client.onCloudCommand("ActivateRelay", onActivateRelayCommand);
  client.onCloudCommand("ActivateZone", onActivateZoneCommand);
}

void loop() {
  //Needed to start WiFi
  if (WiFi.status() != WL_CONNECTED && reqConnect > reqConnectNum && isConnected <2){ 
    reqConnect =  0 ;
    isConnected = 0 ;
    WiFi.disconnect() ;
    WiFi.begin ( ssid2, password );  
    Serial.println ( "Connecting..." );
  }
  addLCDData();
  //Needed to start WiFi
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    reqConnect++ ;
  }

  //Needed to start NTP after WiFi is up
  if (wifiFirstConnected) {
    wifiFirstConnected = false;
    NTP.begin("pool.ntp.org", timeZone, true);
    //NTP.setInterval (5,1800);
  }

  //Needed to update NTP when sync is called
  if (syncEventTriggered) {
    Serial.println("Sync event triggered");
    processSyncEvent (ntpEvent);
    syncEventTriggered = false;
  }

  //add normal loop code inside here 
  if (isConnected == 2) {
    Alarm.delay(0);
    client.run();

    if (client.connected()) {
            //Serial.println("just after client.connected");
            //Serial.println(millis() - lastMillis > 3000);
            //Serial.println(timeStatus() != timeNotSet);
        // Publish a message roughly every 3 second. Only after time is retrieved and set properly.
        if(millis() - lastMillis > 20000 && timeStatus() != timeNotSet) {
            lastMillis = millis();
            //Serial.println("just after millis");
            //Read the actual temperature from sensor
            float temp, press;
            readSensor(&temp, &press);

            //Get current timestamp, using Time lib
            time_t currentTime = now();

            // You can do this to publish payload to IoT Hub
            
            String payload = "{\"DeviceId\":\"" + String(DEVICE_ID) + "\", \"MTemperature\":" + String(temp) + ", \"EventTime\":" + String(currentTime) + "}";
            Serial.println(payload);
            client.publish(MQTT::Publish("devices/" + String(DEVICE_ID) + "/messages/events/", payload).set_qos(1));
            client.sendEvent(payload);
            
            Serial.println("Sending message to Azure");
            //Or instead, use this more convenient way
            AzureIoTHubMQTTClient::KeyValueMap keyVal = {{"MTemperature", temp}, {"MPressure", press}, {"DeviceId", DEVICE_ID}, {"EventTime", currentTime}};
            client.sendEventWithKeyVal(keyVal);
        }
    }
    else {
      //Serial.println("just after client.noconnected");
    }
  }

  delay(0);
}
