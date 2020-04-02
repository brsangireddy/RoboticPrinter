/*********************************************************************
 This is a software for Laser Becaon Module
 Written by Dr Azam, Date : 18th july 2019
 Laser Beacon on Nano Controller
 Serial port to give commands
 Sfot Serial Port connect to laser
*********************************************************************/
#include "p3rp_printhead_control.h"

// ************* Setup functions *************************
void setup(void)   
{
  SetupVariables();
  SetupSerial(); //Setup debug/console and command serial ports
  SetupPins();
  SetupMotors(); //Setup stepper motors
  SetupPrintHead();
  SetupServo(); 
  MovePrintheadToHome();//to (x,y)=(0,0)
  if(SelfTest() == P3RP_SUCCESS)     //call self test routine
  {
    Serial.println("Printhead Carriage Controller Ready!!!");
  }
/*
ledcWrite(SERVO_CHANNEL,255);
while(1)
{
  for(int dc=0; dc<=255; dc++)
  {
    ledcWrite(SERVO_CHANNEL,dc);
    delay(15);
  }
  for(int dc=255; dc>=0; dc--)
  {
    ledcWrite(SERVO_CHANNEL,dc);
    delay(15);
  }
}
*/
}

// ************ Loop Function ***************************

void loop(void)
{
  uint8_t read_indx = 0;

  memset(cmd_buf,0,CMD_BUF_SZ+1);
  if(Serial2.available())
  {
    read_indx = ReadCommand(TIMEOUT_READ_CMDPKT);
  }
  if(read_indx != 0) //read some data from softserial port
  {
    ProcessCommand();
  }
  RunMotors();
  //delay(3000);
}
// ******************** Sub routunes start here **********************
void SetupVariables()
{
  memset(cmd_buf,0,CMD_BUF_SZ+1);
  pos_x_max = POS_X_MAX;
  pos_y_max = POS_Y_MAX;
  cur_pos_x = 0; //current x position of the carriage in the layout
  cur_pos_y = 0; //current y position of the carriage in the layout
  req_pos_x = 0; //requested new position x of the carriage in the layout
  req_pos_y = 0; //requested new position y of the carriage in the layout
  ack_nack_sent = false;
  x_motor_steps = 0;
  y_motor_steps = 0;
  x_motor_speed = CFG_XMOTOR_SPEED;
  y_motor_speed = CFG_YMOTOR_SPEED;
}
void SetupSerial()
{ 
  Serial.begin(DBG_SER_BAUD);    //For sending console messages to Serial Termial on PC
  Serial2.begin(CMD_SER_BAUD); //For receiving control commands & sending command replies
}

void SetupPins()
{
  digitalWrite(ENDIS_MOTORS_PIN, LOW); //X,Y motors Enable = LOW (EN is active low)
  
  pinMode(ENDIS_MOTORS_PIN, OUTPUT);
  pinMode(X0_SENSOR_PIN,INPUT);    //printhead home position x switch pin as input to read switch status
  pinMode(Y0_SENSOR_PIN,INPUT);    //printhead home position y switch pin as input to read switch status

}

void SetupPrintHead()
{
//  ledcSetup(SERVO_CHANNEL, SERVO_FREQ_HZ, SERVO_RESOLUTION_BITS);
//  ledcAttachPin(SERVO_PIN, SERVO_CHANNEL);
//  //pen_con_servo.attach(PEN_CON_SERVO_PIN);//attach servo motor control pin
//  TurnOffPrinting();
}

void SetupServo()
{
  MarkerUpDownServo.attach(SERVO_PIN);
  MarkerOff();
}

