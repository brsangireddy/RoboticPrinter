#ifndef SCMS_CONER_BEACON_NRF52840_H
#define SCMS_CONER_BEACON_NRF52840_H

char m;
uint32_t wheelSteps=0;

long temp1=0;
long steps;
long temp2=0 ;


bool dir;
int32_t new_pos=0;

long current_pos=0;
//long new_pos;

#define  led1 7
#define  dirPin 29
#define  stepPin 28
#define sensor_pin 5


#define EnMotorsPin 30

#define SCP_180D 40000 // Sttep Count Per 180 Degrees rotation
#define SCP_360D 80000 // Sttep Count Per 360 Degrees rotation
#define SCP_10D  6000 // Sttep Count Per 10 Degrees rotation
#define SCP_20D  12000 // Sttep Count Per 20 Degrees rotation

#define NUM_OF_LMMS      3 //Number of Lase Measurement Modes
#define LMC_SEQ_SIZE     9 //Laser Measurement Command array size
#define LMR_SEQ_SIZE    13 //Laser Measurement Result array size
//Laser measurement command & result sequences
const uint8_t laser_meas_seq[NUM_OF_LMMS][LMC_SEQ_SIZE] = {
                                                      {0xAA,0x00,0x00,0x20,0x00,0x01,0x00,0x02,0x23}, //OneShotFast mode
                                                      {0xAA,0x00,0x00,0x20,0x00,0x01,0x00,0x01,0x22}, //OneShotSlow mode
                                                      {0xAA,0x00,0x00,0x20,0x00,0x01,0x00,0x00,0x21}  //OneShotAuto mode 
                                                    };
//const unsigned char OneShotFast[] = {0xAA,0x00,0x00,0x20,0x00,0x01,0x00,0x02,0x23};
//const unsigned char OneShotSlow[] = {0xAA,0x00,0x00,0x20,0x00,0x01,0x00,0x01,0x22};
//const unsigned char OneShotAuto[] = {0xAA,0x00,0x00,0x20,0x00,0x01,0x00,0x00,0x21};

const unsigned char ReadStatus[] = {0xAA,0x80,0x00,0x00,0x80};
const uint8_t laser_onoff_cmd_seq[2][LMC_SEQ_SIZE] = {
                                                        {0xAA,0x00,0x01,0xBE,0x00,0x01,0x00,0x00,0xC0},  
                                                        {0xAA,0x00,0x01,0xBE,0x00,0x01,0x00,0x01,0xC1}
                                                      };
//const uint8_t LaserONdata[] = {0xAA,0x00,0x01,0xBE,0x00,0x01,0x00,0x01,0xC1};
//const uint8_t LaserOFFdata[] ={0xAA,0x00,0x01,0xBE,0x00,0x01,0x00,0x00,0xC0};

