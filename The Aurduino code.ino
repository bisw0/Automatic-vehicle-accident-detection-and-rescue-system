#include <Wire.h>  // Wire library - used for I2C communication
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

#include <BoltIoT-Arduino-Helper.h>
#ifndef API_KEY
#define API_KEY "bf3436b3-f7d0-4fce-a192-71a383764b7f"
#endif
#ifndef DEVICE_ID
#define DEVICE_ID "BOLT3263244"
#endif

static const int RXPin = 0, TXPin = 1;
static const uint32_t GPSBaud = 9600;

int ADXL345 = 0x53; // The ADXL345 sensor I2C address; Connections ->  SCL to A5, SDA to A4
float X_out, Y_out, Z_out;  // Output od Axes variation
float roll,pitch,rollF,pitchF;

int IRSensor1 = 6;// connect ir sensor to arduino Analog 0
int IRSensor2 = 7;// connect ir sensor to arduino Analog 1
int IR1_value,IR2_value;

int Vibration_check = 8;
int high_vib;

int LED_buzzer = 12; // connect LED_buzzer to arduino pin 13

int Temperature_check = A0;
float cel;

// Create a TinyGPS++ object
TinyGPSPlus gps;
// Create a software serial port called "gpsSerial"
SoftwareSerial ss(RXPin, TXPin);

void Acce_init();
void IR_read();
void Tempurature_read();
void Vibration_read();
long Vibration_time();
void Accelerometer_read();
String getGPSData(String *data);
String SensorValues(String *data);
void Print_output();

void setup() 
{
  boltiot.begin(Serial);//Bolt Tx -> 3,Bolt Rx -> 4
  
  Serial.begin(9600); // Initiate serial communication for printing the results on the Serial monitor
  ss.begin(GPSBaud);
  
  boltiot.setCommandString("GetGPSData",getGPSData);
  boltiot.setCommandString("GetSensorData",SensorValues);  
  
  Accelerometer_init();//Initializing Accelerometer sensor
  
  pinMode(IRSensor1, INPUT); //IR sensor1 pin INPUT
  pinMode(IRSensor2, INPUT); //IR sensor2 pin INPUT
  
  pinMode(Vibration_check, INPUT);  // Vibration_check pin INPUT 

  pinMode(LED_buzzer, OUTPUT);  // LED_buzzer pin OUTPUT 
}

void Accelerometer_init()
{ 
  Wire.begin(); // Initiate the Wire library
  // Set ADXL345 in measuring mode
  Wire.beginTransmission(ADXL345); // Start communicating with the device
  Wire.write(0x2D); // Access/ talk to POWER_CTL Register - 0x2D
  // Enable measurement
  Wire.write(8); // Bit D3 High for measuring enable (8dec -> 0000 1000 binary)
  Wire.endTransmission();
  delay(10);
  //Off-set Calibration
  //X-axis
  Wire.beginTransmission(ADXL345);
  Wire.write(0x1E);
  Wire.write(1);
  Wire.endTransmission();
  delay(10);
  //Y-axis
  Wire.beginTransmission(ADXL345);
  Wire.write(0x1F);
  Wire.write(-2);
  Wire.endTransmission();
  delay(10);
  //Z-axis
  Wire.beginTransmission(ADXL345);
  Wire.write(0x20);
  Wire.write(-9);
  Wire.endTransmission();
  delay(10);
}

void loop()
{
  float roll,pitch,rollF,pitchF;
  int IR1_value,IR2_value;
  int high_vib;
  float cel;
  Vibration_read();
  Accelerometer_read();
  Tempurature_read();
  IR_read(); 
  boltiot.handleCommand();
  Print_output();
}

void Vibration_read()
{
  int val = digitalRead(Vibration_check);
  long duration = Vibration_time()/100;
  high_vib = duration; 
}

long Vibration_time()
{
  long measurement = pulseIn(Vibration_check,HIGH);
  return measurement;
} 

