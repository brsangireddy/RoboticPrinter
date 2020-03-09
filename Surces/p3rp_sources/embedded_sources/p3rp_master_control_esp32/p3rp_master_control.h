#include "p3rp_globals.h"
#include "p3rp_carriage_control.h"
#include "p3rp_printhead_control.h"

//System configuration functions
void ConfigurePortPins();
void ConfigureWiFi();
void ConfigureHttpServer();
//http server handler functions
void HndRoot();
void HndNotFound();
void HndDisplayFileListCmd();
void HndDeleteFilesCmd();

uint8_t ProcessPrintSegmentCmd();
uint8_t DownloadSegmentFile();
void eFail();
uint8_t readServerResp();
uint8_t DownloadSegmentFile();

struct strSysConfig {
  String  ssid;
  String  password;
  byte    IP[4];
  byte    Netmask[4];
  byte    Gateway[4];
  boolean dhcp;
  String  DeviceName;
}sysConfig;

/*********************************************************************************************
 * Initialize all the necessary port pins INPUT/OUTPUT
 *********************************************************************************************/
void ConfigurePortPins()
{
  //Serial port selection pins initialization & configuration
  digitalWrite(SP_S1, LOW);
  digitalWrite(SP_S2, LOW);
  digitalWrite(SP_S3, LOW);
  pinMode(SP_S1, OUTPUT);
  pinMode(SP_S2, OUTPUT);
  pinMode(SP_S3, OUTPUT);

   digitalWrite(BUZZER_PIN, LOW);
   pinMode(BUZZER_PIN, OUTPUT);
}

/*********************************************************************************************
 *Wifi configuration to connect to SSID and set the mode as AP/STA/AP_STA 
 *********************************************************************************************/
void ConfigureWiFi()
{
  Serial.print("Configuring WiFi & connecting to...");
  Serial.println(SSID);
  WiFi.begin (SSID, PWD);
  
  if( WiFi.waitForConnectResult() != WL_CONNECTED )//while ( WiFi.waitForConnectResult() != WL_CONNECTED )   //while(WiFi.status() != WL_CONNECTED )
  {
      Serial.print("Could not connect to ");
      Serial.println(SSID);
      Serial.println("Please check the SSID & Password...");
      WiFi.mode(WIFI_AP);
  }
  
  if(WiFi.status() == WL_CONNECTED)
  {
    Serial.print("Connected to ");
    Serial.print(SSID);
    Serial.print(", IP Address:");
    Serial.println(WiFi.localIP());

    MDNS.begin(AP_DISP_NAME);
    //otaUpdater.setup(&httpserver);
    //httpserver.begin();
    MDNS.addService("http", "tcp", 80);       
  }
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP( AP_DISP_NAME , AP_PASSWORD);
}
/*********************************************************************************************
 * Make S1,S2,S3 pins HIGH/LOW depending on the port number selected
 *********************************************************************************************/
void SelectSerialPort(uint8_t port_num)
{
  switch(port_num)
  {
    case 1:
      digitalWrite(SP_S1,LOW);
      digitalWrite(SP_S2,LOW);
      digitalWrite(SP_S3,LOW);
      break;
    case 2:
      digitalWrite(SP_S1,HIGH);
      digitalWrite(SP_S2,LOW);
      digitalWrite(SP_S3,LOW);
      break;
    case 3:
      digitalWrite(SP_S1,LOW);
      digitalWrite(SP_S2,HIGH);
      digitalWrite(SP_S3,LOW);
      break;
    case 4:
      digitalWrite(SP_S1,HIGH);
      digitalWrite(SP_S2,HIGH);
      digitalWrite(SP_S3,LOW);
      break;
    case 5:
      digitalWrite(SP_S1,LOW);
      digitalWrite(SP_S2,LOW);
      digitalWrite(SP_S3,HIGH);
      break;
    case 6:
      digitalWrite(SP_S1,HIGH);
      digitalWrite(SP_S2,LOW);
      digitalWrite(SP_S3,HIGH);
      break;
    case 7:
      digitalWrite(SP_S1,LOW);
      digitalWrite(SP_S2,HIGH);
      digitalWrite(SP_S3,HIGH);
      break;
    case 8:
      digitalWrite(SP_S1,HIGH);
      digitalWrite(SP_S2,HIGH);
      digitalWrite(SP_S3,HIGH);
      break;
    default:
      Serial.println("Invalid port number.");
      break;
  }
}

/*********************************************************************************************
 * Configure HTTP server handler functions
 *********************************************************************************************/
