/*********************************************************************
  This is a software for Mecanum Robot
  Written by Dr Azam, Date : 1st July 2019

*********************************************************************/

#include <bluefruit.h>
#include <string.h>
#include <Arduino.h>
#include <SoftwareSerial.h>
#include "scms_master_control_nrf52840.h"
#include "scms_master_control_bleuart.h"
// Uart over BLE service
//BLEUart bleuart;

// ************* Functions decalarion here *************************
//uint8_t readPacket1 (BLEUart *ble_uart, uint16_t timeout);
uint8_t readPacket2 (uint8_t *serial_uart, uint16_t timeout);
float   parsefloat (uint8_t *buffer);
char    packetbuffer[READ_BUFSIZE + 1];
char corner_beacon_responce[20+1] = { 0 };
String  packetbuf;


void SelectPort(uint8_t p_num); //Select the Serial Port number
void MoveRobot(uint8_t dir, uint32_t dist_ang); //Robot Movement function
void RotateSmRear(uint8_t dir, uint32_t angle); //Rear Stepper Motors Rotation
void RotateSmFront(uint8_t dir, uint32_t angle); //Front Stepper Motors Rotation
void LaserControl(uint8_t on_off); //Laser Contoller (ON or OFF)
uint32_t GetMesurment_FL(uint8_t mode, uint32_t n_times); //Front Laser Distance
uint32_t GetMesurment_RL(uint8_t mode, uint32_t n_times); //Rear Laser Distance
uint32_t GetDistanceFromCornerBeacon(uint8_t lm_mode, uint32_t n_times);
void RotateCornerBeacon(uint8_t dir, uint32_t angle);

// ************ Variables used ***************************

//int i, j, k,flag=0;
int32_t Distance=0, FLDistance =0 ,RLDistance=0;
int8_t SmFCAng=0, SmRCAng=0;

uint32_t wheelSteps = 0;

//SoftwareSerial Serial1 (2,3); //Rx,TX


// ************* Setup functions *************************
void setup(void)
{
  SetupSerialPort(); // Setup Serial Port UART Multiplexor
  SetupSerial(); // Setup PC Serial Port for Testing
  SetupSerial1(); // Rx/Tx pins of controller
  SetupFLaser(); //Setup Front Laser
  SetupRLaser(); //Setup Back Laser
  //SelfTest();  // call self test routine
  
  SetupBlue(); // setup Bluetooth
  StartAdv(); // Set up and start advertising
  //Serial.begin(19200);
  // while ( !Serial ) delay(10);   // for nrf52840 with native usb for testing
  Serial.println("LCbb_RCONTROL_00 Testing !!!");
  //BeaconDetection();
  /*if(Serial1)
  {
    Serial1.begin(19200);
    delay(1000);
  }
 else
  Serial.println("portOne ERROR: ");*/
}

// ************ Loop Function ***************************

void loop(void)
{
  ReadData();          // Read data from bluetooth 
  
  ReadResult();        // receives results from respective controllers and process
}
// ******************** Sub routunes start heare **********************

void SetupFLaser()  // Setup Laser
{
  pinMode(EN_FrontLaser, OUTPUT); // Turn Laser Module ON
  digitalWrite(EN_FrontLaser, HIGH); 
  delay(500);
  // Open serial communications and wait for port to open:
  Serial.begin(19200);
 // while ( !Serial ) delay(10);   // for nrf52840 with native usb
  Serial.println("Laser Beacon Distance Measurement!");
  // set the data rate for the SoftwareSerial port
  Serial1.begin(19200);
  Serial1.write(0x55); // Autobaud laser device
  delay(100);      
}
void SetupRLaser()  // Setup Laser
{
  pinMode(EN_RearLaser, OUTPUT); // Turn Laser Module ON
  digitalWrite(EN_RearLaser, HIGH); 
  delay(500);
  // Open serial communications and wait for port to open:
  Serial.begin(19200);
 // while ( !Serial ) delay(10);   // for nrf52840 with native usb
  Serial.println("Laser Beacon Distance Measurement!");
  // set the data rate for the SoftwareSerial port
  Serial1.begin(19200);
  Serial1.write(0x55); // Autobaud laser device
  delay(100);      
}
void SetupSerial()
{ 
  Serial.begin(19200);       // PC Serial Termial through USB
  delay(1000);
}

