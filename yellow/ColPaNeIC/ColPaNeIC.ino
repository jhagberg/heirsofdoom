/*************************************************************************
* File Name          : IR_Ultrasonic.ino
* Author             : Ander, Mark Yan
* Updated            : Ander, Mark Yan
* Version            : V01.01.003
* Date               : 01/19/2016
* Description        : Firmware for Makeblock Electronic modules with Scratch.  
* License            : CC-BY-SA 3.0
* Copyright (C) 2013 - 2016 Maker Works Technology Co., Ltd. All right reserved.
* http://www.makeblock.cc/
**************************************************************************/
// Alternaive names:
// neic_tank
// Disasterbot
// General disarray

#include <Wire.h>
#include <SoftwareSerial.h>
#include <Arduino.h>
#include <MeOrion.h>

MeDCMotor dc;
MeUltrasonicSensor us(PORT_3);
Me7SegmentDisplay sd(PORT_4);
MeLimitSwitch killswitch(PORT_7,SLOT2);
MeLimitSwitch buttswitch(PORT_5,SLOT2);
MeInfraredReceiver infraredReceiverDecode(PORT_6);
MeBuzzer buzzer;

int analogs[8]={A0,A1,A2,A3,A4,A5,A6,A7};
String mVersion = "01.01.003";
unsigned char irRead = 0;

//Just for Start
int moveSpeed = 220;
int turnSpeed = 200;
int minSpeed = 45;
int factor = 23;
int distance=0;
int randnum = 0;
boolean leftflag;
boolean rightflag;
int starter_mode = 1;

void Forward()
{
  dc.reset(M1);
  dc.run(moveSpeed);
  dc.reset(M2);
  dc.run(moveSpeed);
}

void Backward()
{
  dc.reset(M1);
  dc.run(-moveSpeed);
  dc.reset(M2);
  dc.run(-moveSpeed);
}

void BackwardAndTurnLeft()
{
  dc.reset(M1);
  dc.run(-moveSpeed/2);
  dc.reset(M2);
  dc.run(-moveSpeed);
}

void BackwardAndTurnRight()
{
  dc.reset(M1);
  dc.run(-moveSpeed);
  dc.reset(M2);
  dc.run(-moveSpeed/2);
}

void TurnLeft()
{
  dc.reset(M1);
  dc.run(-moveSpeed);
  dc.reset(M2);
  dc.run(moveSpeed);
}

void TurnRight()
{
  dc.reset(M1);
  dc.run(moveSpeed);
  dc.reset(M2);
  dc.run(-moveSpeed);
}

void Stop()
{
  dc.reset(M1);
  dc.run(0);
  dc.reset(M2);
  dc.run(0);
}
void ChangeSpeed(int spd)
{
  moveSpeed = spd;
}

void ultrCarProcess()
{
  distance = us.distanceCm();
  Serial.println(distance);
  randomSeed(analogRead(A4));
  if((distance > 10) && (distance < 40))
  {
    randnum=random(300);
    if((randnum > 190) && (!rightflag))
    {
      leftflag=true;
      TurnLeft();   
    }
    else
    {
      rightflag=true;
      TurnRight();  
    }
  }
  else if(distance < 10)
  {
    randnum=random(300);
    if(randnum > 190)
    {
      BackwardAndTurnLeft();
    }
    else
    {
      BackwardAndTurnRight();
    }
  }
  else
  {
    leftflag=false;
    rightflag=false;
    Forward();
  }
}

void IrProcess()
{
  infraredReceiverDecode.loop();
  irRead = infraredReceiverDecode.getCode();
  if((irRead != IR_BUTTON_TEST) && (starter_mode != 0))
  {
    return;
  }
  switch(irRead)
  {
    case IR_BUTTON_PLUS: 
      Forward();
      break;
    case IR_BUTTON_MINUS:
      Backward();
      break;
    case IR_BUTTON_NEXT:
      TurnRight();
      break;
    case IR_BUTTON_PREVIOUS:
      TurnLeft();
      break;
    case IR_BUTTON_9:
      ChangeSpeed(factor*9+minSpeed);
      break;
    case IR_BUTTON_8:
      ChangeSpeed(factor*8+minSpeed);
      break;
    case IR_BUTTON_7:
      ChangeSpeed(factor*7+minSpeed);
      break;
    case IR_BUTTON_6:
      ChangeSpeed(factor*6+minSpeed);
      break;
    case IR_BUTTON_5:
      ChangeSpeed(factor*5+minSpeed);
      break;
    case IR_BUTTON_4:
      ChangeSpeed(factor*4+minSpeed);
      break;
    case IR_BUTTON_3:
      ChangeSpeed(factor*3+minSpeed);
      break;
    case IR_BUTTON_2:
      ChangeSpeed(factor*2+minSpeed);
      break;
    case IR_BUTTON_1:
      ChangeSpeed(factor*1+minSpeed);
      break;
    case IR_BUTTON_TEST:
      Stop();
      while(infraredReceiverDecode.buttonState() != 0)
      {
        infraredReceiverDecode.loop();
      }
      starter_mode = starter_mode + 1;
      if(starter_mode == 2)
      { 
        starter_mode = 0;
      }
      break;
    default:
      Stop();
      break;
  }
}

int ramlength = 100; // miliseconds to spend in all ahead full ramming speed.
int turnlength = 500; // miliseconds it takes to do a 90 degree turn 
int dismountlength = 250; // miliseconds it takes to climb back down off a wall
int correctionlength = 50; // miliseconds it takes to do a minor course correction.
double personal_space = 8.0; // desired minimum distance to wall in cm.
double agoraphobia = 12.0; // desired maximum distance to wall in cm.

void Hunt()
{
  // Turn ca 90 degrees left.
  TurnLeft();
  delay(turnlength);
  Stop();
}

void Dismount()
{
  // Back up, turn ca 90 degrees left.
  Backward();
  delay(dismountlength);
  Hunt();
  Stop();
}

void CorrectLeft()
{
  dc.reset(M1);
  //dc.run(-moveSpeed);
  dc.run(moveSpeed/8);
  dc.reset(M2);
  dc.run(moveSpeed);
  delay(correctionlength);
  Stop();
}

void CorrectRight()
{
  dc.reset(M1);
  dc.run(moveSpeed);
  dc.reset(M2);
  //dc.run(-moveSpeed);
  dc.run(moveSpeed/8);
  delay(correctionlength);
  Stop();
}

void Ram()
{
  double distance = us.distanceCm();
  sd.display((int) distance);
  if (distance < personal_space ) {
    CorrectLeft();
  } else if (distance > agoraphobia) {
    CorrectRight(); 
  }
  Forward();
  delay(ramlength);
}

void Destroy()
{
  if(killswitch.touched())
  {
    // Target destroyed.
    Hunt();
  }
  if(buttswitch.touched())
  {
    // Target destroyed.
    Dismount();
  }
  // TODO: unfinished.
  Ram();
}

void setup(){
  pinMode(13,OUTPUT);
  digitalWrite(13,HIGH);
  delay(300);
  digitalWrite(13,LOW);
  Serial.begin(115200);
  delay(500);
  buzzerOn();
  delay(100);
  buzzerOff();
  delay(500);
  infraredReceiverDecode.begin();
  leftflag=false;
  rightflag=false;
  randomSeed(analogRead(0));
  Stop();
  Serial.print("Version: ");
  Serial.println(mVersion);
}

void loop(){
  IrProcess();
  if(starter_mode == 1)
  {
    //ultrCarProcess();
    Destroy();    
  }
}


