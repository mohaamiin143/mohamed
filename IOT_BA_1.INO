// BLYNK APP CONTROL
#define BLYNK_TEMPLATE_ID "TMPL2YT730Sk-"
#define BLYNK_DEVICE_NAME "IOT Water Quality For Fish And Aquaculture System"
#define BLYNK_AUTH_TOKEN "079gBDvJZeAz5g_WAAUHz0vdYEvhrG6B"

// PH declarations
#define PhSensor 35         // the pH meter Analog output is connected with the Arduino’s Analog
unsigned long int avgValue;  //Store the average value of the sensor feedback
float b;
int buf[10],temp;

// Turbidity sensor
int turbiditypin = 34;
int senseData;

#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <SimpleTimer.h>

//------------ water temperature --------
#include <OneWire.h>
#include <DallasTemperature.h>

#define SENSOR_PIN  17 // ESP32 pin GIOP17 connected to DS18B20 sensor's DATA pin

OneWire oneWire(SENSOR_PIN);
DallasTemperature DS18B20(&oneWire);

float temp_C; // temperature in Celsius
float temp_F; // temperature in Fahrenheit

  // LCD DISPLAY LIBRARY
#include <LiquidCrystal_I2C.h> // library for I2C LCD  
LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

//lCD Diclarations
// int SDA - Pin G21
// int SCL - Pin G22
// VCC - 5V INPUT SIDE
// GND - GND 

// PUMP RELAY declarations

int PumpRelay1 = 18;
int PumpRelay2 = 19;

bool eventTrigger;

//------------Indicators of the system -------
int greenled_Pin = 12;
int redled_Pin = 14;
int Activebuzzer_Pin = 27;
//-----------------------------------------

BlynkTimer timer;

//BLYNK APP AND INTENET CONNECTIONS
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "abdi";
char pass[] = "12345678";

void setup()
{
  Serial.begin(9600);
  Serial.println("Ready");    //Test the serial monitor
  // output indicators ------
  pinMode(greenled_Pin, OUTPUT);
  pinMode(redled_Pin, OUTPUT);
  pinMode(Activebuzzer_Pin, OUTPUT);

  // ph SENSOR
  pinMode(PhSensor,INPUT); 
  pinMode(PumpRelay1, OUTPUT);
  pinMode(PumpRelay2, OUTPUT);   
 

  // turbidity SENSOR
  pinMode(turbiditypin, INPUT);
  
  Blynk.begin(auth, ssid, pass);
  
  digitalWrite(PumpRelay1, HIGH);
  digitalWrite(PumpRelay2, HIGH);
  
  timer.setInterval(1000L, Water_Sensor1);
  timer.setInterval(1000L, PH_Sensor2);
  timer.setInterval(1000L, Turbidity_Sensor3);
  
  
  //other important setups
  

  DS18B20.begin();    // initialize the water temp DS18B20 sensor
  //other importants
  
  lcd.begin();
  lcd.backlight();
  lcd.clear();  

  lcd.setCursor(0,0);
  lcd.print("IOT AQUACULTURE");
  lcd.setCursor(0,1);
  lcd.print(" CONTROL SYSTEM");
  delay(5000);
  lcd.clear();
  
  lcd.setCursor(0,0);
  lcd.print("ph=7,0");

  lcd.setCursor(9,0);
  lcd.print("WTp=29C");

  lcd.setCursor(2,1);
  lcd.print("Water Quality Data");
  delay(500);
}

//----------------------------------- First Function -----------------
void Water_Sensor1() { 
  DS18B20.requestTemperatures();       // send the command to get temperatures
  temp_C = DS18B20.getTempCByIndex(0);  // read temperature in °C
  temp_F = temp_C * 9 / 5 + 32; // convert °C to °F

  Serial.print("Temperature: ");
  Serial.print(temp_C,1);    // print the temperature in °C
  Serial.print("°C");
  Serial.print("  ~  ");  // separator between °C and °F
  Serial.print(temp_F);    // print the temperature in °F
  Serial.println("°F");
  
  if (temp_C==24 && temp_F==75.2 || temp_C==27 && temp_F==80.6){
    Blynk.virtualWrite(V6, LOW);
    digitalWrite(redled_Pin,LOW);
    digitalWrite(Activebuzzer_Pin,LOW);
  }
  else if (temp_C<=18.5 && temp_F<=65.3 || temp_C>=32.5 && temp_F>=90.5){
    Blynk.logEvent("tempwatergreaterorlessthan","heerkulka waterka wuu kayaryahy 18.5C ama wuu kabadanyahay 32.5!"); 
    Blynk.virtualWrite(V6, HIGH);
    digitalWrite(redled_Pin,HIGH);
    digitalWrite(Activebuzzer_Pin,HIGH);
    delay(900);
    Blynk.virtualWrite(V6, LOW);
    digitalWrite(redled_Pin,LOW);
    digitalWrite(Activebuzzer_Pin,LOW);
    delay(900);    
  }
  else{
    Blynk.virtualWrite(V6, LOW);
    digitalWrite(redled_Pin,LOW);
    digitalWrite(Activebuzzer_Pin,LOW);
    Blynk.logEvent("TempRangeOUT","HEERKUL BIYAHA MEESHA LOOGU TALAGALAY AYUU KABAXAY!"); 
  }
  lcd.setCursor(8,0);
  lcd.print("WTp=");
  lcd.print(temp_C,1);
  lcd.print("C");
  Blynk.virtualWrite(V2, temp_C); //v2 will show data into the blynk app 
  } 


