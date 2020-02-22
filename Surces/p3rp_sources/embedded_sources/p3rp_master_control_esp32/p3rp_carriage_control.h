#ifndef P3RP_CARRIAGE_CONTROL
#define P3RP_CARRIAGE_CONTROL

#include "p3rp_globals.h"

char car_con_cmd      = CMD_UNKNOWN;
char car_rot_dir      = DIR_CLOCKWISE;
float car_mov_dist    = 0;
float car_posX_rotDeg = 0;
float car_posY_rotMin = 0;

void hndCarriageControlCmd();
void ProcessCarriageControlCmd();

/*********************************************************************************************
 * Extract carriage control commands and arguments
 * /CarriageControl?cmd=SetDim&x=30000&y=30000&unit=mm (unit can be mm/cm/inch/foot/meter)
 * /CarriageControl?cmd=GoTo&x=10000&y=10000&unit=mm   (unit can be mm/cm/inch/foot/meter)
 * /CarriageControl?cmd=Rotate&deg=75&min=30&dir=CW    (deg=0-359,min=0-59,
 *                                                      dir=Clockwise(CW)/Anticlockwise(ACW)
 *********************************************************************************************/
void hndCarriageControlCmd()
{
  car_con_cmd     = CMD_UNKNOWN;
  car_rot_dir     = DIR_CLOCKWISE;
  car_mov_dist    = 0;
  car_posX_rotDeg = 0;
  car_posY_rotMin = 0;
  pos_val_unit    = DFLT_UNIT;
  
  char resp_str[64]={0,};
  
  for(uint8_t i=0; i<httpserver.args(); i++)
  {
    if(httpserver.argName(i) == "dir")
    {
      car_rot_dir = httpserver.arg(i).c_str()[0];
      Serial.print("Carriage rotate dir:");Serial.println(car_rot_dir);
    }
    if(httpserver.argName(i) == "dist")
    {
      car_mov_dist = httpserver.arg(i).toFloat();
      Serial.println(car_mov_dist);
    }
    if(httpserver.argName(i) == "cmd")
    {
      car_con_cmd = CMD_UNKNOWN;
      strcpy(resp_str,httpserver.arg(i).c_str());
      if(httpserver.arg(i) == "goto")
      {
        car_con_cmd = CMD_GOTO;
      }
      else if(httpserver.arg(i) == "rotate")
      {
        car_con_cmd = CMD_ROTATE;
      }
      else if(httpserver.arg(i) == "setdim")
      {
        car_con_cmd = CMD_SETDIM;
      }
      Serial.print("Carriage Control Command: ");Serial.println(car_con_cmd);
    }
    if(httpserver.argName(i) == "x")
    {
      car_posX_rotDeg = httpserver.arg(i).toFloat();
      Serial.print("Carriage Goto X: ");Serial.println(car_posX_rotDeg);
    }
    if(httpserver.argName(i) == "y")
    {
      car_posY_rotMin = httpserver.arg(i).toFloat();
      Serial.print("Carriage Goto Y: ");Serial.println(car_posY_rotMin);
    }
    if(httpserver.argName(i) == "deg")
    {
      car_posX_rotDeg = httpserver.arg(i).toFloat();
      Serial.print("Carriage Rotate Deg: ");Serial.println(car_posX_rotDeg);
    }
    if(httpserver.argName(i) == "min")
    {      
      car_posY_rotMin = httpserver.arg(i).toFloat();
      Serial.print("Carriage Rotate Min: ");Serial.println(car_posY_rotMin);
    }            
    if(httpserver.argName(i) == "unit")
    {
      pos_val_unit = toupper(httpserver.arg(i).c_str()[0]);
      Serial.print("X,Y values unit: ");Serial.println(pos_val_unit);
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
  char cmd_buf[CMD_BUF_SZ+1]={0,}; //+1 for holding null char.
  SelectSerialPort(CARRIAGE_COM_PORT);
  cmd_buf[INDX_SOCF] = SOCF;             //Start of command or command qualifier
  cmd_buf[INDX_TGT]  = TGT_CARRIAGE_CON; //Target's ASCII value '1'  
  cmd_buf[INDX_CMD]  = car_con_cmd;      //Command type - Move command
  switch(car_con_cmd)
  {
    case CMD_GOTO:
    case CMD_SETDIM:
      cmd_buf[INDX_MODE] = pos_val_unit;
      switch(pos_val_unit)
      {
        case UNIT_CM:
          //conver cm to mm
          car_posX_rotDeg = car_posX_rotDeg * CM2MM;
          car_posY_rotMin = car_posY_rotMin * CM2MM;
          break;
        case UNIT_IN:
          //conver inch to mm
          car_posX_rotDeg = car_posX_rotDeg * IN2MM;
          car_posY_rotMin = car_posY_rotMin * IN2MM;
          break;    
        case UNIT_FT:
          //conver foot to mm
          car_posX_rotDeg = car_posX_rotDeg * FT2MM;
          car_posY_rotMin = car_posY_rotMin * FT2MM;
          break;    
        case_UNIT_MT:
          //conver meter to mm
          car_posX_rotDeg = car_posX_rotDeg * MT2MM;
          car_posY_rotMin = car_posY_rotMin * MT2MM;
          break;    
        default:
        //consider it as mm
          break;
      }      
      break;
    case CMD_ROTATE:
      float dec_part = 0.0f; 
      cmd_buf[INDX_MODE] = car_rot_dir;
      car_posX_rotDeg = car_posX_rotDeg + car_posY_rotMin/60.0; //convert everything to degrees and send to reduce computing on nano
      dec_part = car_posX_rotDeg - (uint32_t)car_posX_rotDeg;
      car_posX_rotDeg = (uint32_t)car_posX_rotDeg%360 + dec_part; //Restrict rotation to 360 deg. 
      car_posY_rotMin = 0.0;
      Serial.print("Carriage rotation deg/dir: ");Serial.print(car_posX_rotDeg);Serial.print("/");Serial.println(car_rot_dir);
      break;
  }
  uData.f = car_posX_rotDeg;
  sprintf(&cmd_buf[INDX_VAL1],"%08X",(uint32_t)uData.i); //write carriage goto position value x or rotate deg value to cmd buffer
  uData.f = car_posY_rotMin;
  sprintf(&cmd_buf[INDX_VAL2],"%08X",(uint32_t)uData.i); //write carriage goto position value y or rotate min value to cmd buffer
  Serial2.write((uint8_t*)cmd_buf,CMD_BUF_SZ); //Send command string to carriage control module. Just send CMD_BUF_SZ.
  Serial.println(cmd_buf); 
}

#endif /*P3RP_CARRIAGE_CONTROL*/
