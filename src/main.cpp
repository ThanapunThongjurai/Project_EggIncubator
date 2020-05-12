//** จะไม่มีการ ใช้ ESP32.reset เด็ดขาดทุกอย่างต้องทำงานแยกกัน 
//** no  delay
#include <Arduino.h>
#include <elapsedMillis.h>
#include <Wire.h>
#include <AHT10.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <EEPROM.h>
#include <iSYNC.h>



WiFiClient client;
iSYNC iSYNC(client);


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
  /*
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
  */
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

//*Teampreture
uint8_t readStatus = 0;
AHT10 myAHT10(AHT10_ADDRESS_0X38);

//*TIMER
elapsedMillis timeElapsed;
unsigned int interval = 60 * 1000;//secend *1000(is millisec)

//*GPIO_SETTING
#define Relay1 16
#define Relay2 17
#define LedBulidIn 2
//#define Fan 15

//*OLED
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET 4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//*BLYNK + *iSYNC
char auth[] = "uQE9tc0pxF3kvNageuLAk9ifvoVbElpi";
char ssid[] = "SSID";
char pass[] = "";
char serv[] = "blynk.ecp.in.th";

//*GOBAL_VAR
float Temperature = 0;
float Humidity = 0;
float Count = 0;

//*EEPROM
//ต้องใช้ code รี eeprom
#define EEPROM_SIZE 3 //1.start กี่ครั้ง     
                      //2.reconnect กี่ครั้ง 
                      //
int startCount = 0;
int reconnectCount = 0;
void startCountEEPROM()
{
  startCount = EEPROM.read(0);
  startCount++;
  EEPROM.write(0, startCount);
  EEPROM.commit();
}
void reconnectCountEEPROM()
{
  reconnectCount = EEPROM.read(1);
  reconnectCount++;
  EEPROM.write(1, reconnectCount);
  EEPROM.commit();
}

void connnect2Sensor()
{
  //*AHT10
  if (myAHT10.begin() != true)
  {
    Serial.println(F("-----AHT10 FAIL-----")); //(F()) save string to flash & keeps dynamic memory free
  }
  else
  {
    Serial.println(F("-----AHT10 OK-----"));
  }

  //*OLED
  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3c) != false)
  {
    Serial.println(F("-----SSD1306 FAIL-----"));
  }
  else
  {
    Serial.println(F("-----OLED OK-----"));
  }
  display.display();
  display.clearDisplay();

  //*BLYNK
  Blynk.begin(auth, ssid, pass, serv, 8080);
  if (Blynk.connected() == true)
  {
    Serial.println(F("-----BLYNK OK-----"));
    startCountEEPROM();
  }
  else
    Serial.println(F("-----BLYNK FAIL TO START-----"));
}





void Reconnect()
{
  //*ใช้ i2c scanner เช็คเอาก็ได้นะแต่ขี้เกียจละ

  if (readStatus == AHT10_ERROR)
  {
    Serial.print(F("ATH10 softReset Process: "));
    Serial.println(myAHT10.softReset()); //reset 1-success, 0-failed
    myAHT10.softReset();
    Temperature = 0;
    Humidity = 0;
  }
  if (Blynk.connected() != true)
  {
    Serial.println(F("-----Blynk Reconnect-----"));
    Blynk.begin(auth, ssid, pass, serv, 8080);
    if (Blynk.connected() == true)

    {
      Serial.println(F("-----BLYNK OK-----"));
      reconnectCountEEPROM();
    }
    else
      Serial.println(F("-----BLYNK FAIL TO START-----"));
  }
}

void WorldRead()
{
  readStatus = myAHT10.readRawData(); //read 6 bytes from AHT10 over I2C
  if (readStatus != AHT10_ERROR)
  {
    //Serial.print(F("Temperature: ")); Serial.print(myAHT10.readTemperature(AHT10_USE_READ_DATA)); Serial.println(F(" +-0.3C"));
    //Serial.print(F("Humidity...: ")); Serial.print(myAHT10.readHumidity(AHT10_USE_READ_DATA));    Serial.println(F(" +-2%"));
    Temperature = (float)myAHT10.readTemperature(AHT10_USE_READ_DATA);
    Humidity = (float)myAHT10.readHumidity(AHT10_USE_READ_DATA);
  }
  /*
  else
  {
    //Serial.print(F("Failed to read - reset: "));
    //Serial.println(myAHT10.softReset());//reset 1-success, 0-failed
    //myAHT10.softReset();
  }
  */
}

