/*********************************************************************
 This is a software for Laser Becaon Module
 Written by Dr Azam, Date : 18th july 2019
 Laser Beacon on Nano Controller
 Serial port to give commands
 Sfot Serial Port connect to laser
*********************************************************************/

#include <string.h>
#include <AccelStepper.h>
#include <SoftwareSerial.h>
#include "scms_mecanum_control_nano.h"

#define READ_BUFS1ZE                    (20)

AccelStepper LeftFrontWheel(1, 11, 12); // defines 3 as the step pin, 2 as direction - Stepper1
AccelStepper LeftBackWheel(1,9, 10); // Stepper2
AccelStepper RightFrontWheel(1,5, 6); // Stepper3
AccelStepper RightBackWheel(1,7, 8); // Stepper4
/*
AccelStepper LeftFrontWheel(1, 2, 3); // defines 3 as the step pin, 2 as direction - Stepper1
AccelStepper LeftBackWheel(1,4, 5); // Stepper2
AccelStepper RightFrontWheel(1,6,7); // Stepper3
AccelStepper RightBackWheel(1,8, 9); // Stepper4*/

// ************ Variables used ***************************

char packetbuffer[READ_BUFS1ZE+1];
String packetbuf;
char charBuf[25];

String ver = "Mecanum FW Ver 1.0";

int i,j,k;
int Distance = 0;
int Status=0;
bool mfl = 0;
int32_t MxSpeed = 1000;//300;
int MxAcc   = 200;

const byte EnMotorsPin=4;      // All Motors enable
//const byte EnMotorsPin=13;      // All Motors enable
uint32_t wheelSteps = 0;

SoftwareSerial S1(3, 2); // RX, TX
//SoftwareSerial S1(10, 11); // RX, TX 

// ************* Setup functions *************************
void setup(void)   
{
  SetupSerial();  //Setup PC Serial Port for Testing
  SetupSerial1(); //Setup Serial1 Port for Testing
  SetupMecanum(); //Setup mecanum 
  SelfTest();     //call self test routine
  //while ( !Serial ) delay(10);   // for nrf52840 with native usb for testing
  Serial.println("Mecanum Robot Testing !!!");
}

// ************ Loop Function ***************************

void loop(void)
{
  ReadData();
  RunMotors();
}
// ******************** Sub routunes start heare **********************
void SetupSerial()
{ 
  Serial.begin(19200);       // PC Serial Termial
  delay(1000);
}

void SetupSerial1()
{ 
  S1.begin(19200);       // Mux Serial input
  delay(1000);
}

void ReadData(void)
{
  int dir; //Direction in which robot/chassis to be moved
  uint32_t distance; //distance in mm for which robot/chassis has to be moved 
  char valArray[9];  
  if(S1.available())
  {
    uint8_t len = readPacket(500);  // Wait for new data arrive
    if (len == 0) return;
    charToString(packetbuffer,packetbuf);
    Serial.println("Serial PC: ");
    for(uint8_t z=0;z<len;z++) //print the readpacket in pc serial 
    {
      Serial.print(packetbuffer[z],HEX);Serial.print(" ");
    }
  Serial.println();
    if (packetbuffer[2] == 'V')   // VerS1on
    {
        DispVer();
    }
    if (packetbuffer[2] == 'M')   // Motors Accelerated movement to poSition
    {
      SetupMecanum();
      /*String inp;
      inp = packetbuf.substring(4);
      Serial.println(inp);
      
      inp.toCharArray(charBuf,25); 
      int n = sscanf(charBuf, "%d,%d", &m, &distance);
      
      if (n==0)
      return;
      */
      dir = packetbuffer[3]-0x30;
      //distance = packetbuffer[6];
      //sscanf(&packetbuffer[6], "%d", &distance);

        for(i=0; i<8; i++)
        {
          valArray[i] = packetbuffer[i+4];
        }
        valArray[i]='\0';
        uData.i = strtoul(valArray, NULL, 16);
        Serial.print("Val1: ");Serial.println(uData.f);
        distance = uData.f;
      
      //memcpy(&distance, &packetbuffer[6], sizeof(distance));
      Serial.print("Direction: ");Serial.print(dir);Serial.print(" Distance(mm): ");Serial.println(distance);
      
      MecanumMove(distance,dir);
      //Serial.println("Mov Done");
    }
    if (packetbuffer[2] == 'E')   // Motors Enable/Disable  E0=Enable E1 is disable
    {
      int mot_en_dis = (packetbuffer[3] - 48); // Enable / Disable Motors
      Serial.print("Motors Enable Distable: ");Serial.println(mot_en_dis);
      EnMecanum(mot_en_dis);  
    }
    if (packetbuffer[2] == 'S')   // Sets maxspeed
    {
      //int32_t MxSpeed;
      /*String inp;
      inp = packetbuf.substring(3);
      inp.toCharArray(charBuf,25); 
      int n = sscanf(charBuf, "%d", &MxSpeed);
      if (n==0)
        return;
        */
        for(i=0; i<8; i++)
        {
          valArray[i] = packetbuffer[i+4];
        }
        valArray[i]='\0';
        uData.i = strtoul(valArray, NULL, 16);
        Serial.print("Val1: ");Serial.println(uData.f);
        MxSpeed = uData.f;
      
      //memcpy(&MxSpeed, &packetbuffer[4], sizeof(MxSpeed));
      Serial.print("MaxSpped: ");Serial.println(MxSpeed);
      SetSpeed(MxSpeed);
    }
          
    if (packetbuffer[2] == 'A')   // Sets Acceleration
    {
      int MxAcc;
      /*String inp;
      inp = packetbuf.substring(3);
      inp.toCharArray(charBuf,25); 
      int n = sscanf(charBuf, "%d", &MxAcc);
      if (n==0)
      return;
      */
        for(i=0; i<8; i++)
        {
          valArray[i] = packetbuffer[i+4];
        }
        valArray[i]='\0';
        uData.i = strtoul(valArray, NULL, 16);
        Serial.print("Val1: ");Serial.println(uData.f);
        MxAcc = uData.f;
      //sscanf(&packetbuffer[4], "%d", &MxAcc);
      Serial.print("MaxAcc :");Serial.println(MxAcc);
        SetAcc(MxAcc);  
    }
    if (packetbuffer[2] == 'H')   // Displays help on Commands
    {
        ComHelp();
        Serial.println("Displayed commands.");  
    }
    if (packetbuffer[2] == 'X')   // Emergency Stop
    {
        StopMecanum();   
        Serial.println("Emergency Stop.");
    }
    //S1.write('F');
  }                 
}

