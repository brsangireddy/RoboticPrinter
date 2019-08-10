#ifndef SCMS_CORNER_BEACON_NRF52840_H
#define SCMS_CORNER_BEACON_NRF52840_H

#define   led1        7
#define   dirPin      29
#define   stepPin     28
#define   sensor_pin  5


#define EnMotorsPin 30
#define CW          1
#define ACW         0
#define SPD         222.222     //steps per one degree rotation of stepper motor
//#define READ_BUFSIZE                    (20)


char m;
uint32_t wheelSteps=0;

long temp1=0;
long steps;
long temp2=0 ;


bool dir;
int32_t new_pos=0;

long current_pos=0;
//long new_pos;

void Home_pos();
void rotate_sm(bool dir, int32_t new_pos);
uint8_t set_angle();

void rotate_sm(bool dir, int32_t new_pos)  //new_pos in degree
{
  int32_t net_angle=0;
  if(new_pos > 90)
  {
    new_pos = 90;
  }
  Serial.print("cut_off_pos: ");
  Serial.println(new_pos,DEC);
  
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
    //Serial.print("dir pin :LOW");
  }
  else
  {
    digitalWrite(dirPin,HIGH);
    //Serial.print("dir pin :HIGH");

  }
  if(net_angle > 90)
  {
    net_angle=180;
  }
  wheelSteps=net_angle*SPD;  
  delay(500);
  for(long steps = 0; steps < wheelSteps; steps++)
  {
    digitalWrite(stepPin,HIGH); 
    delayMicroseconds(100); 
    digitalWrite(stepPin,LOW); 
    delayMicroseconds(100);
   }
}


uint8_t set_angle()
{
  digitalWrite(EnMotorsPin,LOW);
  digitalWrite(dirPin,LOW);
  for(int x = 0; x < 40000; x++)
  {
    if(digitalRead(sensor_pin))
    {
      digitalWrite(EnMotorsPin,HIGH);   
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
  for(int x = 0; x < 80000; x++)
  {
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
    for(int x = 0; x < 6000; x++)
    {
      digitalWrite(stepPin,HIGH); 
      delayMicroseconds(100); 
      digitalWrite(stepPin,LOW); 
      delayMicroseconds(100); 
    }
    delay(1000);
    digitalWrite(dirPin,HIGH);   //anti_clk wise
    for(int x = 0; x < 12000; x++)
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

#endif //SCMS_CORNER_BEACON_NRF52840_H
