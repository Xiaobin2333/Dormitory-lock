#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Servo.h>

Servo lockservo;

const char* ssid = "ssid"; //接入wifi
const char* password = "password";

const char* assid = "ssid"; //创建ap接入点，新版本已取消ap模式使esp8266进入睡眠模式降低功耗
const char* asecret = "password";

WiFiServer server(80);

int connCount = 0;
int lockState = 0;
unsigned long previousMillis = 0;
const long interval = 5000; //开锁时长ms

void setup() {
  WiFi.mode(WIFI_STA);
  /*需要ap模式请取消此处注释，使用ap模式将无法进入睡眠模式降低功耗
  //WiFi.mode(WIFI_AP_STA);
  //WiFi.softAP(assid, asecret);  
  */
  WiFi.begin(ssid, password); 
  lockservo.attach(2); //定义舵机接口
  lockservo.write(0); //初始化舵机角度
 
  while (WiFi.waitForConnectResult() != WL_CONNECTED) { //30次连接不上跳出循环
    delay(500);
    if(connCount > 30) {
      break;
    }
    connCount += 1;
  }

  ArduinoOTA.begin();
  server.begin();
}



String prepareHtmlPage(){
  String htmlPage =
     String("HTTP/1.1 200 OK\r\n") +
            "Content-Type: text/html\r\n" +
            "Connection: close\r\n" +
            "\r\n" +

            "<!DOCTYPE html> <html>\n"+
            "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n"+
            "<title>LOCK Control</title>\n"+
            "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n"+
            "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n"+
            ".button {display: block;width: 120px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n"+
            ".button-on {background-color: #5AD2FF;}\n"+
            ".button-on:active {background-color: #DE341E;}\n"+
           
            ".button-off {background-color: #34495e;}\n"+
            ".button-off:active {background-color: #DE341E;}\n"+
            "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n"+
            "</style>\n"+
            "</head>\n"+
            "<body>\n"+

            "<h1>LOCK state: <strong>""" + lockState + """</strong></h1>"
            "<form id='F1' action='LOCKON'><input class='button button-on' type='submit' value='ON' ></form><br>"+
                        
            "</html>" +
            "\r\n";
  return htmlPage;
}

void loop() {
  ArduinoOTA.handle();
  
  WiFiClient client = server.available();
  if (client){
    while (client.connected()){
      if (client.available()){
        String line = client.readStringUntil('\r');
      if (line.indexOf("LOCKON") > 0 && lockState == 0){
        lockservo.write(180);
        delay(100); previousMillis = millis();
        lockState = 1;
        }
      if (line.length() == 1 && line[0] == '\n'){
        client.println(prepareHtmlPage());
        break;
        }
      }
    }
    wifi_set_sleep_type(LIGHT_SLEEP_T);
    delay(1000); //delay单位ms越高越省电建议不要超过5000ms
    client.flush();
  } 
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval && lockState == 1){
    lockservo.write(0);
    delay(100);
    lockState = 0;
    }
}
