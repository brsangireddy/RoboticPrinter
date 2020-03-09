#ifndef P3RP_PRINTHEAD_CONTROL
#define P3RP_PRINTHEAD_CONTROL

#include "p3rp_globals.h"

float ph_pos_x  = 0.0;
float ph_pos_y  = 0.0;
char ph_con_cmd = CMD_UNKNOWN; 

void HndPrintHeadControlCmd();
void HndPrintSegmentCmd();
void ProcessPrintHeadControlCmd();
uint8_t PrintSegment(); 
/*********************************************************************************************
 * Extract carriage movement control parameters/argument
 *********************************************************************************************/
void HndPrintHeadControlCmd()
{
  char resp_str[120]={0,};

  ph_pos_x     = 0.0;
  ph_pos_y     = 0.0;
  ph_con_cmd   = CMD_UNKNOWN;
  pos_val_unit = DFLT_UNIT;
  
  for(uint8_t i=0; i<httpserver.args(); i++)
  {
    if(httpserver.argName(i) == "cmd")
    {
      strcpy(resp_str,httpserver.arg(i).c_str());
      if(httpserver.arg(i) == "goto")
      {
        ph_con_cmd = CMD_GOTO;
      }
      else if(httpserver.arg(i) == "setdim")
      {
        ph_con_cmd = CMD_SETDIM;
      }
      Serial.print("Printhead Control Command: ");Serial.println(ph_con_cmd);
    }
    if(httpserver.argName(i) == "x")
    {
      ph_pos_x = httpserver.arg(i).toFloat();
    }
    if(httpserver.argName(i) == "y")
    {
      ph_pos_y = httpserver.arg(i).toFloat();
    }
    if(httpserver.argName(i) == "unit")
    {
      pos_val_unit = httpserver.arg(i).c_str()[0];
      Serial.print("X,Y Values unit: ");Serial.println(pos_val_unit);
    }        
  }
  ProcessPrintHeadControlCmd();
  strcat(resp_str," command sent to printhead control module successfully.");
  httpserver.send(200, "text/html", resp_str);
}

/********************************************************************************************
 * Send the command to wheel control subsystem/module over serial port number 1 of 8:1 serial
 * mux
 ********************************************************************************************/
void ProcessPrintHeadControlCmd()
{
  char cmd_buf[CMD_BUF_SZ+1]={0,}; //+1 for holding null char
  
  SelectSerialPort(PRINTHEAD_COM_PORT);
  cmd_buf[INDX_SOCF] = SOCF;               //Start of command or command qualifier
  cmd_buf[INDX_TGT]  = TGT_PRINTHEAD_CON; //Target's ASCII value '3'  
  cmd_buf[INDX_CMD]  = ph_con_cmd;        //Command type - goto command
  cmd_buf[INDX_MODE] = pos_val_unit;
  switch(pos_val_unit)
  {
    case UNIT_CM:        
      ph_pos_x = ph_pos_x * CM2MM; //conver cm to mm
      ph_pos_y = ph_pos_y * CM2MM; //conver cm to mm
      break;
    case UNIT_IN:        
      ph_pos_x = ph_pos_x * IN2MM; //conver inch to mm
      ph_pos_y = ph_pos_y * IN2MM; //conver inch to mm
      break;    
    case UNIT_FT:        
      ph_pos_x = ph_pos_x * FT2MM; //conver foot to mm
      ph_pos_y = ph_pos_y * FT2MM; //conver foot to mm
      break;    
    case_UNIT_MT:        
      ph_pos_x = ph_pos_x * MT2MM; //conver meter to mm
      ph_pos_y = ph_pos_y * MT2MM; //conver meter to mm
      break;    
    default:
    //consider it as mm
      break;
  }        
  //sprintf(&cmd_buf[INDX_VAL1],"%08X",ph_x); //Copy x value to val1 field of the command buffer
  //sprintf(&cmd_buf[INDX_VAL2],"%08X",ph_y); //Copy y value to val2 field of the command buffer
  uData.f = ph_pos_x;
  sprintf(&cmd_buf[INDX_VAL1],"%08X",(uint32_t)uData.i);
  uData.f = ph_pos_y;
  sprintf(&cmd_buf[INDX_VAL2],"%08X",(uint32_t)uData.i);  
  Serial.println(cmd_buf); //Print for local debugging 
  Serial2.write((uint8_t*)cmd_buf,CMD_BUF_SZ);//Send command string to carriage control module
}

uint8_t PrintSegment()
{
  File sfh; //segment file handle
  String gc_line = "";
  String sub_line = "";
  int32_t x_motion = 0;
  int32_t y_motion = 0;
  int32_t x_indx=-1;
  int32_t y_indx=-1;
  char cmd_buf[CMD_BUF_SZ+1]={0,}; //+1 for holding null char
  
  sfh = SPIFFS.open(segfile_name,"r");
  if(!sfh)
  {
    Serial.print(segfile_name);Serial.println(" file open failed.");
    return RESULT_ERROR;
  }

  Serial.println("Printing segment file started.");
  while(sfh.available())
  {
    //case G00,G01:
    gc_line = sfh.readStringUntil('\n');
    x_motion = 0;
    y_motion = 0;
    cmd_buf[INDX_MODE] = 0;//Clear X,Y-motion flags in the command mode

    x_indx = gc_line.indexOf('X');//returns -1 if X-motion is not present
    if(x_indx != -1) 
    {
      sub_line = gc_line.substring(x_indx+1,gc_line.indexOf('\0')-1);
      x_motion = sub_line.toInt();
      cmd_buf[INDX_MODE] |= X_MOTION;
      Serial.print("x = ");Serial.print(x_motion);Serial.print(" ");
    }
    y_indx = gc_line.indexOf('Y');
    if(y_indx != -1)//returns -1 if Y-motion is not present
    {
      sub_line = gc_line.substring(y_indx+1,gc_line.indexOf('\0')-1);
      y_motion = sub_line.toInt();
      cmd_buf[INDX_MODE] |= Y_MOTION;
      Serial.print("y = ");Serial.print(y_motion);
    }    
    Serial.println();
    //Send these values to print head control module in the frame packet.
    if((cmd_buf[INDX_MODE] & X_MOTION)||(cmd_buf[INDX_MODE] & Y_MOTION))
    {
      SelectSerialPort(PRINTHEAD_COM_PORT);
      cmd_buf[INDX_SOCF] = SOCF;               //Start of command or command qualifier
      cmd_buf[INDX_TGT]  = TGT_PRINTHEAD_CON; //Target's ASCII value '3'  
      cmd_buf[INDX_CMD]  = CMD_MOVE;        //Command type - goto command
      uData.f = x_motion;
      sprintf(&cmd_buf[INDX_VAL1],"%08X",(uint32_t)uData.i);
      uData.f = y_motion;
      sprintf(&cmd_buf[INDX_VAL2],"%08X",(uint32_t)uData.i);  
      Serial.println(cmd_buf); //Print for local debugging 
      //Serial2.write((uint8_t*)cmd_buf,CMD_BUF_SZ);//Send command string to carriage control module
    }  
  }
  sfh.close();
  return RESULT_SUCCESS;
}

#endif /*P3RP_PRINTHEAD_CONTROL*/