void MovePrintheadToHome()
{
  XdirMotor.setSpeed(-x_motor_speed);
  YdirMotor.setSpeed(-y_motor_speed);
  Serial.println("Move the printhead carriage to home (X0,Y0) position.");
  while((XdirMotor.currentPosition() <= 0) || (YdirMotor.currentPosition() <= 0))
  {
    if(digitalRead(X0_SENSOR_PIN))
    {
      XdirMotor.runSpeed();
    }
    if(digitalRead(Y0_SENSOR_PIN))
    {
      YdirMotor.runSpeed();
    }
    if((digitalRead(X0_SENSOR_PIN) == LOW) && (digitalRead(Y0_SENSOR_PIN) == LOW))
    {
      XdirMotor.setCurrentPosition(0);
      YdirMotor.setCurrentPosition(0);
      break;
    }
  }
/*
  //move PHC by 10mm in x-direction and 10mm in y-direction
  x_motor_steps = SPMM * 10.0;
  y_motor_steps = SPMM * 10.0;
  XdirMotor.setSpeed(x_motor_speed);
  YdirMotor.setSpeed(y_motor_speed);

  while((XdirMotor.distanceToGo() != -x_motor_steps) || (YdirMotor.distanceToGo() != -y_motor_steps))
  {
    if(XdirMotor.distanceToGo() != -x_motor_steps)
    {
      XdirMotor.runSpeed();
    }
    if(YdirMotor.distanceToGo() != -y_motor_steps)
    {
      YdirMotor.runSpeed();
    }
  }

  cur_pos_x = 10;
  cur_pos_y = 10;
*/  
}

void ProcessCommand(void)
{
  Serial.print("Command Received: ");Serial.println(cmd_buf);
  switch(cmd_buf[INDX_CMD])
  {
    case CMD_ENDIS_MOTS:
      EnableDisableMotors();
    case CMD_GOTO: //'G'
      SetPrintheadCarriagePosition();//to an absolute position from home position
      break;
    case CMD_MOVE: //'M'
      MovePrintheadCarriageByDistance();//Moves phc (print head carriage) by specified distance left/right and up/down
      break;
    case CMD_SETDIM: //'D'
      SetPrintFrameDimentions();
      break;
    case CMD_SETSPEED: //'S'
      SetMotorsSpeed();
      break;
    case CMD_SETACCEL: //'A'
      SetMotorsAcceleration();
      break;
    case CMD_FWVERSION: //'V'
      DisplayFwVersion();
      break;
    case CMD_STOP: //'X'
      StopCarriage(); //Emergency stop
      break;
    case 'I': //'X'
      MovePrintheadToHome(); //INITIAL postion
      break;
    default:
      //do nothing
      break;
  }
}

void SetPrintFrameDimentions()
{
  char val_array[VAL_STR_SZ+1]={0,};//+1 is for null char
  memcpy(val_array,&cmd_buf[INDX_VAL1],VAL_STR_SZ);
  uData.i = strtoul(val_array, NULL, CMD_VALS_FORMAT);
  pos_x_max = uData.f;
  memcpy(val_array,&cmd_buf[INDX_VAL2],VAL_STR_SZ); //read val string characters to val_array
  uData.i = strtoul(val_array, NULL, CMD_VALS_FORMAT);           //convert to integer
  pos_y_max = uData.f;                             //get float value  
  Serial.print("Layout Dim x,y (mm): (");Serial.print(pos_x_max);Serial.print(",");Serial.print(pos_y_max);Serial.println(")");    
}
/***********************************************************************************************************************
 * 
 ***********************************************************************************************************************/
