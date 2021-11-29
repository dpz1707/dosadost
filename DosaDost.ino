/*D2: input from distance sensor echo
* D3: input from distance sensor trig
* D4: input from button
* D5: output to music voltmeter
* D7: input from storage button
* M1: output to linear actuator
* M2: output to linear actuator 2
* M3: output to motor spreader
*/

#include <Vex.h>

#define echoPin 2 // attach pin D2 Arduino to pin Echo of HC-SR04
#define trigPin 3 //attach pin D3 Arduino to pin Trig of HC-SR04
#define buttonOutputPin 4 //attach pin D4 to Button
#define voltmeterPin 5 //the digital pin the voltmeter is attached to
#define storageButtonPin 7 //the digital pin for the storage button
#define actuator1Pin 1 //the motor pin the first linear actuator is attached to
#define actuator2Pin 2 //the motor pin the second linear actuator is attached to
#define spreaderPin 3 //the motor pin the spreader is attached to
#define secondsButtonPressedShutoff 4 //number of seconds button needs to be held for the system to restart/stop
#define lowerSpeed 100 //the power that will be given to the actuators while they are lowering
#define lowerTime 0.5 //the increments of time with which the actuators will lower before testing the distance
#define raiseSpeed -100 //the power that will be given to the actuators while they are raising
#define spreadSpeed 20 //the power that will be given to the motor while it is spreading the batter
#define spreadTime 10 //the time, in seconds, for which the motor will run to spread the batter
#define bufferSize 2 //Number of distance sensor readings that need to be within ellipson units of the desired distance
#define ellipson 1 //The degree of leniency with which a distance sensor reading will be considered valid
#define distanceSensorDistance 25 //the number of units the distance sensor will need to be away from the pan for the platform to be considered "fully lowered"
#define minuteTimer 5 //the number of minutes that the maker will count to before playing the music voltmeter
#define MILLISECONDSPERMINUTE 60000 //the number of milliseconds in one minute
#define timeStorageButtonPressed 3 //the number of seconds the storage button needs to be held for to store the machine
Vex Motor;


// defines variables
int state = 0; //the state in which the machine is at any given time
int timeElapsed = 0; //the time for which the button has been held down
long startTime; //variable to track the time at which the machine started counting for the alarm
Adafruit_DCMotor *actuator1 = Motor.setMotor(actuator1Pin); //the first actuator
Adafruit_DCMotor *actuator2 = Motor.setMotor(actuator2Pin); //the second actuator
Adafruit_DCMotor *spreader = Motor.setMotor(spreaderPin); ///the spreader motor

/**
 * Sets up the motors, pins, and raises the actuators all of the way.
 */
void setup() 
{
  Motor.begin();
  Motor.moveTank(actuator1, actuator2, -lowerSpeed, -lowerSpeed, 33);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT); // Sets the echoPin as an INPUT
  pinMode(buttonOutputPin, INPUT_PULLUP);
  Serial.begin(9600); // // Serial Communication is starting with 9600 of baudrate speed
  digitalWrite(voltmeterPin, HIGH);
  Serial.println("Finished raising actuators");
}

/**
 * Tests to see if the user is trying to sleep/idle the machine.
 * If not, checks the state condition.
 *    If the state is 0 (idle) 
 *        Tests if the user is trying to shut off and store the machine.
 *              if so, lowers the machine fully and shuts it off.
 *              if not, continues in an idle state.
 *    If the state is 1 (lowering)
 *        Lowers the actuators to the spreading level and sets the state to 2
 *    If the state is 2 (spreadng)
 *        Spreads the batter, starts the alarm timer, and sets the state to 3
 *    If the state is 3 (raising)
 *        Raises the actuators and sets the state to 4
 *    If the state is 4 (alarm)
 *        Buzzes the alarm until the user turns it off, at which point returns to idle.
 */
void loop() 
{
  if(state != 0 && state != 4 && promptShutoff())
      state = 0;
  switch (state) 
  {
  //state 0 indicates that device not started
  case 0: 
//    if(promptStorage())
//    {
//      lowerFully();
//      exit(0);
//    }
    state = state0Idle();
  break;
    
  //state 1 indicates lowering of platform
  case 1:
      state1Lowering();
  break;

  //state 2 indicates spreading of batter
  case 2:
    state2Spreading();
  break;

  //state 3 indicates raising of platform
  case 3:
    state3Raising();
  break;

  case 4:
    state4Alarm();
  break;
  }
}

