#include "p3rp_globals.h"
#include "p3rp_carriage_control.h"
#include "p3rp_printhead_control.h"

//System configuration functions
void ConfigurePortPins();
void ConfigureWiFi();
void ConfigureHttpServer();
void HndRoot();
void HndNotFound();
void HndDisplayFileListCmd();
void HndDeleteFilesCmd();

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
