/*********************************************************************
 This is a software for Laser Becaon Module
 Written by Dr Azam, Date : 21st June 2019
 
*********************************************************************/

#include <bluefruit.h>
#include <string.h>
#include <Arduino.h>
#include <SoftwareSerial.h>


#define READ_BUFSIZE                    (20)

// Uart over BLE service
BLEUart bleuart;
BLEDfu  bledfu;  // OTA DFU service
BLEDis  bledis;  // device information

BLEBas  blebas;  // battery
// ************* Functions decalarion here *************************

void SelfTest();  // Self Test
void SetupLaser(); // Setup Laser
void SetupMotor(); // Setup Stepper Motor
void SetupBlue(); // Setup Blutooth

void StartAdv();  // Start Adv
void DispVer();   // Display Version
void DispOneshot(); // Display Oneshot measurement
void DispStatus(); // Display Status
//void LaserON();   //Turn ON Laser
//void LaserOFF();  // Turn OFF Laser
void LaserControl();  // Laser Control
void LaserEnable(); // Laser module Enable/Disbale 
void StepMotor();  // Stepper Motor functions

void D_Oneshot();
 
uint8_t readPacket (BLEUart *ble_uart, uint16_t timeout);
float   parsefloat (uint8_t *buffer);
char packetbuffer[READ_BUFSIZE+1];

// ************ Variables used ***************************

String ver = "Laser Beacon FW Ver 1.0";
//Serial.println(ver);

extern char packetbuffer[];   // Packet buffer



bool mode;
int32_t n_time;
bool s_button;


const byte EN_LASER =4;
const byte SmDir=22;
const byte SmPulse=21;
const byte SmEn=20;
const byte s1=19;
const byte s2=20;
const byte s3=21;
const byte en=22;

int i,j,k;
int Distance = 0;
uint32_t summ=0;
uint16_t replyidx = 0;
//SoftwareSerial portOne(15,17);


#include "scms_corner_beacon_nrf52840.h"

// ************* Setup functions *************************
void setup(void)   
{
  //pinMode(s1,OUTPUT);
  //pinMode(s2,OUTPUT);
  //pinMode(s3,OUTPUT);
  //pinMode(en,OUTPUT);
  SetupLaser();  // Setup Laser
  SetupMotor(); // Setup Stepper Motor
  SetupBlue(); // setup Bluetooth 
  StartAdv(); // Set up and start advertising
  SelfTest();  // call self test routine
 //if(portOne)
   // portOne.begin(9600);
 //else
 // Serial.println("portOne ERROR: ");
  
  //digitalWrite(s1,LOW);
  //digitalWrite(s2,HIGH);
 // digitalWrite(s3,LOW);
  //digitalWrite(en,LOW);

  Home_pos();
  Serial.println("Home detected.");
}

// ************ Loop Function ***************************

