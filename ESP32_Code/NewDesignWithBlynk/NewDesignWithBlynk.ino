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

//Defining pins for mux and ADC
#define ADC 34
#define MUX_S0 32 //*Need to look at digital out for mux select bits
#define MUX_S1 33

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//*Input necessary Blynk and WiFi info
char auth[] = "AUTHENTICATION HERE";
char ssid[] = "SSID HERE";
char pass[] = "PASSWORD HERE";

int pinValue = -1;

//*Hard-coded values to show that blynk will reflect
int z = 0;
float power[3] = {56, 61, 65};
float cost = 0;
float volts[3] = {119, 120, 121};
float amps[3] = {5.0, 5.1, 5.0};
float powerNum = 0;

BLYNK_WRITE(V0) // V0 is the number of Virtual Pin  
{
  pinValue = param.asInt(); //Value is written from Blynk as $10^-5/kWH
}

void checkPin()
{
  BLYNK_WRITE(V0);
}

void setup()
{
  Serial.begin(9600);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  Blynk.begin(auth, ssid, pass);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);

  // Make pin 2 default HIGH, and attach INT to our handler
  pinMode(2, INPUT_PULLUP);

  //Make mux select digital out and low
  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  
  digitalWrite(MUX_S0, LOW);
  digitalWrite(MUX_S1, LOW);
}

void loop()
{
  Blynk.run();
  checkPin();
  
  Serial.print("Pin Value:");
  Serial.println(pinValue);

  if(pinValue != -1)
  {
    ReadADC();
    
    delay(500);
    
    pushContent();
    
    display.clearDisplay();
    showUpperRight();
    showContent();
    display.display();
  }
  else
  {
    display.clearDisplay();
    showUpperRight();
    showErrorMessage();
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
  dtostrf(volts[z], 4, 3, voltsArray);
  dtostrf(amps[z], 4, 3, ampsArray);

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

void showErrorMessage()
{
  display.setCursor(18,24);
  display.println("Please input cost");
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
  powerNum = volts[z] * amps[z];
  cost = ((pinValue / 10000) * powerNum) / 1000;
  cost = (cost > 0.0f)? cost : 0.0f;
  
  Blynk.virtualWrite(V1, powerNum);
  Blynk.virtualWrite(V2, cost);
  Blynk.virtualWrite(V3, volts[z]);
  Blynk.virtualWrite(V4, amps[z]);
  z = (z + 1) % 3;
}

void ReadADC() //*Adjust number for each current and voltage
{
  for(int a = 0; a < 4; a++)
  {
    int number = 0;
    //*Change mux here based off of integer a with short delay
    MuxSelect(a);
    delay(10);
    
    for(int i = 0; i < 10; i++)
    {
      number += analogRead(ADC);
      delay(10);
    }

    number /= 10;
    Serial.print("Number:");
    Serial.println(number);
  }
}

void MuxSelect(int select)
{
  switch(select){
    case 1:   //Current 1 read
      digitalWrite(MUX_S0, HIGH);
      digitalWrite(MUX_S1, LOW);
      break;
    case 2:   //Current 2 read
      digitalWrite(MUX_S0, LOW);
      digitalWrite(MUX_S1, HIGH);
      break;
    case 3:   //Current 3 read
      digitalWrite(MUX_S0, HIGH);
      digitalWrite(MUX_S1, HIGH);
      break;
    default:  //Voltage read
      digitalWrite(MUX_S0, LOW);
      digitalWrite(MUX_S1, LOW);
      break;
  }
}
