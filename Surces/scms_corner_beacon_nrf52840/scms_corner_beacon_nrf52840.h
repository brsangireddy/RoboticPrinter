#include <bluefruit.h>
#include <string.h>
#include <Arduino.h>

BLEUart bleuart;

char m;
uint32_t wheelSteps=0;

long temp1=0;
long steps;
long temp2=0 ;



long current_pos=0;
//long new_pos;






#define  led1 7
#define  dirPin 29
#define  stepPin 28
#define sensor_pin 5


#define EnMotorsPin 30

#define READ_BUFSIZE                    (20)

void Home_pos();
void rotate_sm(bool dir, int32_t new_pos);

uint8_t readPacket1 (BLEUart *ble_uart, uint16_t timeout);
uint8_t readPacket2 (uint8_t *serial_uart, uint16_t timeout);
float   parsefloat (uint8_t *buffer);
char packetbuffer[READ_BUFSIZE + 1];

String ver =   "Mecanum FW Ver 1.0";


uint8_t set_angle();
//uint8_t set_corner();
//uint8_t motor_beacon();

void setup(void)
{
  SelfTest();  // call self test routine
  SetupSerial(); // Setup PC Serial Port for Testing
  SetupSerial1(); // Rx/Tx pins of controller
  SetupBlue(); // setup Bluetooth
  StartAdv(); // Set up and start advertising
  //SetupMecanum(); //Setup mecanum 
 // while ( !Serial ) delay(10);   // for nrf52840 with native usb for testing
  Serial.println("Mecanum Robot Testing !!!");
  Serial1.println("Mecanum Robot Testing !!!");
  pinMode(stepPin,OUTPUT);
  pinMode(dirPin,OUTPUT);
  pinMode(EnMotorsPin,OUTPUT);
  pinMode(sensor_pin,INPUT);
  digitalWrite(EnMotorsPin,LOW);
  digitalWrite(dirPin,LOW);

//attachInterrupt(sensor_pin,set_angle,RISING);


     Home_pos();



}

// ************ Loop Function ***************************

void loop(void)
{
  ReadData();
  //RunMotors();
}


void SetupSerial()
{ 
  Serial.begin(9600);       // PC Serial Termial
  delay(1000);
}

void SetupSerial1()
{ 
  Serial1.begin(9600);       // Rx/Tx pins of controllers
  delay(1000);
  Serial1.println("Mecanum Robot Testing !!!");
}

void ReadData()
{

  
  bool dir;
  int32_t new_pos;
if(bleuart.available()||Serial1.available())
 {
  if(bleuart.available())
  {
  uint8_t len = readPacket1(&bleuart, 100);  // Wait for new data arrive
  if (len == 0) return;
  }
  if(Serial1.available())
  {
  uint8_t len = readPacket2(100);  // Wait for new data arrive
  
  if (len == 0) return; 
  }
  for(int i=0;i<20;i++)
  {
    Serial.print(packetbuffer[i]);
    Serial.print(" ");
  }
    


  Serial.println(packetbuffer[0]);
  Serial.println(packetbuffer[1]);
  
  if (packetbuffer[1] == 'V')   // Version
      DispVer();
  if (packetbuffer[1] == 'M')   // Motors Accelerated movement to position
      dir = packetbuffer[2]-0x30;
      sscanf(&packetbuffer[3],"%d",&new_pos);
      bleuart.print(new_pos);
      Serial.print("new pos: ");
      Serial.println(new_pos,DEC);
      //new_pos=90;
      rotate_sm(dir,new_pos);
      
  //if (packetbuffer[1] == 'E')   // Motors Enable/Disable  E0=Enable E1 is disable
      //enable_m();  
  if (packetbuffer[1] == 'A')   // Sets maxspeed
      led_1();  
  if (packetbuffer[1] == 'S')   // Sets Acceleration
      set_angle();  
  //if (packetbuffer[1] == 'H')   // Displays help on Commands
     // ComHelp();  
  //if (packetbuffer[1] == 'X')   // at corner Stop
     // set_corner();                
   //Serial.println("beacon.....");
 }
}



void SetupBlue()
{
  Bluefruit.begin();
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values
  Bluefruit.setName("Vgroup beacon");  // Name of the Bluetooth
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

void SelfTest()
{
pinMode(EnMotorsPin, OUTPUT);
//digitalWrite(EnMotorsPin, LOW);        // Enable = HIGH/ Disable LOW
}

void DispVer()
{
 bleuart.println(ver);
}

/**************************Read Packet from Mobile Device ************/
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
  //Serial.println(packetbuffer[replyidx]);
  if (!replyidx)  // no data or timeout
    return 0;
  if (packetbuffer[0] != '!')  // doesn't start with '!' packet beginning
    return 0;
  return replyidx;
}


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






