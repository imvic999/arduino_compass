/***************************************************************************
  This is a library example for the HMC5883 magnentometer/compass

  Designed specifically to work with the Adafruit HMC5883 Breakout
  http://www.adafruit.com/products/1746
 
  *** You will also need to install the Adafruit_Sensor library! ***

  These displays use I2C to communicate, 2 pins are required to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Kevin Townsend for Adafruit Industries with some heading example from
  Love Electronics (loveelectronics.co.uk)
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the version 3 GNU General Public License as
 published by the Free Software Foundation.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 ***************************************************************************/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>
#include <Adafruit_ADXL345_U.h>
#include <Arduino_GFX_Library.h>

#define CALIBRATE 0
/* Assign a unique ID to this sensor at the same time */
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

#ifdef CALIBRATE
float maxX = -9999;
float maxY = -9999;
float maxZ = -9999;
float minX = 9999;
float minY = 9999;
float minZ = 9999;
#endif 

//X: -37.82,-77.00  Y: -7.64,-47.82  Z: 102.35,37.04
//Vic@20220729 Add accel sensor X: 45.07,-144.62  Y: 59.09,-119.49  Z: 134.18,108.47
const float offsetX = (45.07 + -144.62)/2;
const float offsetY = (59.09 + -119.49)/2;
const float offsetZ = (134.18 + 108.47)/2;
// Once you have your heading, you must then add your 'Declination Angle', which is the 'Error' of the magnetic field in your location.
// Find yours here: http://www.magnetic-declination.com/
// Mine is: -13* 2' W, which is ~13 Degrees, or (which we need) 0.22 radians
// If you cannot find your Declination, comment out these two lines, your compass will be slightly off.
//Vic@20220727 We are -4* 55', around 0.0858 raduans
const float declinationAngle = (-4.0 - (55.0 / 60.0)) / (180 / M_PI); 

//設定接線PIN腳，主要分為ESP32和ESP8266
/*
 * RST：33
 * DC：27
 * SDA(MOSI)：23
 * BLK：22
 * SCL(SCK)：18
 * CS：5
 * GND：GND
 * VCC：3.3V
 */

#define GFX_BL 16

#if defined(ESP32)
#define TFT_CS 5
// #define TFT_CS -1 // for display without CS pin
// #define TFT_DC 16
#define TFT_DC 27
// #define TFT_DC -1 // for display without DC pin (9-bit SPI)
// #define TFT_RST 17
#define TFT_RST 33
#define TFT_BL 22
#elif defined(ESP8266)
#define TFT_CS 15
#define TFT_DC 5
#define TFT_RST -1
// #define TFT_BL 4
#else
#define TFT_CS 9
#define TFT_DC 8
#define TFT_RST 7
#define TFT_BL 6
#endif

// 使用軟體SPI用這行
// Arduino_DataBus *bus = new Arduino_SWSPI(TFT_DC, TFT_CS, 18 /* SCK */, 23 /* MOSI */, -1 /* MISO */);

// 通用的硬體 SPI 寫法
Arduino_DataBus *bus = new Arduino_HWSPI(TFT_DC, TFT_CS);

// 如果要更自訂ESP32的接腳，用這行去改
 //Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, 18 /* SCK */, 23 /* MOSI */, -1 /* MISO */, VSPI /* spi_num */);

// 使用GC9A01 IPS LCD 240x240
Arduino_GC9A01 *gfx = new Arduino_GC9A01(bus, TFT_RST, 0 /* rotation */, true /* IPS */);

void displaySensorDetails(void)
{
  sensor_t sensor;
  mag.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" uT");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" uT");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" uT");  
  Serial.print  ("offsetX:      "); Serial.print(offsetX); Serial.println(" uT");  
  Serial.print  ("offsetY:      "); Serial.print(offsetY); Serial.println(" uT");  
  Serial.print  ("offsetZ:      "); Serial.print(offsetZ); Serial.println(" uT");  
  Serial.println("------------------------------------");
  Serial.println("");
  
  accel.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" m/s^2");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" m/s^2");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" m/s^2");  
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}

void displayRange(void)
{
  Serial.print  ("Range:         +/- "); 
  
  switch(accel.getRange())
  {
    case ADXL345_RANGE_16_G:
      Serial.print  ("16 "); 
      break;
    case ADXL345_RANGE_8_G:
      Serial.print  ("8 "); 
      break;
    case ADXL345_RANGE_4_G:
      Serial.print  ("4 "); 
      break;
    case ADXL345_RANGE_2_G:
      Serial.print  ("2 "); 
      break;
    default:
      Serial.print  ("?? "); 
      break;
  }  
  Serial.println(" g");  
}

