#ifndef P3RP_CARRIAGE_CONTROL
#define P3RP_CARRIAGE_CONTROL

#include "p3rp_globals.h"

char car_con_cmd   = CMD_UNKNOWN;
char car_cmd_mode  = CMD_MODE_UNKNOWN;
float car_mov_dist = 0;
float cmd_val1     = 0.0;//holds values of req pos x, rotation degrees, distance to move.
float cmd_val2     = 0.0;//holds values of req pos y, rotation minutes. zero incase of move command

void HndCarriageControlCmd();
void ProcessCarriageControlCmd();

/*********************************************************************************************
 * Extract carriage control commands and arguments
 * /CarriageControl?cmd=SetDim&x=30000&y=30000&unit=mm (unit can be mm/cm/inch/foot/meter)
 * /CarriageControl?cmd=GoTo&x=10000&y=10000&unit=mm   (unit can be mm/cm/inch/foot/meter)
 * /CarriageControl?cmd=Rotate&deg=75&min=30&dir=CW    (deg=0-359,min=0-59,
 *                                                      dir=Clockwise(CW)/Anticlockwise(ACW)
 *********************************************************************************************/
void HndCarriageControlCmd()
{
  car_con_cmd   = CMD_UNKNOWN;
  car_cmd_mode  = CMD_MODE_UNKNOWN;
  cmd_vals_unit = DFLT_UNIT;
  car_mov_dist  = 0;
  cmd_val1      = 0;
  cmd_val2      = 0;
  
  char resp_str[64]={0,};
  
  for(uint8_t i=0; i<httpserver.args(); i++)
  {
    if(httpserver.argName(i) == "dir")
    {
      car_cmd_mode = httpserver.arg(i).c_str()[0];
      Serial.print("Carriage rotate dir:");Serial.println(car_cmd_mode);
    }
    if(httpserver.argName(i) == "dist")
    {
      cmd_val1 = httpserver.arg(i).toFloat(); //distance by which carriage has to be moved
      Serial.print("Distance to move");Serial.println(cmd_val1);
    }
    if(httpserver.argName(i) == "cmd")
    {
      car_con_cmd = CMD_UNKNOWN;
      strcpy(resp_str,httpserver.arg(i).c_str());
      if(httpserver.arg(i) == "goto")
      {
        car_con_cmd = CMD_GOTO;
      }
      else if(httpserver.arg(i) == "move")
      {
        car_con_cmd = CMD_MOVE;
      }
      else if(httpserver.arg(i) == "rotate")
      {
        car_con_cmd = CMD_ROTATE;
      }
      else if(httpserver.arg(i) == "setdimlayout")
      {
        car_con_cmd = CMD_SETDIM;
        car_cmd_mode = MODE_SETDIM_LAYOUT;
      }
      else if(httpserver.arg(i) == "setdimsegment")
      {
        car_con_cmd = CMD_SETDIM;
        car_cmd_mode = MODE_SETDIM_SEGMENT;
      }
      Serial.print("Carriage Control Command: ");Serial.println(car_con_cmd);
    }
    if(httpserver.argName(i) == "x")
    {
      cmd_val1 = httpserver.arg(i).toFloat();
      Serial.print("Carriage Goto X: ");Serial.println(cmd_val1);
    }
    if(httpserver.argName(i) == "y")
    {
      cmd_val2 = httpserver.arg(i).toFloat();
      Serial.print("Carriage Goto Y: ");Serial.println(cmd_val2);
    }
    if(httpserver.argName(i) == "deg")
    {
      cmd_val1 = httpserver.arg(i).toFloat();
      Serial.print("Carriage Rotate Deg: ");Serial.println(cmd_val1);
    }
    if(httpserver.argName(i) == "min")
    {      
      cmd_val2 = httpserver.arg(i).toFloat();
      Serial.print("Carriage Rotate Min: ");Serial.println(cmd_val2);
    }            
    if(httpserver.argName(i) == "unit")
    {
      cmd_vals_unit = toupper(httpserver.arg(i).c_str()[0]);
      Serial.print("X,Y values unit: ");Serial.println(cmd_vals_unit);
    }
  }
  ProcessCarriageControlCmd();
  strcat(resp_str," command sent to carriage control module successfully.");
  httpserver.send(200, "text/html", resp_str);
}

