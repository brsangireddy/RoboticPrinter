#include "p3rp_globals.h"
#include "p3rp_carriage_control.h"
#include "p3rp_printhead_control.h"

//System configuration functions
void ConfigurePortPins();
void ConfigureWiFi();
void ConfigureHttpServer();
void hndRoot();
void hndNotFound();


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
  httpserver.on("/", hndRoot);
  httpserver.on("/CarriageControl", hndCarriageControlCmd);
  httpserver.on("/PrintHeadControl",hndPrintHeadControlCmd);
  httpserver.onNotFound(hndNotFound);
  httpserver.begin();
  Serial.println("http server started");
}

/*********************************************************************************************
 * 
 *********************************************************************************************/
void hndRoot()
{
  httpserver.send(200, "text/html", "ok");
}
/*********************************************************************************************
 * 
 *********************************************************************************************/
void hndNotFound()
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
