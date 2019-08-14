/*********************************************************************
 This is a software for Laser Becaon Module
 Written by Dr Azam, Date : 18th july 2019
 Laser Beacon on Nano Controller
 Serial port to give commands
 Sfot Serial Port connect to laser
*********************************************************************/

#include <string.h>
#include <Arduino.h>
#include <SoftwareSerial.h>
#include "scms_onchassis_motcon_nano.h"

#define READ_BUFSIZE                    (20)


// ************* Functions decalarion here *************************

uint8_t readPacket (uint16_t timeout);
float   parsefloat (uint8_t *buffer);
char packetbuffer[READ_BUFSIZE+1];
String packetbuf;
char charBuf[25];

// ************ Variables used ***************************
double wheelSteps=0;
int32_t fsm_curr_pos=0;
int32_t rsm_curr_pos=0;
int i,j,k;

//long temp1F=0,temp1B=0;
//long steps;
//int Status=0;

SoftwareSerial chassisBeacon(12,8);
// ************* Setup functions *************************
void setup(void)   
{
  SetupSerial();
  SetupSerial1();
  SetupFrontMotor();
  SetupBackMotor(); // Setup Stepper Motor
  Serial.println("Chassis Laser and Motor Testing !!!"); 
  HomePosition();

  //Status=1;
  //chassisBeacon.write(Status);
}

// ************ Loop Function ***************************

void loop(void)
{
  ReadData();         
}

// ******************** Sub routunes start heare **********************
void SetupSerial()
{ 
  Serial.begin(19200);       // PC Serial Termial
  delay(1000);
}

void SetupSerial1()
{ 
  chassisBeacon.begin(19200);       // Mux Serial input
  delay(1000);
}


void SetupFrontMotor() // Setup Stepper Motor
{
  pinMode(SmFPulse, OUTPUT);
  pinMode(SmFDir, OUTPUT);
  pinMode(SmFEn, OUTPUT);
  pinMode(SmFHD,INPUT); 
  digitalWrite(SmFPulse,LOW);
  digitalWrite(SmFDir,LOW);
  digitalWrite(SmFEn,HIGH);           // Low is Enable, High is Disable 
  
}

void SetupBackMotor() // Setup Stepper Motor
{
  pinMode(SmBPulse, OUTPUT);
  pinMode(SmBDir, OUTPUT);
  pinMode(SmBEn, OUTPUT); 
  pinMode(SmBHD,INPUT);
  digitalWrite(SmBPulse,LOW);
  digitalWrite(SmBDir,LOW);
  digitalWrite(SmBEn,HIGH);           // Low is Enable, High is Disable 
  
}


 void DispVer()
 {  
    //chassisBeacon.println(ver);
    Serial.println(ver);
    chassisBeacon.write('F');
 }

void ReadData(void)
{
  bool dir;int32_t new_angle;
  if(chassisBeacon.available())
  {
    uint8_t len = readPacket(500);  // Wait for new data arrive
    
    if (len == 0) return;
    
    charToString(packetbuffer,packetbuf);
    Serial.print("Serial PC: ");
    for(int z=0;z<len;z++)
    {
      Serial.print(packetbuffer[z]);
    }
    // printHex(packetbuffer, len);           // for debug
    if (packetbuffer[2] == VERSION)   // Version
    {
        DispVer();
    }
    if(packetbuffer[2] == SMR)   // StepperMotor Rotation Control
    {
      
      dir = packetbuffer[4]-0x30;
      //sscanf(&packetbuffer[6], "%d",&Degree);
      new_angle =packetbuffer[6];
      //chassisBeacon.println("FrontMotor");
      Serial.print("Mode: ");Serial.println(dir);
      Serial.print("Degree: ");Serial.println(new_angle);
      if(packetbuffer[1] == FSM)
      {
        FrontMotor(dir,new_angle);
      }
      if(packetbuffer[1] == RSM)
      {
        BackMotor(dir,new_angle);
      }
    }
    chassisBeacon.write('F');
    
    /*if (packetbuffer[1] == FrontSM)
        FrontMotor();
    if (packetbuffer[1] == RearSM)
        BackMotor();  */
  }
}
 
/**************************Read Packet from Mobile Device ************/
uint8_t  readPacket(uint16_t timeout) 
{
  uint16_t origtimeout = timeout, replyidx = 0;
  while (timeout--) 
  {
    if (replyidx >= 20) break;
    while (chassisBeacon.available()) 
    {
      char c =  chassisBeacon.read();
      if (c == '!') {
        replyidx = 0;
      }
      packetbuffer[replyidx] = c;
      replyidx++;
      timeout = origtimeout;
    }
    if (timeout == 0) break;
    delay(1);
  }
  packetbuffer[replyidx] = 0;  // null term
  if (!replyidx)  // no data or timeout 
    return 0;
  if (packetbuffer[0] != '!')  // doesn't start with '!' packet beginning
    return 0;
 return replyidx;
}


