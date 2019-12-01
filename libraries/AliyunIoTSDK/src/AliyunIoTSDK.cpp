
#include "AliyunIoTSDK.h"
#include <PubSubClient.h>
#include <SHA256.h>

#define CHECK_INTERVAL 10000

static const char *deviceName = NULL;
static const char *productKey = NULL;
static const char *deviceSecret = NULL;
static const char *region = NULL;

#define MQTT_PORT 1883

#define SHA256HMAC_SIZE 32
#define DATA_CALLBACK_SIZE 20

#define ALINK_BODY_FORMAT "{\"id\":\"123\",\"version\":\"1.0\",\"method\":\"thing.event.property.post\",\"params\":%s}"
#define ALINK_EVENT_BODY_FORMAT "{\"id\": \"123\",\"version\": \"1.0\",\"params\": %s,\"method\": \"thing.event.%s.post\"}"

static unsigned long lastMs = 0;

static PubSubClient *client = NULL;

char AliyunIoTSDK::clientId[256] = "";
char AliyunIoTSDK::mqttUsername[100] = "";
char AliyunIoTSDK::mqttPwd[256] = "";
char AliyunIoTSDK::domain[150] = "";

char AliyunIoTSDK::ALINK_TOPIC_PROP_POST[150] = "";
char AliyunIoTSDK::ALINK_TOPIC_PROP_SET[150] = "";
char AliyunIoTSDK::ALINK_TOPIC_EVENT[150] = "";

static String hmac256(const String &signcontent, const String &ds)
{
    byte hashCode[SHA256HMAC_SIZE];
    SHA256 sha256;

    const char *key = ds.c_str();
    size_t keySize = ds.length();

    sha256.resetHMAC(key, keySize);
    sha256.update((const byte *)signcontent.c_str(), signcontent.length());
    sha256.finalizeHMAC(key, keySize, hashCode, sizeof(hashCode));

    String sign = "";
    for (byte i = 0; i < SHA256HMAC_SIZE; ++i)
    {
        sign += "0123456789ABCDEF"[hashCode[i] >> 4];
        sign += "0123456789ABCDEF"[hashCode[i] & 0xf];
    }

    return sign;
}

static void parmPass(JsonVariant parm)
{
    //    const char *method = parm["method"];
    for (int i = 0; i < DATA_CALLBACK_SIZE; i++)
    {
        if (poniter_array[i].key)
        {
            bool hasKey = parm["params"].containsKey(poniter_array[i].key);
            if (hasKey)
            {
                poniter_array[i].fp(parm["params"]);
            }
        }
    }
}
// 所有云服务的回调都会首先进入这里，例如属性下发
static void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    payload[length] = '\0';
    Serial.println((char *)payload);

    if (strstr(topic, AliyunIoTSDK::ALINK_TOPIC_PROP_SET))
    {

        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, payload); //反序列化JSON数据

        if (!error) //检查反序列化是否成功
        {
            parmPass(doc.as<JsonVariant>()); //将参数传递后打印输出
        }
    }
}

// void AliyunIoTSDK::bind(MQTT_CALLBACK_SIGNATURE)
// {

//     //    return *this;
// }
static bool mqttConnecting = false;
void AliyunIoTSDK::mqttCheckConnect()
{
    if (client != NULL && !mqttConnecting)
    {
        if (!client->connected())
        {
            client->disconnect();
            Serial.println("Connecting to MQTT Server ...");
            mqttConnecting = true;
            if (client->connect(clientId, mqttUsername, mqttPwd))
            {
                Serial.println("MQTT Connected!");
            }
            else
            {
                Serial.print("MQTT Connect err:");
                Serial.println(client->state());
                // delay(65000);
            }
            mqttConnecting = false;
        }else{
            Serial.println("state is connected");
        }
    }
}

