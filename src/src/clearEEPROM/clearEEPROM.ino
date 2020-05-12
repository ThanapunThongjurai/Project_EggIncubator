#include <EEPROM.h>

int count = 0;
void setup() {
  // put your setup code here, to run once:
  EEPROM.begin(5);
  Serial.begin(9600);

  for(int i = 0 ; i < 5;i++)
  {
    EEPROM.write(i,0);
  }
  EEPROM.commit();

}

void loop() {
  // put your main code here, to run repeatedly:
  for(int i = 0 ; i < 10;i++)
  {
    Serial.println(EEPROM.read(i));
    delay(100);
  }
  
}
