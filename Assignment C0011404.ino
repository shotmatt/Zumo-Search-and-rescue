#include <Wire.h>
#include <Zumo32U4.h>

#define NUM_SENSORS 5

unsigned int lineSensorValues[NUM_SENSORS];
// linesence 0 is left 2 is front 4 is right

int16_t lastError = 0;
const uint16_t maxSpeed = 200;
int16_t ModeSelection = 0;

Zumo32U4LineSensors lineSensors;
Zumo32U4ProximitySensors proxSensors;
Zumo32U4Motors motors;
Zumo32U4Buzzer buzzer;

const char oops[] PROGMEM = "v12 L16 o4 frc32<b32c32c#8cr8.erf";
const char BootUp[] PROGMEM = "v12 L16 o4 c8.e8f#8ag8.e8c8<a<f#<f#<f#<g";
void setup(){
  Serial1.begin(9600); 
  lineSensors.initFiveSensors();  //two of the sensors were not functional
  proxSensors.initThreeSensors();
  char input = (char) Serial.read();
  Serial1.println("Press Space to start");
}

int PersonCheck(){  //using the proximity sensors the robot will check for phisical objects that it will see as people.
      proxSensors.read();
      int left_sensor = proxSensors.countsLeftWithLeftLeds();
      int center_left_sensor = proxSensors.cwountsFrontWithLeftLeds();
      int center_right_sensor = proxSensors.countsFrontWithRightLeds();
      int right_sensor = proxSensors.countsRightWithRightLeds(); 
      if (left_sensor == 5 or center_left_sensor == 5 or center_right_sensor == 5 or right_sensor == 5){ //here i set the sensativity of the sensors to as high as possible
        return 1;
      } else {return 0;}
}

void RoomSearch(int mode){ // the room search system goes forwards till it hits a wall then avoids it by either drifiting left or right
        int PerCheck;
        lineSensors.readLine(lineSensorValues);
        while (lineSensorValues[2] < 200){
          lineSensors.readLine(lineSensorValues);
          motors.setSpeeds(150,150);
          while (lineSensorValues[0] > 200 or lineSensorValues[2] > 200){ //if the front or left sensor goes off the robot goes right
            motors.setSpeeds(150, -150);
            delay(100);
            motors.setSpeeds(0,0);
            lineSensors.readLine(lineSensorValues);
          } while (lineSensorValues[4] > 200){ // if the right sensor goes off it turns left
            motors.setSpeeds(-150, 150);
            delay(100);
            motors.setSpeeds(0,0);
            lineSensors.readLine(lineSensorValues);
          }
          
        if ((PerCheck = PersonCheck()) == 1){ // the person check function is then called to check if there is a person near
        Serial1.println("Suspected Person found"); 
        motors.setSpeeds(0,0);
        delay(10);
        if (mode == 1){ // the robot then either stops or slows as it goes past the person 
          Serial1.println("Stopping");
          motors.setSpeeds(0, 0);
          ModeSelection = 0;  
          PerCheck = 0;  
          lineSensorValues[2] = 1000; }    
        }
    }
}

void FollowLine(){ //the follow line function works via error detection, it is taken from the examples
  int16_t position = lineSensors.readLine(lineSensorValues);
  int16_t error = position - 2000;
  int16_t speedDifference = error / 4 + 6 * (error - lastError);
  lastError = error;  
  int16_t leftSpeed = (int16_t)maxSpeed + speedDifference;
  int16_t rightSpeed = (int16_t)maxSpeed - speedDifference;
  leftSpeed = constrain(leftSpeed, 0, (int16_t)maxSpeed);
  rightSpeed = constrain(rightSpeed, 0, (int16_t)maxSpeed);

  motors.setSpeeds(leftSpeed, rightSpeed);
}


