#ifndef P3RP_CARRIAGE_CONTROL_H
#define P3RP_CARRIAGE_CONTROL_H

#include <string.h>
#include <AccelStepper.h>
#include <SoftwareSerial.h>

#define CARCON_VERSION_STR "Carriage Controller FW Ver:1.0.0.0"

#define ENDIS_MOTORS_PIN 4 //Pin number on Arduino nano for ENable/DISabling motors 0:enable,1:disable

#define CMD_SER_RX 3 //D3 as Rx for receiving control commands
#define CMD_SER_TX 2 //D2 as Tx for sending reply for commands
#define DBG_SER_BAUD 115200 //For sending console messages to serial terminal.
#define CMD_SER_BAUD 19200  //For receiving commands from master controller

#define CFG_MOTORS_MAXSPEED 800
#define CFG_MOTORS_MAXACCEL 200 

#define TIMEOUT_READ_CMDPKT 1000
#define DELAY_BETWEEN_CONSEQUTIVE_MOVEMENTS 1000

#define SPMM  10.19 //Steps Per MilliMeter @Wheel dia:10cm, pulses/revoluton:3200
#define SPD   48.35 //Steps per Degree rotation

//Command codes
#define CMD_SETDIM     'D' //Set layout dimensions
#define CMD_GOTO       'G' //Move carriage to new x,y positions
#define CMD_ROTATE     'R' //Rotate carriage by degrees+minutes clockwise/anticlockwise
#define CMD_ENDIS_MOTS 'E' //Enable/disable motors
#define CMD_SETSPEED   'S' //Set max speed for the motors
#define CMD_SETACCEL   'A' //Set max acceleration for the motors
#define CMD_FWVERSION  'V' //Get/display fw version
#define CMD_STOP       'X' //Emergency stop of the carriage
#define CMD_ACK        'A' //ACKnowledgement for commands
#define CMD_NACK       'N' //ACKnowledgement for commands

#define INDX_SOCF  0 //Index for Start of Command Frame
#define INDX_TGT   1 //Index for Target: Carriage control, Printhead control etc.
#define INDX_CMD   2 //Index for Command code like Move, Set speed etc.
#define INDX_MODE  3 //Index for Command mode like direction 
#define INDX_VAL1  4 //Index for Command value 1 (dist,angle, x_pos etc.)
#define INDX_VAL2 12 //Index for Command value 2 (y_pos etc.)


//Carriage/chassis rotate/turn direction codes
#define MODE_ROT_DIR_CW     'C'
#define MODE_ROT_DIR_ACW    'A'

//Motors enable/disable codes
#define MODE_ENABLE_MOTORS  'E'
#define MODE_DISABLE_MOTORS 'D'

//Error codes
#define P3RP_SUCCESS 0
#define P3RP_ERROR   1

#define SOCF       '!' //Start Of Command Frame
#define VAL_STR_SZ 8   //size of value string in command buffer (8 ascii characters) ex:0x12345678 comes as 12345678
#define CMD_BUF_SZ 20

#define CMD_VALS_FORMAT_DEC 10
#define CMD_VALS_FORMAT_OCT 8
#define CMD_VALS_FORMAT_HEX 16
#define CMD_VALS_FORMAT CMD_VALS_FORMAT_HEX

char cmd_buf[CMD_BUF_SZ+1]={0,}; //+1 to hold null character
uint32_t pos_x_max = 91440; //1ft=304.8mm, 300ft=91440mm
uint32_t pos_y_max = 91440; //1ft=304.8mm, 300ft=91440mm
uint32_t cur_pos_x = 0; //current x position of the carriage in the layout
uint32_t cur_pos_y = 0; //current y position of the carriage in the layout
uint32_t req_pos_x = 0; //requested new position x of the carriage in the layout
uint32_t req_pos_y = 0; //requested new position y of the carriage in the layout
float rot_degs     = 0.0;
char  rot_dir      = MODE_ROT_DIR_CW; //Clockwise (default)
uint32_t wheel_steps = 0;
uint32_t pending_action_wheel_steps = 0;
bool action_pending = false;
bool ack_nack_sent = false;

void (*PendingActionConfigMotors)(void);

//Instantiate stepper motor objects for four wheels. arguments:(1,dir_pin,step_pulse_pin) 
//1st argument is fixed as 1 in all. 2nd is for direction pin number, 3rd is for step pulse pin number of arduino nano
AccelStepper LeftFrontWheel(1, 5, 6);//(1, 11, 12); //Set D5:dir, D6:step pulse pins for left side front wheel stepper motor
AccelStepper LeftBackWheel(1, 7, 8);//(1, 9, 10);   //Set D7:dir, D8:step pulse pin for left side back wheel stepper motor
AccelStepper RightBackWheel(1, 9, 10);//(1, 7, 8);   //Set D9:dir, D10:step pulse pins for right side back wheel stepper motor
AccelStepper RightFrontWheel(1, 11, 12);//(1, 5, 6);  //Set D11:dir, D12:step pulse pins for right side front wheel stepper motor

SoftwareSerial CmdSerial(CMD_SER_RX, CMD_SER_TX);

union 
{
  uint32_t i;
  float f;
}uData;

void ConfigMotorsToMoveForward();
void ConfigMotorsToMoveBackward();
void ConfigMotorsToMoveLeftForward();
void ConfigMotorsToMoveRightForward();
void ConfigMotorsToMoveLeftBackward();
void ConfigMotorsToMoveRightBackward();
void ConfigMotorsToMoveSidewaysLeft();
void ConfigMotorsToMoveSidewaysRight();
void ConfigMotorsToRotateLeft();
void ConfigMotorsToRotateRight();

////Function pointers array of motors configuration functions
//void (*ConfigMotors[])(void) = {ConfigMotorsToRotateLeft, ConfigMotorsToRotateRight,
//                                ConfigMotorsToMoveLeftForward, ConfigMotorsToMoveRightForward,
//                                ConfigMotorsToMoveLeftBackward, ConfigMotorsToMoveRightBackward,
//                                ConfigMotorsToMoveSidewaysLeft, ConfigMotorsToMoveSidewaysRight,
//                                ConfigMotorsToMoveForward, ConfigMotorsToMoveBackward };                                
#endif /*P3RP_CARRIAGE_CONTROL_H*/