void SetupMecanum()
{
  //MxSpeed = 1500;
  Serial.print("Setiing Wheel Speed to: ");Serial.println(MxSpeed);
// pinMode(EnMotorsPin, OUTPUT);
// digitalWrite(EnMotorsPin, LOW);        // Enable = LOW
  LeftFrontWheel.setMaxSpeed(MxSpeed);
  LeftFrontWheel.setAcceleration(MxAcc); // Accelartion Max 
  LeftBackWheel.setMaxSpeed(MxSpeed);
  LeftBackWheel.setAcceleration(MxAcc);
  RightFrontWheel.setMaxSpeed(MxSpeed);
  RightFrontWheel.setAcceleration(MxAcc);
  RightBackWheel.setMaxSpeed(MxSpeed);
  RightBackWheel.setAcceleration(MxAcc);
  ResetMotors();                         // reset poS1tions to zero
  
}

void StopMecanum() // Emergency Stop format = !3X
{
  mfl==0;
  ResetMotors();
  S1.println("Emergency Stop!!");
  //S1.write('F');
  //Status=1;
  //S1.write(Status);
  
}
 
void EnMecanum(int m_en_dis)  // Format is !3E,X where X is 0 or 1 
{
  if(m_en_dis == 1)
  {
    digitalWrite(EnMotorsPin, LOW);        // Enable = LOW
    S1.println("Motors Enabled!!");
  }
  else if(m_en_dis == 0)
  {
    digitalWrite(EnMotorsPin, HIGH);      // Diasable = High
    S1.println("Motors Disable!!");
  }
  else
  {
    S1.println("Unknown Command!!");
  }
  //S1.write('F');
  //Status=1;
  //S1.write(Status);
}

void SetSpeed(int32_t MxSpeed)  // Format is !3S,XXXX where XXXX is max speed 1000
{
  SetupMecanum();
 S1.print("Max Speed set to=");
 S1.println(MxSpeed);
 //S1.write('F');
 //Status=1;
 //S1.write(Status);
}

void SetAcc(int MxAcc)  // Format is !3A,XXXX where XXXX is ACCELRATION
{ 
  S1.print("Max Accelearation set to=");
  S1.println(MxAcc);
  //S1.write('F');
  //Status=1;
  //S1.write(Status);
}

void ComHelp() //Format is !3H
{
 S1.println("Mecanum Robot Command Help !!!");
 S1.println("==============================");
 S1.println("!V : Display verS1on of Software");
 S1.println("!H : Display Help on comands");
 S1.println("!M,XX,YYYYYYY : Move to command : XX = Mode,YYYYYYY = steps");
 S1.println("!E,X : X = Motors Enable/Disable (1/0)");
 S1.println("!S,XXXX : Sets Speed : XXXX steps per second");
 S1.println("!A,XXXX : Sets Acceleration : XXXX rate of acceleration");
 S1.println("!X : Emergency Stop !!!");
 //S1.write('F');
 //Status=1;
 //S1.write(Status);
}

void RunMotors()
{
  if(mfl == 1)
  {
    LeftFrontWheel.run();
    LeftBackWheel.run();
    RightFrontWheel.run();
    RightBackWheel.run();
    if(LeftFrontWheel.distanceToGo() == 0 && RightFrontWheel.distanceToGo() == 0&& LeftBackWheel.distanceToGo() == 0 && RightBackWheel.distanceToGo() == 0)
    {
      ResetMotors();
      S1.println("Moved!!");
      S1.write('F');
      mfl=0;
    }
  }
  
}

