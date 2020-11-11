#include <Wire.h>
#include <stdlib.h>

void setup()
{
  // Debug console
  Wire.begin();
  //Serial.begin(9600);
  Serial.begin(500000);
}

void loop()
{
  updateContent();
}

void updateContent()
{
  Wire.requestFrom(0x07, 1);

  while(Wire.available())
  {
    int c = Wire.read();
    Serial.println(c);
  }
  //delay(500);
}