/********************************************************************************************
 * Send the command to wheel control subsystem/module over serial port number 1 of 8:1 serial
 * mux
 ********************************************************************************************/
void ProcessCarriageControlCmd()
{
  float ang = 0.0;
  uint8_t ack_nack = CMD_RESP_NACK;
  char cmd_buf[CMD_BUF_SZ+1]={0,}; //+1 for holding null char.
  SelectSerialPort(CARRIAGE_COM_PORT);
  cmd_buf[INDX_SOCF] = SOCF;             //Start of command or command qualifier
  cmd_buf[INDX_TGT]  = TGT_CARRIAGE_CON; //Target's ASCII value '1'  
  cmd_buf[INDX_CMD]  = car_con_cmd;      //Command type - Move command
  cmd_buf[INDX_MODE] = car_cmd_mode;

  switch(car_con_cmd)
  {
    case CMD_MOVE:
    case CMD_GOTO:
    case CMD_SETDIM:
      switch(cmd_vals_unit)
      {
        case UNIT_CM:
          //conver cm to mm
          cmd_val1 = cmd_val1 * CM2MM;
          cmd_val2 = cmd_val2 * CM2MM;
          break;
        case UNIT_IN:
          //conver inch to mm
          cmd_val1 = cmd_val1 * IN2MM;
          cmd_val2 = cmd_val2 * IN2MM;
          break;    
        case UNIT_FT:
          //conver foot to mm
          cmd_val1 = cmd_val1 * FT2MM;
          cmd_val2 = cmd_val2 * FT2MM;
          break;    
        case_UNIT_MT:
          //conver meter to mm
          cmd_val1 = cmd_val1 * MT2MM;
          cmd_val2 = cmd_val2 * MT2MM;
          break;    
        default:
        //consider it as mm
          break;
      }      
      break;
    case CMD_ROTATE:
      ang = cmd_val1 + cmd_val2/60.0; //requested rotation angle, used for transforming the print frame coordinates.
      if(car_cmd_mode == MODE_ROTATE_CLOCKWISE)
      {
        car_cur_ang += ang;
      }
      else if(car_cmd_mode == MODE_ROTATE_ANTICLOCKWISE)
      {
        car_cur_ang -= ang;
      }
      car_cur_boundary = car_cur_ang/90.0;
      Serial.print("Current net angle: ");Serial.print(car_cur_ang);Serial.print("Current boundary: ");Serial.println(car_cur_boundary);
      Serial.print("Rotate Carriage by: ");Serial.print(cmd_val1);Serial.print("°");Serial.print(cmd_val2);Serial.print("'");Serial.println(car_cmd_mode);
      break;
  }
  uData.f = cmd_val1;
  sprintf(&cmd_buf[INDX_VAL1],"%08X",(uint32_t)uData.i); //write carriage goto position value x or rotate deg value to cmd buffer
  uData.f = cmd_val2;
  sprintf(&cmd_buf[INDX_VAL2],"%08X",(uint32_t)uData.i); //write carriage goto position value y or rotate min value to cmd buffer
  Serial.println(cmd_buf);
#if 1       
  Serial2.write((uint8_t*)cmd_buf,CMD_BUF_SZ); //Send command string to carriage control module. Just send CMD_BUF_SZ.
  while(1)//Wait until ACK is received from print head controller
  {       
    if(Serial2.available()>0)
    {
      ack_nack = Serial2.read();
      if(ack_nack == CMD_RESP_ACK)
      {
        break;
      }
    }
    delay(100);
  }
#endif 
}

#endif /*P3RP_CARRIAGE_CONTROL*/