void ConfigureHttpServer()
{
  httpserver.on("/", HndRoot);
  httpserver.on("/CarriageControl",HndCarriageControlCmd);
  httpserver.on("/PrintHeadControl",HndPrintHeadControlCmd);
  httpserver.on("/PrintSegment", HndPrintSegmentCmd);
  httpserver.on("/DisplayFileList", HndDisplayFileListCmd);
  httpserver.on("/DeleteFiles", HndDeleteFilesCmd);  
  httpserver.onNotFound(HndNotFound);
  httpserver.begin();
  Serial.println("http server started");
}

 void HndDisplayFileListCmd()
 {
    String ibuf    = "";
    uint32_t fsize = 0;
    int fcnt       = 0;
    String fname   = "";
    boolean filefound = false;
    File rdh; //root dir handle
    ibuf += "<form method='GET'>";
    ibuf += "<table border='1' cellspacing='0' cellpadding='0'>";
    ibuf += "<tr bgcolor='#DDDDDD' ><td><strong>S#</strong></td><td align='center'><strong>Name</strong></td><td><strong>Size</strong></td><tr>";
    rdh = SPIFFS.open("/"); //display all files from /edata dir
    if(!rdh)
    {
      Serial.println("Open root dir failed.");
      return;
    }
    File fh = rdh.openNextFile();
    while(fh)
    {
      fcnt++;      
      fname = fh.name();
      filefound = true;
      fsize = fh.size(); 
      ibuf += "<tr><td>" + String(fcnt) + "</td><td>" + String(fname) + "</td><td>" + String(fsize) + "</td></tr>";
      Serial.println(fname);
      fh = rdh.openNextFile();
    } //while
    if (filefound == false)
    {
       //no file    
       ibuf = "No File(s) Found.";
    }
    else
    {
      ibuf += "</table>";
      ibuf += "</form>";
    }
      
    httpserver.send(200, "text/html", ibuf);    
 }

void HndDeleteFilesCmd()
{
  File rdh; //root dir handle
  String ibuf = "";
  int fcnt    = 0;
  rdh = SPIFFS.open("/"); //display all files from /edata dir
  if(!rdh)
  {
    Serial.println("Open root dir failed.");
    ibuf = "Open root dir failed.";
    httpserver.send(200, "text/html", ibuf);      
    return;
  }
  File fh = rdh.openNextFile();
  while(fh)
  {
    fcnt++;      
    SPIFFS.remove(fh.name());
    fh = rdh.openNextFile();
  } //while
  ibuf = "Deleted " + String(fcnt)+ " file(s)";
  httpserver.send(200, "text/html", ibuf);      
}
/*********************************************************************************************
 * 
 *********************************************************************************************/
void HndRoot()
{
  httpserver.send(200, "text/html", "ok");
}
/*********************************************************************************************
 * 
 *********************************************************************************************/
void HndNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += httpserver.uri();
  message += "\nMethod: ";
  message += (httpserver.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += httpserver.args();
  message += "\n";

  for (uint8_t i = 0; i < httpserver.args(); i++) {
    message += " " + httpserver.argName(i) + ": " + httpserver.arg(i) + "\n";
  }

  httpserver.send(404, "text/plain", message);
}

void HndPrintSegmentCmd()
{
  uint8_t result;
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
  result = ProcessPrintSegmentCmd();
  if(result == RESULT_SUCCESS)
  {
    strcat(resp_str," command sent to printhead control module successfully.");
    httpserver.send(200, "text/html", resp_str);  
  }
  else
  {
    strcat(resp_str, "command FAILED");
    httpserver.send(400, "text/html", resp_str);  
  }
}

uint8_t ProcessPrintSegmentCmd()
{
  uint8_t result;
  int32_t x_seg = 0;
  int32_t y_seg = 0;
  String file_name = "/jjxxxyyy.gcd";//dummy file name, resembles segment gcode file name
  char cmd_buf[CMD_BUF_SZ+1]={0,}; //+1 for holding null char

  result = DownloadSegmentFile();
  if(result == RESULT_SUCCESS)//when the file is successfully downloaded
  {
    for(int i=0; i<(FILE_NAME_SZ-1);i++)
    {
      file_name[i] = segfile_name[i];
    }
    Serial.println(file_name);
    x_seg = file_name.substring(3,6).toInt();
    y_seg = file_name.substring(6,9).toInt();
    Serial.print("x,y segment numbers: ");Serial.print(x_seg);Serial.print(",");Serial.println(y_seg);

    car_posX_rotDeg = x_seg * hpcXmax;
    car_posY_rotMin = y_seg * hpcYmax;
    Serial.print("Carriage to be moved to new x,y: ");Serial.print(car_posX_rotDeg);Serial.print(",");Serial.println(car_posY_rotMin);

    //Send command to carriage control unit to move the carriage to desired segment to be printed
    SelectSerialPort(CARRIAGE_COM_PORT);
    
    cmd_buf[INDX_SOCF] = SOCF;               //Start of command or command qualifier
    cmd_buf[INDX_TGT]  = TGT_CARRIAGE_CON; //Target's ASCII value '3'  
    cmd_buf[INDX_CMD]  = CMD_GOTO;        //Command type - goto command
    cmd_buf[INDX_MODE] = UNIT_MM;
    
    uData.f = car_posX_rotDeg;
    sprintf(&cmd_buf[INDX_VAL1],"%08X",(uint32_t)uData.i);
    uData.f = car_posY_rotMin;
    sprintf(&cmd_buf[INDX_VAL2],"%08X",(uint32_t)uData.i);  
    Serial.println(cmd_buf); //Print for local debugging 
    //Serial2.write((uint8_t*)cmd_buf,CMD_BUF_SZ);//Send command string to carriage control module

    //****************wait until the carriage moves to desired segment
    result = PrintSegment(); //handled by printhead control unit for printing.
  }
  return result;  
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
