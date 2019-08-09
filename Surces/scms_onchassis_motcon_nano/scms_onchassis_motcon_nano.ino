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

#define FrontSM '1'
#define RearSM '2'
#define SmRotation 'M'
#define GetDegrees 'G'
#define Version 'V'

#define CW 1
#define ACW 0
#define SPD 222.222     //steps per one degree rotation of stepper motor
// ************* Functions decalarion here *************************

void SetupMotor(); // Setup Stepper Motor
void StepMotor();  // Stepper Motor functions

/*
int SetFrontBeaconLaser();
int set_angleFront();
*/


 
uint8_t readPacket (uint16_t timeout);
float   parsefloat (uint8_t *buffer);
char packetbuffer[READ_BUFSIZE+1];
String packetbuf;
char charBuf[25];

// ************ Variables used ***************************

String ver = "Laser Beacon FW Ver 1.0";
//Serial.println(ver);

const byte SmFEn=9;
const byte SmFPulse=11;
const byte SmFDir=10;
const byte SmFHD=3;

const byte SmBEn=5;
const byte SmBPulse=7;
const byte SmBDir=6;
const byte SmBHD=4;

double wheelSteps=0;
long temp1F=0,temp1B=0;
long steps;
long temp2F=0,temp2B=0 ;

int32_t fsm_curr_pos=0;
int32_t rsm_curr_pos=0;

