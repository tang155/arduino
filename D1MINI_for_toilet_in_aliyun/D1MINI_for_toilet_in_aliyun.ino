#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
 
#include "aliyun_mqtt.h"
 
// GPIO 13, D7 on the Node MCU v3
#define SENSOR_PIN 13
 
#define WIFI_SSID        "Honor99"//替换自己的WIFI
#define WIFI_PASSWD      "12345678"//替换自己的WIFI
 
#define PRODUCT_KEY      "a18gLLRjBlh" //替换自己的PRODUCT_KEY
#define DEVICE_NAME      "BK7orLCxF4u82QoinUdi" //替换自己的DEVICE_NAME
#define DEVICE_SECRET    "SYwp0VyQWLsfshC1SYw3lxeE93LQE0hj"//替换自己的DEVICE_SECRET
 
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
uint8 data_of_bt_rf[32]; 
uint8 * serial_data_of_bt_rf_command_excute( uint8 function_code, uint8 value_code)//生成AXENT COM32字节数据
{
  uint8 custom_code = 0X30;
    data_of_bt_rf[0] = 0X02;
    data_of_bt_rf[1] = 0X0A;
    data_of_bt_rf[2] = custom_code;
    data_of_bt_rf[3] = function_code;
    data_of_bt_rf[4] = value_code;
    data_of_bt_rf[5] = function_code+value_code;
    data_of_bt_rf[6] = 0X00;
    data_of_bt_rf[7] = 0X00;
    data_of_bt_rf[8] = 0X00;
    data_of_bt_rf[9] = 0X00;
    data_of_bt_rf[10] = 0X00;
    data_of_bt_rf[11] = 0X00;
    data_of_bt_rf[12] = 0X00;
    data_of_bt_rf[13] = 0X00;
    data_of_bt_rf[14] = 0X00;
    data_of_bt_rf[15] = 0X00;
    data_of_bt_rf[16] = 0X00;
    data_of_bt_rf[17] = 0X00;
    data_of_bt_rf[18] = 0X00;
    data_of_bt_rf[19] = 0X00;
    data_of_bt_rf[20] = 0X00;
    data_of_bt_rf[21] = 0X00;
    data_of_bt_rf[22] = 0X00;
    data_of_bt_rf[23] = 0X00;
    data_of_bt_rf[24] = 0X00;
    data_of_bt_rf[25] = 0X00;
    data_of_bt_rf[26] = 0X00;
    data_of_bt_rf[27] = 0x49;//标记符 IOT
    data_of_bt_rf[28] = 0x4F;//标记符 IOT
    data_of_bt_rf[29] = 0x54;//标记符 IOT
    unsigned char  i ;
  for(i=2;i<29;i++)
    {
        data_of_bt_rf[29] ^= data_of_bt_rf[i];
    }
    data_of_bt_rf[30] = 0X0B;
    data_of_bt_rf[31] = 0X04;
    return data_of_bt_rf;
}  
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
//    Serial.print("Message arrived [");//显示接收到云端的下发数据
//    Serial.print(topic);
//    Serial.print("] ");
    payload[length] = '\0';
//    Serial.println((char *)payload);

    if(strstr((char *)payload,"LightSwitch\":1"))
    { 
       digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level  // but actually the LED is on; this is because
    // it is active low on the ESP-01)
              sprintf(param, "{\"LightSwitch\":1}");
              sprintf(jsonBuf, ALINK_BODY_FORMAT, ALINK_METHOD_PROP_POST, param); //增加LED灯的状态反馈
              Serial.println(jsonBuf);
              mqttClient.publish(ALINK_TOPIC_PROP_POST, jsonBuf);
    }
    else if(strstr((char *)payload,"Stop\":")) //停止命令
    {  
        serial_data_of_bt_rf_command_excute(0,0);
        Serial.write(serial_data_of_bt_rf_command_excute(0,0),32);
    }
    else if(strstr((char *)payload,"LightSwitch\":0")) 


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
 
void mqtt_interval_post()//向云端上报消息
{
    char param[512];
    char jsonBuf[1024];
 
    //sprintf(param, "{\"MotionAlarmState\":%d}", digitalRead(13));
//    sprintf(param, "{\"CurrentHumidity\":%d,\"CurrentTemperature\":12,\"GeoLocation\":{\"CoordinateSystem\":2,\"Latitude\":2,\"Longitude\":123,\"Altitude\":1}}", digitalRead(13));
//    sprintf(jsonBuf, ALINK_BODY_FORMAT, ALINK_METHOD_PROP_POST, param);
//    Serial.println(jsonBuf);
//    mqttClient.publish(ALINK_TOPIC_PROP_POST, jsonBuf);//向云端上报消息
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
  Serial.setTimeout(1000);//串口接收20ms
}
 
// the loop function runs over and over again forever
void loop()
{
    char serial_rx[50];
    int serial_rx_num=0;
    if (millis() - lastMs >= 20000)
    {
        lastMs = millis();
        mqtt_check_connect();
        /* Post */        
        mqtt_interval_post();
    }
 
    mqttClient.loop();
 
    unsigned int WAIT_MS = 1000;
//    if (digitalRead(SENSOR_PIN) == HIGH)
//    {
//        Serial.println("Motion detected!");
//    }
//    else
//    {
//        Serial.println("Motion absent!");
//    }
//    delay(WAIT_MS); // ms
//    Serial.println(millis() / WAIT_MS);
  serial_rx_num = Serial.readBytes(serial_rx,32);//接收最多32字节
//  char * serial_rx_num_string ;
//  itoa(serial_rx_num,serial_rx_num_string,10);
  if(serial_rx_num)
  {
    Serial.print("receive:");    
    Serial.print(serial_rx_num);
    Serial.print("bytes=");

    serial_rx[serial_rx_num] = 0;
    Serial.print(serial_rx);
    char param[512];
    char jsonBuf[1024];
    sprintf(param, "{%s}", serial_rx);
    sprintf(jsonBuf, ALINK_BODY_FORMAT, ALINK_METHOD_PROP_POST, param);
    Serial.println(jsonBuf);
    mqttClient.publish(ALINK_TOPIC_PROP_POST, jsonBuf);//向云端上报消息
  }
  
}
