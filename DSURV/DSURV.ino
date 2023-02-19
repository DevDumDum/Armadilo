#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>


#define SERVOMIN  110 // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  600 // this is the 'maximum' pulse length count (out of 4096)
uint8_t servonum = 0;//servo # counter
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define motorA1 3
#define motorA2 9
#define motorB1 10
#define motorB2 11

#define PH 3.14
//const int ldrPin = A2; // analog pin A2
float L1 = 10.5; //arm length 1
float L2 = 9.8;  //arm length 2

const int s1MinLimit = 30;
const int s2MinLimit = 0; 
const int s3MinLimit = 5;

int angleToPulse(int ang);

String data = "";
char tempS;
String command = "";

int input[4] = {}; // terminal input for each servo
int inputPast[4] = {0,s1MinLimit,s2MinLimit,s3MinLimit};
String tempInput = "";


void setup() {
  Serial.begin(9600);

  pwm.begin();
  pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates
  //pinMode(ldrPin, INPUT);
  //yield();
  pwm.setPWM(0, 0, angleToPulse(0) );
  pwm.setPWM(1, 0, angleToPulse(90) );
  pwm.setPWM(2, 0, angleToPulse(90) );
  pwm.setPWM(3, 0, angleToPulse(90) );
}

void moveElbows(float b, float a1, float a2){
  if(b < s3MinLimit){ b=s3MinLimit;}
  if(a1 < s2MinLimit){ a1=s2MinLimit;}// if(a2 < s2MinLimit){ a2=s2MinLimit;}
  if(a2 < s1MinLimit){ a2=s1MinLimit;}// if(a1 < s1MinLimit){ a1=s1MinLimit;}
  pwm.setPWM(3, 0, angleToPulse(b) );
  pwm.setPWM(2, 0, angleToPulse(a1) );
  pwm.setPWM(1, 0, angleToPulse(a2) );
  pwm.setPWM(0, 0, angleToPulse(0) );
}

void toInverseK(float x, float y, float z) {
  // float b = atan2(y,x) * (180 / 3.1415); // base angle
  // float l = sqrt(x*x + y*y);
  // float h = sqrt (l*l + z*z);
  // float phi = atan(z/l) * (180 / 3.1415);
  // float theta = acos((h/2)/100) * (180 / 3.1415);

  // float a1 = phi + theta; // angle for first part of the arm
  // float a2 = phi - theta; // angle for second part of the arm

  float b = z;
  // x = 5;
  // y = 3;
  float h = sqrt((x*x)+(y*y));
  float max_h = L1+L2;
  while(h > max_h){
    (x<0)?x++:x--;
    (y<0)?y++:y--;
    h = sqrt((x*x)+(y*y));
    Serial.println(h);
  }
  //while h
  float thetaE = acos((pow(x,2)+pow(y,2)-pow(L1,2)-pow(L2,2))/(2*L1*L2));
  float thetaQ = acos((pow(x,2)+pow(y,2)+pow(L1,2)-pow(L2,2))/(2*L1*(sqrt((pow(x,2)+pow(y,2))))));
  float thetaS = atan2(y,x)-thetaQ;
  float new_thetaE = PH-thetaE;
  float new_thetaS = atan2(y,x)+thetaQ;

  float angle_E =(unsigned short)((new_thetaE*180)/PH);
  float angle_S =(unsigned short)((new_thetaS*180)/PH);
  

  Serial.print("Base: ");
  Serial.print(b);
  Serial.print(" = A1: ");
  Serial.print(angle_S);
  Serial.print(" = A2: ");
  Serial.println(angle_E);
  Serial.print(" = H: ");
  Serial.print(h);
  moveElbows(b, angle_S, angle_E);
}


void angle(String command){
  String xyz[3];
  int index = 0;

  for(int t=0; t < command.length();t++){
    if(command[t] != ','){
      xyz[index] += command[t];
      //Serial.println(command[t]);
    }else{
      index++;
    }
  }
  float x = xyz[0].toFloat();
  float y =  xyz[1].toFloat();
  float z = xyz[2].toFloat();
  toInverseK(x, y, z);
  //moveElbows(z, y, x);
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

void loop() {
  //int ldrStatus = analogRead(ldrPin);
  int index = 0;
  int availableBytes = Serial.available();

  data = "";
  command = "";
  bool textStatus = false; //if text command started

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
      
      // if(tempS == ',' or i+1 > availableBytes){
      //   int conIn = tempInput.toInt();

      //   if(conIn > 180 || conIn < 0){ // if angle exceeded range
      //     Serial.println("Error input!");
      //     conIn = 0;
      //   }

      //   input[index] = conIn;
      //   tempInput = "";
      //   index++;
      // }else{
      //   tempInput += tempS;
      // }
    }
    if(textStatus == true){
      command = "Error: decoded closing undetected";
      Serial.println(command);
    } else if((command == "") && (textStatus == false)){
      command = "Error: decoded starting line undetected";
      Serial.println(command);
    } else {

      //c1 = angle
      //c2 = wheel
      //r1 = claw sensor status check
      //r2 = baro sensor check???

      if(command[0] == 'c'){
        switch(command[1]){
          case '1': command.remove(0,3);angle(command); break;
          case '2': command.remove(0,3); Serial.print("@");motor(command); break;
        }
      }else{
        Serial.print(command[0]);
        Serial.print(command[1]);
        Serial.print(" : ");
        Serial.println(command);
      }
    
    }

    // //delay movement
    // if(input[3] < s3MinLimit){ input[3]=s3MinLimit;}
    // if(input[2] < s2MinLimit){ input[2]=s2MinLimit;}
    // //if(input[1] < s1MinLimit){ input[1]=s1MinLimit;}
    
    // for(int x = 0; x <= 3; x++){
    //   if(inputPast[x] < input[x]){
    //     // for(int i = inputPast[x]; i < input[x]+10; i++){
    //     //   pwm.setPWM(x, 0, angleToPulse(i) );
    //     //   delay(10);
    //     // }
    //     pwm.setPWM(x, 0, angleToPulse(input[x]+10) );
    //     delay(100);
    //     pwm.setPWM(x, 0, angleToPulse(input[x]) ); // to remove stutter
    //   } else{
    //     // for(int i = inputPast[x]; i > input[x]; i--){
    //     //   pwm.setPWM(x, 0, angleToPulse(i) );
    //     //   delay(10);
    //     // }
    //     pwm.setPWM(x, 0, angleToPulse(input[x]) );
    //   }
      
    //   inputPast[x] = input[x];
    //   delay(100);
    // }
    // Serial.println(String(input[0])+":"+String(input[1])+":"+String(input[2])+":"+String(input[3]));
  }
  //Serial.println(ldrStatus);
  delay(500);
} 

/*
 * angleToPulse(int ang)
 * gets angle in degree and returns the pulse width
 * also prints the value on seial monitor
 * written by Ahmad Nejrabi for Robojax, Robojax.com
 */
int angleToPulse(int ang){
  int pulse = map(ang,0, 180, SERVOMIN,SERVOMAX);// map angle of 0 to 180 to Servo min and Servo max 
  //Serial.print("Angle: ");Serial.print(ang);
  //Serial.print(" pulse: ");Serial.println(pulse);
  return pulse;
}
