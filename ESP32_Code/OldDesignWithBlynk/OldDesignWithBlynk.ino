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
#define DC_OFFSET 338     //338 steps for 1.65 V offset //*change to 2.5 V offset (512 from ADC)

const double resolution = 5.0 / 1024.0;
const double pf_clk = 2.0 / 3685000.0;
const double frequency = 60.0;

//*Eventually utilize a factor to multiply rms voltage and currents to get actual rms values
const double voltage_factor = 80.0;
const double current_factor = 250.0;

int conv_type = 0;
int pinValue = -1;
int voltage[SAMPLE_SIZE] = {0};
int current1[SAMPLE_SIZE] = {0};
int current2[SAMPLE_SIZE] = {0};
int current3[SAMPLE_SIZE] = {0};
int PF_Timings[PF_RESULT_SIZE] = {0};
double RMS_Voltage = 0;
double RMS_Current1 = 0;
double RMS_Current2 = 0;
double RMS_Current3 = 0;
double RMS_Total_Current = 0;
double PF = 0;
double Real_Power = 0;
double Cost = 0;

//*Input necessary Blynk and WiFi info
char auth[] = "wVqLBlWW1v8qSdj-H5Nt_1fjVV1o7O3C";
char ssid[] = "ItBurnsWhenIP";
char pass[] = "legitRHINO323";

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
  } while(count < 1);
  display.stopscroll();

  Blynk.begin(auth, ssid, pass);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);

  // Make pin 2 default HIGH, and attach INT to our handler
  pinMode(2, INPUT_PULLUP); //*When integrated into circuit may need to be output high w/ no pullup
}

void loop()
{
  Blynk.run();
  
  checkPin();
  
  updateContent();

  rmsCalculations();

  powerCalculations();

  //printResults();

  if (pinValue != -1)
  {
    costCalculations();
    
    pushContent();
    
    display.clearDisplay();
    showUpperRight();
    showContent();
    display.display(); 
  }
  else
  {
    delay(100);
  
    display.clearDisplay();
    showUpperRight();
    showContent();
    //showErrorMessage();
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

        //Uncomment to see ADC results via serial
//      char stringA[5];
//      char stringB[5];
//      char string[11];
//      
//      itoa(sample_num + 1, stringA, 10);
//      itoa(result, stringB, 10);
//
//      strcpy(string, stringA);
//      strcat(string, ":");
//      strcat(string, stringB);
//      
//      Serial.println(string);

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
  const double *factor;
  conv_type = 0;

  //Serial.println("Calculation Values:");

  for(int i = 0; i < 4; i++)
  {
    switch(i)
    {
      case 0:
        adc_values = voltage;
        rms_value = &RMS_Voltage;
        factor = &voltage_factor;
        break;
      case 1:
        adc_values = current1;
        rms_value = &RMS_Current1;
        factor = &current_factor;
        break;
      case 2:
        adc_values = current2;
        rms_value = &RMS_Current2;
        factor = &current_factor;
        break;
      case 3:
        adc_values = current3;
        rms_value = &RMS_Current3;
        factor = &current_factor;
        break;
    }

    double summed_value = 0;
    
    for(int j = 0; j < SAMPLE_SIZE; j++)
    {
      double not_quantized = ((double)*(adc_values + j)) - DC_OFFSET;
      double quantized_value = not_quantized * resolution;

//      Serial.print("Quantized ");
//      Serial.print(j);
//      Serial.print(":");
//      printDouble(quantized_value,1000);

      quantized_value *= quantized_value;
      summed_value += quantized_value;
    }

//    Serial.print("Summed Value:");
//    printDouble(summed_value,1000);
    
    double mean = summed_value / (double)SAMPLE_SIZE;

//    Serial.print("Meaned Value:");
//    printDouble(mean,1000);
    
    *rms_value = sqrt(mean);
    *rms_value *= *factor;
  }

  RMS_Total_Current = (RMS_Current1 + RMS_Current2 + RMS_Current3) / 1000.0;
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
