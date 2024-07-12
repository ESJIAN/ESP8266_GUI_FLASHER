#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h> // 引入ArduinoJson库
                         // 自行备好相关库


// GPIO 2 D1 
#define LED 5  //corresponding to the plate D1 pin

// WiFi
const char *ssid = "04"; // Enter your WiFi name
const char *password = "0124578";  // Enter WiFi password
const char *mahjiongcode="03";  //C++创建字符串不简单
// MQTT Broker

const char *mqtt_broker = "182.92.143.75"; //MQTT服务器的地址


const char *topic = strcat("mahjiong",mahjiongcode);  //定义了麻将机名称，实际要修改此参数的数字值部分
                                              //千万要对准确名字，要不然开不了机器



const char *mqtt_username = strcat(strcat("majiong",mahjiongcode),"_8266"); //定义了链接MQTT服务器的用户名
const char *mqtt_password = "powerstate"; //定义了链接MQTT服务器的用户密码,可以在服务端自定义
const int mqtt_port = 1883; //定义了端口号

long int currentMillis=0;
long int period_ms=0;
long int Task_curentMillis=0; //

bool ledState =false;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {  //setup阶段做一些前提准备
    // Set software serial baud to 115200;
    Serial.begin(115200);
    pinMode(LED,OUTPUT);
    delay(1000); // Delay for stability
    // Connecting to a WiFi network
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to the WiFi network");

    // Connecting to an MQTT broker
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);//传参传入函数本身，把这个函数设置为回调函数，在设备订阅的主题接收到新的消息时执行或者在MQTT重新链接后执行，即cilent.loop的执行
    while (!client.connected()) 
    {
        String client_id = "ESP8266-client";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str()); //client_id.c_str()
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Public EMQX MQTT broker connected");
        } else {
            Serial.print("Failed with state ");
            Serial.print(client.state());
            delay(2000);
        }
    }

    // Publish and subscribe
    client.publish(topic, "hello emqx"); //发布主题
    client.subscribe(topic);             //订阅主题
}
//The parameters of the callback function are passed in automatically 
void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message: ");
    String message;                  //C++ declares string variables
                                     //payload is a pointer of type byte*,and its noumenon type is byte arrays that each of its bype can express a int ranging from 0 to 255
    for (int i = 0; i < length; i++) 
    {
        message += (char) payload[i]; // Convert *byte to string
    }

                                      // 使用ArduinoJson解析JSON字符串
    StaticJsonDocument<200> jsonDoc;  // 假设JSON对象的大小不超过200字节

    deserializeJson(jsonDoc, message);
    Serial.print(message);           //format of the parameters passed in Serial.print is similar to python
                                     //cheak the imformation is whether right
                                     // 检查解析是否成功--进行下一步操作前得验证有没有进行下一步操作的条件
    if (jsonDoc.is<JsonObject>()) {
        JsonObject jsonObj = jsonDoc.as<JsonObject>();
        
                                                     // 假设JSON对象中有一个键值对的键名为"msg"，其值为"on"或"off"
        const char* command = jsonObj["status"];     // 获取"status"字段的值
        signed char period_h=jsonObj["time_on"];     //获取time_on字段的值
                                                     //不能使用单独的char
        //Serial.print("OK\n");
        if (strcmp(command, "on") == 0 && !ledState) 
        {
            Task_curentMillis=millis();
            period_ms=period_h*1000*60*60; //1s=1000ms,Convert the hours to milliseconds
            digitalWrite(LED, HIGH);  // the parameter passed in function digitalWrite is numbers of GPIO
                                      // before you use it you must configure the mode of GPIO
            ledState = true;          //avoid set the led repeatly
            Serial.print("HIGH"+String(Task_curentMillis)); //此print不支持百分号格式符输出

        } else if (strcmp(command, "off") == 0 && ledState) 
        {
            digitalWrite(LED, LOW); // 关闭LED
            ledState = false;
        }
    }


}

void loop()
{
    client.loop();// 周期性处理 MQTT 客户端的网络通信和消息维护，如果有消息则会调用一次回调函数
    delay(100); // Delay for a short period in each loop iteration
    currentMillis=millis();
    //Serial.print(String(currentMillis)+" "+String(Task_curentMillis)+" "+String(period_ms)+" "+String(currentMillis-Task_curentMillis)+"\n");
    if((currentMillis- Task_curentMillis>= period_ms)&&Task_curentMillis!=0) //Pay attention to unit conversion,add condition Task_curentMillis!=0 to avoid pass the if at start
    {
      Serial.print("OK\n"); //此print不支持百分号格式符输出
      digitalWrite(LED, LOW);
      ledState= false;      //record the state of led
      Task_curentMillis=0; //reset taskcurrent timer
      if(Task_curentMillis!=0)
      {
        Serial.print("Reset");
      }
    }
}

//下一步实现装载定时器中断来实现开启的定时功能，继电器控制,IO口控制
//116行的这个主循环中的判断其实是定时器中断的逻辑本质，且你要考虑初始条件引起的误进中断，且中断进程里面得恢复中断初始条件以备下次中断能够正常进入