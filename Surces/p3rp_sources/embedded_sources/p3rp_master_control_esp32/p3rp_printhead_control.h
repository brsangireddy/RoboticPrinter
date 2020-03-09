#ifndef P3RP_PRINTHEAD_CONTROL
#define P3RP_PRINTHEAD_CONTROL

#include "p3rp_globals.h"

float ph_pos_x  = 0.0;
float ph_pos_y  = 0.0;
char ph_con_cmd = CMD_UNKNOWN; 

void HndPrintHeadControlCmd();
void HndPrintSegmentCmd();
void ProcessPrintHeadControlCmd();
void ProcessPrintSegmentCmd();
void eFail();
uint8_t readServerResp();
uint8_t DownloadSegmentFile();
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

void HndPrintSegmentCmd()
{
  char resp_str[120]={0,};

  for(uint8_t i=0; i<httpserver.args(); i++)
  {
    if(httpserver.argName(i) == "segfile")
    {
      strcpy(resp_str,"PrintSegment ");
      strcat(resp_str, httpserver.arg(i).c_str());
      strcpy(segfile_name, "/");
      strcat(segfile_name, httpserver.arg(i).c_str());
      strcpy(segfile_path_name,segfile_name);
      //strcpy(&segfile_path_name[FILE_NAME_START_INDX],segfile_name);
    }
  }
  ProcessPrintSegmentCmd();
  strcat(resp_str," command sent to printhead control module successfully.");
  httpserver.send(200, "text/html", resp_str);  
}

void ProcessPrintSegmentCmd()
{
  DownloadSegmentFile();
  PrintSegment();
}

char outbuf[128];
char outcount;

uint8_t DownloadSegmentFile()
{
  File rdfh; //Receive Data File Handle for writing to the device
    
  if(client.connect(ftp_server, FTP_PORT)) //connect to ftp server
  { 
    Serial.println(F("Connected to FTP server"));
  } 
  else
  {
    Serial.println(F("Command connection failed"));
    return 0;
  }  
  if(!readServerResp()) return 0;

  Serial.println("Send USER");
  client.println("USER ftp");
  if (!readServerResp()) return 0;
  
  Serial.println("Send PASSWORD");
  client.println("PASS guest");
  if(!readServerResp()) return 0;

  client.println(F("SYST"));
  if(!readServerResp()) return 0;

  client.println(F("Type I"));
  if(!readServerResp()) return 0;
  
  client.println(F("PASV"));
  if (!readServerResp()) return 0;

  char *tstr = strtok(outbuf, "(,");
  int array_pasv[6];
  for ( int i = 0; i < 6; i++)
  {
    tstr = strtok(NULL, "(,");
    array_pasv[i] = atoi(tstr);
    if (tstr == NULL)
    {
      Serial.println(F("Bad PASV Answer"));
    }
  }
  unsigned int hiport, loport;
  hiport = array_pasv[4] << 8;
  loport = array_pasv[5] & 255;

  Serial.print(F("Data port: "));
  hiport = hiport | loport;
  Serial.println(hiport);

  if(dclient.connect(ftp_server, hiport))
  {
    Serial.println(F("Data connected"));
  }
  else
  {
    Serial.println(F("Data connection failed"));
    client.stop();
    return 0;
  }

  Serial.println("Send RETR filename");
  client.print(F("RETR "));
  Serial.println(&segfile_path_name[1]);
  client.println(&segfile_path_name[1]);
  if (!readServerResp())
  {
    dclient.stop();
    return 0;
  }

  Serial.println(F("Reading Segment File..."));
  Serial.println(segfile_path_name);

  rdfh = SPIFFS.open(segfile_name,"w");//open file for writing
  if(!rdfh)
  {
    dclient.stop();
    Serial.println(F("SPIFFS file open failed."));
    return 0;
  }
  else
  {
    while(dclient.connected())
    {
      while(dclient.available())
      {
        char ch = dclient.read();
        rdfh.write(ch);
        Serial.print(ch);
      }
    }
  }

  dclient.stop();
  Serial.println(F("Data client disconnected"));
  if(!readServerResp()) return 0;

  client.println(F("QUIT"));
  if(!readServerResp()) return 0;

  client.stop();
  Serial.println(F("Comm client disconnected"));

  rdfh.close();
  Serial.print(segfile_name);Serial.println(" download complete.");
  return 1;
}

void efail() 
{
  byte thisbyte = 0;

  client.println(F("QUIT"));

  while (!client.available()) delay(1);

  while (client.available())
  {
    thisbyte = client.read();
    Serial.write(thisbyte);
  }

  client.stop();
  Serial.println(F("Command disconnected"));
}// efail

/*
 * readServerResp()
 */
uint8_t readServerResp() 
{
  byte respcode;
  byte thisbyte;

  while (!client.available()) delay(1);
  respcode = client.peek();
  outcount = 0;

  while (client.available())
  {
    thisbyte = client.read();
    Serial.write(thisbyte);

    if (outcount < 127)
    {
      outbuf[outcount] = thisbyte;
      outcount++;
      outbuf[outcount] = 0;
    }
  }

  if (respcode >= '4')
  {
    efail();
    return 0;
  }
  return 1;
}// readServerResp()

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
    return 0;
  }

  Serial.println("Printing segment file started.");
  while(sfh.available())
  {
    //case G00,G01:
    gc_line = sfh.readStringUntil('\n');
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
  sfh.close();
  return 1;
}

#endif /*P3RP_PRINTHEAD_CONTROL*/