void SetPrintheadCarriagePosition()
{ 
  int32_t dx,dy;
  bool x_movement_required = true;//for sending ACK from this function
  bool y_movement_required = true;//for sending ACK from this function
  
  char val_array[VAL_STR_SZ+1]={0,};//+1 is for accommodating null char

  memcpy(val_array,&cmd_buf[INDX_VAL1],VAL_STR_SZ);
  uData.i = strtoul(val_array, NULL, CMD_VALS_FORMAT);
  req_pos_x = (int32_t)uData.f;
  if(req_pos_x > pos_x_max)
  {
    req_pos_x = pos_x_max; //Restrict it to max value
  }

  memcpy(val_array,&cmd_buf[INDX_VAL2],VAL_STR_SZ);//read val string characters to val_array
  uData.i = strtoul(val_array, NULL, CMD_VALS_FORMAT);//convert to integer
  req_pos_y = (int32_t)uData.f;
  if(req_pos_y > pos_y_max)
  {
    req_pos_y = pos_y_max;//Restrict it to max value  
  }
  Serial.print("Current pos x,y (mm): (");Serial.print(cur_pos_x);Serial.print(",");Serial.print(cur_pos_y);Serial.println(")");  
  Serial.print("Request pos x,y (mm): (");Serial.print(req_pos_x);Serial.print(",");Serial.print(req_pos_y);Serial.println(")");  

  //Move printhead carriage Right/Left Side (X-axis direction) based on dx '+'ve/'-'ve by x_motor_steps
  dx = req_pos_x - cur_pos_x;
  x_motor_steps = (int32_t)((float)dx*SPMM);
  Serial.print("dx:");Serial.print(dx);Serial.print("mm");Serial.print(" x_motor_steps:");Serial.println(x_motor_steps);
  if(dx > 0)
  {
    XdirMotor.setSpeed(x_motor_speed);
  }
  else if(dx < 0)
  {
    XdirMotor.setSpeed(-x_motor_speed);
  }
  else
  {
    Serial.println("No x-movement required");
    x_motor_steps = 0;
    x_movement_required = false;//there is no x-movement
  }
  
  //Move carriage in Forward/Backward direction based on dy '+'ve/'-'ve by y_motor_steps
  dy = req_pos_y - cur_pos_y;
  y_motor_steps = (int32_t)((float)dy*SPMM);
  Serial.print("dy:");Serial.print(dy);Serial.print("mm");Serial.print(" y_motor_steps:");Serial.println(y_motor_steps);
  if(dy > 0)
  {
    YdirMotor.setSpeed(y_motor_speed);
  }
  else if(dy < 0)
  {
    YdirMotor.setSpeed(-y_motor_speed);
  }
  else
  {
    Serial.println("No y-movement required.");
    y_motor_steps = 0;
    y_movement_required = false;//no y-movment
  }
  cur_pos_x = req_pos_x;
  cur_pos_y = req_pos_y;
  MarkerOff();
  if((x_movement_required == false) && (y_movement_required == false))//when there is no x & y movement, send ACK here
  {
    Serial2.write(CMD_RESP_ACK);//Send acknoledgement for goto command.
    phc_movement_required = false;
  }
  else
  {
    phc_movement_required = true;
  }
}
/**********************************************************************************************************
 * 
 **********************************************************************************************************/
