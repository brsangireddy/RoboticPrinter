#ifndef P3RP_GLOBALS_H
#define P3RP_GLOBALS_H

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <FS.h>
#include <SPIFFS.h>

#if 1
#define SSID "BETABULLS1"
#define PWD "1234@bulls"
#else
#define SSID "1stfloor-2.4ghz"
#define PWD "Tata@4321"
#endif

#define AP_DISP_NAME  "p3rp_controller"
#define AP_PASSWORD "12345678"

#define CARRIAGE_COM_PORT   1
#define PRINTHEAD_COM_PORT  2

#define DBG_SER_BAUD 115200 //For sending console message to serial terminal
#define CMD_SER_BAUD 19200  //For sending commands to control modules

//8:1 Serial port expander port selection pins
#define SP_S1   5
#define SP_S2   18
#define SP_S3   19

#define BUZZER_PIN 12
#define LASERX_IDENTIFY_PIN 13

#define IDENTIFY_BLINKS_COUNT 3

#define TGT_CARRIAGE_CON  '1'
#define TGT_PRINTHEAD_CON '2' //previously on-chassis rear stepper motor

#define SOCF '!' //Start of Command string
#define CMD_BUF_SZ 20

#define UNIT_MM 'M'
#define UNIT_CM 'C'
#define UNIT_IN 'I'
#define UNIT_FT 'F'
#define UNIT_MT 'T'
#define DFLT_UNIT UNIT_MM //distance values default unit for mm ('c' for cm, 'i' for inch, 'f' for foot, 'M' for meter

#define CMD_VALS_FORMAT_DEC 10
#define CMD_VALS_FORMAT_OCT 8
#define CMD_VALS_FORMAT_HEX 16
#define CMD_VALS_FORMAT CMD_VALS_FORMAT_HEX

//Command codes
#define CMD_SETDIM  'D' //length & breadth set by control panel software (running on the tab/laptop) after loading the layout to it
#define CMD_GOTO    'G' //Goto x,y position (CMD='G', MODE='m'/'c'/'i'/'f'/'M', VAL1=x, VAL2=y in command string to target
                        //'m'=mm,'c'=cm,'i'=inches,'f'=foot, 'M'=meter
#define CMD_MOVE    'M' //Move distances in x,y direction from the current position
                        //(CMD='M', MODE='1'/'2'/'3' ('1':X-movement only, '2':y-movement only, '3':both x-movement & y-movement)
                        //VAL1=x-distance(mm), VAL2=y-ditance (mm) in command string to target
#define CMD_ROTATE  'R' //Clockwise/Anticlockwise, deg, min (CMD='R', MODE='A'/'C' (AntiCW/Clockwise), VAL1=deg, VAL2=min in command string)
#define CMD_UNKNOWN 'U' //Unknown command

#define CMD_RESP_ACK 'A'
#define CMD_RESP_NACK 'N'

//Direction related constants & strings
#define DIR_CLOCKWISE 'C'
#define DIR_ANTICLOCK 'A'
#define DIR_LF '2'
#define DIR RF '3'
#define DIR_LB '4'
#define DIR RB '5'
#define DIR_SL '6'
#define DIR SR '7'
#define DIR_FW '8'
#define DIR_BW '9'

const PROGMEM String dir_strs[] = {"Clockwise","Anticlockwise","Front-Left","Front-Right","Back-Left","Back-Right","Sideways Left","Sideways Right","Forward","Backward"};

#define INDX_SOCF  0 //Index for Start of Command Frame
#define INDX_TGT   1 //Index for Target: Carriage control, Printhead control etc.
#define INDX_CMD   2 //Index for Command code like Move, Set speed etc.
#define INDX_MODE  3 //Index for Command mode like direction 
#define INDX_VAL1  4 //Index for Command value 1 (dist,angle, x_pos etc.)
#define INDX_VAL2 12 //Index for Command value 2 (y_pos etc.)

//Conversion factors to mm
#define CM2MM 10.0   //Cm to mm
#define IN2MM 25.4   //Inch to mm
#define FT2MM 304.8  //Foot to mm
#define YD2MM 914.4  //Yard to mm
#define MT2MM 1000.0 //Meter to mm 

uint32_t cplX    = 0;     //Carriage Position in Layout(CPL) at any instance in X directions
uint32_t cplY    = 0;     //Carriage Position in Layout(CPL) at any instance in Y directions
uint16_t hpcX    = 0;     //printerHead Position in Carriage (HPC) at any instance in X directions 
uint16_t hpcY    = 0;     //printerHead Position in Carriage (HPC) at any instance in Y directions 
uint32_t cplXmax = 91440; //Carriage Position in Layout (CPL) maximum in X direction 300' (feet) (Plan to cover 300'X300' area)
uint32_t cplYmax = 91440; //Carriage Position in Layout (CPL) maximum in Y direction 300' (feet) (Plan to cover 300'X300' area)
float hpcXmax = 457.2;//640;   //Maximum Head Position in Carriage (millimeters) in X-direction
float hpcYmax = 304.8;//380;   //Maximum Head Position in Carriage (millimeters) in Y-direction
char pos_val_unit = DFLT_UNIT;
char segfile_name[] = "jjxxxyyy.gcd";//jj:job code/id 2 chars, xxx:segment number in x-direction 000-999,yyy:segment number in y direction 000-999, gcd:GCoDe
char segfile_path_name[]="/jjxxxyyyy.gcd";
//char segfile_path_name[]="/p3rp_layout_files/gcode/jjxxxyyyy.gcd";
#define FILE_NAME_START_INDX 0//24
#define FILE_NAME_SZ 14 //one character for '/', 8 characters for name, one for '.', 3 for extension 'gcd', 1 for null termination

uint8_t ftps_ip[4] = {192, 168, 0, 3};
IPAddress ftp_server(ftps_ip[0], ftps_ip[1], ftps_ip[2], ftps_ip[3]);//(192,168,0,2);//FTP server IP address

#define FTP_PORT 21
char ftp_user[32] = "ftp";
char ftp_pwd[32] = "guest";

WebServer httpserver(80); //accessed by all the modules
WiFiClient client;  //for connection
WiFiClient dclient; //for reading/writing data over ftp

#define X_MOTION '1'//0x31
#define Y_MOTION '2'//0x32
#define PEN_ON   '4' //0x34

#define RESULT_SUCCESS 1
#define RESULT_ERROR   0

void SelectSerialPort(uint8_t);

union 
{
  uint32_t i;
  float f;
}uData;

#endif /*P3RP_GLOBALS_H*/
