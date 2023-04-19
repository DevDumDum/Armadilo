#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>


#define SERVOMIN  110 // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  600 // this is the 'maximum' pulse length count (out of 4096)
#define motorA1 6
#define motorA2 5
#define motorB1 7
#define motorB2 8
#define sensorDE A3
#define sensorDT A2 

#define PH 3.14


unsigned long currentMillis;
long previousMillis = 0;    // set up timers
long interval = 5;        // time constant for timer
int stepFlag = 0;
long previousStepMillis = 0;
long previousStepMillis2 = 0;

float smoothA1;
float smoothA1Prev;

uint8_t servonum = 0;//servo # counter
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

float L1 = 10.4; //arm length 1
float L2 = 17.2;  //arm length 2

const int s1MinLimit = 30;
const int s2MinLimit = 0; 
const int s3MinLimit = 5;

long duration;
int distance;

int angleToPulse(int ang);

String data = "";
char tempS;
String command = "";

int input[4] = {}; // terminal input for each servo
int inputPast[4] = {0,s1MinLimit,s2MinLimit,s3MinLimit};
String tempInput = "";

float lastB = 90;
float lastA1 = 90;
float lastA2 = 90;
float lastA3 = 100;

float fB = 90;
float fA1 = 90;
float fA2 = 90;
float fA3 = 100;

float currentB = 90;
float currentA1 = 90;
float currentA2 = 90;
float currentA3 = 100;
int ratioP = 100;
int ratioP_slower = 100;
float LV = 0;
float UV = 0;
bool slow = false;

bool claw = true; //true for close, false for open

void setup() {
  Serial.begin(9600);
  pinMode(sensorDT, OUTPUT); // Sets the trigPin as an Output
  pinMode(sensorDE, INPUT); // Sets the echoPin as an Input
  pwm.begin();
  pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates
  pwm.setPWM(0, 0, angleToPulse(90) );
  pwm.setPWM(1, 0, angleToPulse(90) );
  pwm.setPWM(2, 0, angleToPulse(90) );
  pwm.setPWM(3, 0, angleToPulse(90) );
  pwm.setPWM(4, 0, angleToPulse(10) );
}

void moveElbows(float b, float a1, float a2, float a3){
  // if(b < s3MinLimit){ b=s3MinLimit;}
  // if(a1 < s2MinLimit){ a1=s2MinLimit;}// if(a2 < s2MinLimit){ a2=s2MinLimit;}
  // if(a2 < s1MinLimit){ a2=s1MinLimit;}// if(a1 < s1MinLimit){ a1=s1MinLimit;}
  pwm.setPWM(3, 0, angleToPulse(b) );
  pwm.setPWM(2, 0, angleToPulse(a1) );
  pwm.setPWM(1, 0, angleToPulse(a2) );
  pwm.setPWM(0, 0, angleToPulse(a3) );
}

void toInverseK(float x, float y, float z) {
  float b = z;
  float h = sqrt((x*x)+(y*y));
  float max_h = L1+L2;
  while(h > max_h){
    (x<0)?x++:x--;
    (y<0)?y++:y--;
    h = sqrt((x*x)+(y*y));
  }
  float thetaE = acos((pow(x,2)+pow(y,2)-pow(L1,2)-pow(L2,2))/(2*L1*L2));
  float thetaQ = acos((pow(x,2)+pow(y,2)+pow(L1,2)-pow(L2,2))/(2*L1*(sqrt((pow(x,2)+pow(y,2))))));
  float thetaS = atan2(y,x)-thetaQ;
  float new_thetaE = PH-thetaE;
  float new_thetaS = atan2(y,x)+thetaQ;

  float angle_E =(unsigned short)((new_thetaE*180)/PH);
  float angle_S =(unsigned short)((new_thetaS*180)/PH);
  
  float angle_F = (270 - angle_E) - angle_S;
  
  //moveElbows(b, angle_S, angle_E, angle_F);
  currentB = b;
  currentA1 = angle_S;
  currentA2 = angle_E;
  currentA3 = angle_F;
  ratioP = 0;
  ratioP_slower = 0;
  Serial.print("Base: ");
  Serial.print(currentB);
  Serial.print(" = A1: ");
  Serial.print(currentA1);
  Serial.print(" = A2: ");
  Serial.print(currentA2);
  Serial.print(" = A3: ");
  Serial.print(currentA3);

  Serial.print(" = R: ");
  Serial.print(ratioP);
  Serial.print(" = H: ");
  Serial.println(h);
}

void angle(String command){
  String xyz[3];
  int index = 0;

  for(int t=0; t < command.length();t++){
    if(command[t] != ','){
      xyz[index] += command[t];
    }else{
      index++;
    }
  }
  float x = xyz[0].toFloat();
  float y =  xyz[1].toFloat();
  float z = xyz[2].toFloat();
  toInverseK(x, y, z);
}

