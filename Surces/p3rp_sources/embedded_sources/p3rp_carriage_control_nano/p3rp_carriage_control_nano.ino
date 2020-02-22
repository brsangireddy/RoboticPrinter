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
}
// ******************** Sub routunes start heare **********************
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
      //SetMaxSpeed();
      break;
    case CMD_SETACCEL: //'A'
      //SetMaxAccel();
      break;
    case CMD_FWVERSION: //'V'
      DisplayFwVersion();
      break;
    case CMD_STOP: //'X'
      StopMecanum(); //Emergency stop
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
  Serial.print("Req pos x,y (mm): (");Serial.print(req_pos_x);Serial.print(",");Serial.print(req_pos_y);Serial.println(")");    
}
void MoveCarriage()
{ 
  float dx,dy;
  char val_array[VAL_STR_SZ+1]={0,};//+1 is for null char
  SetupMotors();
  memcpy(val_array,&cmd_buf[INDX_VAL1],VAL_STR_SZ);
  uData.i = strtoul(val_array, NULL, CMD_VALS_FORMAT);
  req_pos_x = uData.f;
  if(req_pos_x > pos_x_max)
  {
    req_pos_x = pos_x_max; //Restrict it to max value
  }
  memcpy(val_array,&cmd_buf[INDX_VAL2],VAL_STR_SZ);//read val string characters to val_array
  uData.i = strtoul(val_array, NULL, CMD_VALS_FORMAT);//convert to integer
  req_pos_y = uData.f;//get float value
  if(req_pos_y > pos_y_max)
  {
    req_pos_y = pos_y_max;//Restrict it to max value  
  }
  Serial.print("Req pos x,y (mm): (");Serial.print(req_pos_x);Serial.print(",");Serial.print(req_pos_y);Serial.println(")");  

  dx = req_pos_x - cur_pos_x;
  dy = req_pos_y - cur_pos_y;
  cur_pos_x = req_pos_x;
  cur_pos_y = req_pos_y;
  //Move carriage Right side/left side based on dx '+'ve/'-'ve by wheel_steps
  wheel_steps = abs(dx)*SPMM;
  steps_togo_remaining = true; //to continue untill the carriage moves wheel_steps (RunMotors called untill stepsToGo becomes zero)
  Serial.print("dx: ");Serial.print(dx);
  if(dx > 0.0)
  {
    Serial.print(" wheel_steps: ");Serial.print(wheel_steps);Serial.println(" Move right");
    ConfigMotorsToMoveSidewaysRight();
  }
  else if(dx < 0.0)
  {
    Serial.print(" wheel_steps: ");Serial.print(wheel_steps);Serial.println(" Move left");
    ConfigMotorsToMoveSidewaysLeft();
  }
  else
  {
    Serial.println();
  }
  RunMotors();
  //Move carriage in Forward/Backward direction based on dy '+'ve/'-'ve by wheel_steps
  delay(DELAY_BETWEEN_CONSEQUTIVE_MOVEMENTS); //Wait for carriage to stabilize before taking different direction
  wheel_steps = abs(dy)*SPMM;
  steps_togo_remaining = true; //to continue untill the carriage moves wheel_steps (RunMotors called untill stepsToGo becomes zero)
  Serial.print("dy: ");Serial.print(dy);
  if(dy > 0.0)
  {
    Serial.print(" wheel_steps: ");Serial.print(wheel_steps);Serial.println(" Move forward");
    ConfigMotorsToMoveForward();
  }
  else if(dy < 0.0)
  {
    Serial.print(" wheel_steps: ");Serial.print(wheel_steps);Serial.println(" Move back");
    ConfigMotorsToMoveBackward();
  }
  else
  {
    Serial.println();
  }
  RunMotors();
  CmdSerial.write(CMD_ACK);//Send acknoledgement for goto command.
}
void RotateCarriage()
{
  char val_array[VAL_STR_SZ+1]={0,};//+1 is for null char
  SetupMotors();
  rot_dir = cmd_buf[INDX_MODE];
  memcpy(val_array,&cmd_buf[INDX_VAL1],VAL_STR_SZ);
  uData.i = strtoul(val_array, NULL, CMD_VALS_FORMAT);
  rot_degs = uData.f;
//  memcpy(val_array,&cmd_buf[INDX_VAL2],VAL_STR_SZ); //read val string characters to val_array
//  uData.i = strtoul(val_array, NULL, CMD_VALS_FORMAT);           //convert to integer
//  rot_mins = uData.f;                             //get float value  
  Serial.print("Req rot deg:");Serial.println(rot_degs);  
  wheel_steps = rot_degs * SPD;
  steps_togo_remaining = true; //to continue untill the carriage moves wheel_steps (RunMotors called untill stepsToGo becomes zero)
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
  RunMotors();
  CmdSerial.write(CMD_ACK);//Send acknoledgement for rotate command.
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
  //CmdSerial.write('F');
}

