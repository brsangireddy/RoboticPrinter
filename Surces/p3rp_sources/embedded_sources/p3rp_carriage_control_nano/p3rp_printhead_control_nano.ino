/*********************************************************************
 This is a software for Laser Becaon Module
 Written by Dr Azam, Date : 18th july 2019
 Laser Beacon on Nano Controller
 Serial port to give commands
 Sfot Serial Port connect to laser
*********************************************************************/

#include "p3rp_carriage_control.h"

// ************* Setup functions *************************
void setup(void)   
{
  SetupVariables();
  SetupSerial();  //Setup PC Serial Port for Testing
  SetupPins();
  SetupMotors(); //Setup mecanum 
  if(SelfTest() == P3RP_SUCCESS)     //call self test routine
  {
    Serial.println("Carriage Controller Ready!!!");
  }
}

// ************ Loop Function ***************************

void loop(void)
{
  uint8_t read_indx = 0;

  memset(cmd_buf,0,CMD_BUF_SZ+1);
  if(CmdSerial.available())
  {
    read_indx = ReadCommand(TIMEOUT_READ_CMDPKT);
  }
  if(read_indx != 0) //read some data from softserial port
  {
    ProcessCommand();
  }
  RunMotors();
}
// ******************** Sub routunes start here **********************
void SetupVariables()
{
  memset(cmd_buf,0,CMD_BUF_SZ+1);
  pos_x_max = 91440; //1ft=304.8mm, 300ft=91440mm
  pos_y_max = 91440; //1ft=304.8mm, 300ft=91440mm
  cur_pos_x = 0; //current x position of the carriage in the layout
  cur_pos_y = 0; //current y position of the carriage in the layout
  req_pos_x = 0; //requested new position x of the carriage in the layout
  req_pos_y = 0; //requested new position y of the carriage in the layout
  rot_degs  = 0;
  rot_dir   = MODE_ROT_DIR_CW; //Clockwise (default)
  ack_nack_sent  = false;
  action_pending = false;
  wheel_steps    = 0;
  pending_action_wheel_steps = 0;
}
void SetupSerial()
{ 
  Serial.begin(DBG_SER_BAUD);    //For sending console messages to Serial Termial on PC
  CmdSerial.begin(CMD_SER_BAUD); //For receiving control commands & sending command replies
}

void SetupPins()
{
  digitalWrite(ENDIS_MOTORS_PIN, HIGH);     // Enable = LOW
  pinMode(ENDIS_MOTORS_PIN, OUTPUT);
}
void ProcessCommand(void)
{
  Serial.print("Command Received: ");Serial.println(cmd_buf);
  switch(cmd_buf[INDX_CMD])
  {
    case CMD_ENDIS_MOTS:
      EnableDisableMotors();
    case CMD_GOTO: //'G'
      MoveCarriage();
      break;
    case CMD_ROTATE: //'R'
      RotateCarriage();
      break;
    case CMD_SETDIM: //'D'
      SetLayoutDimentions();
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
    default:
      //do nothing
      break;
  }
}