int pulsew=1; // milliseconds
int delaypluse=1; // delay between pulses
int i,j,k;
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
  //SetFrontBeaconLaser(); 
  //SetBackBeaconLaser();
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
    if (packetbuffer[1] == Version)   // Version
    {
        DispVer();
    }
    if(packetbuffer[2] == SmRotation)   // StepperMotor Control
    {
      
      dir = packetbuffer[4]-0x30;
      //sscanf(&packetbuffer[6], "%d",&Degree);
      new_angle =packetbuffer[6];
      //chassisBeacon.println("FrontMotor");
      Serial.print("Mode: ");Serial.println(dir);
      Serial.print("Degree: ");Serial.println(new_angle);
      if(packetbuffer[1] == FrontSM)
      {
        FrontMotor(dir,new_angle);
      }
      if(packetbuffer[1] == RearSM)
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
uint8_t CrossCheckFront()
{
  digitalWrite(SmFEn,LOW);
    digitalWrite(SmFDir,LOW);   //clk wise
    for(int x = 0; x < 6000; x++) {
      digitalWrite(SmFPulse,HIGH); 
      delayMicroseconds(100); 
      digitalWrite(SmFPulse,LOW); 
      delayMicroseconds(100); 
    }
    delay(1000);
    digitalWrite(SmFDir,HIGH);   //anti_clk wise
    for(int x = 0; x < 12000; x++) {
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
uint8_t set_angleFront()
{
  digitalWrite(SmFEn,LOW);
  digitalWrite(SmFDir,LOW);
  for(long x = 0; x < 40000; x++) 
  {
    if(digitalRead(SmFHD))
    {
      digitalWrite(SmFEn,HIGH);
     
      Serial.println("set zero angle is done");
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
  for(long x = 0; x < 80000; x++) {
    if(digitalRead(SmFHD))
    {
      digitalWrite(SmFEn,HIGH);
      
      Serial.println("set zero angle is done");
    }
    digitalWrite(SmFPulse,HIGH); 
    delayMicroseconds(100); 
    digitalWrite(SmFPulse,LOW); 
    delayMicroseconds(100);
  }
  delay(10);//delay(1000);
      
}

//Intially set the Front Beacon position to Zero(0th position)
uint8_t SetFrontBeaconLaser()
{
  if(digitalRead(SmFHD))
  {
    CrossCheckFront();
    //Serial.println("Fzero angle is done");
    chassisBeacon.println("Fzero angle is done.");
  }
  else
  {
    set_angleFront();
    //Serial.println("Fzero angle is done");
    chassisBeacon.println("Fzero angle is done.");
   
  }
}

// crosscheck the Back beacon position
uint8_t CrossCheckBack()
{
  digitalWrite(SmBEn,LOW);
    digitalWrite(SmBDir,LOW);   //clk wise
    for(int x = 0; x < 6000; x++) {
      digitalWrite(SmBPulse,HIGH); 
      delayMicroseconds(100); 
      digitalWrite(SmBPulse,LOW); 
      delayMicroseconds(100); 
    }
    delay(1000);
    digitalWrite(SmBDir,HIGH);   //anti_clk wise
    for(int x = 0; x < 12000; x++) {
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
uint8_t set_angleBack()
{
  digitalWrite(SmBEn,LOW);
  digitalWrite(SmBDir,LOW);
  for(long x = 0; x < 40000; x++) {
    if(digitalRead(SmBHD))
    {
      digitalWrite(SmBEn,HIGH);
     
      Serial.println("Fzero angle is done");
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
  for(long x = 0; x < 80000; x++) {
    if(digitalRead(SmBHD))
    {
      digitalWrite(SmBEn,HIGH);
      
      Serial.println("set zero angle is done");
    }
    digitalWrite(SmBPulse,HIGH); 
    delayMicroseconds(100); 
    digitalWrite(SmBPulse,LOW); 
    delayMicroseconds(100);
  }
  delay(10);
      
}

//Intially set the Back Beacon position to Zero(0th position)
uint8_t SetBackBeaconLaser()
{
  if(digitalRead(SmBHD))
  {
    CrossCheckBack();
    //Serial.println("Rzero angle is done");
    chassisBeacon.println("Rzero angle is done.");
  }
  else
  {
    set_angleBack();
    //Serial.println("Rzero angle is done");
    chassisBeacon.println("Rzero angle is done.");
   
  }
}

void FrontMotor(bool dir,int32_t new_pos) // format is !1M,X,YYYY, X = Direction (0,1, if 2, Enable/Disable), YYYY is number of steps
{
  Serial.println("Rotating Front Motor.");
  digitalWrite(SmFEn,LOW); 
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
  for(long steps = 0; steps < wheelSteps; steps++){
       
         digitalWrite(SmFPulse,HIGH); 
         delayMicroseconds(100); 
         digitalWrite(SmFPulse,LOW); 
         delayMicroseconds(100);
   }

/*
  digitalWrite(SmFEn,LOW);
  wheelSteps = (Degree*222.222);
  Serial.println(wheelSteps);

  if(mode == '1') 
  {
    digitalWrite(SmFDir,LOW);// clock wise direction
    if(temp2F>0 || temp1F>0)
    {
      if(temp2F > 0)
      {
        for(long steps = temp2F;steps > 0;steps--)
        {
          temp2F=steps-1;
          digitalWrite(SmFPulse,HIGH);  
          delayMicroseconds(100); 
          digitalWrite(SmFPulse,LOW); 
          delayMicroseconds(100);         
        } 
      } 
      if(temp1F > 0)
      {
        digitalWrite(SmFDir,HIGH);
        for(long steps = temp1F;steps > 0;steps--)
        {
          temp1F=steps-1;
          digitalWrite(SmFPulse,HIGH);  
          delayMicroseconds(100); 
          digitalWrite(SmFPulse,LOW); 
          delayMicroseconds(100);         
        } 
      }   
       //if(temp1 > 40000) digitalWrite(EnMotorsPin,HIGH); 
       for(long steps = 0; steps < wheelSteps; steps++)
       {
         if(temp1F > 40000)
         {
           digitalWrite(SmFEn,HIGH);           
           break;           
         }
         temp1F++;
         digitalWrite(SmFPulse,HIGH); 
         delayMicroseconds(100); 
         digitalWrite(SmFPulse,LOW); 
         delayMicroseconds(100);
       }
       
    }
    else
    {
      // if(temp1 > 40000) digitalWrite(EnMotorsPin,HIGH);       
      for(long steps = 0; steps < wheelSteps; steps++)
      {
        if(temp1F > 40000)
        {
          digitalWrite(SmFEn,HIGH);
          break;
        }
        temp1F++;
        //Serial.println(temp1);                 
        digitalWrite(SmFPulse,HIGH); 
        delayMicroseconds(100); 
        digitalWrite(SmFPulse,LOW); 
        delayMicroseconds(100);
       }
       
     }
     
  }
  if(mode == '0')
  {
    digitalWrite(SmFDir,HIGH);  
    if(temp1F>0 || temp2F>0)
    {
      if(temp1F>0)
      {
        for(long steps = temp1F;steps > 0;steps--)
        {
          temp1F=steps-1;
          digitalWrite(SmFPulse,HIGH); 
          delayMicroseconds(100); 
          digitalWrite(SmFPulse,LOW); 
          delayMicroseconds(100);         
        }
      }
      if(temp2F>0)
      {
        digitalWrite(SmFDir,LOW);
        for(long steps = temp2F;steps > 0;steps--)
        {
          temp2F=steps-1;
          digitalWrite(SmFPulse,HIGH); 
          delayMicroseconds(100); 
          digitalWrite(SmFPulse,LOW); 
          delayMicroseconds(100);         
        }
      }
       for(long steps = 0; steps < wheelSteps; steps++)
       {
         //Serial.println(temp2);
         if(temp2F > 40000) 
         {
           digitalWrite(SmFEn,HIGH);  
           break;     
         }
         temp2F++;
         digitalWrite(SmFPulse,HIGH); 
         delayMicroseconds(100); 
         digitalWrite(SmFPulse,LOW); 
         delayMicroseconds(100);         
        }
             
     }
     else
     {
       for(long steps = 0; steps < wheelSteps; steps++)
       {  
         if(temp2F > 40000) 
         {
           digitalWrite(SmFEn,HIGH);
           break;
          }
          temp2F++;
           //Serial.println(temp2);       
          digitalWrite(SmFPulse,HIGH); 
          delayMicroseconds(100); 
          digitalWrite(SmFPulse,LOW); 
          delayMicroseconds(100);      
        }                
      }
 }
 */
 //chassisBeacon.write('F');
} 
void BackMotor(bool dir,int32_t new_pos) // format is !2M,X,YYYY, X = Direction (0,1, if 2, Enable/Disable), YYYY is number of steps
 {
  Serial.println("Rotating Rear Motor."); 
  digitalWrite(SmBEn,LOW);
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
  for(long steps = 0; steps < wheelSteps; steps++){
       
         digitalWrite(SmBPulse,HIGH); 
         delayMicroseconds(100); 
         digitalWrite(SmBPulse,LOW); 
         delayMicroseconds(100);
   }


  /*
  digitalWrite(SmBEn,LOW);
  wheelSteps = (Degree*222.222);
  Serial.println(wheelSteps);

  if(mode == '1') 
  {
    Serial.println("Enter:");
    digitalWrite(SmBDir,LOW);// clock wise direction
    if(temp2B>0 || temp1B>0)
    {
      if(temp2B > 0)
      {
        for(long steps = temp2B;steps > 0;steps--)
        {
          temp2B=steps-1;
          digitalWrite(SmBPulse,HIGH);  
          delayMicroseconds(100); 
          digitalWrite(SmBPulse,LOW); 
          delayMicroseconds(100);         
        } 
      } 
      if(temp1B > 0)
      {
        digitalWrite(SmBDir,HIGH);
        for(long steps = temp1B;steps > 0;steps--)
        {
          temp1B=steps-1;
          digitalWrite(SmBPulse,HIGH);  
          delayMicroseconds(100); 
          digitalWrite(SmBPulse,LOW); 
          delayMicroseconds(100);         
        } 
      } 
      //if(temp1 > 40000) digitalWrite(EnMotorsPin,HIGH); 
      for(long steps = 0; steps < wheelSteps; steps++)
      {                                 
        //Serial.println(temp1);
        if(temp1B > 40000)
        {
          digitalWrite(SmBEn,HIGH);           
          break;           
        }
        temp1B++;
        digitalWrite(SmBPulse,HIGH); 
        delayMicroseconds(100); 
        digitalWrite(SmBPulse,LOW); 
        delayMicroseconds(100);                                               
      }
      
    }
    else
    {
      // if(temp1 > 40000) digitalWrite(EnMotorsPin,HIGH);       
      for(long steps = 0; steps < wheelSteps; steps++)
      {
        if(temp1B > 40000)
        {
          digitalWrite(SmBEn,HIGH);
          break;
        }
        temp1B++;
        //Serial.println(temp1);                 
        digitalWrite(SmBPulse,HIGH); 
        delayMicroseconds(100); 
        digitalWrite(SmBPulse,LOW); 
        delayMicroseconds(100);
       }
       
     }
    
  }
  if(mode == '0')
  {
    digitalWrite(SmBDir,HIGH);  
    if(temp1B>0 || temp2B>0)
    {
      if(temp1B>0)
      {
        for(long steps = temp1B;steps > 0;steps--)
        {
          temp1B=steps-1;
          digitalWrite(SmBPulse,HIGH); 
          delayMicroseconds(100); 
          digitalWrite(SmBPulse,LOW); 
          delayMicroseconds(100);         
        }
      }
      if(temp2B>0)
      {
        digitalWrite(SmBDir,LOW);
        for(long steps = temp2B;steps > 0;steps--)
        {
          temp2B=steps-1;
          digitalWrite(SmBPulse,HIGH); 
          delayMicroseconds(100); 
          digitalWrite(SmBPulse,LOW); 
          delayMicroseconds(100);         
        }
      }
      for(long steps = 0; steps < wheelSteps; steps++)
      {
        if(temp2B > 40000) 
        {
          digitalWrite(SmBEn,HIGH);  
          break;     
        }
        temp2B++;
        digitalWrite(SmBPulse,HIGH); 
        delayMicroseconds(100); 
        digitalWrite(SmBPulse,LOW); 
        delayMicroseconds(100);         
      }
          
     }
     else
     {
       for(long steps = 0; steps < wheelSteps; steps++)
       {  
         if(temp2B > 40000) 
         {
           digitalWrite(SmBEn,HIGH);
           break;
         }
         temp2B++;
         //Serial.println(temp2);       
         digitalWrite(SmBPulse,HIGH); 
         delayMicroseconds(100); 
         digitalWrite(SmBPulse,LOW); 
         delayMicroseconds(100);             
        } 
                   
      }
    
 }
 */
 //chassisBeacon.write('F');
} 
void charToString(char S[], String &D)
{
 String rc(S);
 D = rc;
}
 
