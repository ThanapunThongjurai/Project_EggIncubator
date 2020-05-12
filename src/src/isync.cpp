#include <WiFi.h>
#include <iSYNC.h>

WiFiClient client;
iSYNC iSYNC(client);

String ssid = "SSID"; //ชื่อ WIFI ที่ต้องการให้ ESP32 เชื่อมต่อ
String pass = ""; //รหัส WIFI

String iSYNC_USERNAME = "NamelessKingMeow"; //ใส่ USERNAME ของผู้ใช้งาน
String iSYNC_KEY = "5eba39baed95c1464946b9f2"; //ใส่ ISYNC KEY ที่ได้รับมา
String iSYNC_AUTH = "5eba3949ed95c1464946b87f"; //ใส่ ISYNC AUTH (Project Auth)

void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  Serial.print("[iSYNC]-> ");
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
    Serial.print((char)payload[i]);
  }
  Serial.println();
  if (msg.startsWith("LINE:"))msg = msg.substring(5);

  /* command control */
  if (msg.equals("ปิดไฟเบอร์ 1")) {
    digitalWrite(2, LOW);
    iSYNC.mqPub(iSYNC_KEY, "ปิดไฟเบอร์ 1 LED on board แล้วค่ะเจ้านาย");  //Publish
  } else if (msg.equals("เปิดไฟเบอร์ 1")) {
    digitalWrite(2, HIGH);
    iSYNC.mqPub(iSYNC_KEY, "เปิดไฟเบอร์ 1 LED on board แล้วค่ะเจ้านาย");  //Publish
  } else if (msg.equals("สวัสดี")) {
    iSYNC.mqPub(iSYNC_KEY, "สวัสดีค่ะเจ้านาย");  //Publish
  } else if (msg.equals("เปิดไฟเบอร์ 2")) {
    digitalWrite(15, HIGH);
    iSYNC.mqPub(iSYNC_KEY, "เปิดไฟเบอร์ 2 แล้วค่ะเจ้านาย");  //Publish
  } else if (msg.equals("ปิดไฟเบอร์ 2")) {
    digitalWrite(15, LOW);
    iSYNC.mqPub(iSYNC_KEY, "ปิดไฟเบอร์ 2 แล้วค่ะเจ้านาย");  //Publish
  }
}

void connectMQTT() {
  while (!iSYNC.mqConnect()) {
    Serial.println("Reconnect MQTT...");
    delay(3000);
  }
  iSYNC.mqPub(iSYNC_KEY, "พร้อมรับคำสั่งแล้วค่ะเจ้านาย");  //Publish on Connect
  // iSYNC.mqSubProject(); //subscribe all key in your project
  iSYNC.mqSub(iSYNC_KEY); //subscribe key
}

void setup() {
  Serial.begin(115200);
  Serial.println(iSYNC.getVersion());

  pinMode(2, OUTPUT); // led on/off set output
  pinMode(15, OUTPUT);

  iSYNC.begin(ssid, pass);
  iSYNC.mqInit(iSYNC_USERNAME, iSYNC_AUTH);
  iSYNC.MQTT->setCallback(callback);
  connectMQTT();
}

void loop() {
  if (!iSYNC.mqConnected())connectMQTT();
  iSYNC.mqLoop();
}