uint8_t MeasResult[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
 



//#define READ_BUFSIZE                    (20)

void Home_pos();
void rotate_sm(bool dir, int32_t new_pos);
uint8_t set_angle();
void detection_chasis();


#define CW 1
#define ACW 0
#define SPD 222.222     //steps per one degree rotation of stepper motor

void rotate_sm(bool dir, int32_t new_pos)  //new_pos in degree
{
  int32_t net_angle=0;
  if(new_pos > 90)
  {
    new_pos = 90;
    Serial.print("cut_off_pos: ");
    Serial.println(new_pos,DEC);
  }
  
  
  if(dir==ACW)
  {
    new_pos = -new_pos;
    
  }
  net_angle= new_pos -current_pos;
  current_pos=new_pos;
  //Serial.print("current pos: ");Serial.print(current_pos);
  if(net_angle<0)
  {
    digitalWrite(dirPin,LOW);      //
    net_angle = -net_angle;
    Serial.println(net_angle);
    //Serial.print("dir pin :LOW");
  }
  else
  {
    digitalWrite(dirPin,HIGH);
    //Serial.print("dir pin :HIGH");

  }
  if(net_angle > 180)
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
    
}



void detection_chasis()  //new_pos in degree
{
  char buf[20];
  //char buf1[20];
  wheelSteps=10000;
  int32_t angle=0;
  int32_t det=0;
  byte i=0;
  byte dir=0;
 digitalWrite(dirPin,LOW);     
   while(i<2)
   {
      angle=0;
      det=0;  
      for(long steps = 0; steps < wheelSteps; steps++){
    
       
        if(steps==det+223)
        {
          D_Oneshot();
          det=steps;
          if(Distance < 500) 
          {           
            Serial.print("Distance :");
            Serial.println(Distance);
            angle=(steps*360)/80000;
           
            Serial.print("angle :");
            Serial.println(angle);
            i=1;
            break;        
          }
        }     
             digitalWrite(stepPin,HIGH); 
             delayMicroseconds(100); 
             digitalWrite(stepPin,LOW); 
             delayMicroseconds(100);
       }
    
      digitalWrite(dirPin,HIGH);
      dir=1;
      delay(2000); 
      i++;
   } 

    buf[0] = dir;
    memcpy(&buf[1], &Distance, sizeof(Distance));
    memcpy(&buf[sizeof(Distance)+1], &angle, sizeof(angle));
    Serial.print("Transmitting buffer: ");
    for(int s=0;s<20;s++)
    {
      Serial.print(buf[s],HEX); Serial.print(" ");
    }
   //sprintf(&buf,"%d",Distance);
   bleuart.write(buf,20);
//   sprintf(&buf1,"%d\n",dir);
//   
//   
//   sprintf(&buf1[1],"%d",angle);
//   bleuart.write(buf1);
//   
//   
//      bleuart.print(angle);
//      bleuart.print(Distance);
  
     
}




uint8_t set_angle()
{
  long x;bool return_startP = 1;
  digitalWrite(EnMotorsPin,LOW);
  digitalWrite(dirPin,LOW);
  for(x = 0; x < SCP_180D; x++)
  {
    if(digitalRead(sensor_pin))
    {
      digitalWrite(EnMotorsPin,HIGH);
      return_startP = 0;
      break;   
    }
    digitalWrite(stepPin,HIGH); 
    delayMicroseconds(100); 
    digitalWrite(stepPin,LOW); 
    delayMicroseconds(100); 
    //Serial.println("laser.....");
    //bleuart.println("beta bulls");  
  }
  delay(1000);
  digitalWrite(dirPin,HIGH);
  for(x = 0; x < SCP_360D; x++) 
  {
    if(digitalRead(sensor_pin))
    {
      digitalWrite(EnMotorsPin,HIGH);
      return_startP = 0;
      break;
    }
    digitalWrite(stepPin,HIGH); 
    delayMicroseconds(100); 
    digitalWrite(stepPin,LOW); 
    delayMicroseconds(100);
  }
  if(return_startP)
  {
    digitalWrite(dirPin,LOW);
    for(x; x > SCP_180D; x--)
    {
      digitalWrite(stepPin,HIGH); 
      delayMicroseconds(100); 
      digitalWrite(stepPin,LOW); 
      delayMicroseconds(100);
    }
  }
  delay(1000);
      digitalWrite(EnMotorsPin,LOW);
}


void Home_pos()
{
  if(digitalRead(sensor_pin))
  {
    digitalWrite(EnMotorsPin,LOW);
    digitalWrite(dirPin,LOW);   //clk wise
    for(int x = 0; x < SCP_10D; x++) 
    {
      digitalWrite(stepPin,HIGH); 
      delayMicroseconds(100); 
      digitalWrite(stepPin,LOW); 
      delayMicroseconds(100); 
    }
    delay(1000);
    digitalWrite(dirPin,HIGH);   //anti_clk wise
    for(int x = 0; x < SCP_20D; x++) 
    {
      if(digitalRead(sensor_pin))
      {
        digitalWrite(EnMotorsPin,HIGH);
      }
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

#endif //SCMS_CONER_BEACON_NRF52840_H