void SetLayoutDimentions()
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
void MoveCarriage()
{ 
  uint32_t dx,dy;
  char val_array[VAL_STR_SZ+1]={0,};//+1 is for null char
  //SetupMotors();
  memcpy(val_array,&cmd_buf[INDX_VAL1],VAL_STR_SZ);
  uData.i = strtoul(val_array, NULL, CMD_VALS_FORMAT);
  req_pos_x = (uint32_t)uData.f;
  if(req_pos_x > pos_x_max)
  {
    req_pos_x = pos_x_max; //Restrict it to max value
  }
  memcpy(val_array,&cmd_buf[INDX_VAL2],VAL_STR_SZ);//read val string characters to val_array
  uData.i = strtoul(val_array, NULL, CMD_VALS_FORMAT);//convert to integer
  req_pos_y = (uint32_t)uData.f;
  if(req_pos_y > pos_y_max)
  {
    req_pos_y = pos_y_max;//Restrict it to max value  
  }
  Serial.print("Current pos x,y (mm): (");Serial.print(cur_pos_x);Serial.print(",");Serial.print(cur_pos_y);Serial.println(")");  
  Serial.print("Request pos x,y (mm): (");Serial.print(req_pos_x);Serial.print(",");Serial.print(req_pos_y);Serial.println(")");  

  //Move carriage Right/Left Side (X-axis direction) based on dx '+'ve/'-'ve by wheel_steps
  //if(dx > 0.0)
  if(req_pos_x > cur_pos_x)
  {
    dx = req_pos_x - cur_pos_x;
    wheel_steps = (uint32_t)((float)dx*SPMM);
    Serial.print("dx:");Serial.print(dx);Serial.print("mm");Serial.print(" X-dir wheel_steps:");Serial.println(wheel_steps);
    ConfigMotorsToMoveSidewaysRight();
  }
  //else if(dx < 0.0)
  else if(req_pos_x < cur_pos_x)
  {
    dx = cur_pos_x - req_pos_x;
    wheel_steps = (uint32_t)((float)dx*SPMM);
    Serial.print("dx:");Serial.print(dx);Serial.print("mm");Serial.print(" X-dir wheel_steps:");Serial.println(wheel_steps);
    ConfigMotorsToMoveSidewaysLeft();
  }
  else
  {
    Serial.println("No movement required");
  }
  //Move carriage in Forward/Backward direction based on dy '+'ve/'-'ve by wheel_steps
  action_pending = true;//There is a followup action for the carriage to move in y-direction after moving in x-direction.
  //if(dy > 0.0)
  if(req_pos_y > cur_pos_y)
  {
    dy = req_pos_y - cur_pos_y;
    pending_action_wheel_steps = (uint32_t)((float)dy*SPMM);
    Serial.print("dy:");Serial.print(dy);Serial.print("mm");Serial.print(" Y-dir wheel_steps:");Serial.println(pending_action_wheel_steps);
    PendingActionConfigMotors = ConfigMotorsToMoveForward;
  }
  //else if(dy < 0.0)
  else if(req_pos_x < cur_pos_x)
  {
    dy = cur_pos_y - req_pos_y;
    pending_action_wheel_steps = (uint32_t)((float)dy*SPMM);
    Serial.print("dy:");Serial.print(dy);Serial.print("mm");Serial.print(" Y-dir wheel_steps:");Serial.println(pending_action_wheel_steps);
    PendingActionConfigMotors = ConfigMotorsToMoveBackward;
  }
  else
  {
    Serial.println("No pending movement.");
    action_pending = false;
  }
  cur_pos_x = req_pos_x;
  cur_pos_y = req_pos_y;
  //CmdSerial.write(CMD_ACK);//Send acknoledgement for goto command.
}
/***************************************************************************************************************************
 * 
 ***************************************************************************************************************************/
void RotateCarriage()
{
  char val_array[VAL_STR_SZ+1]={0,};//+1 is for null char
  SetupMotors();
  rot_dir = cmd_buf[INDX_MODE];
  memcpy(val_array,&cmd_buf[INDX_VAL1],VAL_STR_SZ);
  uData.i = strtoul(val_array, NULL, CMD_VALS_FORMAT);
  rot_degs = uData.f;
  Serial.print("Req rot deg:");Serial.println(rot_degs);  
  wheel_steps = rot_degs * SPD;
  action_pending = false; //No followup action for rotate command is expected
  Serial.print("Rotate carriage by (deg):");Serial.print(rot_degs);Serial.print("/wheel_steps:");Serial.print(wheel_steps);
  if(rot_dir == MODE_ROT_DIR_CW)
  {
    Serial.println(" clockwise.");  
    ConfigMotorsToRotateRight();      
  }
  else if(rot_dir == MODE_ROT_DIR_ACW)
  {
    Serial.println(" anti-clockwise.");  
    ConfigMotorsToRotateLeft();
  }
  else
  {
    Serial.println("Unknown rotation direction.");
  }
  //CmdSerial.write(CMD_ACK);//Send acknoledgement for rotate command.
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
  //CmdSerial.write(CMD_ACK);
}