void loop(void)
{
  //char ack[20];
  //ack[0]='C';
  //ack[1]='B';
  

  
  uint16_t len = readPacket(&bleuart, 500);  // Wait for new data arrive
  if (len == 0) return;
  Serial.print("Serial Received packet: ");
  for(uint16_t i=0;i<len;i++)
  {
    Serial.print(packetbuffer[i],HEX);Serial.print(" ");
  }
  //SendToOnChassisBeaconF();
//#if 1
  // printHex(packetbuffer, len);           // for debug
  if (packetbuffer[1] == 'V')   // Version
      DispVer();
  if (packetbuffer[1] == 'D')   // Version
      detection_chasis();    
  if (packetbuffer[1] == 'O')   // One shot Measurment Comamnd
  {
      mode = packetbuffer[2]-0x30;
      //memcpy(&n_time,&packetbuffer[3],sizeof(n_time));
      sscanf(&packetbuffer[3],"%d",&n_time);
      Serial.print("n_time: ");
      Serial.println(n_time,DEC);
      //new_pos=90;
      DispOneshot(mode,n_time);
      //bleuart.write(ack,20);
      //bleuart.write(n_time);
  }
      
  if (packetbuffer[1] == 'S')   // Status
      set_angle();
  if (packetbuffer[1] == 'L')   // Laser Control
  {
    s_button = packetbuffer[2]-0x30;
    LaserControl(s_button);
  }
      
  if (packetbuffer[1] == 'E')   // Laser Module Enable/Disable
      LaserEnable(); 
  if (packetbuffer[1] == 'M')   // StepperMotor Control
  {
      dir = packetbuffer[2]-0x30;
      //memcpy(&new_pos ,&packetbuffer[3],sizeof(new_pos));
      sscanf(&packetbuffer[3],"%d",&new_pos);
      bleuart.print(new_pos);
      Serial.print("new pos: ");
      Serial.println(new_pos,DEC);
      //new_pos=90;
      rotate_sm(dir,new_pos);
      //Serial.write('C');
      //bleuart.write("ABCD",4);
      //bleuart.write(new_pos);
  }
  
  
//#endif              
}
/*
uint8_t SendToOnChassisBeaconF()
{
  for(int i=0; i<replyidx;i++)
  {
    Serial.print(packetbuffer[i]);
    portOne.write(packetbuffer[i]);
  }
}*/
// ******************** Sub routunes start heare **********************
void SetupLaser()  // Setup Laser
{
  pinMode(EN_LASER, OUTPUT); // Turn Laser Module ON
  digitalWrite(EN_LASER, HIGH); 
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

void SetupMotor() // Setup Stepper Motor
{
   pinMode(stepPin,OUTPUT);
  pinMode(dirPin,OUTPUT);
  pinMode(EnMotorsPin,OUTPUT);
  pinMode(sensor_pin,INPUT);
  digitalWrite(EnMotorsPin,LOW);
  digitalWrite(dirPin,LOW);

}

// callback invoked when central connects
void connect_callback(uint16_t conn_handle)
{
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));

  Serial.print("Connected to ");
  Serial.println(central_name);
}

/**
 * Callback invoked when a connection is dropped
 * @param conn_handle connection where this event happens
 * @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
 */
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println();
  Serial.println("Disconnected");
}
void SetupBlue()
{
   Bluefruit.autoConnLed(true);

  // Config the peripheral connection with maximum bandwidth 
  // more SRAM required by SoftDevice
  // Note: All config***() function must be called before begin()
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);

  Bluefruit.begin();
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values
  Bluefruit.setName("LCbb_CBEACON_00");
  //Bluefruit.setName(getMcuUniqueID()); // useful testing with multiple central connections
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  // To be consistent OTA DFU should be added first if it exists
  bledfu.begin();

  // Configure and Start Device Information Service
  bledis.setManufacturer("Adafruit Industries");
  bledis.setModel("Bluefruit Feather52");
  bledis.begin();

  // Configure and Start BLE Uart Service
  bleuart.begin();

  // Start BLE Battery Service
  blebas.begin();
  blebas.write(100);

  // Set up and start advertising
  //startAdv();
  /*
  Bluefruit.begin();
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values
  Bluefruit.setName("LCbb_CBEACON_00");  // Name of the Bluetooth Laser Beacon 1
  bleuart.begin(); // Configure and start the BLE Uart service
  */
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

void SelfTest()
{
  DispStatus();
  delay(1000);
  for (i=0;i<LMC_SEQ_SIZE;i++)
  {
    Serial1.write(laser_onoff_cmd_seq[0][i]);
  }
  delay(1000);
  for (i=0;i<LMC_SEQ_SIZE;i++)
  {
    Serial1.write(laser_onoff_cmd_seq[1][i]);
  }
/*  LaserON();      // Turn Laser ON
  delay(1000);    // Delay
  LaserOFF();     // Turn Laser OFF
*/
}