void MovePrintheadCarriageByDistance()
{ 
  int32_t dist_x_dir, dist_y_dir, dx,dy;
  bool x_movement_required = true;//for sending ACK from this function
  bool y_movement_required = true;//for sending ACK from this function
  
  char val_array[VAL_STR_SZ+1]={0,};//+1 is for accommodating null char

  memcpy(val_array,&cmd_buf[INDX_VAL1],VAL_STR_SZ);
  uData.i = strtoul(val_array, NULL, CMD_VALS_FORMAT);
  dist_x_dir = (int32_t)uData.f;
  req_pos_x = dist_x_dir;
  //if((cur_pos_x + dist_x_dir) > pos_x_max)
  if(dist_x_dir > pos_x_max)
  {
    req_pos_x = pos_x_max; //Restrict it to max value
  }
  //else if((cur_pos_x+dist_x_dir) < 0)
  else if(dist_x_dir < 0)
  {
    req_pos_x = 0;
  }
  memcpy(val_array,&cmd_buf[INDX_VAL2],VAL_STR_SZ);//read val string characters to val_array
  uData.i = strtoul(val_array, NULL, CMD_VALS_FORMAT);//convert to integer
  dist_y_dir = (int32_t)uData.f;
  req_pos_y = dist_y_dir;
  //if((cur_pos_y + dist_y_dir) > pos_y_max)
  if(dist_y_dir > pos_y_max)
  {
    req_pos_y = pos_y_max;//Restrict it to max value  
  }
  //else if((cur_pos_y + dist_y_dir) < 0)
  else if(dist_y_dir < 0)
  {
    req_pos_y = 0;
  }
  Serial.print("Current pos x,y (mm): (");Serial.print(cur_pos_x);Serial.print(",");Serial.print(cur_pos_y);Serial.println(")");  
  Serial.print("Request pos x,y (mm): (");Serial.print(req_pos_x);Serial.print(",");Serial.print(req_pos_y);Serial.println(")");  

  //Move printhead carriage Right/Left Side (X-axis direction) based on dx '+'ve/'-'ve by x_motor_steps
  dx = req_pos_x - cur_pos_x;
  x_motor_steps = (int32_t)((float)dx*SPMM);
  Serial.print("dx:");Serial.print(dx);Serial.print("mm");Serial.print(" x_motor_steps:");Serial.println(x_motor_steps);
  if(dx > 0)
  {
    XdirMotor.setSpeed(x_motor_speed);
  }
  else if(dx < 0)
  {
    XdirMotor.setSpeed(-x_motor_speed);
  }
  else
  {
    Serial.println("No x-movement required");
    x_motor_steps = 0;
    x_movement_required = false;
  }
  
  //Move carriage in Forward/Backward direction based on dy '+'ve/'-'ve by y_motor_steps
  dy = req_pos_y - cur_pos_y;
  y_motor_steps = (int32_t)((float)dy*SPMM);
  Serial.print("dy:");Serial.print(dy);Serial.print("mm");Serial.print(" y_motor_steps:");Serial.println(y_motor_steps);
  if(dy > 0)
  {
    YdirMotor.setSpeed(y_motor_speed);
  }
  else if(dy < 0)
  {
    YdirMotor.setSpeed(-y_motor_speed);
  }
  else
  {
    Serial.println("No y-movement required.");
    y_motor_steps = 0;
    y_movement_required = false;
  }
  cur_pos_x = req_pos_x;
  cur_pos_y = req_pos_y;

  
  if((x_movement_required == false) && (y_movement_required == false))
  {
    Serial.println("Sending ACK");
    Serial2.write(CMD_RESP_ACK);//Send acknoledgement here as it will not be sent from run motors.
    phc_movement_required = false;
  }
  else//ACK will be sent from RunMotors(), after completing the steps.
  {
    if(cmd_buf[INDX_MODE] & PEN_ON_FLAG_MASK)
    {
      Serial.println("Pen On.");
      //TurnOnPrinting();
      MarkerOn();
    }
    else
    {
      Serial.println("Pen Off.");
      //TurnOffPrinting();
      MarkerOff();
    }
    phc_movement_required = true;
  }
}

/*************************************************************************************************************************
 * Command format: !1Ex0000000000000000, x:E/D, x=E:Enable,x=D:Disable
 *************************************************************************************************************************/
void EnableDisableMotors()
{
  Serial.print("Motors Enable Distable: ");Serial.println(cmd_buf[INDX_MODE]);
  if(cmd_buf[INDX_MODE] == MODE_ENABLE_MOTORS)
  {
    digitalWrite(ENDIS_MOTORS_PIN, LOW);        // Enable = LOW
    Serial.println("Motors Enabled!!");
  }
  else if(cmd_buf[INDX_MODE] == MODE_DISABLE_MOTORS)
  {
    digitalWrite(ENDIS_MOTORS_PIN, HIGH);      // Diasable = High
    Serial.println("Motors Disabled!!");
  }
  else
  {
    Serial.println("Unknown Command!!");
  }
  //Serial2.write(CMD_ACK);
}