void SetupSerial1()
{ 
  Serial1.begin(19200);       // Rx/Tx pins of controllers
  delay(1000);
  Serial1.println("Hello !!!");
}

void SetupSerialPort()
{
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  
  digitalWrite(S1, LOW);  // Initialize to 0 LSB
  digitalWrite(S2, LOW);  // Initialize to 0
  digitalWrite(S3, LOW);  // Initialize to 0 MSB
  
}


void ReadData()
{
  byte chas_dir,lm_mode;
  bool mot_dir,on_off;
  int32_t angle,dist_ang,fl_dist,rl_dist,w_speed;
  uint32_t n_times;
  
  if(cmd_packet_received)
  {
    cmd_packet_received = false;
   // uint8_t len = readPacket1(&bleuart, 100);  // Wait for new data arrive
   // if (len == 0) return;
    charToString(packetbuffer,packetbuf);
    
    if (packetbuffer[1] == CMD_VER)   // Version
    {           
      DispVer();      
      //Serial.println("task completed");
    }
    
    //packetbuffer[1] = packetbuffer[1]-0x30;
    //if(packetbuffer[2] == CMD_MVMT)
    //{
      //mot_dir = packetbuffer[4]-0x30;
      //sscanf(&packetbuffer[6],"%d",&angle);
      if(packetbuffer[1] == CT_FSM)
      {
        mot_dir = packetbuffer[4]-0x30;
        sscanf(&packetbuffer[6],"%d",&angle);
        RotateSmFront(mot_dir,angle);
      }
      if(packetbuffer[1] == CT_RSM)
      {
        mot_dir = packetbuffer[4]-0x30;
        sscanf(&packetbuffer[6],"%d",&angle);
        RotateSmRear(mot_dir,angle);
      }
    //}
       
    if(packetbuffer[1] == CT_MWR)
    {
      if(packetbuffer[2] == CMD_MWSPEED)
      {
        sscanf(&packetbuffer[4],"%d",&w_speed);
        Serial.print("Mecanum Wheel speed: ");Serial.println(w_speed);
        SetSpeedMoveRobot(w_speed);
      }
      if(packetbuffer[2] == CMD_MVMT)
      {
        chas_dir = packetbuffer[4]-0x30;
        sscanf(&packetbuffer[6],"%d",&dist_ang);
        Serial.print("dist_ang: ");Serial.println(dist_ang);
        MoveRobot(chas_dir,dist_ang);
      }
      
    }
    if(packetbuffer[1] == CT_FL)
    {
      lm_mode = packetbuffer[4]-0x30;
      sscanf(&packetbuffer[6],"%d",&n_times);
      if(packetbuffer[2] == 'L')
      {
        on_off = packetbuffer[4]-0x30;
        LaserControl(on_off);
      }
      else
      {
        fl_dist = GetMesurment_FL(lm_mode,n_times);
        Serial.print("FL Measured Average distance: ");Serial.println(fl_dist);
        bleuart.print("FL Average Distance in mm: ");bleuart.println(fl_dist);
      }
        
    }
    if(packetbuffer[1] == CT_RL)
    {
      lm_mode = packetbuffer[4]-0x30;
      sscanf(&packetbuffer[6],"%d",&n_times);
      if(packetbuffer[2] == 'L')
      {
        on_off = packetbuffer[4]-0x30;
        LaserControl(on_off);
        bleuart.print("Laser on_off :");bleuart.println(on_off);
      }
      else
      {
        rl_dist = GetMesurment_RL(lm_mode,n_times);
        Serial.print("RL Measured Average distance: ");Serial.println(rl_dist);
        bleuart.print("RL Average Distance in mm: ");bleuart.println(rl_dist);
      }
    }
    if(packetbuffer[1] == CT_CB)
    {
      if(packetbuffer[2] == CMD_LOS)
      {
        lm_mode = packetbuffer[4]-0x30;
        sscanf(&packetbuffer[6],"%d",&n_times);
        Serial.print("CB Laser mode: ");Serial.println(lm_mode);
        Serial.print("CB Laser n_times: ");Serial.println(n_times);
        int l_dist = GetDistanceFromCornerBeacon(lm_mode,n_times);
        Serial.print("Returned value: ");Serial.println(l_dist);
      }
      if(packetbuffer[2] == CMD_MVMT)
      {
        chas_dir = packetbuffer[4] -0x30;
        sscanf(&packetbuffer[6],"%d",&angle);
        Serial.print("CB Stepper Motor Dir: ");Serial.println(chas_dir);
        Serial.print("CB Stepper Motor angle: ");Serial.println(angle);
        RotateCornerBeacon(chas_dir,angle);
        //Serial.print("Returned distance: ");Serial.println(cb_dist);
        
      }
    }
    
/*
    if(packetbuffer[1] == CorBeacon)
    {
      CornerBeacon();
    }
    if(packetbuffer[1] == GetValues)
    {
      CurrentValues();
    }
    if(packetbuffer [1] == 'S')
    {
       BeaconDetection();
    }
*/
    
 }


}




 
void ReadResult()
{
  if(Serial1.available())
  {
  String a= Serial1.readString();// read the incoming data as string
  String b("!");
  String c;
  c=b+a;
  bleuart.println(c);
  }
}    


