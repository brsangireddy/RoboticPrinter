/***************************************************************************************************
 This is software for Mecanum wheel carriage control Module
 Written by Bhaskar Date : 26th March, 2020
 Mecanum wheel driver controlled by 30-pin ESP32 DOIT dev kit ver 1.0
 Serial port (UART0) to print debug messages
 Serial2 port (UART2) connect to master controller and receive commands and send responses
*****************************************************************************************************/
#include "p3rp_carriage_control.h"

// ************* Setup functions *************************
void setup(void)   
{
  SetupVariables();
  SetupSerial();  //Setup PC Serial Port for Testing
  SetupPins();
  SetupMotors(); //Setup mecanum 
  if(SelfTest() == RESULT_SUCCESS)     //call self test routine
  {
    Serial.println("Carriage Controller Ready!!!");
  }
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
}

/*******************************************************************************************************************
 * 
 *******************************************************************************************************************/
void SetupVariables()
{
  memset(cmd_buf,0,CMD_BUF_SZ+1);
  pos_x_max = 91440; //1ft=304.8mm, 300ft=91440mm
  pos_y_max = 91440; //1ft=304.8mm, 300ft=91440mm
  cur_pos_x = 0; //current x position of the carriage in the layout
  cur_pos_y = 0; //current y position of the carriage in the layout
  car_cur_ang = 0.0;
  car_cur_boundary = 0; 
  this_layout_xmax = 0;
  this_layout_ymax = 0;
  req_pos_x = 0; //requested new position x of the carriage in the layout
  req_pos_y = 0; //requested new position y of the carriage in the layout
  rot_dir   = MODE_ROTATE_CLOCKWISE; //Clockwise (default)
  ack_nack_sent  = false;
  action_pending = false;
  wheel_steps    = 0;
  pending_action_wheel_steps = 0;
  carriage_movement_required = false;
}

/*******************************************************************************************************************
 * 
 *******************************************************************************************************************/
void SetupSerial()
{ 
  Serial.begin(DBG_SER_BAUD);    //For sending console messages to Serial Termial on PC
  Serial2.begin(CMD_SER_BAUD); //For receiving control commands & sending command replies
}

/*******************************************************************************************************************
 * 
 *******************************************************************************************************************/
void SetupPins()
{
  digitalWrite(ENDIS_MOTORS_PIN, HIGH);     // Enable = LOW
  pinMode(ENDIS_MOTORS_PIN, OUTPUT);
}
/*******************************************************************************************************************
 * 
 *******************************************************************************************************************/