void SetMotorsSpeed()
{
  char val_array[VAL_STR_SZ+1]={0,};//+1 is for null char

  memcpy(val_array, &cmd_buf[INDX_VAL1], VAL_STR_SZ);
  uData.i = strtoul(val_array, NULL, CMD_VALS_FORMAT);
  x_motor_speed = uData.f;
  memcpy(val_array, &cmd_buf[INDX_VAL2], VAL_STR_SZ);
  uData.i = strtoul(val_array, NULL, CMD_VALS_FORMAT);
  y_motor_speed = uData.f;
  Serial.print("Setting X-motor speed to: ");Serial.println(x_motor_speed);
  XdirMotor.setSpeed(x_motor_speed);
  Serial.print("Setting Y-motor speed to: ");Serial.println(y_motor_speed);
  YdirMotor.setSpeed(y_motor_speed);
  //Serial2.write(CMD_ACK);
}

void SetMotorsAcceleration()
{
  uint32_t s_accel = 0;
  char val_array[VAL_STR_SZ+1]={0,};//+1 is for null char

  memcpy(val_array, &cmd_buf[INDX_VAL1], VAL_STR_SZ);
  uData.i = strtoul(val_array, NULL, CMD_VALS_FORMAT);
  s_accel = uData.f;
  Serial.print("Setting acceleration to: ");Serial.println(s_accel);
  XdirMotor.setAcceleration(s_accel);
  YdirMotor.setAcceleration(s_accel);
  //Serial2.write(CMD_ACK);
}

void SetupMotors()
{
  //Serial.print("Setting Motors Max Speed to: ");Serial.println(CFG_MOTORS_MAXSPEED);
  XdirMotor.setMaxSpeed(CFG_MOTORS_MAXSPEED);
  YdirMotor.setMaxSpeed(CFG_MOTORS_MAXSPEED);
 // Serial.print("Setting Motors Max Acceleration to: ");Serial.println(CFG_MOTORS_MAXACCEL);
 // XdirMotor.setAcceleration(CFG_MOTORS_MAXACCEL);
  //YdirMotor.setAcceleration(CFG_MOTORS_MAXACCEL);
  ResetMotors(); // reset positions to zero  
}

void StopCarriage() // Cmd format:!3X00000000000000000
{
  ResetMotors();
  Serial.println("Emergency Stop!!");
  //Serial2.write(CMD_ACK);
}
 

/**************************************************************************************************************
 *Run all the motors by 1 step 
 **************************************************************************************************************/
void RunMotors()
{
  //Serial.print("X,Y motors to run steps: ");Serial.print(XdirMotor.distanceToGo());Serial.print(" ");Serial.println(YdirMotor.distanceToGo());
  if(phc_movement_required == true)
  {
    if((XdirMotor.distanceToGo() != -x_motor_steps) || (YdirMotor.distanceToGo() != -y_motor_steps))
    {
      //if(XdirMotor.distanceToGo() != -x_motor_steps)
      if((XdirMotor.distanceToGo() != -x_motor_steps) && (x_motor_steps != 0))// && (digitalRead(X0_SENSOR_PIN) == LOW))//XMAX_SENSOR_PIN check to be added
      {
        XdirMotor.runSpeed();
      }
      //if(YdirMotor.distanceToGo() != -y_motor_steps)
      if((YdirMotor.distanceToGo() != -y_motor_steps) && (y_motor_steps != 0))// && (digitalRead(Y0_SENSOR_PIN) == LOW))//YMAX_SENSOR_PIN check to be added
      {
        YdirMotor.runSpeed();
      }
      ack_nack_sent = false;
    }
    else
    {//All the actions are completed
      if(ack_nack_sent == false)
      {
        //TurnOffPrinting();
        MarkerOff();
        ResetMotors();
        Serial.println("ACK sent from RunMotors().");
        Serial2.write(CMD_RESP_ACK); //Send reply to master controller to go ahead with another command.
                                  //Master will be waiting untill 'F' is received when GOTO/ROTATE commands are issued to this module
                                  //This reply is required because GOTO/ROTATE will take some time to complete.
                                  //Other commands will finish immediately. No need for master to wait.
        ack_nack_sent = true;     //To send ACK only once per move command
        phc_movement_required = false;
        //ResetMotors();
      }
    }
  }
}
/****************************************************************************************************************
 * Self test functionality
 ****************************************************************************************************************/
