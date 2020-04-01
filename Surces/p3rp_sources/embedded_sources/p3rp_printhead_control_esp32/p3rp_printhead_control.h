#ifndef P3RP_PRINTHEAD_CONTROL_H
#define P3RP_PRINTHEAD_CONTROL_H

#include <string.h>
#include <AccelStepper.h>
#include <ESP32_Servo.h>

#define PHCON_VERSION_STR "Printhead Controller FW Ver:1.0.0.0"

#define ENDIS_MOTORS_PIN 26 //Pin number on ESP32 DOIT Devkit for ENable/DISabling motors 0:enable,1:disable
#define X0_SENSOR_PIN   35
#define Y0_SENSOR_PIN   34
#define XMAX_SENSOR_PIN 39
#define XMAX_SENSOR_PIN 36

#define SERVO_PIN         25
//#define SERVO_CHANNEL     0
//#define SERVO_FREQ_HZ     1000
//#define SERVO_RESOLUTION_BITS  8

#define MARKER_UP_ANG  0
#define MARKER_DOWN_ANG 90

#define POS_X_MAX 457//1.5' //640 //mm (=64cm)
#define POS_Y_MAX 305//1.0' //380 //mm (=38cm)

#define DBG_SER_BAUD 115200 //For sending console messages to serial terminal.
#define CMD_SER_BAUD 19200  //For receiving commands from master controller

#define CFG_MOTORS_MAXSPEED 12000.0
#define CFG_MOTORS_MAXACCEL 200.0 
#define CFG_XMOTOR_SPEED CFG_MOTORS_MAXSPEED
#define CFG_YMOTOR_SPEED CFG_MOTORS_MAXSPEED

#define XMOTOR_MAX_STEPS POS_X_MAX*SPMM //set to detect home position
#define YMOTOR_MAX_STEPS POS_Y_MAX*SPMM //set to detect home position

#define TIMEOUT_READ_CMDPKT 1000
#define DELAY_BETWEEN_MOVEMENTS 1000

#define SPMM  81.5 //Steps Per MilliMeter

//Command codes
#define CMD_SETDIM     'D' //Set layout dimensions
#define CMD_GOTO       'G' //Move Printhead carriage to new x,y position
#define CMD_MOVE       'M' //Rotate carriage by degrees+minutes clockwise/anticlockwise
#define CMD_ROTATE     'R' //Rotate carriage by degrees+minutes clockwise/anticlockwise
#define CMD_ENDIS_MOTS 'E' //Enable/disable motors
#define CMD_SETSPEED   'S' //Set max speed for the motors
#define CMD_SETACCEL   'A' //Set max acceleration for the motors
#define CMD_FWVERSION  'V' //Get/display fw version
#define CMD_STOP       'X' //Emergency stop of the carriage
#define CMD_RESP_ACK        'A' //ACKnowledgement for commands
#define CMD_RESP_NACK       'N' //ACKnowledgement for commands

#define INDX_SOCF  0 //Index for Start of Command Frame
#define INDX_TGT   1 //Index for Target: Carriage control, Printhead control etc.
#define INDX_CMD   2 //Index for Command code like Move, Set speed etc.
#define INDX_MODE  3 //Index for Command mode like x-motion, y-motion, pen/print - ON 
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

#define PEN_ON_FLAG_MASK   0x04 //If pen/print to be on, this module will receive 3rd bit set in cmd_buf[INDX_MODE]

//Instantiate stepper motor objects for four wheels. arguments:(1,step_pulse_pin,dir_pin) 
//1st argument is fixed as 1 in all. 2nd is for direction pin number, 3rd is for step pulse pin number of arduino nano
AccelStepper XdirMotor(1, 14, 27);//Set D14:step pulse, D27:dir pin for left side back wheel stepper motor
AccelStepper YdirMotor(1, 13, 12);//Set D13:step pulse, D12:dir pins for left side front wheel stepper motor

char cmd_buf[CMD_BUF_SZ+1]={0,}; //+1 to hold null character
int32_t pos_x_max   = POS_X_MAX;
int32_t pos_y_max   = POS_Y_MAX;
int32_t cur_pos_x   = 0; //current x position of the carriage in the layout
int32_t cur_pos_y   = 0; //current y position of the carriage in the layout
int32_t req_pos_x   = 0; //requested new position x of the carriage in the layout
int32_t req_pos_y   = 0; //requested new position y of the carriage in the layout
int32_t x_motor_steps = 0;
int32_t y_motor_steps = 0;
float x_motor_speed = CFG_XMOTOR_SPEED;
float y_motor_speed = CFG_YMOTOR_SPEED;
bool ack_nack_sent   = false;

Servo MarkerUpDownServo; //Pen control servo

union 
{
  uint32_t i;
  float f;
}uData;

//For X-direction movement control
void ConfigXdirMotorToMoveLeft();
void ConfigXdirMotorToMoveRight();
//For Y-direction movement control
void ConfigYdirMotorToMoveForward();
void ConfigYdirMotorToMoveBackward();

#endif /*P3RP_PRINTHEAD_CONTROL_H*/