void ProcessCommand(void)
{
  Serial.print("Command Received: ");Serial.println(cmd_buf);
  switch(cmd_buf[INDX_CMD])
  {
    case CMD_ENDIS_MOTS:
      EnableDisableMotors();
    case CMD_GOTO: //'G'
      MoveCarriageToReqPosition();
      break;
    case CMD_MOVE: //'M'
      MoveCarriageByReqDistance();
      break;
    case CMD_ROTATE: //'R'
      RotateCarriage();
      break;
    case CMD_SETDIM: //'D'
      SetDimensions();
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

/*******************************************************************************************************************
 * 
 *******************************************************************************************************************/
void SetDimensions()
{
  char val_array[VAL_STR_SZ+1]={0,};//+1 is for null char
  memcpy(val_array,&cmd_buf[INDX_VAL1],VAL_STR_SZ);
  uData.i = strtoul(val_array, NULL, CMD_VALS_FORMAT);
  if(cmd_buf[INDX_MODE] == MODE_SETDIM_LAYOUT)
  {
    pos_x_max = (uint32_t)uData.f;
  }
  if(cmd_buf[INDX_MODE] == MODE_SETDIM_SEGMENT)
  {
    seg_size_x = (uint32_t)uData.f;
  }
  memcpy(val_array,&cmd_buf[INDX_VAL2],VAL_STR_SZ); //read val string characters to val_array
  uData.i = strtoul(val_array, NULL, CMD_VALS_FORMAT);           //convert to integer
  if(cmd_buf[INDX_MODE] == MODE_SETDIM_LAYOUT)
  {
    pos_y_max = (uint32_t)uData.f;
  }
  if(cmd_buf[INDX_MODE] == MODE_SETDIM_SEGMENT)
  {
    seg_size_y = (uint32_t)uData.f;
  }
  Serial.print("Layout Dim x,y (mm): (");Serial.print(pos_x_max);Serial.print(",");Serial.print(pos_y_max);Serial.println(")");    
  Serial2.write(CMD_ACK);//Send acknoledgement to master controller.
}
/*******************************************************************************************************************
 * 
 *******************************************************************************************************************/
void MoveCarriageByReqDistance()
{ 
  uint32_t dx,dy;
  uint32_t dist_to_move = 0;
  carriage_movement_required = true;
  char val_array[VAL_STR_SZ+1]={0,};//+1 is for null char
  
  memcpy(val_array,&cmd_buf[INDX_VAL1],VAL_STR_SZ);
  uData.i = strtoul(val_array, NULL, CMD_VALS_FORMAT);
  dist_to_move = (uint32_t)uData.f;
//  if(dist_to_move > pos_x_max)
//  {
//    dist_to_move = pos_x_max; //Restrict it to max value
//  }
//  else if(dist_to_move < 0)
//  {
//    dist_to_move = 0; //Restrict it to max value
//  }
  
  Serial.print("Current pos x,y (mm): (");Serial.print(cur_pos_x);Serial.print(",");Serial.print(cur_pos_y);Serial.println(")");  
  Serial.print("Request pos x,y (mm): (");Serial.print(req_pos_x);Serial.print(",");Serial.print(req_pos_y);Serial.println(")");  

  wheel_steps = (uint32_t)((float)dist_to_move*SPMM);

  //Move carriage Right/Left Side (X-axis direction) based on dx '+'ve/'-'ve by wheel_steps
  switch(cmd_buf[INDX_MODE])
  {
    case MODE_MOVE_FORWARD:
      ConfigMotorsToMoveForward();
      break;
    case MODE_MOVE_BACKWARD:
      ConfigMotorsToMoveBackward();
      break;
    case MODE_MOVE_SIDEWAYSRGIHT:
      ConfigMotorsToMoveSidewaysRight();
      break;
    case MODE_MOVE_SIDEWAYSLEFT:
      ConfigMotorsToMoveSidewaysLeft();
      break;
    case MODE_MOVE_RIGHTFORWARD:
      ConfigMotorsToMoveRightForward();
      break;
    case MODE_MOVE_LEFTFORWARD:
      ConfigMotorsToMoveLeftForward();
      break;
    case MODE_MOVE_RIGHTBACKWARD:
      ConfigMotorsToMoveRightBackward();
      break;
    case MODE_MOVE_LEFTBACKWARD:
      ConfigMotorsToMoveLeftBackward();
      break;
  }
//  cur_pos_x = req_pos_x;
//  cur_pos_y = req_pos_y;
  action_pending = false;
  
  if(dist_to_move == 0)
  {
    carriage_movement_required = false;
    Serial2.write(CMD_ACK);//Send acknoledgement here when there is no movement required.
  }
}

/*******************************************************************************************************************
 * 
 *******************************************************************************************************************/
void MoveCarriageToReqPosition()
{ 
  uint32_t dx,dy,swap_temp;
  bool x_movement_required = true;
  bool y_movement_required = true;
  carriage_movement_required = true;
  
  char val_array[VAL_STR_SZ+1]={0,};//+1 is for null char
  
  //SetupMotors();
  memcpy(val_array,&cmd_buf[INDX_VAL1],VAL_STR_SZ);
  uData.i = strtoul(val_array, NULL, CMD_VALS_FORMAT);
  req_pos_x = (uint32_t)uData.f;
  if(req_pos_x > pos_x_max)
  {
    req_pos_x = pos_x_max; //Restrict it to max value
  }
  else if(req_pos_x < 0)
  {
    req_pos_x = 0; //Restrict it to max value
  }
  memcpy(val_array,&cmd_buf[INDX_VAL2],VAL_STR_SZ);//read val string characters to val_array
  uData.i = strtoul(val_array, NULL, CMD_VALS_FORMAT);//convert to integer
  req_pos_y = (uint32_t)uData.f;
  if(req_pos_y > pos_y_max)
  {
    req_pos_y = pos_y_max;//Restrict it to max value  
  }
  else if(req_pos_y < 0)
  {
    req_pos_y = 0;//Restrict it to min/0 value  
  }

  switch(car_cur_boundary)
  {
    case CARLOC_LEFT_SEGS:               
    case CARLOC_INTERNAL_SEGS:
      break;      
    case CARLOC_TOP_SEGS:
      swap_temp = req_pos_x;
      req_pos_x = req_pos_y;
      req_pos_y = swap_temp;
      break;
    case CARLOC_RIGHT_SEGS:
      req_pos_y = this_layout_ymax - req_pos_y;
      break;
    case CARLOC_BOTTOM_SEGS:
      req_pos_y = this_layout_xmax - req_pos_x;
      req_pos_x = this_layout_xmax;
      break;
  }
  
  Serial.print("Move (");Serial.print(cur_pos_x);Serial.print(",");Serial.print(cur_pos_y);Serial.print(")");  
  Serial.print("-->(");Serial.print(req_pos_x);Serial.print(",");Serial.print(req_pos_y);Serial.println(")");  

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
    Serial.println("No movement required in x-direction.");
    x_movement_required = false;
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
  else if(req_pos_y < cur_pos_y)
  {
    dy = cur_pos_y - req_pos_y;
    pending_action_wheel_steps = (uint32_t)((float)dy*SPMM);
    Serial.print("dy:");Serial.print(dy);Serial.print("mm");Serial.print(" Y-dir wheel_steps:");Serial.println(pending_action_wheel_steps);
    PendingActionConfigMotors = ConfigMotorsToMoveBackward;
  }
  else
  {
    Serial.println("No pending/y-movement.");
    action_pending = false;
    y_movement_required = false;
  }
  cur_pos_x = req_pos_x;
  cur_pos_y = req_pos_y;

  if((x_movement_required == false) && (y_movement_required == false))
  {
    carriage_movement_required = false;
    Serial2.write(CMD_ACK);//Send acknoledgement here when there is no movement required.
  }
}

/***************************************************************************************************************************
 * 
 ***************************************************************************************************************************/
void RotateCarriage()
{
  uint32_t swap_temp;
  float rot_deg = 0.0, rot_min = 0.0;
  char val_array[VAL_STR_SZ+1]={0,};//+1 is for null char
  carriage_movement_required = true;
  SetupMotors();
  rot_dir = cmd_buf[INDX_MODE];
  memcpy(val_array,&cmd_buf[INDX_VAL1],VAL_STR_SZ);
  uData.i = strtoul(val_array, NULL, CMD_VALS_FORMAT);
  rot_deg = uData.f;
  memcpy(val_array,&cmd_buf[INDX_VAL2],VAL_STR_SZ);
  uData.i = strtoul(val_array, NULL, CMD_VALS_FORMAT);
  rot_min = uData.f;
  rot_deg = rot_deg + rot_min/60.0;
  Serial.print("Req rot deg:");Serial.println(rot_deg);    
  wheel_steps = rot_deg * SPD;
  action_pending = false; //No followup action for rotate command is expected
  Serial.print("Rotate carriage by (deg):");Serial.print(rot_deg);Serial.print("/wheel_steps:");Serial.print(wheel_steps);
  if(rot_deg > 0.0)
  {
    if(rot_dir == MODE_ROTATE_CLOCKWISE)
    {
      Serial.println(" clockwise.");  
      ConfigMotorsToRotateRight();
      car_cur_ang = car_cur_ang + rot_deg;
      car_cur_boundary = car_cur_ang/90; 

      //current position values transforms everytime the carriage takes ( 90°?) turn at the corner while printing boundary segments
      //Requested segment positions also transforms, which is taken care in GOTO command handler.
      switch(car_cur_boundary)
      {
        case CARLOC_LEFT_SEGS:
          cur_pos_x = 0;          
          cur_pos_y = 0;
          Serial.println("Currently printing left boundary/internal segments.");
          break;      
        case CARLOC_TOP_SEGS:
          this_layout_ymax = cur_pos_y;//Save ymax for this layout for future calculations
          swap_temp = cur_pos_x;
          cur_pos_x = cur_pos_y;
          cur_pos_y = swap_temp;
          Serial.println("Currently printing top boundary segments.");
          break;
        case CARLOC_RIGHT_SEGS:
          this_layout_xmax = cur_pos_y;//Save xmax for this layout for future calculations
          cur_pos_x = cur_pos_y;
          cur_pos_y = 0;
          Serial.println("Currently printing right boundary segments.");
          break;
        case CARLOC_BOTTOM_SEGS:
          req_pos_x = this_layout_xmax;
          cur_pos_y = 0;
          Serial.println("Currently printing bottom boundary segments.");          
          break;
        case CARLOC_INTERNAL_SEGS:
          cur_pos_x = seg_size_x;          
          cur_pos_y = 0;
          Serial.println("Currently printing internal segments.");          
          break;          
      }
    }
    else if(rot_dir == MODE_ROTATE_ANTICLOCKWISE)
    {
      Serial.println(" anti-clockwise.");  
      ConfigMotorsToRotateLeft();
      car_cur_ang = car_cur_ang - rot_deg;
    }
    else
    {
      carriage_movement_required = false;
      Serial.println("Unknown rotation direction.");
      Serial2.write(CMD_ACK);//Send acknoledgement for rotate command.
    }
  }
  else
  {
    carriage_movement_required = false;
    Serial.println("Resquested 0° rotation.");
    Serial2.write(CMD_ACK);//Send acknoledgement for rotate command.
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

/*******************************************************************************************************************
 * 
 *******************************************************************************************************************/
void SetMotorsSpeed()
{
  uint32_t s_speed = 0;
  char val_array[VAL_STR_SZ+1]={0,};//+1 is for null char

  memcpy(val_array, &cmd_buf[INDX_VAL1], VAL_STR_SZ);
  s_speed = strtoul(val_array, NULL, CMD_VALS_FORMAT);
  //Serial.print("Setting Wheel Speed to: ");Serial.println(s_speed);
  LeftFrontWheel.setSpeed(s_speed);
  LeftBackWheel.setSpeed(s_speed);
  RightFrontWheel.setSpeed(s_speed);
  RightBackWheel.setSpeed(s_speed);
  //Serial2.write(CMD_ACK);
}

/*******************************************************************************************************************
 * 
 *******************************************************************************************************************/
void SetMotorsAcceleration()
{
  uint32_t s_accel = 0;
  char val_array[VAL_STR_SZ+1]={0,};//+1 is for null char

  memcpy(val_array, &cmd_buf[INDX_VAL1], VAL_STR_SZ);
  s_accel = strtoul(val_array, NULL, CMD_VALS_FORMAT);
  //Serial.print("Setting acceleration to: ");Serial.println(s_accel);
  LeftFrontWheel.setAcceleration(s_accel);
  LeftBackWheel.setAcceleration(s_accel);
  RightFrontWheel.setAcceleration(s_accel);
  RightBackWheel.setAcceleration(s_accel);
  //Serial2.write(CMD_ACK);
}

/*******************************************************************************************************************
 * 
 *******************************************************************************************************************/
void SetupMotors()
{
  //Serial.print("Setting Motors Max Speed to: ");Serial.println(CFG_MOTORS_MAXSPEED);
  LeftFrontWheel.setMaxSpeed(CFG_MOTORS_MAXSPEED);
  LeftBackWheel.setMaxSpeed(CFG_MOTORS_MAXSPEED);
  RightFrontWheel.setMaxSpeed(CFG_MOTORS_MAXSPEED);
  RightBackWheel.setMaxSpeed(CFG_MOTORS_MAXSPEED);
  //Serial.print("Setting Motors Max Acceleration to: ");Serial.println(CFG_MOTORS_MAXACCEL);
  LeftFrontWheel.setAcceleration(CFG_MOTORS_MAXACCEL);
  LeftBackWheel.setAcceleration(CFG_MOTORS_MAXACCEL);
  RightFrontWheel.setAcceleration(CFG_MOTORS_MAXACCEL);
  RightBackWheel.setAcceleration(CFG_MOTORS_MAXACCEL);
  ResetMotors(); // reset poS1tions to zero  
}

/*******************************************************************************************************************
 * 
 *******************************************************************************************************************/
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
  if(carriage_movement_required == true)
  {
    if((LeftFrontWheel.distanceToGo() != 0) || (RightFrontWheel.distanceToGo() != 0) || (LeftBackWheel.distanceToGo() != 0) || (RightBackWheel.distanceToGo() != 0))
    {
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
        Serial2.write(CMD_ACK); //Send reply to master controller to go ahead with another command.
                                  //Master will be waiting untill 'F' is received when GOTO/ROTATE commands are issued to this module
                                  //This reply is required because GOTO/ROTATE will take some time to complete.
                                  //Other commands will finish immediately. No need for master to wait.
        Serial.println("ACK sent from RunMotors()");
        ack_nack_sent = true;
        carriage_movement_required = false;
      }
    }
  }  
}
/****************************************************************************************************************
 * Self test functionality
 ****************************************************************************************************************/
uint16_t SelfTest()
{
  Serial.println();
  Serial.println(CARCON_VERSION_STR);
  Serial.print("Build Date & Time: ");Serial.print(__DATE__);Serial.print(",");Serial.println(__TIME__);
  //Other self test features to be implemented
  return(RESULT_SUCCESS);
}

/*******************************************************************************************************************
 * 
 *******************************************************************************************************************/
void DisplayFwVersion() // Format is !2V
{  
  //Serial2.println(CARCON_VERSION_STR); //Send version string to master controller
  Serial.print(CARCON_VERSION_STR);Serial.print(" ");Serial.print("Date & Time: ");Serial.print(__DATE__);Serial.print(",");Serial.println(__TIME__);
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
  LeftFrontWheel.setCurrentPosition(0);
  LeftBackWheel.setCurrentPosition(0);
  RightBackWheel.setCurrentPosition(0);
  RightFrontWheel.setCurrentPosition(0);  
}

// *********************** Move functions **************
/*******************************************************************************************************************
 * 
 *******************************************************************************************************************/
void ConfigMotorsToRotateRight() 
{
  Serial.println("Configuring for rotate right.");
  LeftFrontWheel.moveTo(wheel_steps);
  LeftBackWheel.moveTo(wheel_steps);
  RightFrontWheel.moveTo(-wheel_steps);
  RightBackWheel.moveTo(-wheel_steps);
}

/*******************************************************************************************************************
 * 
 *******************************************************************************************************************/
void ConfigMotorsToRotateLeft() 
{
  Serial.println("Configuring for rotate left.");
  LeftFrontWheel.moveTo(-wheel_steps);
  LeftBackWheel.moveTo(-wheel_steps);
  RightFrontWheel.moveTo(wheel_steps);
  RightBackWheel.moveTo(wheel_steps);
}

/*******************************************************************************************************************
 * 
 *******************************************************************************************************************/
void ConfigMotorsToMoveSidewaysLeft() 
{
  Serial.println("Configuring for moving right side.");
  LeftFrontWheel.moveTo(-wheel_steps);
  LeftBackWheel.moveTo(wheel_steps);
  RightFrontWheel.moveTo(wheel_steps);
  RightBackWheel.moveTo(-wheel_steps);
}

/*******************************************************************************************************************
 * 
 *******************************************************************************************************************/
void ConfigMotorsToMoveSidewaysRight() 
{
  Serial.println("Configuring for moving left side.");
  LeftFrontWheel.moveTo(wheel_steps);
  LeftBackWheel.moveTo(-wheel_steps);
  RightFrontWheel.moveTo(-wheel_steps);
  RightBackWheel.moveTo(wheel_steps);
}

/*******************************************************************************************************************
 * 
 *******************************************************************************************************************/
void ConfigMotorsToMoveForward() 
{
  Serial.println("Configuring for moving forward.");
  LeftFrontWheel.moveTo(-wheel_steps);
  LeftBackWheel.moveTo(-wheel_steps);
  RightFrontWheel.moveTo(-wheel_steps);
  RightBackWheel.moveTo(-wheel_steps);
}

/*******************************************************************************************************************
 * 
 *******************************************************************************************************************/
void ConfigMotorsToMoveBackward() 
{
  Serial.println("Configuring for moving backward.");
  LeftFrontWheel.moveTo(wheel_steps);
  LeftBackWheel.moveTo(wheel_steps);
  RightFrontWheel.moveTo(wheel_steps);
  RightBackWheel.moveTo(wheel_steps);
}

/*******************************************************************************************************************
 * 
 *******************************************************************************************************************/
void ConfigMotorsToMoveLeftForward() 
{
  Serial.println("Configuring for moving forward left.");
  LeftFrontWheel.moveTo(-wheel_steps);
  RightBackWheel.moveTo(wheel_steps);
}

/*******************************************************************************************************************
 * 
 *******************************************************************************************************************/
void ConfigMotorsToMoveLeftBackward() 
{
  Serial.println("Configuring for moving backward left.");
  LeftBackWheel.moveTo(wheel_steps);
  RightFrontWheel.moveTo(-wheel_steps);
}

/*******************************************************************************************************************
 * 
 *******************************************************************************************************************/
void ConfigMotorsToMoveRightForward() 
{
  Serial.println("Configuring for moving forward right.");
  LeftBackWheel.moveTo(-wheel_steps);
  RightFrontWheel.moveTo(wheel_steps);
}

/*******************************************************************************************************************
 * 
 *******************************************************************************************************************/
void ConfigMotorsToMoveRightBackward()
{
  Serial.println("Configuring for moving backward right.");
  LeftFrontWheel.moveTo(wheel_steps);
  RightBackWheel.moveTo(wheel_steps);
}
