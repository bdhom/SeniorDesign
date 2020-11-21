#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <stdlib.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int sineValues[256];
int convType = 0;

void setup()
{
  float conversionFactor = (2*PI)/256;

  float RadAngle;
  for(int Angle = 0; Angle < 256; Angle++)
  {
    RadAngle = Angle * conversionFactor;
    sineValues[Angle] = (sin(RadAngle)*127) + 128;
  }
  
  Serial.begin(9600);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  int count = 0;
  do
  {
    loadingSymbol(); //Until some process is complete
    count++;
  } while(count < 2);
  display.stopscroll();

  // Make pin 2 default HIGH, and attach INT to our handler
  pinMode(2, INPUT_PULLUP); //*When integrated into circuit will need to be output high w/ no pullup
}

void loop() //*Connect signal generator up for better results
{
//  for(int x = 0; x < 2; x++)
//  {
//    for(int i = 0; i < 255; i++)
//    {
//      //delay(55); //delayMicroseconds(55);
//      //dacWrite(25,sineValues[i]);
//      updateContent();
//    }
//  }
  convType = 0;
  
  updateContent();
}

void loadingSymbol()
{
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Loading");
  display.display();

  display.startscrolldiagright(0x07, 0x00);
  delay(5000);
  display.startscrolldiagleft(0x00, 0x07);
  delay(5000);
}

void updateContent()
{  
  int No_Comm_Num = 0;
  bool NoComm;
  
  while(convType < 1027 && No_Comm_Num < 10000)
  {
    NoComm = true;
    Wire.requestFrom(0x07, 2);
  
    while(Wire.available())
    {

      int a = Wire.read();
      int b = Wire.read();
    
      int c = a<<8;
      c += b;
      
      if((convType % 256) == 0)
      {
        switch(convType / 256)
        {
          case 0:
            Serial.println(' ');
            Serial.println("Voltage");
            break;
          case 1:
            Serial.println(' ');
            Serial.println("Current 1");
            break;
          case 2:
            Serial.println(' ');
            Serial.println("Current 2");
            break;
          case 3:
            Serial.println(' ');
            Serial.println("Current 3");
            break;
          case 4:
            Serial.println(' ');
            Serial.println("PF Timings");
            break;
        }
      }
  
      char stringA[5];
      char stringB[5];
      char string[11];
      
      itoa((convType % 256) + 1, stringA, 10);
      itoa(c, stringB, 10);

      strcpy(string, stringA);
      strcat(string, ":");
      strcat(string, stringB);
      
      Serial.println(string);

      NoComm = false;
      convType++;
    }

    if(NoComm) No_Comm_Num++;
  }
}
