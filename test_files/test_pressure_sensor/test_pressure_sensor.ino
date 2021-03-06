
/******************************************************************************
SparkFun_MS5803_Demo.ino
Demo Program for MS5803 pressure sensors.
Casey Kuhns @ SparkFun Electronics
7/20/2014
https://github.com/sparkfun/MS5803-14BA_Breakout/

The MS58XX MS57XX and MS56XX by Measurement Specialties is a low cost I2C pressure
sensor.  This sensor can be used in weather stations and for altitude
estimations. It can also be used underwater for water depth measurements. 

Resources:
This library uses the Arduino Wire.h to complete I2C transactions.

Development environment specifics:
	IDE: Arduino 1.0.5
	Hardware Platform: Arduino Pro 3.3V/8MHz
	T5403 Breakout Version: 1.0

**Updated for Arduino 1.6.4 5/2015**
	
This code is beerware. If you see me (or any other SparkFun employee) at the
local pub, and you've found our code helpful, please buy us a round!

Distributed as-is; no warranty is given.
******************************************************************************/

#include <Wire.h>
#include <SparkFun_MS5803_I2C.h>

MS5803 pressure_sensor(ADDRESS_HIGH);

//Create variables to store results
float temperature_c, temperature_f;
double pressure_abs, pressure_relative, altitude_delta, pressure_baseline;

// Create Variable to store altitude in (m) for calculations;
double base_altitude = 329.0; // Altitude of Waterloo (m)

void setup() {
    Serial.begin(115200);
    //Retrieve calibration constants for conversion math.
    pressure_sensor.reset();
    pressure_sensor.begin();
    pressure_baseline = pressure_sensor.getPressure(ADC_4096);
}

void loop() {
  // Read pressure from the sensor in mbar.
  pressure_abs = pressure_sensor.getPressure(ADC_4096);
  // Let's do something interesting with our data.
  
  // Convert abs pressure with the help of altitude into relative pressure
  // This is used in Weather stations.
  pressure_relative = sealevel(pressure_abs, base_altitude);
  // Taking our baseline pressure at the beginning we can find an approximate
  // change in altitude based on the differences in pressure.   
  altitude_delta = altitude(pressure_abs , pressure_baseline);

  Serial.print("Pressure abs (mbar)= ");
  Serial.println(pressure_abs);
   
  Serial.print("Pressure relative (mbar)= ");
  Serial.println(pressure_relative); 
  delay(1000);

}
double sealevel(double P, double A)
// Given a pressure P (mbar) taken at a specific altitude (meters),
// return the equivalent pressure (mbar) at sea level.
// This produces pressure readings that can be used for weather measurements.
{
	return(P/pow(1-(A/44330.0),5.255));
}


double altitude(double P, double P0)
// Given a pressure measurement P (mbar) and the pressure at a baseline P0 (mbar),
// return altitude (meters) above baseline.
{
	return(44330.0*(1-pow(P/P0,1/5.255)));
}
  
