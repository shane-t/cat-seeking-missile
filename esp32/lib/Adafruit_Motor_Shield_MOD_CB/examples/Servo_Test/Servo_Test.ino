/*************************************************** 
  This is an example for the NodeMcu I2C Motor and Servo Shield v1 by 
  BurgessWorld Custom Electronics.
  It uses the Adafruit 16-channel PWM & Servo driver to drive 4 servos
  one after the other on servo ports 1-4
  
  These drivers use I2C to communicate, 2 pins are required to  
  interface.
 ****************************************************/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>


// called this way, it uses the default address 0x40
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
// you can also call it with a different address you want
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);
// you can also call it with a different address and I2C interface
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(&Wire, 0x40);

// Depending on your servo make, the pulse width min and max may vary, you 
// want these to be as small/large as possible without hitting the hard stop
// for max range. You'll have to tweak them as necessary to match the servos you
// have!
#define SERVOMIN  350 // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  550 // this is the 'maximum' pulse length count (out of 4096)

// our servo # counter
uint8_t servonum = 0;
uint8_t count = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("8 channel Servo test!");

  pwm.begin();
  
  pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates

  delay(10);
}


void loop() {
  //waiting for input

  while (Serial.available() == 0);

  int val = Serial.parseInt(); //read int or parseFloat for ..float...

  Serial.println(val);
  pwm.setPWM(servonum, 0, val);
  // Drive each servo one at a time
//  Serial.println("servo:");
//  Serial.println(servonum);
//  Serial.println("pulses:");
//  for (uint16_t pulselen = SERVOMIN; pulselen < SERVOMAX; pulselen++) {
//    Serial.println(pulselen);
//    pwm.setPWM(servonum, 0, pulselen);
//    delay(500);
//  }
//
//  delay(500);
//  for (uint16_t pulselen = SERVOMAX; pulselen > SERVOMIN; pulselen--) {
//    pwm.setPWM(servonum, 0, pulselen);
//    Serial.println(pulselen);
//    delay(500);
//  }

//  delay(500);

  
}
