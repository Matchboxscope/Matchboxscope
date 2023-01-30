/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-stepper-motor-28byj-48-uln2003/

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  Based on Stepper Motor Control - one revolution by Tom Igoe
*/

#include <Stepper.h>

const int stepsPerRevolution = 2048;  // change this to fit the number of steps per revolution

// ULN2003 Motor Driver Pins
#define IN1 32
#define IN2 25
#define IN3 22
#define IN4 27

#define analogIn 4

// initialize the stepper library
Stepper stepper(stepsPerRevolution, IN1, IN3, IN2, IN4);

void setup() {
  // set the speed at 5 rpm
  stepper.setSpeed(10);
  // initialize the serial port
  Serial.begin(115200);
}

void loop() {
  // read analog value from the potentiometer
  int nAverage = 20;
  long valSum = 0;
  int val = 0;
  for(int iAverage = 0; iAverage<nAverage; iAverage++){
    // we use PWM, so we have to create some kind of mean..
    valSum += analogRead(analogIn); // 0..4095
  
  }
  val = (float)valSum/(float)nAverage;
  Serial.println(val);

  // if the joystic is in the middle ===> stop the motor
  if (  (val > 2000) && (val < 2100) )
  {
    // STOP
    Serial.println("STOP");
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
  }

  else if (val >= 2100)
  {
    // map the speed between 5 and 500 rpm
    int speed_ = map(val, 2100, 4095, 1, 15);
    // set motor speed
    stepper.setSpeed(speed_);

    // move the motor (1 step)
    stepper.step(1);
  }

  // move the motor in the other direction
  else
  {
    // map the speed between 5 and 500 rpm
    int speed_ = map(val, 2000, 0, 1, 15);
    // set motor speed
    stepper.setSpeed(speed_);

    // move the motor (1 step)
    stepper.step(-1);
  }
}