void motor (String command){
  char direction = command[0];
  command.remove(0,2);

  if(direction == 'f'){
    analogWrite(motorA1, 0);
    analogWrite(motorA2, 255);
    analogWrite(motorB1, 0);
    analogWrite(motorB2, 255);
  } else {
    analogWrite(motorA1, 255);
    analogWrite(motorA2, 0);
    analogWrite(motorB1, 255);
    analogWrite(motorB2, 0);
  }
  Serial.print("direction: ");
  Serial.println(direction);

  int tempC = atoi(command.c_str());
  delay(tempC);

  digitalWrite(motorA1, LOW);
  digitalWrite(motorA2, LOW);
  digitalWrite(motorB1, LOW);
  digitalWrite(motorB2, LOW);
}

void sensorD (){
  digitalWrite(sensorDT, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(sensorDT, HIGH);
  delayMicroseconds(10);
  digitalWrite(sensorDT, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(sensorDE, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2;
  // Prints the distance on the Serial Monitor
  Serial.println(distance);
}

void loop() {  
  int index = 0;
  int availableBytes = Serial.available();
  data = "";
  command = "";
  bool textStatus = false; //if text command started

  currentMillis = millis();

  if (currentMillis - previousStepMillis > 500) {
    previousStepMillis = currentMillis;

    if(availableBytes > 2){
      for(int i=0; i <= availableBytes; i++){
        tempS = Serial.read();
        if(tempS == '>'){
          textStatus = false;
          break;
          
        } else if(tempS == '<'){
          i++;
          tempS = Serial.read();
          textStatus = true;
        }

        if(textStatus == true){
          command += tempS;
        }
      }
      if(textStatus == true){
        command = "Error: decoded closing undetected";
        Serial.println(command);
      } else if((command == "") && (textStatus == false)){
        command = "Error: decoded starting line undetected";
        Serial.println(command);
      } else {
        if(command[0] == 'c'){
          switch(command[1]){
            case '1': command.remove(0,3);angle(command); break;
            case '2': command.remove(0,3);motor(command); break;
            case '3': command.remove(0,3);sensorD(); break;
            case '4':
              if(claw){
                Serial.print("close");
                pwm.setPWM(4, 0, angleToPulse(90) );
                claw = false;
              } else {
                Serial.print("open");
                pwm.setPWM(4, 0, angleToPulse(10) );
                claw = true;
              }
              Serial.println(claw);
              break;
            case '5':
              command.remove(0,3);
              String bxyz[4];
              int index = 0;
              for(int t=0; t < command.length(); t++){
                if(command[t] != ','){
                  bxyz[index] += command[t];
                }else{
                  index++;
                }
              }
              float b = bxyz[0].toFloat();
              float x = bxyz[1].toFloat();
              float y =  bxyz[2].toFloat();
              float z = bxyz[3].toFloat();
              Serial.print("Base: ");
              Serial.print(b);
              Serial.print(" = A1: ");
              Serial.print(x);
              Serial.print(" = A2: ");
              Serial.print(y);

              Serial.print(" = A3: ");
              Serial.println(z);
              moveElbows(b,x,y,z);
              break;
          }
        }else{
          Serial.print(command[0]);
          Serial.print(command[1]);
          Serial.print(" : ");
          Serial.println(command);
        }
      
      }
    }
  }

  
  if (currentMillis - previousStepMillis2 > 20) {
    previousStepMillis2 = currentMillis;
    if (ratioP == 0){
      // Serial.print("LBase: ");
      // Serial.print(lastB);
      // Serial.print(" = LA1: ");
      // Serial.print(lastA1);
      // Serial.print(" = LA2: ");
      // Serial.print(lastA2);

      // Serial.print(" = LA3: ");
      // Serial.println(lastA3);
      // Serial.println("=======================");
    }

    if (ratioP < 100 || ratioP_slower < 100){
      if(currentB!=fB){
        fB = float(lastB) + (float(currentB) - float(lastB)) * (float(ratioP)/100.00);
      }

      if(currentA1!=fA1){
        fA1 = float(lastA1) + (float(currentA1) - float(lastA1)) * (float(ratioP)/100.00);
      }

      if(currentA2!=fA2){
        fA2 = float(lastA2) + (float(currentA2) - float(lastA2)) * (float(ratioP)/100.00);
      }

      currentA3 = (280 - fA2) - fA1;
      fA3 = currentA3;
      // Serial.println(" ");
      // Serial.print(" A1: ");
      // Serial.print(fA1);
      // Serial.print(" A2: ");
      // Serial.print(fA2);
      // Serial.print(" A3: ");
      // Serial.println(fA3);

      ratioP_slower+=5;
      ratioP+=10;
    } else {
        lastB = fB;
        lastA1 = fA1;
        lastA2 = fA2;
        lastA3 = fA3;
    }
  }
  moveElbows(fB, fA1, fA2, fA3);
} 

int angleToPulse(int ang){
  int pulse = map(ang,0, 180, SERVOMIN,SERVOMAX);// map angle of 0 to 180 to Servo min and Servo max 
  return pulse;
}
