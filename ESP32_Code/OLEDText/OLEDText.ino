#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <stdlib.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(115200);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  int count = 0;
  do{
    loadingSymbol(); //Until some process is complete
    count++;
  } while(count < 2);
  display.stopscroll();
}

void loop() {
  delay(50);
  
  display.clearDisplay();
  showUpperRight();
  showContent(8.1,23.2,25.3,60.4);
  display.display();
}

void showUpperRight() {
  display.setCursor(60, 0);
  display.println("Wi-Fi: 100%");
}

void showContent(float power, float cost, float volts, float amps) {
  char powerArray[10];
  char costArray[10];
  char voltsArray[10];
  char ampsArray[10];

  dtostrf(power, 4, 3, powerArray);
  dtostrf(cost, 4, 2, costArray);
  dtostrf(volts, 4, 3, voltsArray);
  dtostrf(amps, 4, 3, ampsArray);

  String powerString = powerArray;
  String costString = "$";
  String voltsString = voltsArray;
  String ampsString = ampsArray;

  powerString += " W";
  costString += costArray;
  voltsString += " V";
  ampsString += " mA";
  
  display.setCursor(0,16);
  display.println("Power: ");
  display.println("Cost:");
  display.println("Voltage:");
  display.println("Current:");
  display.setCursor(55,16);
  display.println(powerString);
  display.setCursor(55,24);
  display.println(costString);
  display.setCursor(55,32);
  display.println(voltsString);
  display.setCursor(55,40);
  display.println(ampsString);
}

void loadingSymbol() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Loading");
  display.display();

  display.startscrolldiagright(0x07, 0x00);
  delay(5000);
  display.startscrolldiagleft(0x00, 0x07);
  delay(5000);
}