void calibrateSensors()
{

  // Wait 1 second and then begin automatic sensor calibration
  // by rotating in place to sweep the sensors over the line
  delay(1000);
  for(uint16_t i = 0; i < 120; i++)
  {
    if (i > 30 && i <= 90)
    {
      motors.setSpeeds(-200, 200);
    }
    else
    {
      motors.setSpeeds(200, -200);
    }

    lineSensors.calibrate();
  }
  motors.setSpeeds(0, 0);
}


void loop(){
  int incomingByte = 0;
  int loop = 0;
  int PerCheck = 0;
  Serial1.println("Select Mode: 1 - Manual, 2 - SemiAuto, 3 - Auto,"); //menu for the options
  Serial1.println(" 4 - follow a line, 5 - DebugMode");  
  if (Serial1.available() > 0) {
    ModeSelection = Serial1.read();
    while (ModeSelection == 49){
      incomingByte = Serial1.read();
      if (incomingByte == 119){ //movement works as labled and the serial input is different keys being pressed (wasdqe)
        motors.setSpeeds(385, 400); //when going forwards there was drift this acounted for that, your mileage may vary
        Serial1.println("Going forwards");
        delay(40);
        motors.setSpeeds(0, 0);
      } else if (incomingByte == 115) {
        motors.setSpeeds(-400, -400);
        Serial1.println("Going backwards");
        delay(40);
        motors.setSpeeds(0, 0);
      } else if (incomingByte == 97){
        motors.setSpeeds(-400, 400);
        Serial1.println("Going left");
        delay(40);
        motors.setSpeeds(0, 0);
      } else if (incomingByte == 100){
        motors.setSpeeds(400, -400);
        Serial1.println("Going right");
        delay(40);
        motors.setSpeeds(0, 0);
      } else if (incomingByte == 101){
        motors.setSpeeds(400, 200);
        Serial1.println("Going forward right");
        delay(40);
        motors.setSpeeds(0, 0);
      } else if (incomingByte == 113){
        motors.setSpeeds(200, 400);
        Serial1.println("Going forward left");
        delay(40);
        motors.setSpeeds(0, 0);
      } else if (incomingByte == 102){ // the F key plays a song
        buzzer.playFromProgramSpace(oops);
      } 
      else if (incomingByte == 108){ //the L key takes the user out of the current program
        ModeSelection = 0;        
      }
      }while (ModeSelection == 50){ //room search with a stop if it comes across a person
      incomingByte = Serial1.read();      
      if (incomingByte == 108){
        ModeSelection = 0;        
      } else if (loop == 0){
        calibrateSensors();  //calibration only occurs once per run
        loop = 1;
      }
      RoomSearch(1);
      
    }while (ModeSelection == 51){ // room search with a slow instead of a stop
      incomingByte = Serial1.read();      
      if (incomingByte == 108){
        ModeSelection = 0;        
      } else if (loop == 0){
        calibrateSensors();  
        loop = 1;
      }
      RoomSearch(0);
    }while(ModeSelection == 52){ // line following
      incomingByte = Serial1.read();      
      if (incomingByte == 108){
        ModeSelection = 0;        
      } else if (loop == 0){
        calibrateSensors();  
        loop = 1;
      }
      FollowLine();
    }while (ModeSelection == 53){
      proxSensors.read();
      int left_sensor = proxSensors.countsLeftWithLeftLeds();
      int center_left_sensor = proxSensors.countsFrontWithLeftLeds();
      int center_right_sensor = proxSensors.countsFrontWithRightLeds(); //testing for the proximity sensors
      int right_sensor = proxSensors.countsRightWithRightLeds(); 
      Serial1.println(left_sensor);     
      Serial1.println(center_left_sensor);
      Serial1.println(center_right_sensor);
      Serial1.println(right_sensor);
      delay (500) ;
      incomingByte = Serial1.read(); //a simple way of finding what keys corrospond to what numbers
      Serial1.println(incomingByte);
      delay(1000);
      if (incomingByte == 108){
        ModeSelection = 0;
      }      
    }
    
  }
}
