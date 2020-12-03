#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <stdlib.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define SAMPLE_SIZE 256
#define PF_RESULT_SIZE 3

//Debugging purposes
bool debugging = true;  //false is faster
double highest_overall_voltage = 0;
double highest_overall_current1 = 0;
double highest_overall_current2 = 0;
double highest_overall_current3 = 0;
double highest_overall_PF = 0;

//PIC Constants
const double resolution = 5.0 / 4096.0;
const double pf_clk = 2.0 / 3685000.0;
const double frequency = 60.0;

//*Eventually utilize a factor to multiply rms voltage and currents to get actual rms values
const double voltage_factor = 80.0;
const double current_factor = 10.0;

int pinValue = -1;
int conv_type = 0;
int voltage[SAMPLE_SIZE] = {0};
int current1[SAMPLE_SIZE] = {0};
int current2[SAMPLE_SIZE] = {0};
int current3[SAMPLE_SIZE] = {0};
int PF_Timings[PF_RESULT_SIZE] = {0};
double voltage_offset = 0;
double current1_offset = 0;
double current2_offset = 0;
double current3_offset = 0;
double RMS_Voltage = 0;
double RMS_Current1 = 0;
double RMS_Current2 = 0;
double RMS_Current3 = 0;
double RMS_Total_Current = 0;
double PF = 0;
double Real_Power = 0;
double Cost = 0;

//*Input necessary Blynk and WiFi info
bool runBlynk = false;
char auth[] = "wVqLBlWW1v8qSdj-H5Nt_1fjVV1o7O3C";
char ssid[] = "ItBurnsWhenIP";
char pass[] = "legitRHINO323";

//char auth[] = "AUTHENTICATIONS HERE";
//char ssid[] = "SSID HERE";
//char pass[] = "PASSWORD HERE";

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

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
  Serial.begin(9600);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  int loading_count = 1;
  for(int i = 0;i < loading_count;i++)
  {
    loadingSymbol();
  }
  display.stopscroll();

  if(debugging)
  {
    Serial.println(" ");
    Serial.println("Debug Mode Active");
    Serial.println(" ");
  }

  if(runBlynk)
  {
    Blynk.begin(auth, ssid, pass);
  }
  
  pinMode(2, INPUT_PULLUP); //*When integrated into circuit may need to be output high w/ no pullup
}

void loop()
{
  if(runBlynk)
  {
    Blynk.run();
    checkPin();
  }
  
  debugValues();
  updateContent();
  rmsCalculations();
  powerCalculations();
  
  if (pinValue != -1)
  {
    costCalculations();

    display.clearDisplay();
    pushContent();
    showWifiStrength();
    showContent();
    display.display(); 
  }
  else
  {
    display.clearDisplay();
    showWifiStrength();
    showContent();
    display.display();
  }
}

