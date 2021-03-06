#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
 
#include "aliyun_mqtt.h"
 
// GPIO 13, D7 on the Node MCU v3
#define SENSOR_PIN 13
 
#define WIFI_SSID        "Honor99"//替换自己的WIFI
#define WIFI_PASSWD      "12345678"//替换自己的WIFI
 
#define PRODUCT_KEY      "a1y9PjGEleF" //替换自己的PRODUCT_KEY
#define DEVICE_NAME      "D1_MINI" //替换自己的DEVICE_NAME
#define DEVICE_SECRET    "MG8lo2ddXORNT03dUjvT5JLLYhyGXd70"//替换自己的DEVICE_SECRET
 
#define DEV_VERSION       "S-TH-WIFI-v1.0-20190220"        //固件版本信息
 
#define ALINK_BODY_FORMAT         "{\"id\":\"123\",\"version\":\"1.0\",\"method\":\"%s\",\"params\":%s}"
#define ALINK_TOPIC_PROP_POST     "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/event/property/post"
#define ALINK_TOPIC_PROP_POSTRSP  "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/event/property/post_reply"
#define ALINK_TOPIC_PROP_SET      "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/service/property/set"
#define ALINK_METHOD_PROP_POST    "thing.event.property.post"
#define ALINK_TOPIC_DEV_INFO      "/ota/device/inform/" PRODUCT_KEY "/" DEVICE_NAME ""    
#define ALINK_VERSION_FROMA      "{\"id\": 123,\"params\": {\"version\": \"%s\"}}"
unsigned long lastMs = 0;
 
WiFiClient   espClient;
PubSubClient mqttClient(espClient);
 
void init_wifi(const char *ssid, const char *password)
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi does not connect, try again ...");
        delay(500);
    }
 
    Serial.println("Wifi is connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}
 
void mqtt_callback(char *topic, byte *payload, unsigned int length)
{
      char param[512];
    char jsonBuf[1024];
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    payload[length] = '\0';
    Serial.println((char *)payload);

    if(strstr((char *)payload,"LightSwitch\":1"))
    { 
       digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level  // but actually the LED is on; this is because
    // it is active low on the ESP-01)
              sprintf(param, "{\"LightSwitch\":1}");
              sprintf(jsonBuf, ALINK_BODY_FORMAT, ALINK_METHOD_PROP_POST, param); //增加LED灯的状态反馈
              Serial.println(jsonBuf);
              mqttClient.publish(ALINK_TOPIC_PROP_POST, jsonBuf);
    }
    else if(strstr((char *)payload,"LightSwitch\":0")) 
    {  
       digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
              sprintf(param, "{\"LightSwitch\":0}");
              sprintf(jsonBuf, ALINK_BODY_FORMAT, ALINK_METHOD_PROP_POST, param); //增加LED灯的状态反馈
              Serial.println(jsonBuf);
              mqttClient.publish(ALINK_TOPIC_PROP_POST, jsonBuf);
    }

//    Serial.println((char *)payload);
    if (strstr(topic, ALINK_TOPIC_PROP_SET))//判断当前字段是否是属性设置内容
    {
        StaticJsonBuffer<100> jsonBuffer;
        JsonObject &root = jsonBuffer.parseObject(payload);
        if (!root.success())
        {
            Serial.println("parseObject() failed");
            return;
        }
    }
    
    

  
}
void mqtt_version_post()
{
    char param[512];
    char jsonBuf[1024];
 
    //sprintf(param, "{\"MotionAlarmState\":%d}", digitalRead(13));
    sprintf(param, "{\"id\": 123,\"params\": {\"version\": \"%s\"}}", DEV_VERSION);
   // sprintf(jsonBuf, ALINK_BODY_FORMAT, ALINK_METHOD_PROP_POST, param);
    Serial.println(param);
    mqttClient.publish(ALINK_TOPIC_DEV_INFO, param);
}
void mqtt_check_connect()
{
    while (!mqttClient.connected())//mqttδ����
    {
        while (connect_aliyun_mqtt(mqttClient, PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET))
        {
            Serial.println("MQTT connect succeed!");
            //client.subscribe(ALINK_TOPIC_PROP_POSTRSP);
            mqttClient.subscribe(ALINK_TOPIC_PROP_SET);
            
            Serial.println("subscribe done");
//            mqtt_version_post();
        }
    }
    
}
 
void mqtt_interval_post()
{
    char param[512];
    char jsonBuf[1024];
 
    //sprintf(param, "{\"MotionAlarmState\":%d}", digitalRead(13));
    sprintf(param, "{\"CurrentHumidity\":%d,\"CurrentTemperature\":12,\"GeoLocation\":{\"CoordinateSystem\":2,\"Latitude\":2,\"Longitude\":123,\"Altitude\":1}}", digitalRead(13));
    sprintf(jsonBuf, ALINK_BODY_FORMAT, ALINK_METHOD_PROP_POST, param);
    Serial.println(jsonBuf);
    mqttClient.publish(ALINK_TOPIC_PROP_POST, jsonBuf);
}
 
 
void setup()
{
 
    pinMode(SENSOR_PIN, INPUT);
    /* initialize serial for debugging */
    Serial.begin(115200);
 
    Serial.println("Demo Start");
 
    init_wifi(WIFI_SSID, WIFI_PASSWD);
 
    mqttClient.setCallback(mqtt_callback);
     pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output

}
 
// the loop function runs over and over again forever
void loop()
{
    if (millis() - lastMs >= 20000)
    {
        lastMs = millis();
        mqtt_check_connect();
        /* Post */        
        mqtt_interval_post();
    }
 
    mqttClient.loop();
 
    unsigned int WAIT_MS = 2000;
    if (digitalRead(SENSOR_PIN) == HIGH)
    {
        Serial.println("Motion detected!");
    }
    else
    {
        Serial.println("Motion absent!");
    }
    delay(WAIT_MS); // ms
    Serial.println(millis() / WAIT_MS);
  
}