uint16_t SelfTest()
{
  Serial.println(PHCON_VERSION_STR);
  //Other self test features to be implemented
  return(P3RP_SUCCESS);
}

void DisplayFwVersion() // Format is !2V00000000000000000
{  
  Serial2.println(PHCON_VERSION_STR); //Send version string to master controller
  Serial.println(PHCON_VERSION_STR);    //For console display
  //Serial2.write(CMD_ACK);
}

/**************************************************************************************************************** 
 *Read command packet from master controller
 ****************************************************************************************************************/
uint8_t  ReadCommand(uint16_t timeout) 
{
  uint8_t reply_indx = 0;
  uint16_t origtimeout = timeout;
  
  while(timeout--) 
  {
    if(reply_indx >= CMD_BUF_SZ)
    {
      break;
    }
    while (Serial2.available()) 
    {
      char c =  Serial2.read();
      if(c == '!')
      {
        reply_indx = 0;
      }
      cmd_buf[reply_indx] = c;
      reply_indx++;
      timeout = origtimeout;
    }
    if(timeout == 0)
    {
      break;
    }
    delay(1);
  }
  cmd_buf[reply_indx] = 0; // null term
  if(!reply_indx)          // no data or timeout 
  {
    return 0;
  }
  if(cmd_buf[0] != '!')    // doesn't start with '!' packet beginning
  {
    return 0;              //invalid packet
  }
  return reply_indx;
}

void ResetMotors()
{
  XdirMotor.setCurrentPosition(0);
  YdirMotor.setCurrentPosition(0);
//  Serial.print("Setting X-motor speed to: ");Serial.println(x_motor_speed);
//  XdirMotor.setSpeed(x_motor_speed);
//  Serial.print("Setting Y-motor speed to: ");Serial.println(y_motor_speed);
//  YdirMotor.setSpeed(y_motor_speed);
}

// *********************** Move Subroutines **************
void ConfigXdirMotorToMoveRight() 
{
  Serial.println("Configuring to move x->");
  while(XdirMotor.currentPosition()!= x_motor_steps)
  {
    XdirMotor.setSpeed(x_motor_speed);//1000);
    XdirMotor.runSpeed();
  }
  //XdirMotor.moveTo(x_motor_steps);
}
void ConfigXdirMotorToMoveLeft() 
{
  Serial.println("Configuring to move <-x");
  while(XdirMotor.currentPosition()!= -x_motor_steps)
  {
    XdirMotor.setSpeed(x_motor_speed);//-1000);
    XdirMotor.runSpeed();
  }
  //XdirMotor.moveTo(-x_motor_steps);
}
void ConfigYdirMotorToMoveForward() 
{
  Serial.println("Configuring to move y↑.");
  while(YdirMotor.currentPosition()!= y_motor_steps)
  {
    YdirMotor.setSpeed(y_motor_speed);//1000);
    YdirMotor.runSpeed();
  }
  //YdirMotor.moveTo(-y_motor_steps);
}
void ConfigYdirMotorToMoveBackward() 
{
  Serial.println("Configuring to move y↓");
  while(YdirMotor.currentPosition()!= -y_motor_steps)
  {
    YdirMotor.setSpeed(-y_motor_speed);//-1000);
    YdirMotor.runSpeed();
  }
  //YdirMotor.moveTo(-y_motor_steps);
}

void MarkerOn()
{
  //ledcWrite(SERVO_CHANNEL, 0);
  MarkerUpDownServo.write(MARKER_ON_ANG);
  delay(100);
}

void MarkerOff()
{
  //ledcWrite(SERVO_CHANNEL, 128);
  MarkerUpDownServo.write(MARKER_OFF_ANG);
  delay(100); 
}
