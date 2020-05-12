t#include <Arduino.h>
#include <elapsedMillis.h>
#include <Wire.h>
#include <AHT10.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Teampreture
uint8_t readStatus = 0;
AHT10 myAHT10(AHT10_ADDRESS_0X38);
//TIMER
elapsedMillis timeElapsed;
unsigned int interval = 10*1000;
//RELAY_SETTING
#define Relay1 16
#define Relay2 17
#define LedBulidIn 2
#define Fan 15
//OLED
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
  // Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

float Temperature = 0;
float Humidity = 0;
int Count = 0;

void setup()
{
  //Relay
  pinMode(Relay1,OUTPUT);
  pinMode(Relay2,OUTPUT);
  pinMode(Fan,OUTPUT);
  pinMode(LedBulidIn,OUTPUT);
  digitalWrite(Relay1,HIGH);
  digitalWrite(Relay2,HIGH);
  digitalWrite(Fan,HIGH);
  digitalWrite(LedBulidIn,HIGH);
  //SENSOR_Teampreture 
  Serial.begin(115200);
  Serial.println(); 
  while (myAHT10.begin() != true)
  {
    Serial.println(F("AHT10 not connected or fail to load calibration coefficient")); //(F()) save string to flash & keeps dynamic memory free
    delay(5000);
  }
  Serial.println(F("AHT10 OK"));
  //OLED
  //display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
  while (display.begin(SSD1306_SWITCHCAPVCC, 0x3c) != true)
  {
    Serial.println(F("SSD1306 allocation failed"));
    delay(5000);
  }
  display.display();
  display.clearDisplay();
  Serial.println(F("OLED OK"));
  
    //Wire.setClock(400000); //experimental I2C speed! 400KHz, default 100KHz
  

  WorldRead();
}

void Reconnect()
{
  
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
    else
    {
      Serial.print(F("Failed to read - reset: ")); 
      Serial.println(myAHT10.softReset());         //reset 1-success, 0-failed
    }
    Serial.print(F("Temperature: ")); Serial.print(Temperature); Serial.println(F(" +-0.3C"));
    Serial.print(F("Humidity...: ")); Serial.print(Humidity);    Serial.println(F(" +-2%"));
    Serial.println(Count);
    Count++;
}

boolean relayStatus = false;
boolean statusCheck = false;
void RelayControl()
{
    if(Temperature > 38.5)
    {
      relaystatus = false;
      digitalWrite(Relay1,HIGH);
      digitalWrite(Relay2,HIGH);
      //digitalWrite(LedBulidIn,HIGH);

    }
    if(Temperature < 36.5 )
    {
      relayStatus = true;
      digitalWrite(Relay1,LOW);
      digitalWrite(Relay2,LOW);
      //digitalWrite(LedBulidIn,LOW);
    }
}
void OLED()
{
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,20);
    display.print("T ");display.print(Temperature);display.println("C");
    display.print("H ");display.print(Humidity);display.println("RH%");
    display.display();
}

void loop()
{ 
  //TIME
  if (timeElapsed > interval)
  {
    timeElapsed = 0; 
    
  }
  WorldRead();
  RelayControl();
  OLED();
}