void SelfTest()
{
}

void DispVer() // Format is !2V
{  
  S1.println(ver);
  //S1.write('F');
}

 
/**************************Read Packet from Mobile Device ************/
uint8_t  readPacket(uint16_t timeout) 
{
  uint16_t origtimeout = timeout, replyidx = 0;
  while (timeout--) 
  {
    if (replyidx >= 20) break;
    while (S1.available()) 
    {
      char c =  S1.read();
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

// ******************* Mecanum Movements ********************

void MecanumMove(uint32_t distance, int chas_dir) // format is !3M,XX,YYYYYYY, XX = Mode of Mecanum Robot, YYYYYYY = Distance in cm
{
  if((chas_dir == CHAS_DIR_CW) || (chas_dir == CHAS_DIR_ACW))
  {
    wheelSteps = distance*SPD;
  }
  else
  {
    wheelSteps = distance*SPMM; //Calculate number steps for wheels to be driven for given distance
  }
  Serial.print("Chassis Dir: ");Serial.print(chas_dir);Serial.print(" MecanumMove Distance(mm): ");Serial.print(distance);Serial.print(" Steps: ");Serial.println(wheelSteps);
 // S1.print("Mecanum Moving steps: ");S1.println(wheelSteps);

  mfl = 1;              // Set flag for mecanum move
  moveMecanum[chas_dir]();
/*
  if (m == 0) {
    MoveForward();
  }
  if (m == 1) {
    MoveBackward();
  }
  if (m == 2) {
    MoveLeftForward();
  }
  if (m == 3) {
    MoveRightForward();
  }
  if (m == 4) {
    MoveLeftBackward();
  }
  if (m == 5) {
    MoveRightBackward();
  }
  if (m == 6) {
    MoveS1dewaysLeft();
  }
  if (m == 7) {
    MoveS1dewaysRight();
  }
  if (m == 8) {
    RotateLeft();
  }
  if (m == 9) {
    RotateRight();
  }
  //S1.write('F');
  //Status=1;
  //S1.write(Status);
  */
}
 
void ResetMotors()
{
  LeftFrontWheel.setCurrentPosition(0);
  LeftBackWheel.setCurrentPosition(0);
  RightBackWheel.setCurrentPosition(0);
  RightFrontWheel.setCurrentPosition(0);  
}

// *********************** Move Subroutines **************
void RotateRight() 
{
  LeftFrontWheel.moveTo(wheelSteps);
  LeftBackWheel.moveTo(wheelSteps);
  RightFrontWheel.moveTo(wheelSteps);
  RightBackWheel.moveTo(wheelSteps);
}

void RotateLeft() 
{
  LeftFrontWheel.moveTo(-wheelSteps);
  LeftBackWheel.moveTo(-wheelSteps);
  RightFrontWheel.moveTo(-wheelSteps);
  RightBackWheel.moveTo(-wheelSteps);
}
void MoveSidewaysRight() 
{
  LeftFrontWheel.moveTo(wheelSteps);
  LeftBackWheel.moveTo(-wheelSteps);
  RightFrontWheel.moveTo(wheelSteps);
  RightBackWheel.moveTo(-wheelSteps);
}
void MoveSidewaysLeft() 
{
  LeftFrontWheel.moveTo(-wheelSteps);
  LeftBackWheel.moveTo(wheelSteps);
  RightFrontWheel.moveTo(-wheelSteps);
  RightBackWheel.moveTo(wheelSteps);
}
void MoveLeftForward() 
{
  LeftFrontWheel.moveTo(-wheelSteps);
  RightBackWheel.moveTo(wheelSteps);
}
void MoveLeftBackward() 
{
  LeftBackWheel.moveTo(wheelSteps);
  RightFrontWheel.moveTo(-wheelSteps);
}
void MoveRightForward() 
{
  LeftBackWheel.moveTo(-wheelSteps);
  RightFrontWheel.moveTo(wheelSteps);
}
void MoveRightBackward() 
{
  LeftFrontWheel.moveTo(wheelSteps);
  RightBackWheel.moveTo(-wheelSteps);
}
void MoveForward() 
{
  LeftFrontWheel.moveTo(-wheelSteps);
  LeftBackWheel.moveTo(-wheelSteps);
  RightFrontWheel.moveTo(wheelSteps);
  RightBackWheel.moveTo(wheelSteps);
}
void MoveBackward() 
{
  LeftFrontWheel.moveTo(wheelSteps);
  LeftBackWheel.moveTo(wheelSteps);
  RightFrontWheel.moveTo(-wheelSteps);
  RightBackWheel.moveTo(-wheelSteps);
}


void charToString(char src[], String &dest)
{
  String rc(src);
  dest = rc;
}
 