//#define sensor_pin 20



uint8_t led_1()
{
  digitalWrite(led1,HIGH);
  bleuart.println("hhhh");
  delay(1000);
  digitalWrite(led1,LOW);
  
  
}

#define CW 1
#define ACW 0
#define SPD 222.222     //steps per one degree rotation of stepper motor

void rotate_sm(bool dir, int32_t new_pos)  //new_pos in degree
{
  int32_t net_angle=0;
  if(new_pos>90)
  {
    new_pos = 90;
  }
  
  if(dir==ACW)
  {
    new_pos = -new_pos;
    
  }
  net_angle= new_pos -current_pos;
  current_pos=new_pos;
  Serial.print("current pos: ");Serial.print(current_pos);
  if(net_angle<0)
  {
    digitalWrite(dirPin,LOW);      //
    net_angle = -net_angle;
    Serial.print("dir pin :LOW");
  }
  else
  {
    digitalWrite(dirPin,HIGH);
    Serial.print("dir pin :HIGH");

  }
  if(net_angle > 90)
  {
    net_angle=180;
  }
  wheelSteps=net_angle*SPD;
  
  
  delay(500);
  for(long steps = 0; steps < wheelSteps; steps++){
       
         digitalWrite(stepPin,HIGH); 
         delayMicroseconds(100); 
         digitalWrite(stepPin,LOW); 
         delayMicroseconds(100);
   }
  
  


  
 /*  int net_angle;
  if(dir=ACW)
  {
    net_angle =new_pos-current_pos;
    
    if(net_angle<0)
    {
      net_angle=-net_angle;
      digitalWrite(dirPin,LOW);
      current_pos=new_pos;
    }
    else
    {
      digitalWrite(dirPin,HIGH);
      current_pos=new_pos;
    }
    wheelSteps=net_angle*222.222;
  }
  if(dir==0)
  {
    new_pos=-new_pos;
    net_angle =new_pos+current_pos;
    if(net_angle<0)
    {
      net_angle=-net_angle;
      digitalWrite(dirPin,HIGH);
      current_pos=new_pos;
    }
    else
    {
      digitalWrite(dirPin,LOW);
      current_pos=new_pos;
    }
    wheelSteps=net_angle*222.222;
  }
   for(long steps = 0; steps < wheelSteps; steps++){
                 
                   
         digitalWrite(stepPin,HIGH); 
         delayMicroseconds(100); 
         digitalWrite(stepPin,LOW); 
         delayMicroseconds(100);
   }
*/
  
}