void displayDataRate(void)
{
  Serial.print  ("Data Rate:    "); 
  
  switch(accel.getDataRate())
  {
    case ADXL345_DATARATE_3200_HZ:
      Serial.print  ("3200 "); 
      break;
    case ADXL345_DATARATE_1600_HZ:
      Serial.print  ("1600 "); 
      break;
    case ADXL345_DATARATE_800_HZ:
      Serial.print  ("800 "); 
      break;
    case ADXL345_DATARATE_400_HZ:
      Serial.print  ("400 "); 
      break;
    case ADXL345_DATARATE_200_HZ:
      Serial.print  ("200 "); 
      break;
    case ADXL345_DATARATE_100_HZ:
      Serial.print  ("100 "); 
      break;
    case ADXL345_DATARATE_50_HZ:
      Serial.print  ("50 "); 
      break;
    case ADXL345_DATARATE_25_HZ:
      Serial.print  ("25 "); 
      break;
    case ADXL345_DATARATE_12_5_HZ:
      Serial.print  ("12.5 "); 
      break;
    case ADXL345_DATARATE_6_25HZ:
      Serial.print  ("6.25 "); 
      break;
    case ADXL345_DATARATE_3_13_HZ:
      Serial.print  ("3.13 "); 
      break;
    case ADXL345_DATARATE_1_56_HZ:
      Serial.print  ("1.56 "); 
      break;
    case ADXL345_DATARATE_0_78_HZ:
      Serial.print  ("0.78 "); 
      break;
    case ADXL345_DATARATE_0_39_HZ:
      Serial.print  ("0.39 "); 
      break;
    case ADXL345_DATARATE_0_20_HZ:
      Serial.print  ("0.20 "); 
      break;
    case ADXL345_DATARATE_0_10_HZ:
      Serial.print  ("0.10 "); 
      break;
    default:
      Serial.print  ("???? "); 
      break;
  }  
  Serial.println(" Hz");  
}

void draw(int x, int y, String msg){
  gfx->setCursor(x, y);
  gfx->setTextColor(WHITE);
  gfx->print(msg);
}

void setup(void) 
{
  Serial.begin(9600);
  Serial.println("Dive Compass Test"); Serial.println("");
  
  gfx->begin();             //初始化LCD
  gfx->fillScreen(BLACK);   //用黑色清除螢幕  
  
  #ifdef GFX_BL
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, HIGH);
  #endif

  gfx->setTextSize(1);

  //draw(30, 60, "Starting...");

  /* Initialise the sensor */
  if(!mag.begin())
  {
    /* There was a problem detecting the HMC5883 ... check your connections */
    Serial.println("Ooops, no HMC5883 detected ... Check your wiring!");
    while(1);
  }

    /* Initialise the sensor */
  if(!accel.begin())
  {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.println("Ooops, no ADXL345 detected ... Check your wiring!");
    while(1);
  }

  /* Set the range to whatever is appropriate for your project */
  accel.setRange(ADXL345_RANGE_2_G);
  
  /* Display some basic information on this sensor */
  displaySensorDetails();

  /* Display additional settings (outside the scope of sensor_t) */
  displayDataRate();
  displayRange();
  Serial.println("");  
}

void calibrate(float x, float y, float z){
    if(maxX < x)
        maxX = x;
    if(minX > x)
        minX = x;
    if(maxY < y)
        maxY = y;
    if(minY > y)
        minY = y;
    if(maxZ < z)
        maxZ = z;
    if(minZ > z)
        minZ = z;
}

//無傾斜補償
float noTiltCompensate(sensors_event_t event){
  return atan2(event.magnetic.y, event.magnetic.x);
}

// 傾斜補償
float tiltCompensate(sensors_event_t mag, sensors_event_t normAccel){
  float roll;
  float pitch;
  roll = asin(normAccel.acceleration.y);
  pitch = asin(-normAccel.acceleration.x);
  if (roll > 0.78 || roll < -0.78 || pitch > 0.78 || pitch < -0.78){return -1000; }
  float cosRoll = cos(roll);
  float sinRoll = sin(roll);  
  float cosPitch = cos(pitch);
  float sinPitch = sin(pitch);
  float Xh = mag.magnetic.x * cosPitch + mag.magnetic.z * sinPitch;
  float Yh = mag.magnetic.x * sinRoll * sinPitch + mag.magnetic.y * cosRoll - mag.magnetic.z * sinRoll * cosPitch;
  calibrate(Xh, Yh, mag.magnetic.z);
  return atan2(Yh, Xh);
}

