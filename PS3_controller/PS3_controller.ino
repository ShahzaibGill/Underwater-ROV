// Pin decrelation
#define SSerialRX              10  //Serial Receive pin
#define SSerialTX              11  //Serial Transmit pin
#define LEFT_M_PIN             5
#define RIGHT_M_PIN            6
#define FRONT_M_PIN            7
#define BACK_M_PIN             8

// Motor Threshold
#define FULL_CCW_THROT         1000
#define FULL_CW_THROT          2000

// TODO - make this a range if we have a deadzone
#define FULL_STOP              1500
#define HALFPOINT              500


// Analog Threshold
#define NETURAL               128
#define FULL_DOWN             255
#define FULL_RIGHT            255
#define NOT_USED_SCALE1       100 // only use values below not used scale
#define NOT_USED_SCALE2       150 // only use values not not used scale

// L2 and R1 Threshold
#define MAX_PRESSED            255
#define L2_THRESHOLD           30
#define R2_THRESHOLD           30

#include <PS3USB.h>
#include <SoftwareSerial.h>
#include <Servo.h>

const float analong_throt_interval = float(1000)/(float)(2*FULL_DOWN); // 1000 - range of motor write values
const float turn_scaling_interval = float(1)/float(2*FULL_RIGHT);
const float elevation_scaling_interval = float(1000)/float(2*MAX_PRESSED); //1000 - range of motor write values

USB Usb;
PS3USB PS3(&Usb); // Create an instance of PS3
SoftwareSerial RS485Serial(SSerialRX, SSerialTX); // RX, TX
Servo right_motor, left_motor, back_motor, front_motor;

int left_throt, right_throt, front_throt, back_throt; //Throttle applied to each motors
char * write_to_mega[];

//printing/testing functions 
void print_motor_values(int left, int right, int front, int back);
void wrtie_to_motor();

// ROV motion functions
void move_fwd();
void move_bkwd();
void move_up();
void move_down();

// communication handlers
void send_to_mega();



void setup() {
  Serial.begin(9600);
  if (Usb.Init() == -1) {
    Serial.print(F("\r\nOSC did not start"));
    while (1)m;
  }
  Serial.print(F("\r\nPS3 USB Library Started"));
  RS485Serial.begin(4800); 
  right_motor.attach(RIGHT_M_PIN);
  left_motor.attach(LEFT_M_PIN);
  back_motor.attach(FRONT_M_PIN);
  right_motor.writeMicroseconds(FULL_STOP);
  left_motor.writeMicroseconds(FULL_STOP);
  back_motor.writeMicroseconds(FULL_STOP);
}


void loop() {
  Usb.Task();

  // Get analog values to move and steer
  if(PS3.getAnalogHat(RightHatY)  < NOT_USED_SCALE1){ // FWD case
    move_fwd();
  } else if(PS3.getAnalogHat(RightHatY) > NOT_USED_SCALE2){ // BKWD case
     move_bkwd();
  }else{
     right_throt = FULL_STOP;
     left_throt = FULL_STOP;
  }
  
  // Use R2 and l2 to move up and down
  if(PS3.getAnalogButton(L2) > L2_THRESHOLD  || PS3.getAnalogButton(R2) < R2_THRESHOLD){
    move_down();
  }else if(PS3.getAnalogButton(L2) < L2_THRESHOLD  || PS3.getAnalogButton(R2) > R2_THRESHOLD){
    move_up();
  }
  else{
    front_throt = FULL_STOP;
    back_throt = FULL_STOP;
  }

  print_motor_values(right_throt, left_throt, front_throt, back_throt);
  wrtie_to_motor();


}


// ---------------------- HELPER functions -  communication ---------------------------//
void send_to_mega(){
  //
  // if (Serial.available())
  //  {
  //    byteReceived = Serial.read();
  //    RS485Serial.write(byteReceived);          // Send byte to Remote Arduino
  //  }
}

// ---------------------- HELPER functions -  motion ---------------------------//

void move_fwd(){
  if(PS3.getAnalogHat(RightHatY) < NOT_USED_SCALE1){ // check FWD motion in f'ns as well
    // 128 -> 0 map to 1500 -> 2000 motr values - Use FULL_DOWN(255) as refernce point
    right_throt = (float)FULL_STOP + ((float)FULL_DOWN - (float)PS3.getAnalogHat(RightHatY))* analong_throt_interval;
    left_throt = (float)FULL_STOP + ((float)FULL_DOWN -  (float)PS3.getAnalogHat(RightHatY))* analong_throt_interval;
    if(PS3.getAnalogHat(LeftHatX) < NOT_USED_SCALE1){ // left turn
        // same logic as FWD motion - 128 -> 0 map to change in throttle 
        left_throt = left_throt*(1.00 - ((float)FULL_RIGHT - (float)PS3.getAnalogHat(LeftHatX))*turn_scaling_interval*0.5);
    } else if(PS3.getAnalogHat(LeftHatX) > NOT_USED_SCALE2){ // right turn
        right_throt = right_throt*(1.00 - (float)PS3.getAnalogHat(LeftHatX)*turn_scaling_interval*0.5);
    }
  }
}

// Inverse FWD logic
void move_bkwd(){
  if(PS3.getAnalogHat(RightHatY) > NOT_USED_SCALE1){ //check BKWD motion in f'ns as well
    right_throt = (float)FULL_STOP - (float)PS3.getAnalogHat(RightHatY)* analong_throt_interval;
    left_throt = (float)FULL_STOP - (float)PS3.getAnalogHat(RightHatY)* analong_throt_interval;
    if(PS3.getAnalogHat(LeftHatX) < NOT_USED_SCALE1){ // left turn
        left_throt = left_throt*(1.00 + ((float)FULL_RIGHT - (float)PS3.getAnalogHat(LeftHatX))*turn_scaling_interval);
    } else if(PS3.getAnalogHat(LeftHatX) > NOT_USED_SCALE2){ // right turn
        right_throt = right_throt*(1.00 + (float)PS3.getAnalogHat(LeftHatX)*turn_scaling_interval);
    }
  }
}

void move_up(){
  front_throt = (float)FULL_STOP + (float)PS3.getAnalogButton(R2)* elevation_scaling_interval;
  back_throt = (float)FULL_STOP +  (float)PS3.getAnalogButton(R2)* elevation_scaling_interval;
}

void move_down(){
  front_throt = (float)FULL_STOP - (float)PS3.getAnalogButton(L2)* elevation_scaling_interval;
  back_throt = (float)FULL_STOP - (float)PS3.getAnalogButton(L2)* elevation_scaling_interval;
}

// ---------------------- TESTING functions - remove in final design ---------------------------//

void print_motor_values(int left,int right, int front, int back){
    Serial.print(F("\r\n: left "));
    Serial.print(left);
    Serial.print(F("\r: right "));
    Serial.print(right);
    Serial.print(F("\r: front "));
    Serial.print(front);
    Serial.print(F("\r: back"));
    Serial.print(back);
}

void wrtie_to_motor(){
  left_motor.writeMicroseconds(left_throt);
  right_motor.writeMicroseconds(right_throt);
  front_motor.writeMicroseconds(front_motor);
  back_motor.writeMicroseconds(back_motor);
}