/*

uint8_t motor_beacon()
{
  digitalWrite(EnMotorsPin,LOW);
  Serial.print("wheel:");
  m=(char)packetbuffer[2];
  
  //wheelSteps = (packetbuffer[3] - 48) * 100000 + (packetbuffer[4] - 48)*10000 +(packetbuffer[5] - 48)*1000 + 
               //(packetbuffer[6] - 48)*100 + (packetbuffer[7] - 48)*10 + (packetbuffer[8] - 48); // read the data
 //Serial.println(wheelSteps);
  wheelSteps = (packetbuffer[3] - 48) * 100 + (packetbuffer[4] - 48)*10 +(packetbuffer[5] - 48);
  wheelSteps = (wheelSteps*222.222);
  Serial.println(wheelSteps);

  if(m=='C') 
  {
    digitalWrite(dirPin,LOW);// clock wise direction
              if(temp2>0)
                    {
                      
        
                                    for(long steps = temp2;steps > 0;steps--){
                                      temp2--;
                                      digitalWrite(stepPin,HIGH);  
                                      delayMicroseconds(100); 
                                      digitalWrite(stepPin,LOW); 
                                      delayMicroseconds(100);         

                                 }   
                                    //if(temp1 > 40000) digitalWrite(EnMotorsPin,HIGH); 
                                    for(long steps = 0; steps < wheelSteps; steps++){
                                    
                                    Serial.println(temp1);
                                    if(temp1 > 40000){
                                      digitalWrite(EnMotorsPin,HIGH);           
                                      break;           
                                    }
                                    temp1++;
                                         digitalWrite(stepPin,HIGH); 
                                         delayMicroseconds(100); 
                                         digitalWrite(stepPin,LOW); 
                                         delayMicroseconds(100);
                                                  

                                 }
      
                   }
                   else
                             {
                                       // if(temp1 > 40000) digitalWrite(EnMotorsPin,HIGH);       
                                        for(long steps = 0; steps < wheelSteps; steps++){
                                          
                                         
                                        
                                           if(temp1 > 40000)
                                           {
                                            digitalWrite(EnMotorsPin,HIGH);
                                            break;
                                           }
                                           temp1++;
                                            Serial.println(temp1);                 
                                             digitalWrite(stepPin,HIGH); 
                                             delayMicroseconds(100); 
                                             digitalWrite(stepPin,LOW); 
                                             delayMicroseconds(100);
                                                      

                                        }
   
                }
                   
 
            

 
        }
  if(m=='A')
  {
    digitalWrite(dirPin,HIGH);  
  
    if(temp1>0)
          {
      
                          for(long steps = temp1;steps > 0;steps--){
                            temp1--;
                           digitalWrite(stepPin,HIGH); 
                           delayMicroseconds(100); 
                           digitalWrite(stepPin,LOW); 
                           delayMicroseconds(100);         

                          }
                          for(long steps = 0; steps < wheelSteps; steps++){
                              Serial.println(temp2);
                              if(temp2 > 40000) {
                                digitalWrite(EnMotorsPin,HIGH);  
                                break;     
                              }
                               temp2++;
                                   digitalWrite(stepPin,HIGH); 
                                   delayMicroseconds(100); 
                                   digitalWrite(stepPin,LOW); 
                                   delayMicroseconds(100);         
                                   }      
    
    
     }
     else
     {
                                for(long steps = 0; steps < wheelSteps; steps++){
                                  
                                  
                                  if(temp2 > 40000) {
                                    digitalWrite(EnMotorsPin,HIGH);
                                    break;
                                  }
                                  temp2++;
                                  Serial.println(temp2);       
                                       digitalWrite(stepPin,HIGH); 
                                       delayMicroseconds(100); 
                                       digitalWrite(stepPin,LOW); 
                                       delayMicroseconds(100);
                                         
                                      }
                
    }
 }

} */
uint8_t set_angle()
{
  digitalWrite(EnMotorsPin,LOW);
  digitalWrite(dirPin,LOW);
  for(int x = 0; x < 40000; x++) {
    if(digitalRead(sensor_pin))digitalWrite(EnMotorsPin,HIGH);   
    digitalWrite(stepPin,HIGH); 
    delayMicroseconds(100); 
    digitalWrite(stepPin,LOW); 
    delayMicroseconds(100); 
    //Serial.println("laser.....");
    //bleuart.println("beta bulls");  
  }
  delay(1000);
  digitalWrite(dirPin,HIGH);
  for(int x = 0; x < 80000; x++) {
    if(digitalRead(sensor_pin))digitalWrite(EnMotorsPin,HIGH);
    digitalWrite(stepPin,HIGH); 
    delayMicroseconds(100); 
    digitalWrite(stepPin,LOW); 
    delayMicroseconds(100);
  }
  delay(1000);
      
}


void Home_pos()
{
  if(digitalRead(sensor_pin))
{
  digitalWrite(EnMotorsPin,LOW);
  digitalWrite(dirPin,LOW);   //clk wise
  for(int x = 0; x < 6000; x++) {
    digitalWrite(stepPin,HIGH); 
    delayMicroseconds(100); 
    digitalWrite(stepPin,LOW); 
    delayMicroseconds(100); 
  }
  delay(1000);
  digitalWrite(dirPin,HIGH);   //anti_clk wise
  for(int x = 0; x < 12000; x++) {
    if(digitalRead(sensor_pin))digitalWrite(EnMotorsPin,HIGH);
    digitalWrite(stepPin,HIGH); 
    delayMicroseconds(100); 
    digitalWrite(stepPin,LOW); 
    delayMicroseconds(100);
  }
  delay(1000);
}
else
{
  set_angle();
 
}
digitalWrite(EnMotorsPin,LOW);

}

/*
uint8_t set_corner()
{
    digitalWrite(EnMotorsPin,LOW);
    m=(packetbuffer[2] - 48);
  if(m==1)digitalWrite(dirPin,LOW);
  if(m==0)digitalWrite(dirPin,HIGH);
  
  for(int steps = 0; steps < wheelSteps; steps++){
          if(temp1>40000) digitalWrite(EnMotorsPin,HIGH);  
          temp1 =steps;               
          digitalWrite(stepPin,HIGH); 
          delayMicroseconds(100); 
          digitalWrite(stepPin,LOW); 
          delayMicroseconds(100);  
        }
     
  
}*/
