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
const bool debugging = true;
double highest_overall_voltage = 0;
double highest_overall_current1 = 0;
double highest_overall_current2 = 0;
double highest_overall_current3 = 0;

const double resolution = 5.0 / 4096.0;
const double sample_size = 256.0;
const double pf_clk = 2.0 / 3685000.0;
const double frequency = 60.0;

//*Eventually utilize a factor to multiply rms voltage and currents to get actual rms values
const double voltage_factor = 80.0;
const double current_factor = 10.0;

int conv_type = 0;
int pinValue = -1;
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
char auth[] = "AUTHENTICATIONS HERE";
char ssid[] = "SSID HERE";
char pass[] = "PASSWORD HERE";

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

  int count = 0;
  do{
    loadingSymbol();
    count++;
  }while(count < 1);
  display.stopscroll();

  if(debugging)
  {
    Serial.println("Debug Mode Active");
    Serial.println(" ");
  }

  //*Comment below line out if stuck at loading on oled. Will get rid of Blynk functionality
  //*Also comment out Blynk.run, checkPin, and pushContent in main loop
  //Blynk.begin(auth, ssid, pass);

  // Make pin 2 default HIGH, and attach INT to our handler
  pinMode(2, INPUT_PULLUP); //*When integrated into circuit may need to be output high w/ no pullup
}

void loop()
{
  //Blynk.run();
  //checkPin();

  if(debugging)
  {
    debugValues();
  }
  
  updateContent();
  rmsCalculations();
  powerCalculations();
  //printResults();

  if (pinValue != -1)
  {
    costCalculations();
    
    //pushContent();
    
    display.clearDisplay();
    //showUpperRight();
    showContent();
    display.display(); 
  }
  else
  {
    delay(100);
  
    display.clearDisplay();
    //showUpperRight();
    showContent();
    //showErrorMessage();
    display.display();
  }
}