//----------------------------------- Second Function -----------------
void PH_Sensor2(){
 for(int i=0;i<10;i++)       //Get 10 sample value from the sensor for smooth the value
  { 
    buf[i]=analogRead(PhSensor);
    delay(10);
  }
  for(int i=0;i<9;i++)        //sort the analog from small to large
  {
    for(int j=i+1;j<10;j++)
    {
      if(buf[i]>buf[j])
      {
        temp=buf[i];
        buf[i]=buf[j];
        buf[j]=temp;
      }
    }
  }
  avgValue=0;
  for(int i=2;i<8;i++)                      //take the average value of 6 center sample
    avgValue+=buf[i];
  float phValue=(float)avgValue*5.0/4096/6; //convert the analog into millivolt
  phValue=3.5*phValue-2;                      //convert the millivolt into pH value
  Serial.print("    pH:");  
  Serial.println(phValue,1);
 
if (phValue<4.5 || phValue>9.5){
    Blynk.logEvent("PhChanged","Phka waxa uu kabatay 9.5 ama kayaraaday 4.5!"); 
    Blynk.virtualWrite(V6, HIGH);
    digitalWrite(redled_Pin,HIGH);
    digitalWrite(Activebuzzer_Pin,HIGH);
    delay(900);
    Blynk.virtualWrite(V6, LOW);
    digitalWrite(redled_Pin,LOW);
    digitalWrite(Activebuzzer_Pin,LOW);
    delay(900);    
  }
  else{
    Blynk.virtualWrite(V6, LOW);
    digitalWrite(redled_Pin,LOW);
    digitalWrite(Activebuzzer_Pin,LOW);
  }

  lcd.setCursor(0,0);
  lcd.print("ph=");
  lcd.print(phValue,1);
  Blynk.virtualWrite(V0, phValue);
  }
  


//----------------------------------- Third Function -----------------
void Turbidity_Sensor3()
{
  // put your main code here, to run repeatedly:
senseData = analogRead(turbiditypin);
int RealData = map(senseData, 0, 4095, 45, 0); 
//int RealData = map(senseData, 0, 1500, 45, 0); 
Serial.println(RealData);

if(RealData<15){          
Serial.println("Water Very Clean");
lcd.setCursor(0,1);
lcd.print("Water Very Clean");
}

if(RealData>=15 && RealData<30) {
Serial.println("Water Norm Clean");
lcd.setCursor(0,1);
lcd.print("Water Norm Clean");
}

if(RealData>=30){    
Blynk.virtualWrite(V6, HIGH);
digitalWrite(redled_Pin,HIGH);
digitalWrite(Activebuzzer_Pin,HIGH);       
Serial.println("Water Very Dirty");
lcd.setCursor(0,1);
lcd.print("Water Very Dirty");
digitalWrite(PumpRelay1, LOW);
digitalWrite(PumpRelay2, LOW);
Blynk.virtualWrite(V3, HIGH);
Blynk.virtualWrite(V4, HIGH);
delay(2000);
digitalWrite(PumpRelay1, HIGH);
digitalWrite(PumpRelay2, HIGH);
Blynk.virtualWrite(V3, LOW);
Blynk.virtualWrite(V4, LOW);
delay(2000);
}
 else{
    Blynk.virtualWrite(V6, LOW);
    digitalWrite(redled_Pin,LOW);
    digitalWrite(Activebuzzer_Pin,LOW);
    digitalWrite(PumpRelay1, HIGH);
    digitalWrite(PumpRelay2, HIGH);
    Blynk.virtualWrite(V3, LOW);
    Blynk.virtualWrite(V4, LOW);
  }
  Blynk.virtualWrite(V1,RealData);
}


// -------------- Loop Operations --------------
void loop()
{
Blynk.virtualWrite(V5, HIGH);
digitalWrite(greenled_Pin,HIGH);
timer.run();
Blynk.run();

}