float parsefloat(uint8_t *buffer) 
{
  float f;
  memcpy(&f, buffer, 4);
  return f;
}


// crosscheck the Front beacon position
uint8_t CrossCheck_FSm()
{
  digitalWrite(SmFEn,LOW);
  digitalWrite(SmFDir,LOW);   //clk wise
  for(int x = 0; x < SCP_10D; x++) 
  {
    digitalWrite(SmFPulse,HIGH); 
    delayMicroseconds(100); 
    digitalWrite(SmFPulse,LOW); 
    delayMicroseconds(100); 
  }
  delay(1000);
  digitalWrite(SmFDir,HIGH);   //anti_clk wise
  for(int x = 0; x < SCP_20D; x++) 
  {
    if(digitalRead(SmFHD))
    {
      digitalWrite(SmFEn,HIGH);
      
    }
    digitalWrite(SmFPulse,HIGH); 
    delayMicroseconds(100); 
    digitalWrite(SmFPulse,LOW); 
    delayMicroseconds(100);
  }
  delay(10);//delay(1000);
}

//Set the Front Beacon position to Zero position
uint8_t SetAngle_FSm()
{
  digitalWrite(SmFEn,LOW);
  digitalWrite(SmFDir,LOW);
  long x;uint8_t return_front_startP = 1;
  if(digitalRead(SmFHD))
  {
    CrossCheck_FSm();
    return_front_startP = 0;
    //Serial.println("Fzero angle is done");
    //chassisBeacon.println("Fzero angle is done.");
  }
  else
  {
    for(x = 0; x < SCP_180D; x++) 
    {
      if(digitalRead(SmFHD))
      {
        digitalWrite(SmFEn,HIGH);
        return_front_startP = 0;
        break;
        //Serial.println("set zero angle is done");
      }
      digitalWrite(SmFPulse,HIGH); 
      delayMicroseconds(100); 
      digitalWrite(SmFPulse,LOW); 
      delayMicroseconds(100); 
      //Serial.println("laser.....");
      //bleuart.println("beta bulls");  
    }
    delay(1000);
    digitalWrite(SmFDir,HIGH);
    for(x = 0; x < SCP_360D; x++)
    {
      if(digitalRead(SmFHD))
      {
        digitalWrite(SmFEn,HIGH);
        return_front_startP = 0;
        break;
        //Serial.println("set zero angle is done");
      }
      digitalWrite(SmFPulse,HIGH); 
      delayMicroseconds(100); 
      digitalWrite(SmFPulse,LOW); 
      delayMicroseconds(100);
    }
    delay(100);
    if(return_front_startP)
    {
      digitalWrite(SmFDir,LOW);
      for(x; x > SCP_180D; x--)
      {
        digitalWrite(SmFPulse,HIGH); 
        delayMicroseconds(100); 
        digitalWrite(SmFPulse,LOW); 
        delayMicroseconds(100);
      }
    }
    delay(10);//delay(1000);
  }
  
      
}

// crosscheck the Back beacon position
uint8_t CrossCheck_BSm()
{
  digitalWrite(SmBEn,LOW);
    digitalWrite(SmBDir,LOW);   //clk wise
    for(int x = 0; x < SCP_10D; x++) {
      digitalWrite(SmBPulse,HIGH); 
      delayMicroseconds(100); 
      digitalWrite(SmBPulse,LOW); 
      delayMicroseconds(100); 
    }
    delay(1000);
    digitalWrite(SmBDir,HIGH);   //anti_clk wise
    for(int x = 0; x < SCP_20D; x++) {
      if(digitalRead(SmBHD))
      {
        digitalWrite(SmBEn,HIGH);
        
      }
      digitalWrite(SmBPulse,HIGH); 
      delayMicroseconds(100); 
      digitalWrite(SmBPulse,LOW); 
      delayMicroseconds(100);
    }
    delay(10);
}