void ComHelp()
{
 bleuart.println("Command Help !!!");
 bleuart.println("================");
 bleuart.println("!V :Version");
 bleuart.println("!H :Help   ");
 bleuart.println("!M,XX,YYYYYYY : XX = Mode,YYYYYYY = steps");
 bleuart.println("!E,X : X = Enable/Disable (1/0)");
 bleuart.println("!S,XXXX : Speed");
 bleuart.println("!A,XXXX : Acceleration");
 bleuart.println("!X : Emergency Stop!!!");
}

/*// ********************* Blutooth control routines ***************
void SetupBlue()
{
  Bluefruit.begin();
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values
  Bluefruit.setName("LCbb_Marking_Robot");  // Name of the Bluetooth
  bleuart.begin(); // Configure and start the BLE Uart service
  
}

void StartAdv(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(bleuart); // Include the BLE UART (AKA 'NUS') 128-bit UUID
  Bluefruit.ScanResponse.addName();
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds
}
*/

void SelfTest()
{

}

void DispVer()
{
  bleuart.print(ver);
}

/**************************Read Packet from Mobile Device ************/
/*
uint8_t readPacket1(BLEUart *ble_uart, uint16_t timeout)
{
  uint16_t origtimeout = timeout, replyidx = 0;
  memset(packetbuffer, 0, READ_BUFSIZE);
  while (timeout--)
  {
    if (replyidx >= 20) break;
    while (ble_uart->available())
    {
      char c =  ble_uart->read();
    
      if (c == '!') 
      {
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

*/
/**************************Read Packet from Serial Port Device ************/
uint8_t readPacket2(uint16_t timeout)
{
  uint16_t origtimeout = timeout, replyidx = 0;
  memset(packetbuffer, 0, READ_BUFSIZE);
  while (timeout--)
  {
    if (replyidx >= 20) break;
    while (Serial1.available())
    {
      char c =  Serial1.read();
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


/*
void SelectPortChassisLaserMotor() //port 1 for mecannnum wheels (000)
{
  digitalWrite(S1, LOW);  // Initialize to 0 LSB
  digitalWrite(S2, LOW);  // Initialize to 0
  digitalWrite(S3, LOW);  // Initialize to 0 MSB
  Serial1.println(packetbuf);
}

void SelectPortMecannum() //port 2 for mecannnum wheels (001)
{
  digitalWrite(S1, HIGH);  // Initialize to 1 LSB
  digitalWrite(S2, LOW);  // Initialize to 0
  digitalWrite(S3, LOW);  // Initialize to 0 MSB
  Serial1.println(packetbuf);
}

*/         
void charToString(char S[], String &D)
{
 String rc(S);
 D = rc;
}



void SelectPort(uint8_t p_num)
{
  switch(p_num)
  {
    case 1:
      digitalWrite(S1,LOW);
      digitalWrite(S2,LOW);
      digitalWrite(S3,LOW);
      Serial.print("Selected Port: ");Serial.println(p_num);
      break;
    case 2:
      digitalWrite(S1,HIGH);
      digitalWrite(S2,LOW);
      digitalWrite(S3,LOW);
      Serial.print("Selected Port: ");Serial.println(p_num);
      break;
    case 3:
      digitalWrite(S1,LOW);
      digitalWrite(S2,HIGH);
      digitalWrite(S3,LOW);
      Serial.print("Selected Port: ");Serial.println(p_num);
      break;
    case 4:
      digitalWrite(S1,HIGH);
      digitalWrite(S2,HIGH);
      digitalWrite(S3,LOW);
      Serial.print("Selected Port: ");Serial.println(p_num);
      break; 
    default:
        Serial.println("Port not in use: ");
        break;                 
  }
}
void MoveRobot(uint8_t dir, uint32_t dist_ang)
{
  char sta='p';
  char CmdBuf[20];
  CmdBuf[0]='!';
  CmdBuf[1]='3';
  CmdBuf[2]='M';
  CmdBuf[3]=',';
  CmdBuf[4]=(char)(dir+0x30);
  CmdBuf[5]=',';
  //CmdBuf[6]=dist_ang;
  memcpy(&CmdBuf[6], &dist_ang, sizeof(dist_ang));
  //packetbuffer[6]=(char)dist_ang;
  //sscanf(&CmdBuf[6],"%d",&dist_ang);
  //sprintf(&CmdBuf[6],"%d",dist_ang);
  Serial.print("direction: ");Serial.print(dir); Serial.print("dist_ang: ");Serial.println(dist_ang);
  SelectPort(WC_SPORT);
  for(int s=0; s<20 ; s++)
  {
    Serial1.write(CmdBuf[s]);
    
  }
  //waiting for status form mecanumm arduino
  do
  {
    if(Serial1.available())
    {
      sta=Serial1.read();
    }
  }while((sta!='F')&&(sta!='f'));  
}

void SetSpeedMoveRobot(uint32_t sped)
{
  char sta='p';
  char CmdBuf[20];
  
  CmdBuf[0]='!';
  CmdBuf[1]='3';
  CmdBuf[2]='S';
  CmdBuf[3]=',';
  //CmdBuf[4]=(char)(sped+0x30);
  //CmdBuf[5]=',';
  //CmdBuf[6]=dist_ang;
  memcpy(&CmdBuf[4], &sped, sizeof(sped));
  //packetbuffer[6]=(char)dist_ang;
  //sscanf(&CmdBuf[6],"%d",&dist_ang);
  //sprintf(&CmdBuf[6],"%d",dist_ang);
  Serial.print("Mecanum wheel Speed: ");Serial.println(sped);
  SelectPort(WC_SPORT);
  for(int s=0; s<20 ; s++)
  {
    Serial1.write(CmdBuf[s]);
    
  }
  //waiting for status form mecanumm arduino
  do
  {
    if(Serial1.available())
    {
      sta=Serial1.read();
    }
  }while((sta!='F')&&(sta!='f'));  
}

void RotateSmRear(bool dir, uint32_t angle)
{
  char sta='p';
  char CmdBuf[10];
  CmdBuf[0]='!';
  CmdBuf[1]=(char)CT_RSM;  
  CmdBuf[2]='M';
  CmdBuf[3]=',';
  CmdBuf[4]=(char)(dir+0x30);
  CmdBuf[5]=',';
  CmdBuf[6]=angle;
  //packetbuffer[6]=(char)dist_ang;
  //sscanf(&CmdBuf[6],"%d",angle);
  Serial.print("direction: ");Serial.println(dir);
  Serial.println("Angle: ");Serial.println(angle);
  for(int s=0;s<7;s++)
  {
    Serial.print(CmdBuf[s],HEX);Serial.print(" ");
   
  }
  SelectPort(SM_SPORT);
  for(int s=0; s<7 ; s++)
  {
    Serial1.write(CmdBuf[s]);
    //bleuart.print(CmdBuf[s]);
  }
  do
  {
    if(Serial1.available())
    {
      sta=Serial1.read();
    }
  }while((sta!='F')&&(sta!='f')); 
}

void RotateSmFront(bool dir, uint32_t angle)
{
  char sta='p';
  char CmdBuf[10];
  CmdBuf[0]='!';
  CmdBuf[1]=(char)CT_FSM;
  CmdBuf[2]='M';
  CmdBuf[3]=',';
  CmdBuf[4]=(char)(dir+0x30);
  CmdBuf[5]=',';
  CmdBuf[6]=angle;
  //packetbuffer[6]=(char)dist_ang;
  //sprintf(&CmdBuf[6],"%d",angle);
  Serial.print("direction: ");Serial.println(dir);
  Serial.println("Angle: ");Serial.println((uint32_t)CmdBuf[6],DEC);
  for(int s=0;s<7;s++)
  {
    Serial.print(CmdBuf[s],HEX);Serial.print(" ");
   
  }
  SelectPort(SM_SPORT);
  for(int s=0; s<7 ; s++)
  {
    Serial1.write(CmdBuf[s]);
     //bleuart.print(CmdBuf[s]);
  }
  do
  {
    if(Serial1.available())
    {
      sta=Serial1.read();
    }
  }while((sta!='F')&&(sta!='f')); 
}

//Laser ON or OFF conditions set
void LaserControl(bool on_off) // format is !PLX where X = 0 or 1 and P = 4 or 5
{
  Serial.print("Turning Laser On/Off: ");Serial.println(on_off);
  for (int i=0; i<LMC_SEQ_SIZE; i++)
  {
    Serial1.write(laser_onoff_cmd_seq[on_off][i]);
    Serial.print(laser_onoff_cmd_seq[on_off][i],HEX);Serial.print(" ");
  }
  delay(1000);

/*   
 if (on_off == LASER_ON)
 {
   for (int i=0; i<LMC_SEQ_SIZE; i++)
   {
      Serial1.write(LaserONdata[i]);
   }
   Serial.println(on_off);
   delay(1000);
 }
 if (on_off == LASER_OFF)
 {
   for (int i=0; i<LMC_SEQ_SIZE; i++)
   {
      Serial1.write(LaserOFFdata[i]);
   }
   Serial.println(on_off);
   delay(1000);
 }
 */
}

uint32_t GetMesurment_FL(uint8_t mode=LMM_FAST, uint32_t n_times=1)
{
  FLDistance =0;Distance =0;
  Serial.print("Front Laser Measurement Mode: ");Serial.println(mode);
  Serial.print("Number of times: ");Serial.println(n_times);
  
  SelectPort(FL_SPORT);
  
  //j=(packetbuffer[3]-48)*10+(packetbuffer[4]-48);
  
  for(uint32_t k=0; k<n_times; k++)
  { 
    int timeout=5000;  

    Serial.flush(); //Clear buffer
/*    
    while(Serial1.available()) 
    {
      i=Serial1.read();
    }  // clear Buffer
*/    
    for (int i=0; i<LMC_SEQ_SIZE; i++)
    {
      Serial1.write(laser_meas_seq[mode][i]);
    }
/*    if (mode == LMM_FAST)
    {
      for (i=0;i<9;i++)
      Serial1.write(OneShotFast[i]);
    } 
    if (mode == LMM_SLOW)
    {
      for (i=0;i<9;i++)
        Serial1.write(OneShotSlow[i]);
    } 
    if (mode == LMM_AUTO)
    {
      for (i=0;i<9;i++)
        Serial1.write(OneShotAuto[i]);
    }
*/      
   // delay(100);
    while(timeout-- && !Serial1.available())
    {
      if (timeout==0)break;
      delay(1);
    }
    delay(100);
    if(Serial1.available())
    {
      for (int i=0; i<LMR_SEQ_SIZE; i++)
      {
      if(Serial1.available())
      MeasResult[i]=Serial1.read();
      }
      
      Distance = MeasResult[6]*256*256*256+MeasResult[7]*256*256+MeasResult[8]*256+MeasResult[9];
      Serial.print("FL Measured distance: ");Serial.println(Distance);
      //bleuart.print("FL Distance in mm: ");bleuart.println(Distance);
    }
    FLDistance += Distance;
  }
  //if(n_times)
  //{
    FLDistance = FLDistance/n_times;
  //}
  return FLDistance; 
}

uint32_t GetMesurment_RL(uint8_t mode=LMM_FAST, uint32_t n_times=1)
{
  RLDistance = 0;Distance =0;
  Serial.print("Rear Laser Measurement Mode: ");Serial.println(mode);
  Serial.print("Number of times: ");Serial.println(n_times);
  
  SelectPort(RL_SPORT);
  //j=(packetbuffer[3]-48)*10+(packetbuffer[4]-48);
  
  for(uint32_t k=0; k<n_times; k++)
  { 
    int timeout=5000;

    Serial1.flush(); //Clear buffer
/*    
    while(Serial1.available()) 
    {
      i=Serial1.read();
    }  // clear Buffer
*/ 
   
    for (int i=0; i<LMC_SEQ_SIZE; i++)
    {
      Serial1.write(laser_meas_seq[mode][i]);
    }
/*
    if (mode == LMM_FAST)
    {
      for (i=0;i<9;i++)
      Serial1.write(OneShotFast[i]);
    } 
    if (mode == LMM_SLOW)
    {
      for (i=0;i<9;i++)
        Serial1.write(OneShotSlow[i]);
    } 
    if (mode == LMM_AUTO)
    {
      for (i=0;i<9;i++)
        Serial1.write(OneShotAuto[i]);
    }
*/
   // delay(100);
    while(timeout-- && !Serial1.available())
    {
      if (timeout==0)break;
      delay(1);
    }
    delay(100);
    if(Serial1.available())
    {
      for (int i=0; i<LMR_SEQ_SIZE; i++)
      {
        if(Serial1.available())
        MeasResult[i]=Serial1.read();
      }
      
      Distance = MeasResult[6]*256*256*256+MeasResult[7]*256*256+MeasResult[8]*256+MeasResult[9];
      Serial.print("RL Measured distance: ");Serial.println(Distance);
      //bleuart.print("RL Distance in mm: ");bleuart.println(Distance);
    }
    RLDistance += Distance;
  }
  //if(n_times)
  //{
     RLDistance = RLDistance/n_times;
  //}
  return RLDistance;  
}

uint32_t GetDistanceFromCornerBeacon(uint8_t lm_mode=LMM_FAST, uint32_t n_times=1)
{
  uint32_t cl_dist;
  beacon_responce_flag = false;
  
  char CmdBuf[20] = {0};
  CmdBuf[0] = '!';
  CmdBuf[1] = 'O';
  CmdBuf[2] = lm_mode+0x30;
  memcpy(&CmdBuf[3],&n_times,sizeof(n_times));
  for(int s = 0; s<20;s++)
  {
    clientUart.write(CmdBuf[s]);
  }
  while(!beacon_responce_flag)
  {
    delay(1);
  }
  sscanf(&corner_beacon_responce[0],"%d",&cl_dist);
  /*for(int s=0;s<20;s++)
  {
    Serial.print(corner_beacon_responce[s]);Serial.print(" ");
  }*/
  Serial.print("Distance received from beacon: ");Serial.println(cl_dist);Serial.flush();
  return cl_dist;
}

void RotateCornerBeacon(uint8_t dir, uint32_t angle)
{
  Serial.println("IAM");
  //uint32_t cb_dist;
  beacon_responce_flag = false;
  char CmdBuf[20] = {0};
  CmdBuf[0] = '!';
  CmdBuf[1] = 'M';
  CmdBuf[2] = dir+0x30;
  memcpy(&CmdBuf[3],&angle,sizeof(angle));
  for(int s=0;s<20;s++)
  {
    Serial.print(CmdBuf[s]);Serial.print(" ");
  }
  clientUart.write(CmdBuf,20);
  while(!beacon_responce_flag)
  {
    delay(1);
  }
  
  //sscanf(&corner_beacon_responce[0],"%d",&cb_dist);
  //Serial.print("Corner Beacon set to (Degree): ");Serial.println(cb_dist);
  Serial.flush();
  //return angle;
}
