#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <stdlib.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

char auth[] = "wVqLBlWW1v8qSdj-H5Nt_1fjVV1o7O3C";
char ssid[] = "ItBurnsWhenIP";
char pass[] = "legitRHINO323";

int pinValue = -1;

//*Hard-coded values to show that blynk will reflect
float power[3] = {56, 61, 65};
float cost = 89;
float volts = 77;
float amps = 0.1;
int z = 0;

BLYNK_WRITE(V0) // V0 is the number of Virtual Pin  
{
  pinValue = param.asInt();
}

void checkPin()
{
  BLYNK_WRITE(V0);
}

void setup()
{
  // Debug console
  Serial.begin(9600);
  //Serial.begin(115200);

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

  Blynk.begin(auth, ssid, pass);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);

  // Make pin 2 default HIGH, and attach INT to our handler
  pinMode(2, INPUT_PULLUP);
}

void loop()
{
  Blynk.run();
  checkPin();

  updateContent();//*Currently empty: ESP32 I2C to dsPIC to retrieve power, cost, volts, & amps

  if (pinValue == 1) {
    delay(500);

    pushContent();
    
    display.clearDisplay();
    showUpperRight();
    showContent();
    display.display(); 
  }
  else {
    delay(100);
  
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("Off");
    display.display();
  }
}

void showUpperRight() 
{
  char wifiStrength[12] = "Wi-Fi: ";
  String numberString = String(getStrength(8));
  int i = 1;

  do{
    wifiStrength[6 + i] = numberString[i];
    i++;
  } while (numberString.length() != i);
  
  wifiStrength[9] = '%';
  
  display.setCursor(60, 0);
  display.println(wifiStrength);
}

void showContent() 
{
  char powerArray[10];
  char costArray[10];
  char voltsArray[10];
  char ampsArray[10];

  dtostrf(power[z], 4, 3, powerArray);
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

int getStrength(int points)
{
    long rssi = 0;
    long averageRSSI = 0;
    
    for (int i=0;i < points;i++){
        rssi += WiFi.RSSI();
        delay(20);
    }

   averageRSSI = rssi/points;
   return averageRSSI;
}

void pushContent()
{
  Blynk.virtualWrite(V1, power[z]);
  Blynk.virtualWrite(V2, cost);
  Blynk.virtualWrite(V3, volts);
  Blynk.virtualWrite(V4, amps);
  z = (z + 1) % 3;
}

void updateContent()
{
  Wire.requestFrom(0x07, 1);

  while(Wire.available())
  {
    char c = Wire.read();
    Serial.print(c);
  }
  delay(500);
}