void SetMaxSpeed()
{
  char val_array[VAL_STR_SZ+1]={0,};//+1 is for null char

  memcpy(val_array, &cmd_buf[INDX_VAL1], VAL_STR_SZ);
  uData.i = strtoul(val_array, NULL, CMD_VALS_FORMAT);
  MaxSpeed = uData.f;  
  Serial.print("MaxSpeed: ");Serial.println(MaxSpeed);
  SetSpeed(MaxSpeed);
}

void SetMaxAccel()
{
  char val_array[VAL_STR_SZ+1]={0,};//+1 is for null char

  memcpy(val_array, &cmd_buf[INDX_VAL1], VAL_STR_SZ);
  uData.i = strtoul(val_array, NULL, CMD_VALS_FORMAT);
  MaxAccel = uData.f;  
  Serial.print("MaxAccel: ");Serial.println(MaxAccel);
  SetAcc(MaxAccel);  
}

void SetupMotors()
{
  Serial.print("Setting Wheel Speed to: ");Serial.println(MaxSpeed);
  LeftFrontWheel.setMaxSpeed(MaxSpeed);
  LeftBackWheel.setMaxSpeed(MaxSpeed);
  RightFrontWheel.setMaxSpeed(MaxSpeed);
  RightBackWheel.setMaxSpeed(MaxSpeed);
  Serial.print("Setting Wheel Acceleration to: ");Serial.println(MaxAccel);
  LeftFrontWheel.setAcceleration(MaxAccel);
  LeftBackWheel.setAcceleration(MaxAccel);
  RightFrontWheel.setAcceleration(MaxAccel);
  RightBackWheel.setAcceleration(MaxAccel);
  ResetMotors();                            // reset poS1tions to zero  
}

void StopMecanum() // Cmd format:!3X00000000000000000
{
  steps_togo_remaining = false;
  ResetMotors();
  Serial.println("Emergency Stop!!");
  //CmdSerial.write('F');
}
 
void SetSpeed(int32_t MxSpeed)  // Cmd format:!3SxSSSSSSSS00000000, where SSSSSSSS is max speed
{
  SetupMotors();
  Serial.print("Max Speed set to ");Serial.println(MxSpeed);
  //CmdSerial.write('F');
}

void SetAcc(int MxAcc)  // Format is !3AxAAAAAAAA00000000 where AAAAAAAA is max acceleration
{ 
  SetupMotors();
  Serial.print("Max Accelearation set to ");Serial.println(MxAcc);
  //CmdSerial.write('F');
}

/**************************************************************************************************************
 *Run all the motors by 1 step 
 **************************************************************************************************************/
void RunMotors()
{
  while((LeftFrontWheel.distanceToGo() != 0) || (RightFrontWheel.distanceToGo() != 0) || (LeftBackWheel.distanceToGo() != 0) || (RightBackWheel.distanceToGo() != 0))
  {
    LeftFrontWheel.run();
    LeftBackWheel.run();
    RightFrontWheel.run();
    RightBackWheel.run();
  }
  //Motors have completed running wheel_steps
  ResetMotors();
  Serial.println("Carriage Movement completed.");
}
//void RunMotors()
//{
//  if(steps_togo_remaining == true)
//  {
//    LeftFrontWheel.run();
//    LeftBackWheel.run();
//    RightFrontWheel.run();
//    RightBackWheel.run();
//    if(LeftFrontWheel.distanceToGo() == 0 && RightFrontWheel.distanceToGo() == 0&& LeftBackWheel.distanceToGo() == 0 && RightBackWheel.distanceToGo() == 0)
//    { //Motors have completed running wheel_steps
//      ResetMotors();
//      Serial.println("Carriage Movement completed.");
//      CmdSerial.write(CMD_ACK); //Send reply to master controller to go ahead with another command.
//                                //Master will be waiting untill 'F' is received when GOTO/ROTATE commands are issued to this module
//                                //This reply is required because GOTO/ROTATE will take some time to complete.
//                                //Other commands will finish immediately. No need for master to wait.
//      steps_togo_remaining = false;
//    }
//  }  
//}
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
  //CmdSerial.write('F');
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
 
