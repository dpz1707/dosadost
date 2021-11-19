/*D2: input from distance sensor echo
* D3: input from distance sensor trig
* D4: input from button
* D5: output to music voltmeter
* M1: output to linear actuator
* M2: output to linear actuator 2
* M3: output to motor spreader
*/
#define echoPin 2 // attach pin D2 Arduino to pin Echo of HC-SR04
#define trigPin 3 //attach pin D3 Arduino to pin Trig of HC-SR04
#define buttonOutputPin 4 //attach pin D4 to Button
#define voltmeterPin 5
#define secondsButtonPressedShutoff 4 //number of seconds button needs to be held for the system to restart/stop

// defines variables
long duration; // variable for the duration of sound wave travel
int distance; // variable for the distance measurement
int state = 0; //

void setup() 
{
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT); // Sets the echoPin as an INPUT
  pinMode(buttonOutputPin, INPUT_PULLUP);
  Serial.begin(9600); // // Serial Communication is starting with 9600 of baudrate speed
  Serial.println("Ultrasonic Sensor HC-SR04 Test"); // print some text in Serial Monitor
  Serial.println("with Arduino UNO R3");
}
void loop() 
{
  // Clears the trigPin condition
  //state %= 4;
  if(state != 0)
      if(promptShutoff())
          state = 0;
  switch (state) 
  {
  //state 0 indicates that device not started
  case 0: 
    Serial.println("In state 0");
    state = state0Idle();
  break;
    
  //state 1 indicates lowering of platform
  case 1:
      state1Lowering();
  break;

  //state 2 indicates spreading of batter
  case 2:
    Serial.println("In state 2");
    state2Spreading();
  break;

  //state 3 indicates raising of platform
  case 3:
    Serial.println("In state 3");
  break;
  
  default:
    
    break;
  }
}

boolean promptShutoff()
{
    int timeButtonPressed = 0;
    while(digitalRead(buttonOutputPin) == LOW)
    {
      timeButtonPressed++;
      delay(1000);
    }
    if(timeButtonPressed >= secondsButtonPressedShutoff)
        return true;
    return false;
}


int state0Idle()
{
    if(digitalRead(buttonOutputPin) == LOW && promptShutoff() == false)
        return 1;
    return 0;
}

void state1Lowering()
{
    Serial.println("In state 1");
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
    // Displays the distance on the Serial Monitor
    //Serial.print("Distance: ");
    Serial.println(distance);
    if(distance < 10)
      digitalWrite(voltmeterPin, HIGH);
    else
      digitalWrite(voltmeterPin, LOW);
    //Serial.println(" cm");
}

void state2Spreading()
{

}

void state3Raising()
{
 
}
