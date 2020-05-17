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

//*Teampreture
uint8_t readStatus = 0;
AHT10 myAHT10(AHT10_ADDRESS_0X38);

//*TIMER
elapsedMillis timeElapsed;
unsigned int interval = 20* 1000; //secend *1000(is millisec)

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

//*BLYNK
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
#define EEPROM_SIZE 3 //1.start กี่ครั้ง     \
                      //2.reconnect กี่ครั้ง \
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
    Serial.println(F("\n[SENSOR]:-----AHT10 FAIL-----")); //(F()) save string to flash & keeps dynamic memory free
  }
  else
  {
    Serial.println(F("\n[SENSOR]:-----AHT10 OK-----"));
  }

  //*OLED
  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3c) != false)
  {
    Serial.println(F("[SENSOR]:-----SSD1306 FAIL-----"));
  }
  else
  {
    Serial.println(F("[SENSOR]:-----OLED OK-----"));
  }
  display.display();
  display.clearDisplay();

  //*BLYNK
  Blynk.begin(auth, ssid, pass, serv, 8080);
  if (Blynk.connected() == true)
  {
    Serial.println(F("[APP]:-----BLYNK OK-----"));
    startCountEEPROM();
  }
  else
    Serial.println(F("[APP]:-----BLYNK FAIL TO START-----"));
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
    Serial.println(F("[APP]:-----Blynk Reconnect-----"));
    Blynk.begin(auth, ssid, pass, serv, 8080);
    if (Blynk.connected() == true)

    {
      Serial.println(F("[APP]:-----BLYNK OK-----"));
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

String relayStatus1 = "OFF";
String relayStatus2 = "OFF";
void blynkRead()
{
  Blynk.virtualWrite(V0, Temperature);
  Blynk.virtualWrite(V1, Humidity);
  Blynk.virtualWrite(V20, startCount);
  Blynk.virtualWrite(V21, reconnectCount);
  Blynk.virtualWrite(V3, relayStatus1);
  Blynk.virtualWrite(V4, relayStatus2);
}

//สำคัญสัสๆเลย
void RelayControl()
{
  if (Temperature > 38.5)
  {
    relayStatus1 = "OFF";
    relayStatus2 = "OFF";
    digitalWrite(Relay1, HIGH);
    digitalWrite(Relay2, HIGH);
  }
  if (Temperature < 36.5)
  {
    relayStatus1 = "ON";
    relayStatus2 = "ON";
    digitalWrite(Relay1, LOW);
    digitalWrite(Relay2, LOW);
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
}
String KeyboardIO;
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
}