void loop(void) 
{
  //Clear screen
  //gfx->fillScreen(BLACK);
  gfx->fillRect(30, 60, 240-30, 240-60, BLACK);

  /* Get a new sensor event */ 
  sensors_event_t eventMag; 
  sensors_event_t eventAccl; 
  mag.getEvent(&eventMag);
  accel.getEvent(&eventAccl);

  /* Display the results (magnetic vector values are in micro-Tesla (uT)) */
  Serial.print("X: "); Serial.print(eventMag.magnetic.x); Serial.print("  ");
  Serial.print("Y: "); Serial.print(eventMag.magnetic.y); Serial.print("  ");
  Serial.print("Z: "); Serial.print(eventMag.magnetic.z); Serial.print("  ");Serial.println("uT");
  //String data = "X:"+String(eventMag.magnetic.x, 2)+", Y:"+String(eventMag.magnetic.y, 2)+", Z:"+String(eventMag.magnetic.z, 2);
  draw(30, 60, "X:"+String(eventMag.magnetic.x, 2));
  draw(30, 80, "Y:"+String(eventMag.magnetic.y, 2));
  draw(30, 100, "Z:"+String(eventMag.magnetic.z, 2));
  /* Display the results (acceleration is measured in m/s^2) */
  Serial.print("X: "); Serial.print(eventAccl.acceleration.x); Serial.print("  ");
  Serial.print("Y: "); Serial.print(eventAccl.acceleration.y); Serial.print("  ");
  Serial.print("Z: "); Serial.print(eventAccl.acceleration.z); Serial.print("  ");Serial.println("m/s^2 ");
  //data = "aX:"+String(eventAccl.acceleration.x, 2)+", accY:"+String(eventAccl.acceleration.y, 2)+", Z:"+String(eventAccl.acceleration.z, 2);
  draw(130, 60, "aX:"+String(eventAccl.acceleration.x, 2));
  draw(130, 80, "aY:"+String(eventAccl.acceleration.y, 2));
  draw(130, 100, "aZ:"+String(eventAccl.acceleration.z, 2));
  //gfx->setCursor(30, 60);
  //gfx->print("X:"+String(eventMag.magnetic.x, 2));
  //gfx->println(" aX:"+String(eventAccl.acceleration.x, 2));
  //gfx->print("Y:"+String(eventMag.magnetic.y, 2));
  //gfx->println(" aY:"+String(eventAccl.acceleration.y, 2));
  //gfx->print("Z:"+String(eventMag.magnetic.z, 2));
  //gfx->println(" aZ:"+String(eventAccl.acceleration.z, 2));
  if(CALIBRATE){
    tiltCompensate(eventMag, eventAccl);
    //calibrate(eventMag.magnetic.x, eventMag.magnetic.y, eventMag.magnetic.z);
    Serial.print("X: "); Serial.print(maxX); Serial.print(","); Serial.print(minX);Serial.print("  ");
    Serial.print("Y: "); Serial.print(maxY); Serial.print(","); Serial.print(minY);Serial.print("  ");
    Serial.print("Z: "); Serial.print(maxZ); Serial.print(","); Serial.println(minZ);
  }
  else{
    eventMag.magnetic.x = eventMag.magnetic.x - offsetX;
    eventMag.magnetic.y = eventMag.magnetic.y - offsetY;
    eventMag.magnetic.z = eventMag.magnetic.z - offsetZ;
    /* Display the results (magnetic vector values are in micro-Tesla (uT)) */
    Serial.print("mX: "); Serial.print(eventMag.magnetic.x); Serial.print("  ");
    Serial.print("mY: "); Serial.print(eventMag.magnetic.y); Serial.print("  ");
    Serial.print("mZ: "); Serial.print(eventMag.magnetic.z); Serial.print("  ");Serial.println("Tu");
  }  
      

  
  if(!CALIBRATE){

  // Hold the module so that Z is pointing 'up' and you can measure the heading with x&y
  // Calculate heading when the magnetometer is level, then correct for signs of axis.
  //float heading = noTiltCompensate(eventMag);
  float heading = tiltCompensate(eventMag, eventAccl);
  if(heading == -1000){
    heading = noTiltCompensate(eventMag);
  }
  heading += declinationAngle;
  
  // Correct for when signs are reversed.
  if(heading < 0)
    heading += 2*PI;
    
  // Check for wrap due to addition of declination.
  if(heading > 2*PI)
    heading -= 2*PI;
   
  // Convert radians to degrees for readability.
  float headingDegrees = heading * 180/M_PI; 
  
  Serial.print("Heading (degrees): "); Serial.println(headingDegrees);
  draw(30, 140, String(headingDegrees));
  //gfx->setCursor(30, 140);
  //gfx->println(String(headingDegrees));
  }
  
  delay(100);
}