//Set the Back Beacon position to Zero position
uint8_t SetAngle_BSm()
{
  digitalWrite(SmBEn,LOW);
  digitalWrite(SmBDir,LOW);
  long x;uint8_t return_back_startP = 1;
  if(digitalRead(SmBHD))
  {
    CrossCheck_BSm();
    return_back_startP = 0;
    //Serial.println("Rzero angle is done");
    //chassisBeacon.println("Rzero angle is done.");
  }
  else
  {
    for(x = 0; x < SCP_180D; x++) {
      if(digitalRead(SmBHD))
      {
        digitalWrite(SmBEn,HIGH);
        return_back_startP = 0;
        break;
        //Serial.println("Fzero angle is done");
      }
      digitalWrite(SmBPulse,HIGH); 
      delayMicroseconds(100); 
      digitalWrite(SmBPulse,LOW); 
      delayMicroseconds(100); 
      //Serial.println("laser.....");
      //bleuart.println("beta bulls");  
    }
    delay(1000);
    digitalWrite(SmBDir,HIGH);
    for(x = 0; x < SCP_360D; x++) 
    {
      if(digitalRead(SmBHD))
      {
        digitalWrite(SmBEn,HIGH);
        return_back_startP = 0;
        break;
        //Serial.println("set zero angle is done");
      }
      digitalWrite(SmBPulse,HIGH); 
      delayMicroseconds(100); 
      digitalWrite(SmBPulse,LOW); 
      delayMicroseconds(100);
    }
    delay(100);
    if(return_back_startP)
    {
      digitalWrite(SmBDir,LOW);
      for(x; x > SCP_180D; x--)
      {
        digitalWrite(SmBPulse,HIGH); 
        delayMicroseconds(100); 
        digitalWrite(SmBPulse,LOW); 
        delayMicroseconds(100);
      }
    }
    delay(10);
  }
      
}

uint32_t HomePosition()
{
  SetAngle_FSm();
  chassisBeacon.println("FL is zero angle.");
  SetAngle_BSm();
  chassisBeacon.println("RL is zero angle.");
}

void FrontMotor(bool dir,int32_t new_pos) // format is !1M,X,YYYY, X = Direction (0,1, if 2, Enable/Disable), YYYY is number of steps
{
  Serial.println("Rotating Front Motor.");
  digitalWrite(SmFEn,LOW); 
  digitalWrite(SmFDir,LOW);
  int32_t net_angle=0;
  if(new_pos>90)
  {
    new_pos = 90;
  }
  
  if(dir==ACW)
  {
    new_pos = -new_pos;
    
  }
  net_angle= new_pos - fsm_curr_pos;
  fsm_curr_pos=new_pos;
  Serial.print("current pos: ");Serial.print(fsm_curr_pos,DEC);
  if(net_angle<0)
  {
    digitalWrite(SmFDir,LOW);      //
    net_angle = -net_angle;
    Serial.print("dir pin :LOW");
  }
  else
  {
    digitalWrite(SmFDir,HIGH);
    Serial.print("dir pin :HIGH");

  }
  if(net_angle > 90)
  {
    net_angle=180;
  }
  wheelSteps=net_angle*SPD;
  
  
  delay(500);
  for(long steps = 0; steps < wheelSteps; steps++)
  {       
     digitalWrite(SmFPulse,HIGH); 
     delayMicroseconds(100); 
     digitalWrite(SmFPulse,LOW); 
     delayMicroseconds(100);
   }

 //chassisBeacon.write('F');
} 
void BackMotor(bool dir,int32_t new_pos) // format is !2M,X,YYYY, X = Direction (0,1, if 2, Enable/Disable), YYYY is number of steps
 {
  Serial.println("Rotating Rear Motor."); 
  digitalWrite(SmBEn,LOW);
  digitalWrite(SmBDir,LOW);
  int32_t net_angle=0;
  if(new_pos>90)
  {
    new_pos = 90;
  }
  
  if(dir==ACW)
  {
    new_pos = -new_pos;
    
  }
  net_angle= new_pos - rsm_curr_pos;
  rsm_curr_pos=new_pos;
  Serial.print("current pos: ");Serial.print(rsm_curr_pos,DEC);
  if(net_angle<0)
  {
    digitalWrite(SmBDir,LOW);      //
    net_angle = -net_angle;
    Serial.print("dir pin :LOW");
  }
  else
  {
    digitalWrite(SmBDir,HIGH);
    Serial.print("dir pin :HIGH");

  }
  if(net_angle > 90)
  {
    net_angle=180;
  }
  wheelSteps=net_angle*SPD;
  
  
  delay(500);
  for(long steps = 0; steps < wheelSteps; steps++)
  {  
     digitalWrite(SmBPulse,HIGH); 
     delayMicroseconds(100); 
     digitalWrite(SmBPulse,LOW); 
     delayMicroseconds(100);
   }
 //chassisBeacon.write('F');
} 
void charToString(char S[], String &D)
{
 String rc(S);
 D = rc;
}
 
