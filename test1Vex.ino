#include <Vex.h>
Vex Motor;
int motorPin = 1;
Adafruit_DCMotor *motorA = Motor.setMotor(motorPin);
int TIME = 60;
int POWER = 75; //scale from 0 to 100

void setup() {
 Motor.begin();
 pinMode(9, INPUT);
  

}

void loop() {
  Motor.moveMotor(motorA, POWER, 0.5);

  
  /*if(digitalRead(9) == HIGH){
    Motor.moveMotor(motorA, POWER, 0.5);
  }
  else {
    Motor.moveMotor(motorA, 2*POWER, 0.5);
  }*/
  
  //Motor.end();
}