void AliyunIoTSDK::begin(Client &espClient,
                         const char *_productKey,
                         const char *_deviceName,
                         const char *_deviceSecret,
                         const char *_region)
{

    client = new PubSubClient(espClient);
    productKey = _productKey;
    deviceName = _deviceName;
    deviceSecret = _deviceSecret;
    region = _region;
    long times = millis();
    String timestamp = String(times);

    sprintf(clientId, "%s|securemode=3,signmethod=hmacsha256,timestamp=%s|", deviceName, timestamp.c_str());

    String signcontent = "clientId";
    signcontent += deviceName;
    signcontent += "deviceName";
    signcontent += deviceName;
    signcontent += "productKey";
    signcontent += productKey;
    signcontent += "timestamp";
    signcontent += timestamp;

    String pwd = hmac256(signcontent, deviceSecret);

    strcpy(mqttPwd, pwd.c_str());

    sprintf(mqttUsername, "%s&%s", deviceName, productKey);
    sprintf(ALINK_TOPIC_PROP_POST, "/sys/%s/%s/thing/event/property/post", productKey, deviceName);
    sprintf(ALINK_TOPIC_PROP_SET, "/sys/%s/%s/thing/service/property/set", productKey, deviceName);
    sprintf(ALINK_TOPIC_EVENT, "/sys/%s/%s/thing/event", productKey, deviceName);

    sprintf(domain, "%s.iot-as-mqtt.%s.aliyuncs.com", productKey, region);
    client->setServer(domain, MQTT_PORT); /* 连接WiFi之后，连接MQTT服务器 */
    client->setCallback(callback);

    mqttCheckConnect();
}

void AliyunIoTSDK::loop()
{
    client->loop();
    if (millis() - lastMs >= CHECK_INTERVAL)
    {
        lastMs = millis();
        mqttCheckConnect();
    }
    
}

void AliyunIoTSDK::sendEvent(const char *eventId, const char *param)
{
    char topicKey[156];
    sprintf(topicKey, "%s/%s/post", ALINK_TOPIC_EVENT, eventId);
    char jsonBuf[1024];
    sprintf(jsonBuf, ALINK_EVENT_BODY_FORMAT, param, eventId);
    Serial.println(jsonBuf);
    boolean d = client->publish(topicKey, jsonBuf);
    Serial.print("publish:0 成功:");
    Serial.println(d);
}
void AliyunIoTSDK::sendEvent(const char *eventId)
{
    sendEvent(eventId, "{}");
}

void AliyunIoTSDK::send(const char *param)
{

    char jsonBuf[1024];

    sprintf(jsonBuf, ALINK_BODY_FORMAT, param);
    Serial.println(jsonBuf);
    boolean d = client->publish(ALINK_TOPIC_PROP_POST, jsonBuf);
    Serial.print("publish:0 成功:");
    Serial.println(d);
}
void AliyunIoTSDK::send(char *key, float number)
{
    char param[128];
    sprintf(param, "{\"%s\":%f}", key, number);
    send(param);
}
void AliyunIoTSDK::send(char *key, int number)
{
    char param[128];
    sprintf(param, "{\"%s\":%d}", key, number);
    send(param);
}
void AliyunIoTSDK::send(char *key, double number)
{
    char param[128];
    sprintf(param, "{\"%s\":%f}", key, number);
    send(param);
}

void AliyunIoTSDK::send(char *key, char *text)
{
    char param[1024];
    sprintf(param, "{\"%s\":\"%s\"}", key, text);
    send(param);
}

int AliyunIoTSDK::bindData(char *key, poniter_fun fp)
{
    int i;
    for (i = 0; i < DATA_CALLBACK_SIZE; i++)
    {
        if (!poniter_array[i].fp)
        {
            poniter_array[i].key = key;
            poniter_array[i].fp = fp;
            return 0;
        }
    }
    return -1;
}

int AliyunIoTSDK::unbindData(char *key)
{
    int i;
    for (i = 0; i < DATA_CALLBACK_SIZE; i++)
    {
        if (!strcmp(poniter_array[i].key, key))
        {
            poniter_array[i].key = NULL;
            poniter_array[i].fp = NULL;
            return 0;
        }
    }
    return -1;
}