void Accelerometer_read()
{
  // === Read acceleromter data === //
  Wire.beginTransmission(ADXL345);
  Wire.write(0x32); // Start with register 0x32 (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(ADXL345, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
  X_out = ( Wire.read()| Wire.read() << 8); // X-axis value
  X_out = X_out/64 +1.5; //256 + 0.4; //For a range of +-2g, we need to divide the raw values by 256, according to the datasheet
  Y_out = ( Wire.read()| Wire.read() << 8); // Y-axis value
  Y_out = Y_out/64 -.8; //256 -0.19;
  Z_out = ( Wire.read()| Wire.read() << 8); // Z-axis value
  Z_out = Z_out/64; //256;
  // Calculate Roll and Pitch (rotation around X-axis, rotation around Y-axis)
  roll = atan(Y_out / sqrt(pow(X_out, 2) + pow(Z_out, 2))) * 180 / PI;
  pitch = atan(-1 * X_out / sqrt(pow(Y_out, 2) + pow(Z_out, 2))) * 180 / PI;
  // Low-pass filter
  rollF = 0.94 * rollF + 0.06 * roll;
  pitchF = 0.94 * pitchF + 0.06 * pitch;
}

void IR_read()
{
  IR1_value = digitalRead(IRSensor1);
  IR2_value = digitalRead(IRSensor2);
  if (IR1_value == 1 or IR2_value == 1)
  {
  digitalWrite(LED_buzzer, HIGH);
  delay(500); 
  digitalWrite(LED_buzzer, LOW); 
  delay(500);// 1 sec
  digitalWrite(LED_buzzer, HIGH);
  delay(500); 
  digitalWrite(LED_buzzer, LOW); 
  delay(500);//2 sec
  digitalWrite(LED_buzzer, HIGH);
  delay(500); 
  digitalWrite(LED_buzzer, LOW); 
  delay(500);//3 sec
  }
}

void Tempurature_read()
{
  int Temperature = analogRead(Temperature_check);
  float mv = (Temperature/10.24);
  cel = mv*10;
  if(cel>70)//condition for overheating
  {
    digitalWrite(LED_buzzer, HIGH);
    delay(200); 
    digitalWrite(LED_buzzer, LOW); 
    delay(200);
    digitalWrite(LED_buzzer, HIGH);
    delay(200); 
    digitalWrite(LED_buzzer, LOW); 
    delay(200);
    digitalWrite(LED_buzzer, HIGH);
    delay(200); // 1 second
    digitalWrite(LED_buzzer, LOW); 
    delay(200);
    digitalWrite(LED_buzzer, HIGH);
    delay(200); 
    digitalWrite(LED_buzzer, LOW); 
    delay(200);
    digitalWrite(LED_buzzer, HIGH);
    delay(200); 
    digitalWrite(LED_buzzer, LOW); 
    delay(200);// 2 second
  }
}
void Print_output()
{
  if(high_vib > 100)
  {
    Serial.print("Vibration Sensor : High Vibration Detected : ");
    Serial.println(high_vib);
  }  
  else
  {
     Serial.print("Vibration Sensor : Below Optimum Vibration =");
     Serial.println(high_vib);
  }  
  Serial.print("Accelerometer Sensor : rollF = ");
  Serial.print(rollF);
  Serial.print("    pitchF = ");
  Serial.println(pitchF);
  if(cel>70)//condition for overheating
  {
    Serial.print("Temperature Sensor : Overheating = ");
    Serial.print(cel);
    Serial.println(" *C\n\n");
  }
  else
  { 
    Serial.print("Temperature Sensor : Below Optimum Temperature = ");
    Serial.print(cel); 
    Serial.println(" *C\n\n");
  }
}

String getGPSData(String *data)
{
  String returnString = "";                                                                                                                                                          
  returnString = "12.976337,77.504556\n";
  while(ss.available()>0)
  {
    if (gps.encode(ss.read()))
    {
      returnString = "" + String(gps.location.lat()) + "," + String(gps.location.lng()) +"\n";
    }                                                                                                                                                                              
    returnString = "12.976337,77.504556\n";
    return returnString;
  }
  return returnString;
}

String SensorValues(String *data)
{
  String values="";
  values =  ""+String(high_vib)+","
  +String(rollF)+","
  +String(pitchF)+","
  +String(IR1_value)+","
  +String(IR2_value)+","
  +String(cel)+"\n";
  return values;
}
