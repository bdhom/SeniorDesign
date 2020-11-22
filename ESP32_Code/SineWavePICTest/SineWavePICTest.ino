//Anything denoted with "//*" is TO DO
#include <WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <stdlib.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SAMPLE_SIZE 256
#define PF_RESULT_SIZE 3

const double resolution = 5.0 / 1024.0;
const double pf_clk = 2.0 / 3685000.0;
const double frequency = 60.0;
const int dc_offset = 338; //338 steps for 1.65 V offset //*change to 2.5 V offset

//*Eventually utilize a factor to multiply rms voltage and currents to get actual rms values
const double voltage_factor = 1.0;
const double current_factor = 1.0;

int conv_type = 0;
int voltage[SAMPLE_SIZE] = {0};
int current1[SAMPLE_SIZE] = {0};
int current2[SAMPLE_SIZE] = {0};
int current3[SAMPLE_SIZE] = {0};
int PF_Timings[PF_RESULT_SIZE] = {0};
double RMS_Voltage = 0;
double RMS_Current1 = 0;
double RMS_Current2 = 0;
double RMS_Current3 = 0;
double PF = 0;

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup()
{
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

  //Make pin 2 default HIGH, and attach INT to our handler
  pinMode(2, INPUT_PULLUP);

  Serial.print("Resolution:");
  printDouble(resolution,10000);

  Serial.print("PF Clock:");
  printDouble(pf_clk,10000);

  Serial.print("Frequency:");
  printDouble(frequency,10000);

  Serial.print("DC Offset:");
  Serial.println(dc_offset);
}

void loop()
{
  updateContent();
  
  rmsCalculations();
  
  pfCalculations();

  printResults();
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
  int No_Comm_Num = 0;  //I2C communication failed attempts count
  bool NoComm;
  int *adc_values;

  conv_type = 0;

  Serial.println("ADC Read-In Values:");
  
  while(conv_type < ((4 * SAMPLE_SIZE) + PF_RESULT_SIZE) && No_Comm_Num < 10000)
  {
    NoComm = true;
    
    Wire.requestFrom(0x07, 2);
    while(Wire.available())
    {
      int left = Wire.read();
      int result = Wire.read();
      result += left<<8;

      int sample_num = conv_type % SAMPLE_SIZE;
      
      if(sample_num == 0)
      {
        switch(conv_type / SAMPLE_SIZE)
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

      char stringA[5];
      char stringB[5];
      char string[11];
      
      itoa(sample_num + 1, stringA, 10);
      itoa(result, stringB, 10);

      strcpy(string, stringA);
      strcat(string, ":");
      strcat(string, stringB);
      
      Serial.println(string);

      *(adc_values + sample_num) = result;
      
      NoComm = false;
      conv_type++;
    }

    if(NoComm) No_Comm_Num++;
  }
}

void rmsCalculations()
{
  int *adc_values;
  double *rms_value;
  conv_type = 0;

  Serial.println("Calculation Values:");

  for(int i = 0; i < 4; i++)
  {
    switch(i)
    {
      case 0:
        adc_values = voltage;
        rms_value = &RMS_Voltage;
        break;
      case 1:
        adc_values = current1;
        rms_value = &RMS_Current1;
        break;
      case 2:
        adc_values = current2;
        rms_value = &RMS_Current2;
        break;
      case 3:
        adc_values = current3;
        rms_value = &RMS_Current3;
        break;
    }

    double summed_value = 0;
    
    for(int j = 0; j < SAMPLE_SIZE; j++)
    {
      double not_quantized = ((double)*(adc_values + j)) - dc_offset;
      double quantized_value = not_quantized * resolution;

      Serial.print("Quantized ");
      Serial.print(j);
      Serial.print(":");
      printDouble(quantized_value,1000);

      quantized_value *= quantized_value;
      summed_value += quantized_value;
    }

    Serial.print("Summed Value:");
    printDouble(summed_value,1000);
    
    double mean = summed_value / (double)SAMPLE_SIZE;

    Serial.print("Meaned Value:");
    printDouble(mean,1000);
    
    *rms_value = sqrt(mean);
  }

  RMS_Voltage *= voltage_factor;
  RMS_Current1 *= current_factor;
  RMS_Current2 *= current_factor;
  RMS_Current3 *= current_factor;
}

void pfCalculations()
{
  double pf_time = pf_clk * (double)PF_Timings[0] * (double)PF_Timings[2];

  PF = cos(2.0 * PI * frequency * pf_time);
}

void printResults(void)
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