void debugValues()
{
  if(!debugging)
    return;
    
  if(RMS_Voltage > highest_overall_voltage)
  {
    highest_overall_voltage = RMS_Voltage;
    
    Serial.print("Voltage: ");
    Serial.println(RMS_Voltage);
    Serial.print("Offset: ");
    Serial.println(voltage_offset);
    
    printArrayValues(voltage,SAMPLE_SIZE);
  }
  
  if(RMS_Current1 > highest_overall_current1)
  {
    highest_overall_current1 = RMS_Current1;
    
    Serial.print("Current 1: ");
    Serial.println(RMS_Current1);
    Serial.print("Offset: ");
    Serial.println(current1_offset);
    
    printArrayValues(current1,SAMPLE_SIZE);
  }
  
  if(RMS_Current2 > highest_overall_current2)
  {
    highest_overall_current2 = RMS_Current2;
    
    Serial.print("Current 2: ");
    Serial.println(RMS_Current2);
    Serial.print("Offset: ");
    Serial.println(current2_offset);
    
    printArrayValues(current2,SAMPLE_SIZE);
  }
  
  if(RMS_Current3 > highest_overall_current3)
  {
    highest_overall_current3 = RMS_Current3;
    
    Serial.print("Current 3: ");
    Serial.println(RMS_Current3);
    Serial.print("Offset: ");
    Serial.println(current3_offset);
    
    printArrayValues(current3,SAMPLE_SIZE);
  }

  if(PF > highest_overall_PF)
  {
    highest_overall_PF = PF;

    Serial.print("PF: ");
    Serial.println(PF);

    printArrayValues(PF_Timings,PF_RESULT_SIZE);
  }
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

void showWifiStrength() 
{
  if(runBlynk)
  {
    char wifiStrength[12] = "Wi-Fi: ";
    String numberString = String(getWifiStrength(8));
    int i = 1;
  
    do{
      wifiStrength[6 + i] = numberString[i];
      i++;
    } while (numberString.length() != i);
    
    wifiStrength[9] = '%';
    
    display.setCursor(60, 0);
    display.println(wifiStrength);
  }
}

int getWifiStrength(int points)
{
    int rssi = 0;
    
    for (int i=0;i < points;i++){
        rssi += WiFi.RSSI();
        delay(20);
    }

   return (rssi/points);
}

void showContent() 
{
  char powerArray[10];
  char costArray[10];
  char voltsArray[10];
  char ampsArray[10];
  String costString;

  dtostrf(Real_Power, 4, 3, powerArray);
  dtostrf(RMS_Voltage, 4, 3, voltsArray);
  dtostrf(RMS_Total_Current, 4, 3, ampsArray);

  String powerString = powerArray;
  String voltsString = voltsArray;
  String ampsString = ampsArray;

  powerString += " W";
  voltsString += " V";
  ampsString += " A";

  if(Cost <= 0)
  {
    costString = "Input Cost";
  }
  else
  {
    dtostrf(Cost, 4, 2, costArray);
    
    costString = "$";
    costString += costArray;
  }
  
  display.setCursor(0,16);
  display.println("Power: ");
  display.println("Hour Cost:");
  display.println("Voltage:");
  display.println("Current:");
  display.setCursor(59,16);
  display.println(powerString);
  display.setCursor(59,24);
  display.println(costString);
  display.setCursor(59,32);
  display.println(voltsString);
  display.setCursor(59,40);
  display.println(ampsString);
}

void pushContent()
{
  if(runBlynk)
  {
    Blynk.virtualWrite(V1, Real_Power);
    Blynk.virtualWrite(V2, Cost);
    Blynk.virtualWrite(V3, RMS_Voltage);
    Blynk.virtualWrite(V4, RMS_Total_Current);
  }
}

bool getI2CArray(int *adc_array,int array_size,uint8_t conv_num)
{
  int I2C_No_Comm = 0;
  int sample_num = 0;
  bool complete = false;

  if(debugging)
  {
    Serial.print("Conversion Number: ");
    Serial.println(conv_num);
  }

  while(sample_num < array_size)
  {
    bool no_comm = true;
    uint8_t transmit_data[2] = {conv_num,(uint8_t)sample_num};

    do{
      Wire.beginTransmission(0x07);
      Wire.write(transmit_data,2);
    }while(Wire.endTransmission());
    
    Wire.requestFrom(0x07,2);
    while(Wire.available())
    {
      uint16_t left = Wire.read();
      uint16_t result = Wire.read();
      result += left << 8;
      result = (result > 4095)? 4095 : result;
      *(adc_array + sample_num) = (int)result;
      no_comm = false;
      sample_num++;
    }

    if(no_comm) I2C_No_Comm++;
    
    if(sample_num >= (array_size - 1)) complete = true;
    
    if(I2C_No_Comm > 10000) break;
  }
  
  return complete;
}

void updateContent()
{  
  bool complete = getI2CArray(voltage,SAMPLE_SIZE,0);

  if(!complete && debugging)
  {
    Serial.println(" ");
    Serial.println("Voltage array I2C miscommunication");
    Serial.println(" ");
    return;
  }

  complete = getI2CArray(current1,SAMPLE_SIZE,1);

  if(!complete && debugging)
  {
    Serial.println(" ");
    Serial.println("Current 1 array I2C miscommunication");
    Serial.println(" ");
    return;
  }
  
  complete = getI2CArray(current2,SAMPLE_SIZE,2);

  if(!complete && debugging)
  {
    Serial.println(" ");
    Serial.println("Current 2 array I2C miscommunication");
    Serial.println(" ");
    return;
  }
  
  complete = getI2CArray(current3,SAMPLE_SIZE,3);

  if(!complete && debugging)
  {
    Serial.println(" ");
    Serial.println("Current 3 array I2C miscommunication");
    Serial.println(" ");
    return;
  }

  complete = getI2CArray(PF_Timings,PF_RESULT_SIZE,4);

  if(!complete && debugging)
  {
    Serial.println(" ");
    Serial.println("PF array I2C miscommunication");
    Serial.println(" ");
    return;
  }
}

void calculateOffset(int *adc_array,int array_size,double *offset)
{
  *offset = 0;
  
  for(int i = 0;i < array_size;i++)
  {
    *offset += (double)(*(adc_array + i));
  }

  *offset /= (double)array_size;
}

void calculateRMS(int *adc_array,int array_size,double offset,double *RMS,double factor)
{
  double summed_value = 0;
  
  for(int i = 0;i < array_size;i++)
  {
    double not_quantized = ((double)*(adc_array + i)) - offset;
    double quantized_value = not_quantized * resolution;
    
    quantized_value *= quantized_value;
    summed_value += quantized_value;
  }

  double mean = summed_value / (double)array_size;
  *RMS = sqrt(mean);
  *RMS *= factor;
}

void rmsCalculations()
{
  calculateOffset(voltage,SAMPLE_SIZE,&voltage_offset);
  calculateOffset(current1,SAMPLE_SIZE,&current1_offset);
  calculateOffset(current2,SAMPLE_SIZE,&current2_offset);
  calculateOffset(current3,SAMPLE_SIZE,&current3_offset);

  calculateRMS(voltage,SAMPLE_SIZE,voltage_offset,&RMS_Voltage,voltage_factor);
  calculateRMS(current1,SAMPLE_SIZE,current1_offset,&RMS_Current1,current_factor);
  calculateRMS(current2,SAMPLE_SIZE,current2_offset,&RMS_Current2,current_factor);
  calculateRMS(current3,SAMPLE_SIZE,current3_offset,&RMS_Current3,current_factor);

  RMS_Total_Current = RMS_Current1 + RMS_Current2 + RMS_Current3;
}

void powerCalculations()
{
  double pf_time = pf_clk * (double)PF_Timings[0] * (double)PF_Timings[2];
  PF = cos(2.0 * PI * frequency * pf_time);
  Real_Power = RMS_Voltage * RMS_Total_Current * PF;
}

void costCalculations()
{
  Cost = (pinValue / 10000.0) * (Real_Power / 1000.0);
  Cost = (Cost > 0.0)? Cost : 0.0;
}

void printArrayValues(int *integer_array,int array_size)
{
  for(int j = 0;j < array_size;j++)
  {
    Serial.println(*(integer_array + j));
  }
}