void DispVer()
{  
    Serial.println(ver);
    bleuart.println(ver);
}

 
/**************************Read Packet from Mobile Device ************/
uint8_t readPacket(BLEUart *ble_uart, uint16_t timeout) 
{
  uint16_t origtimeout = timeout;
  replyidx = 0;
  memset(packetbuffer, 0, READ_BUFSIZE);
  while (timeout--) 
  {
    if (replyidx >= 20) break;
    while (ble_uart->available()) 
    {
      char c =  ble_uart->read();
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

// ******************* One Shot Measurement ********************
void DispOneshot(bool mode, int32_t n_time)
{ 
  //char ack[20];
  summ = 0;
  //n_time =(packetbuffer[3]-48)*10+(packetbuffer[4]-48);
  for(k=0;k < n_time;k++)
  { 
    int timeout=5000;  
    Serial1.flush();
    /*while(Serial1.available()) 
    {
      i=Serial1.read();
    }  // clear Buffer
    */
    for(int i = 0; i<LMC_SEQ_SIZE ; i++)
    {
      Serial1.write(laser_meas_seq[mode][i]);
    }
    /*
    if (mode == 1)
    {
      for (i=0;i<9;i++)
      Serial1.write(OneShotFast[i]);
    }
    if (mode == 2)
    {
      for (i=0;i<9;i++)
      Serial1.write(OneShotSlow[i]);
    }
    if (mode == 3)
    {
      for (i=0;i<9;i++)
      Serial1.write(OneShotAuto[i]);
    }   
    */
    while(timeout-- && !Serial1.available())
    {
      if (timeout==0)break;
      delay(1);
    }
    delay(100);
    if(Serial1.available())
     {
      for (i=0;i<13;i++)
         {
          if(Serial1.available())
          MeasResult[i]=Serial1.read();
         }
     Distance = MeasResult[6]*256*256*256+MeasResult[7]*256*256+MeasResult[8]*256+MeasResult[9];
     Serial.println(Distance);
     summ += Distance; 
     }
 }
 summ /= n_time;
 Serial.print("avg :");Serial.println(summ);
 //ack[0]=summ;
 //summ = 20;
 bleuart.print(summ); // return to LCbb_RCONTROL_00
 //bleuart.print("avg:");
 //bleuart.write(summ);
}



void D_Oneshot()
{ 
  
  for(k=0;k < 1;k++)
  { 
    int timeout=5000;  
    Serial1.flush();
   
    for(int i = 0; i<LMC_SEQ_SIZE ; i++)
    {
      Serial1.write(laser_meas_seq[0][i]);
    }
    
    while(timeout-- && !Serial1.available())
    {
      if (timeout==0)break;
      delay(1);
    }
    delay(100);
    if(Serial1.available())
     {
      for (i=0;i<13;i++)
         {
          if(Serial1.available())
          MeasResult[i]=Serial1.read();
         }
     Distance = MeasResult[6]*256*256*256+MeasResult[7]*256*256+MeasResult[8]*256+MeasResult[9];
     Serial.println(Distance);
     
     }
 }

 //bleuart.print(Distance);
 
}



 // ******************* Display Status ********************
void DispStatus()
{  
  for (i=0;i<5;i++)
  {
       Serial1.write(ReadStatus[i]);
  }
    
  if(Serial1.available())//  use time out later
   {
     for (i=0;i<8;i++)
      {
         if (Serial1.available())
         MeasResult[i]=Serial1.read();
      }
         
     Serial.println(MeasResult[7],HEX);
     bleuart.println(MeasResult[7],HEX);
   }
 
}
 
void LaserControl(bool s_button)
{ 
  Serial.print("Turning Laser On/Off: ");Serial.println(s_button);
  for (int i=0; i<LMC_SEQ_SIZE; i++)
  {
    Serial1.write(laser_onoff_cmd_seq[s_button][i]);
    Serial.print(laser_onoff_cmd_seq[s_button][i],HEX);Serial.print(" ");
  }
  delay(1000);
 /*if (s_button == 1)
    LaserON();
 if (s_button == 0)
    LaserOFF();
 */
}

void LaserEnable()
{ 
  if (packetbuffer[2] == '1')
  {
     digitalWrite(EN_LASER, HIGH); 
     delay(500);
     Serial1.write(0x55); // Autobaud laser device
     delay(1000);
     SelfTest();    // Self test laser
  }    
  if (packetbuffer[2] == '0')
    digitalWrite(EN_LASER, LOW);   // Disable Laser Module
}

/*
void LaserON()
{ 
  for (i=0;i<9;i++)
  Serial1.write(LaserONdata[i]);
  delay(1000);
}
 
void LaserOFF()
{ 
  for (i=0;i<9;i++)
  Serial1.write(LaserOFFdata[i]);
  delay(1000);
}
*/