//แสดงผล
void OLED()
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 20);
  display.print("T ");
  display.print(Temperature);
  display.println("C");
  display.print("H ");
  display.print(Humidity);
  display.println("RH%");
  display.display();
}

void data2comport()
{
  Serial.print(F("Temperature\t: "));
  Serial.print(Temperature);
  Serial.println(F(" +-0.3C"));
  Serial.print(F("Humidity...\t: "));
  Serial.print(Humidity);
  Serial.println(F(" +-2%"));
}

boolean relayStatus1 = false;
boolean relayStatus2 = false;
void blynkRead()
{
  Blynk.virtualWrite(V0, Temperature);
  Blynk.virtualWrite(V1, Humidity);
  Blynk.virtualWrite(V20, startCount);
  Blynk.virtualWrite(V21, reconnectCount);
  if (relayStatus1 == true)
  {
    Blynk.virtualWrite(V3, "ON");
  }
  else
  {
    Blynk.virtualWrite(V3, "OFF");
  }
  if (relayStatus2 == true)
  {
    Blynk.virtualWrite(V4, "ON");
  }
  else
  {
    Blynk.virtualWrite(V4, "OFF");
  }
}

//สำคัญสัสๆเลย
void RelayControl()
{
  if (Temperature > 38.5)
  {
    relayStatus1 = false;
    relayStatus2 = false;
    digitalWrite(Relay1, HIGH);
    digitalWrite(Relay2, HIGH);
    //digitalWrite(LedBulidIn,HIGH);
  }
  if (Temperature < 36.5)
  {
    relayStatus1 = true;
    relayStatus2 = true;
    digitalWrite(Relay1, LOW);
    digitalWrite(Relay2, LOW);
    //digitalWrite(LedBulidIn,LOW);
  }
}


void setup()
{
  //*EEPROM
  EEPROM.begin(EEPROM_SIZE);

  //*GPIO
  pinMode(Relay1, OUTPUT);
  pinMode(Relay2, OUTPUT);
  //pinMode(Fan,OUTPUT);
  pinMode(LedBulidIn, OUTPUT);
  digitalWrite(Relay1, HIGH);
  digitalWrite(Relay2, HIGH);
  //digitalWrite(Fan,HIGH);
  digitalWrite(LedBulidIn, HIGH);

  Serial.begin(115200);
  //Wire.setClock(400000); //experimental I2C speed! 400KHz, default 100KHz
  
  

  //*AHT10 and OLED connection
  connnect2Sensor();

  //*WorldRead();
  readStatus = myAHT10.readRawData(); //read 6 bytes from AHT10 over I2C
  if (readStatus != AHT10_ERROR)
  {
    //Serial.print(F("Temperature: ")); Serial.print(myAHT10.readTemperature(AHT10_USE_READ_DATA)); Serial.println(F(" +-0.3C"));
    //Serial.print(F("Humidity...: ")); Serial.print(myAHT10.readHumidity(AHT10_USE_READ_DATA));    Serial.println(F(" +-2%"));
    Temperature = (float)myAHT10.readTemperature(AHT10_USE_READ_DATA);
    Humidity = (float)myAHT10.readHumidity(AHT10_USE_READ_DATA);
  }

  //*iSYNC
  Serial.println(iSYNC.getVersion());
  iSYNC.begin(ssid, pass);
  iSYNC.mqInit(iSYNC_USERNAME, iSYNC_AUTH);
  iSYNC.MQTT->setCallback(callback);
  connectMQTT();
}

void loop()
{
  //TIME
  if (timeElapsed > interval)
  {
    timeElapsed = 0;
  }

  WorldRead();    //-----สำคัณเหนือสิงอื่นได
  RelayControl(); //-----สำคัณเหนือสิงอื่นได

  OLED();
  //data2comport();
  
  Blynk.run();
  blynkRead();
  Reconnect(); //เพราะมี blynk อยู่ ใช้  reconnect sensor ด้วย

  //*iSYNC
  if (!iSYNC.mqConnected())connectMQTT();
  iSYNC.mqLoop();
}