void SetMotorsSpeed()
{
  uint32_t s_speed = 0;
  char val_array[VAL_STR_SZ+1]={0,};//+1 is for null char

  memcpy(val_array, &cmd_buf[INDX_VAL1], VAL_STR_SZ);
  s_speed = strtoul(val_array, NULL, CMD_VALS_FORMAT);
  Serial.print("Setting Wheel Speed to: ");Serial.println(s_speed);
  LeftFrontWheel.setSpeed(s_speed);
  LeftBackWheel.setSpeed(s_speed);
  RightFrontWheel.setSpeed(s_speed);
  RightBackWheel.setSpeed(s_speed);
  //CmdSerial.write(CMD_ACK);
}

void SetMotorsAcceleration()
{
  uint32_t s_accel = 0;
  char val_array[VAL_STR_SZ+1]={0,};//+1 is for null char

  memcpy(val_array, &cmd_buf[INDX_VAL1], VAL_STR_SZ);
  s_accel = strtoul(val_array, NULL, CMD_VALS_FORMAT);
  Serial.print("Setting acceleration to: ");Serial.println(s_accel);
  LeftFrontWheel.setAcceleration(s_accel);
  LeftBackWheel.setAcceleration(s_accel);
  RightFrontWheel.setAcceleration(s_accel);
  RightBackWheel.setAcceleration(s_accel);
  //CmdSerial.write(CMD_ACK);
}

void SetupMotors()
{
  Serial.print("Setting Motors Max Speed to: ");Serial.println(CFG_MOTORS_MAXSPEED);
  LeftFrontWheel.setMaxSpeed(CFG_MOTORS_MAXSPEED);
  LeftBackWheel.setMaxSpeed(CFG_MOTORS_MAXSPEED);
  RightFrontWheel.setMaxSpeed(CFG_MOTORS_MAXSPEED);
  RightBackWheel.setMaxSpeed(CFG_MOTORS_MAXSPEED);
  Serial.print("Setting Motors Max Acceleration to: ");Serial.println(CFG_MOTORS_MAXACCEL);
  LeftFrontWheel.setAcceleration(CFG_MOTORS_MAXACCEL);
  LeftBackWheel.setAcceleration(CFG_MOTORS_MAXACCEL);
  RightFrontWheel.setAcceleration(CFG_MOTORS_MAXACCEL);
  RightBackWheel.setAcceleration(CFG_MOTORS_MAXACCEL);
  ResetMotors(); // reset poS1tions to zero  
}

void StopCarriage() // Cmd format:!3X00000000000000000
{
  ResetMotors();
  Serial.println("Emergency Stop!!");
  //CmdSerial.write(CMD_ACK);
}
 

/**************************************************************************************************************
 *Run all the motors by 1 step 
 **************************************************************************************************************/
void RunMotors()
{
  if((LeftFrontWheel.distanceToGo() != 0) || (RightFrontWheel.distanceToGo() != 0) || (LeftBackWheel.distanceToGo() != 0) || (RightBackWheel.distanceToGo() != 0))
  {
//    Serial.print("Distance to go:");
//    Serial.print(LeftFrontWheel.distanceToGo());
//    Serial.print(",");
//    Serial.print(RightFrontWheel.distanceToGo());
//    Serial.print(",");
//    Serial.print(LeftBackWheel.distanceToGo());
//    Serial.print(",");
//    Serial.println(RightBackWheel.distanceToGo());
    LeftFrontWheel.run();
    LeftBackWheel.run();
    RightFrontWheel.run();
    RightBackWheel.run();
    ack_nack_sent = false;
  }
  else if(action_pending == true)
  {
    ResetMotors();
    wheel_steps = pending_action_wheel_steps;
    Serial.print("pending_action_wheel_steps: ");Serial.println(wheel_steps);
    delay(DELAY_BETWEEN_CONSEQUTIVE_MOVEMENTS);
    PendingActionConfigMotors();
    action_pending = false;    
  }
  else
  {//All the actions are completed
    if(ack_nack_sent != true)
    {
      ResetMotors();
      Serial.println("Carriage Movement completed.");
      CmdSerial.write(CMD_ACK); //Send reply to master controller to go ahead with another command.
                                //Master will be waiting untill 'F' is received when GOTO/ROTATE commands are issued to this module
                                //This reply is required because GOTO/ROTATE will take some time to complete.
                                //Other commands will finish immediately. No need for master to wait.
      ack_nack_sent = true;
    }
  }  
}
/****************************************************************************************************************
 * Self test functionality
 ****************************************************************************************************************/