void debugValues()
{  
  if(RMS_Voltage > highest_overall_voltage)
  {
    highest_overall_voltage = RMS_Voltage;
    
    Serial.print("Voltage: ");
    Serial.println(RMS_Voltage);
    Serial.print("Offset: ");
    Serial.println(voltage_offset);
    
    printADCValues(voltage);
  }
  
  if(RMS_Current1 > highest_overall_current1)
  {
    highest_overall_current1 = RMS_Current1;
    
    Serial.print("Current 1: ");
    Serial.println(RMS_Current1);
    Serial.print("Offset: ");
    Serial.println(current1_offset);
    
    printADCValues(current1);
  }
  
  if(RMS_Current2 > highest_overall_current2)
  {
    highest_overall_current2 = RMS_Current2;
    
    Serial.print("Current 2: ");
    Serial.println(RMS_Current2);
    Serial.print("Offset: ");
    Serial.println(current2_offset);
    
    printADCValues(current2);
  }
  
  if(RMS_Current3 > highest_overall_current3)
  {
    highest_overall_current3 = RMS_Current3;
    
    Serial.print("Current 3: ");
    Serial.println(RMS_Current3);
    Serial.print("Offset: ");
    Serial.println(current3_offset);
    
    printADCValues(current3);
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

void showErrorMessage()
{
  display.setCursor(18,24);
  display.println("Please input cost");
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
  Blynk.virtualWrite(V1, Real_Power);
  Blynk.virtualWrite(V2, Cost);
  Blynk.virtualWrite(V3, RMS_Voltage);
  Blynk.virtualWrite(V4, RMS_Total_Current);
}

void updateContent()
{  
  int No_Comm_Num = 0;  //I2C communication failed attempts count
  bool NoComm;
  int *adc_values;

  conv_type = 0;

  //Serial.println("ADC Read-In Values:");
  
  while(conv_type < ((4 * SAMPLE_SIZE) + PF_RESULT_SIZE) && No_Comm_Num < 10000)
  {
    uint8_t sample_num = conv_type % SAMPLE_SIZE;
    uint8_t conversion = conv_type / SAMPLE_SIZE;
    uint8_t transmit_data[2] = {conversion,sample_num};
    NoComm = true;

    Wire.setClock(10000);

//    Serial.println("Conversion");
//    Serial.println(conversion);
//    Serial.println(sample_num);

    do{
      Wire.beginTransmission(0x07);
      Wire.write(transmit_data,2);
    }while(Wire.endTransmission());
  
    delay(2);
    
    Wire.requestFrom(0x07, 2);
    while(Wire.available())
    {
      uint16_t left = Wire.read();
      uint16_t result = Wire.read();
      result += left << 8;
      
      if(sample_num == 0)
      {
        switch(conversion)
        {
          case 0:
            adc_values = voltage;
            break;
          case 1:
            adc_values = current1;
            break;
          case 2:
            adc_values = current2;
            break;
          case 3:
            adc_values = current3;
            break;
          case 4:
            adc_values = PF_Timings;
            break;
        }
      }

        //Uncomment to see ADC results via serial
//      char stringA[8];
//      char stringB[8];
//      char string[17];
//      
//      itoa(sample_num + 1, stringA, 10);
//      itoa(result, stringB, 10);
//
//      strcpy(string, stringA);
//      strcat(string, ":");
//      strcat(string, stringB);
//      
//      Serial.println(string);

      result = (result > 4095)? 4095 : result;
      *(adc_values + sample_num) = (int)result;
      
      NoComm = false;
      conv_type++;

      delay(1);
    }

    if(NoComm)
    { 
      No_Comm_Num++;
      Serial.println("I2C No Comm");
    }
  }
}

void offsetCalculation(int *adc_array,int array_size,double *offset)
{
  *offset = 0;
  
  for(int i = 0;i < array_size;i++)
  {
    *offset += (double)(*(adc_array + i));
  }

  *offset /= (double)array_size;
}

void rmsCalculations()
{
  int delay_time = 2;

  offsetCalculation(voltage,SAMPLE_SIZE,&voltage_offset);
  offsetCalculation(current1,SAMPLE_SIZE,&current1_offset);
  offsetCalculation(current2,SAMPLE_SIZE,&current2_offset);
  offsetCalculation(current3,SAMPLE_SIZE,&current3_offset);
  
  //For voltage rms
  bool test0 = false;
  double v_summed_value = 0;
  for(int j = 0; j < SAMPLE_SIZE; j++)
  {    
    double not_quantized = ((double)voltage[j]) - voltage_offset;
    double quantized_value = not_quantized * resolution;
    
    quantized_value *= quantized_value;
    v_summed_value += quantized_value;
  }

  double v_mean = v_summed_value / sample_size;
  RMS_Voltage = sqrt(v_mean);
  RMS_Voltage *= voltage_factor;

  //For current 1 rms
  bool test1 = false;
  double c1_summed_value = 0;
  for(int j = 0; j < SAMPLE_SIZE; j++)
  {    
    double not_quantized = ((double)current1[j]) - current1_offset;
    double quantized_value = not_quantized * resolution;
    
    quantized_value *= quantized_value;
    c1_summed_value += quantized_value;
  }

  double c1_mean = c1_summed_value / sample_size;
  //c1_mean -= 0.00008;
  c1_mean = (c1_mean < 0.0)? 0.0 : c1_mean;
  RMS_Current1 = sqrt(c1_mean);
  RMS_Current1 *= current_factor;

  //For current 2 rms
  bool test2 = false;
  double c2_summed_value = 0;
  for(int j = 0; j < SAMPLE_SIZE; j++)
  {    
    double not_quantized = ((double)current2[j]) - current2_offset;
    double quantized_value = not_quantized * resolution;
    
    quantized_value *= quantized_value;
    c2_summed_value += quantized_value;
  }

  double c2_mean = c2_summed_value / sample_size;
  c2_mean = (c2_mean < 0.0)? 0.0 : c2_mean;
  RMS_Current2 = sqrt(c2_mean);
  RMS_Current2 *= current_factor;

  //For current 3 rms
  bool test3 = false;
  double c3_summed_value = 0;
  for(int j = 0; j < SAMPLE_SIZE; j++)
  {
    double not_quantized = ((double)current3[j]) - current3_offset;
    double quantized_value = not_quantized * resolution;
    
    quantized_value *= quantized_value;
    c3_summed_value += quantized_value;
  }

  double c3_mean = c3_summed_value / sample_size;
  c3_mean = (c3_mean < 0.0)? 0.0 : c3_mean;
  RMS_Current3 = sqrt(c3_mean);
  RMS_Current3 *= current_factor;
  
  RMS_Total_Current = RMS_Current1 + RMS_Current2 + RMS_Current3;
  //RMS_Total_Current -= 0.20; //Quantization Error

  RMS_Total_Current = (RMS_Total_Current < 0.0)? 0.0 : RMS_Total_Current;
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

void printResults()
{
  Serial.println(" ");
  Serial.println("Voltage:");
  printDouble(RMS_Voltage,1000);
  
  Serial.println("Current1:");
  printDouble(RMS_Current1,1000);

  Serial.println("Current2:");
  printDouble(RMS_Current2,1000);

  Serial.println("Current3:");
  printDouble(RMS_Current3,1000);

  Serial.println("Power Factor:");
  printDouble(PF,1000);
}

//Used for serial printing double values
void printDouble( double val, unsigned int precision)
{
   Serial.print (int(val));  //prints the int part
   Serial.print("."); // print the decimal point
   unsigned int frac;
   if(val >= 0)
       frac = (val - int(val)) * precision;
   else
       frac = (int(val)- val ) * precision;
   Serial.println(frac,DEC) ;
}

void printADCValues(int *integer_array)
{
  for(int j = 0; j < SAMPLE_SIZE; j++)
  {
    Serial.println(*(integer_array + j));
  }
}
