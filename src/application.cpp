/**
 ******************************************************************************
 * @file    application.cpp
 * @authors  Satish Nair, Zachary Crockett and Mohit Bhoite
 * @version V1.0.0
 * @date    05-November-2013
 * @brief   Tinker application
 ******************************************************************************
  Copyright (c) 2013 Spark Labs, Inc.  All rights reserved.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program; if not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "application.h"

#define BUTTON_PIN D4
#define PWR_PIN D7
#define CTRL_PIN A7
#define MAX_TOPIC_LEN 64

#define CONFIG_DELAY 3500
#define CONF_BUF_LEN 64
#define CONFIG_VERSION 2
#define DEBOUNCE_DELAY 100
#define TRANSIENT_DELAY 5

enum app_type_t { GONG = 0, BUTTON = 1 };

SYSTEM_MODE(MANUAL);

const uint16_t MQTT_COMMAND_TIMEOUT =  8000;

MQTTNetwork *mqttNetwork;
MQTT::Client<MQTTNetwork, Timer> *mqttClient;
BaseApp *app;
Timer wifiResetTimer;
Timer transientTimer;
Timer debounceTimer;

#if defined(DEBUG_BUILD)
Timer debugTimer;
#endif

volatile system_tick_t lastInterrupt = 0;
volatile bool lastInterruptPressed = false;
volatile bool buttonPressed = false;
volatile bool pingReceived = false;
volatile bool buttonReceived = false;
volatile bool ownButtonReceived = false;

const char* topicFormat = "%s/%s";
char pubTopic[MAX_TOPIC_LEN];
char subTopic[MAX_TOPIC_LEN];
char mqttHost[CONF_BUF_LEN];

void onButton()
{
    bool pressed = digitalRead(BUTTON_PIN) == LOW;
    system_tick_t time = millis();

    // Last interrupt was the valid, non-transient edge
    if ((time - lastInterrupt) > TRANSIENT_DELAY)
    {
        buttonPressed = lastInterruptPressed;
    }

    lastInterruptPressed = pressed;
    lastInterrupt = time;
}

void onMessage(MQTT::MessageData* data)
{
    DEBUG("Receieved MQTT message with payload length %d starting with %c",
          data->message->payloadlen,
          ((const char *)data->message->payload)[0]);

    bool ownMessage = (strncmp(data->topicName, pubTopic, data->topicLen) == 0);

    if(strncmp("button",
               (const char *)data->message->payload,
               data->message->payloadlen) == 0)
    {
        if(ownMessage)
        {
            DEBUG("Received own button message");
            ownButtonReceived = true;
        }
        else
        {
            DEBUG("Received button message");
            buttonReceived = true;
        }
    }
    else if(strncmp("ping",
                    (const char *)data->message->payload,
                    data->message->payloadlen) == 0)
    {
        if(ownMessage)
        {
            DEBUG("Receive own ping message, ignoring");
        }
        else
        {
            DEBUG("Received ping message");
            pingReceived = true;
        }
    }
    else
    {
        DEBUG("Received unknown message");
    }
}

void publish(char* content)
{
    MQTT::Message msg = {
        MQTT::QOS0, // qos
        false, // retained
        false, // dup
        NULL, // id
        content, // payload
        strlen(content) // payloadlen
    };

    if(mqttClient->publish(pubTopic, &msg) == MQTT::FAILURE)
    {
        DEBUG("Error publishing message. Disconnecting.");
        mqttNetwork->disconnect();
    };
}

// Copied from the wifi credentials reader
void read_line(char *dst, int max_len)
{
  char c = 0, i = 0;
  while (1)
  {
    if (0 < Serial.available())
    {
      c = Serial.read();

      if (i == max_len || c == '\r' || c == '\n')
      {
        *dst = '\0';
        break;
      }

      if (c == 8 || c == 127)
      {
    	//for backspace or delete
    	if (i > 0)
    	{
          --dst;
          --i;
    	}
    	else
    	{
    	  continue;
    	}
      }
      else
      {
        *dst++ = c;
        ++i;
      }

      Serial.write(c);
    }
  }
  Serial.println();
  while (0 < Serial.available())
    Serial.read();
}

void writeConfig(char* prompt, uint8_t *mempos){
    uint8_t pos = 0;
    char buf[CONF_BUF_LEN];

    Serial.print(prompt);

    buf[0] = '\0';
    do {
        read_line(buf, CONF_BUF_LEN);
    } while(strlen(buf) == 0);

    do {
        EEPROM.write(*mempos, buf[pos]);
        (*mempos)++;
        pos++;
    } while(buf[pos-1] != '\0');

    if(*mempos > EEPROM_SIZE){
        Serial.println("You overran the EEPROM size of 100 bytes! Try again!");
    }
}

void getConfig(char* dest, int len, uint8_t *mempos) {
    int strpos = 0;

    while(strpos < len){
        dest[strpos] = EEPROM.read(*mempos);
        (*mempos)++;
        if(dest[strpos] == '\0'){
            return;
        }
        strpos++;
    }
    dest[len-1] = '\0';
}

void loadConfiguration(app_type_t *appType, bool *publishButton, uint16_t *port, char *mqttHost, char *rootTopic, char *name, int lens){
    uint8_t mempos = 1;

    *appType = (app_type_t)EEPROM.read(mempos++);
    *publishButton = (bool)EEPROM.read(mempos++);
    *port = (uint16_t)EEPROM.read(mempos++) | ((uint16_t)EEPROM.read(mempos++) << 8);

    getConfig(mqttHost, lens, &mempos);
    getConfig(rootTopic, lens, &mempos);
    getConfig(name, lens, &mempos);

}

void configure(){
    system_tick_t start = millis();
    uint8_t mempos = 0;
    uint8_t configVersion = EEPROM.read(mempos++);
    uint16_t port;
    char buf[32];
    char c;
    app_type_t appType;
    bool publishButton;

    delay(1500);

    if(configVersion > 0 && configVersion != CONFIG_VERSION){
        DEBUG("Detected outdated config version %d, need %d", configVersion, CONFIG_VERSION);
    }

    Serial.println("Press enter to start configuration.");

    while(true){
        if((millis() - start) > CONFIG_DELAY){
            if(configVersion == CONFIG_VERSION) {
                return;
            }
            Serial.println("Press enter to start configuration.");
            start = millis();
        }

        c = Serial.read();
        if(c == '\r' || c == '\n'){
            break;
        }
    }
    DEBUG("Starting configuration process");
    delay(100);

    while(true) {
        buf[0] = '\0';
        Serial.print("App type 0=gong 1=button: ");
        read_line(buf, 32);

        if(buf[0] == '1'){
            appType = BUTTON;
            break;
        } else if(buf[0] == '0'){
            appType = GONG;
            break;
        }
    }
    EEPROM.write(mempos++, appType);


    if (appType == GONG) {
        while(true) {
            buf[0] = '\0';
            Serial.print("Publish button presses [y/n]: ");
            read_line(buf, 32);

            if(buf[0] == 'y'){
                publishButton = true;
                break;
            } else if(buf[0] == 'n'){
                publishButton = false;
                appType = GONG;
                break;
            }
        }
    } else {
        publishButton = true
    }
    EEPROM.write(mempos++, publishButton);

    do {
        buf[0] = '\0';
        Serial.print("MQTT Port: ");
        read_line(buf, 32);
    } while((port = (uint16_t)atoi(buf)) == 0);
    EEPROM.write(mempos++, port & 0xFF);
    EEPROM.write(mempos++, (port >> 8) & 0xFF);

    writeConfig("MQTT Host: ", &mempos);
    writeConfig("Root topic (e.g. /foo/bar): ", &mempos);
    writeConfig("Name: ", &mempos);

    Serial.println("Please wait a few moments to start Wifi configuration.");

    // Will force the wifi credentials to update
    WiFi.listen();

    EEPROM.write(0, CONFIG_VERSION);
}

void setup()
{
    char name[CONF_BUF_LEN], rootTopic[CONF_BUF_LEN];
    uint16_t mqttPort;
    app_type_t appType;

    DEBUG("Beginning application setup");
#ifndef DEBUG_BUILD
    Serial.begin(9600);
#endif

    configure();

#ifndef DEBUG_BUILD
    if(!WLAN_SMART_CONFIG_START){
      Serial.end();
    }
#endif

    DEBUG("Loading configuration");
    loadConfiguration(&appType, &mqttPort, mqttHost, rootTopic, name, 64);
    DEBUG("Configuration loaded: '%s:%d%s/%s'", mqttHost, mqttPort, rootTopic, name);

    Serial.println("Starting the gong app.");

    if(appType == BUTTON)
    {
        app = new ButtonApp(CTRL_PIN);
    }
    else
    {
        app = new GongApp(PWR_PIN, CTRL_PIN);
    }

    snprintf(subTopic, MAX_TOPIC_LEN, topicFormat, rootTopic, "+");
    snprintf(pubTopic, MAX_TOPIC_LEN, topicFormat, rootTopic, name);

    mqttNetwork = new MQTTNetwork(mqttHost, mqttPort);
    mqttClient = new MQTT::Client<MQTTNetwork, Timer>(*mqttNetwork, MQTT_COMMAND_TIMEOUT);
    mqttClient->setDefaultMessageHandler(onMessage);

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(BUTTON_PIN, onButton, CHANGE);

    // If we don't connect successfully we have a problem
    wifiResetTimer.countdown(60 * 2);

    DEBUG("Application setup complete");
}

char* wifiStatus()
{
    if(WiFi.ready()){
        return "ready";
    } else if(WiFi.connecting()){
        return "connecting";
    } else if(WiFi.listening()){
        return "listening";
    } else {
        return "disconnected";
    }
}

void mqttProcess()
{
    // Don't reset if we've been connected recently
    wifiResetTimer.countdown(60);

    if(mqttNetwork->available())
    {
        Timer cycleTimer = Timer(MQTT_COMMAND_TIMEOUT);

        DEBUG("MQTT message waiting, processing");
        if(mqttClient->cycle(cycleTimer) == MQTT::FAILURE)
        {
            DEBUG("Failure while recieving MQTT message. Disconnecting.");
            mqttNetwork->disconnect();
        };
    }
    else
    {
        if(mqttClient->keepalive() == MQTT::FAILURE)
        {
            DEBUG("Failure sending MQTT keepalive. Disconnecting.");
            mqttNetwork->disconnect();
        }
    }
}

void mqttConnect()
{
    DEBUG("Opening connection to MQTT broker. Wifi status: %s",
          wifiStatus());

    if(mqttNetwork->connect())
    {
        DEBUG("Connecting to MQTT broker");
        if(mqttClient->connect() == MQTT::SUCCESS)
        {
            LED_SetRGBColor(RGB_COLOR_CYAN);

            DEBUG("Subscribing to %s", subTopic);
            // We pass a null callback and rely on the default
            // callback since the library handles wildcard
            // topics incorrectly
            mqttClient->subscribe(subTopic, MQTT::QOS0, NULL);
        }
        else
        {
            mqttNetwork->disconnect();
        }
    }
    else if (wifiResetTimer.expired())
    {
        DEBUG("Reseting");
        delay(200);
        Spark.sleep(SLEEP_MODE_DEEP, 1);
    }
    else
    {
        DEBUG("Could not connect.  Will reset in %ds.",
              wifiResetTimer.left_ms() / 1000);
        // Slow the iterations a little here for debuggin
        delay(200);
    }
}

void loop()
{
#if defined(DEBUG_BUILD)
    if(debugTimer.expired()){
        DEBUG("Wifi Status: %s\tMQTT Network: %s",
              wifiStatus(),
              mqttNetwork->connected() ? "connected" : "disconnected");
        debugTimer.countdown_ms(2000);
    }
#endif

    // Wait for SmartConfig/SerialConfig
    if(WiFi.listening()){
        wifiResetTimer.countdown(60 * 2);
        return;
    }

    if(mqttNetwork->connected())
    {
        mqttProcess();
    }
    else
    {
        mqttConnect();
    }

    if(buttonPressed && // Button was pressed
        digitalRead(BUTTON_PIN) == HIGH && // Button is currently released
       (millis() - lastInterrupt) > DEBOUNCE_DELAY) // Button is not bouncing
    {
        DEBUG("Handling button press");
        publish("button");
        app->onButtonPress();
        buttonPressed = false;
    }

    if(buttonReceived)
    {
        DEBUG("Handling button message");
        app->onButtonMessage();
        buttonReceived = false;
    }

    if(ownButtonReceived)
    {
        DEBUG("Handling own button message");
        app->onOwnButtonMessage();
        ownButtonReceived = false;
    }

    if(pingReceived)
    {
        DEBUG("Handling ping message");
        publish("pong");

        pingReceived = false;
    }
}

#ifdef __cplusplus
extern "C" {
#endif

void debug_output_(const char *p)
{
  static bool once = false;
  if (!once)
  {
      once = true;
      Serial.begin(9600);
  }

   Serial.print(p);
}

#ifdef __cplusplus
}
#endif