uint16_t SelfTest()
{
  Serial.println(CARCON_VERSION_STR);
  //Other self test features to be implemented
  return(P3RP_SUCCESS);
}

void DisplayFwVersion() // Format is !2V
{  
  CmdSerial.println(CARCON_VERSION_STR); //Send version string to master controller
  Serial.println(CARCON_VERSION_STR);    //For console display
  //CmdSerial.write(CMD_ACK);
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
    while (CmdSerial.available()) 
    {
      char c =  CmdSerial.read();
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
  LeftFrontWheel.setCurrentPosition(0);
  LeftBackWheel.setCurrentPosition(0);
  RightBackWheel.setCurrentPosition(0);
  RightFrontWheel.setCurrentPosition(0);  
}

// *********************** Move Subroutines **************
void ConfigMotorsToRotateRight() 
{
  Serial.println("Configuring for rotate right.");
  LeftFrontWheel.moveTo(wheel_steps);
  LeftBackWheel.moveTo(-wheel_steps);
  RightFrontWheel.moveTo(wheel_steps);
  RightBackWheel.moveTo(-wheel_steps);
}

void ConfigMotorsToRotateLeft() 
{
  Serial.println("Configuring for rotate left.");
  LeftFrontWheel.moveTo(-wheel_steps);
  LeftBackWheel.moveTo(wheel_steps);
  RightFrontWheel.moveTo(-wheel_steps);
  RightBackWheel.moveTo(wheel_steps);
}
void ConfigMotorsToMoveSidewaysRight() 
{
  Serial.println("Configuring for moving right side.");
  LeftFrontWheel.moveTo(wheel_steps);
  LeftBackWheel.moveTo(wheel_steps);
  RightFrontWheel.moveTo(wheel_steps);
  RightBackWheel.moveTo(wheel_steps);
}
void ConfigMotorsToMoveSidewaysLeft() 
{
  Serial.println("Configuring for moving left side.");
  LeftFrontWheel.moveTo(-wheel_steps);
  LeftBackWheel.moveTo(-wheel_steps);
  RightFrontWheel.moveTo(-wheel_steps);
  RightBackWheel.moveTo(-wheel_steps);
}
void ConfigMotorsToMoveForward() 
{
  Serial.println("Configuring for moving forward.");
  LeftFrontWheel.moveTo(-wheel_steps);
  LeftBackWheel.moveTo(wheel_steps);
  RightFrontWheel.moveTo(wheel_steps);
  RightBackWheel.moveTo(-wheel_steps);
}
void ConfigMotorsToMoveBackward() 
{
  Serial.println("Configuring for moving backward.");
  LeftFrontWheel.moveTo(wheel_steps);
  LeftBackWheel.moveTo(-wheel_steps);
  RightFrontWheel.moveTo(-wheel_steps);
  RightBackWheel.moveTo(wheel_steps);
}
void ConfigMotorsToMoveLeftForward() 
{
  Serial.println("Configuring for moving forward left.");
  LeftFrontWheel.moveTo(-wheel_steps);
  RightBackWheel.moveTo(wheel_steps);
}
void ConfigMotorsToMoveLeftBackward() 
{
  Serial.println("Configuring for moving backward left.");
  LeftBackWheel.moveTo(wheel_steps);
  RightFrontWheel.moveTo(-wheel_steps);
}
void ConfigMotorsToMoveRightForward() 
{
  Serial.println("Configuring for moving forward right.");
  LeftBackWheel.moveTo(-wheel_steps);
  RightFrontWheel.moveTo(wheel_steps);
}
void ConfigMotorsToMoveRightBackward() 
{
  Serial.println("Configuring for moving backward right.");
  LeftFrontWheel.moveTo(-wheel_steps);
  RightBackWheel.moveTo(wheel_steps);
}
 