/**
 * Prompts the user to see if the button is being pressed
 * if so, temporarily delays the cooking process and tests how long the user is holding the button
 *    if the button is held for secondsButtonPressedShutoff or more seconds, returns true
 *    otherwise, returns false
 */
boolean promptShutoff()
{
    int timeButtonPressed = 0;
    while(digitalRead(buttonOutputPin) == LOW)
    {
      timeButtonPressed++;
      delay(1000);
      if(timeButtonPressed >= secondsButtonPressedShutoff)
        return true;
    }
    return false;
}

/**
 * Tests to see if the storage button is being pressed
 * if so, temporarily delays the idling and tests how long the user is holding the button
 *    if the button is held for timeStorageButtonPressed or more seconds, returns true
 *    otherwise, returns false
 */
boolean promptStorage()
{
    int timeButtonPressed = 0;
    while(digitalRead(storageButtonPin) == LOW)
    {
      timeButtonPressed++;
      delay(1000);
      if(timeButtonPressed >= timeStorageButtonPressed)
        return true;
    }
    return false;
}

/**
 * Idle state of the arduino
 * Returns 1 if the user has started the machine
 * otherwise, returns 0
 */
int state0Idle()
{
   
    if(digitalRead(buttonOutputPin) == LOW && promptShutoff() == false)
    {
        digitalWrite(voltmeterPin, LOW);
        return 1;
    }
    return 0;
}

/**
 * Tests if the actuator is lowered fully (spreader against the pan).
 *    If so, sets the state to 2 and returns.
 *    otherwise, lowers the actuators more.
 */
void state1Lowering()
{
    if(isLoweredFully())
      state = 2;
    else
      Motor.moveTank(actuator1, actuator2, lowerSpeed, lowerSpeed, lowerTime);
}

/**
 * Moves the spreader motor and then starts the alarm timer.
 */
void state2Spreading()
{
    Motor.moveMotor(spreader, spreadSpeed, spreadTime);
    startTime = millis();
    state = 3;
}

/**
 * Raises the platform and then continually tests the alarm time until minuteTimer minutes have elapsed
 *      Once that much time has elapsed, plays the voltmeter and swaps to state 4.
 */
void state3Raising()
{
    Motor.moveTank(actuator1, actuator2, raiseSpeed, raiseSpeed, 1);
    long currentTime = millis();
    long elapsed = currentTime - startTime;
    Serial.println("Time Elapsed: " + elapsed); 
    if(elapsed >= minuteTimer * MILLISECONDSPERMINUTE)
    {
      digitalWrite(voltmeterPin, HIGH);
      state = 4;
    }
}

/**
 * Plays the alarm until the buttonOutputPin button is pressed, then resets to state 0.
 */
void state4Alarm()
{
    if(digitalRead(buttonOutputPin) == LOW)
    {
        digitalWrite(voltmeterPin, LOW);
        state = 0;
    }
}

/**
 * Tests if the actuator is lowered fully. If it is lowered too far, raises the actuators.
 */
boolean isLoweredFully()
{
  for(int i = 0; i < bufferSize; i++)
  {
    if(abs(takeDistanceReading() - distanceSensorDistance) > ellipson)
    {
      if(takeDistanceReading() - distanceSensorDistance < 0)
          Motor.moveTank(actuator1, actuator2, raiseSpeed, raiseSpeed, 2);
            return false;
    }
  }
  return true;
}

/**
 * Takes a reading from the distance sensor and returns it.
 */
long takeDistanceReading()
{
    long duration; // variable for the duration of sound wave travel  
    long distance; // variable for the distance measurement
    digitalWrite(trigPin, LOW);
    delayMicroseconds(10000);
    // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10000);
    digitalWrite(trigPin, LOW);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(echoPin, HIGH);
    // Calculating the distance
    distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
    Serial.println("Distance: " + distance);
    return distance;
}

/**
 * Lowers the actuators fully.
 */
void lowerFully()
{
  Motor.moveTank(actuator1, actuator2, lowerSpeed, lowerSpeed, 33);
}
