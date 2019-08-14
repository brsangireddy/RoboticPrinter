#ifndef SCMS_MASTER_CONTROL_NRF52840_H
#define SCMS_MASTER_CONTROL_NRF52840_H

String ver =   "Robotic Printer Ver 1.0";

#define SM_SPORT      1 //Stepper Motor (front and rear)control arduino nano Serial Port number
#define WC_SPORT      2 // Robot Wheel(movement) Control arduino nano Serial Port number
#define FL_SPORT      3 //Front Laser Serial Port number
#define RL_SPORT      4 //Rear Laser Serial Port number
#define UNUSED_SPORT1 5
#define UNUSED_SPORT2 6
#define UNUSED_SPORT3 7
#define UNUSED_SPORT4 8

//Robot movement directions
#define RM_CW         0
#define RM_RR         2 //Rear Right diagonal
#define RM_ACW        1
#define RM_FL         3 //Fornt Left Diagonal
#define RM_FR         4 //Fornt Right diagonal
#define RM_RL         5 //Rear Left diagonal
#define RM_LEFT       6
#define RM_RIGHT      7
#define RM_FORWARD    8
#define RM_BACKWARD   9

//Stepper motor directions
#define SM_DIR_CW         0 //Stepper Motor ClockWise
#define SM_DIR_ACW        1 //Stepper Motor Anti ClockWise

//Laser measurement modes
#define LMM_FAST      0 //Laser Measurment Mode Fast
#define LMM_SLOW      1 //Laser Measurment Mode Slow
#define LMM_AUTO      2 //Laser Measurment Mode Auto

//Laser control
#define LASER_OFF         0 //Laser OFF
#define LASER_ON          1 // Laser ON

//Command codes from Mobile/Application
#define CMD_VER       'V' //Firmware version details
#define CMD_MVMT      'M' //Movement control
#define CMD_LOS       'O' //Laser OneShot Measurment 
#define CMD_MWSPEED   'S' //Mecanum Wheel Speed
#define CMD_STO       'D' //Start Operation
//Control Target for commands
#define CT_FSM        '1' //Front Stepper Motor 
#define CT_RSM        '2' //Rear Stepper Motor
#define CT_MWR        '3' //Mecanum Wheel Rotate
#define CT_FL         '4' //Front Laser
#define CT_RL         '5' //Rear Laser
#define CT_CB         '6' //Corner Beacon

//chassis direction constants
#define CHAS_DIR_CW 0 //ClockWise(Rotate Right)
#define CHAS_DIR_ACW 1 //Anti ClockWise(Rotate Left)
#define CHAS_DIR_MLF 2 //Move Left Forward diagonal
#define CHAS_DIR_MRF 3 //Move Right Forward diagonal
#define CHAS_DIR_MLB 4 //Move Left Backward diagonal
#define CHAS_DIR_MRB 5 //Move Right Backward diagonal
#define CHAS_DIR_SWL 6 //SideWay Left
#define CHAS_DIR_SWR 7 //SideWay Right
#define CHAS_DIR_MF 8 //Move Forward
#define CHAS_DIR_MB 9 //Move Backward

//Center point to stepper motor distance
#define SP_CH_CENT_DIST  100 //Distance from Stepper to Chassis Center point in mm 

// Laser Cross Mark pin
const byte lx = 29; //Laser Cross Moark 

//1:8 serial port mux selection pins 
const byte S1 = 19;     // to UART Mux
const byte S2 = 20;     // to UART Mux
const byte S3 = 21;     // to UART Mux

//Fron & rear laser enable pins
const byte EN_FrontLaser = 4;
const byte EN_RearLaser  = 5;

#define NUM_OF_LMMS      3 //Number of Laser Measurement Modes
#define LMC_SEQ_SIZE     9 //Laser Measurement Command array size
#define LMR_SEQ_SIZE    13 //Laser Measurement Result array size
//Laser measurement command & result sequences
const uint8_t laser_meas_seq[NUM_OF_LMMS][LMC_SEQ_SIZE] = {
                                                      {0xAA,0x00,0x00,0x20,0x00,0x01,0x00,0x02,0x23}, //OneShotFast mode
                                                      {0xAA,0x00,0x00,0x20,0x00,0x01,0x00,0x01,0x22}, //OneShotSlow mode
                                                      {0xAA,0x00,0x00,0x20,0x00,0x01,0x00,0x00,0x21}  //OneShotAuto mode 
                                                    };
//const unsigned char OneShotFast[] = {0xAA,0x00,0x00,0x20,0x00,0x01,0x00,0x02,0x23};
//const unsigned char OneShotSlow[] = {0xAA,0x00,0x00,0x20,0x00,0x01,0x00,0x01,0x22};
//const unsigned char OneShotAuto[] = {0xAA,0x00,0x00,0x20,0x00,0x01,0x00,0x00,0x21};

//const unsigned char ReadStatus[] = {0xAA,0x80,0x00,0x00,0x80};
const uint8_t laser_onoff_cmd_seq[2][LMC_SEQ_SIZE] = {
                                                        {0xAA,0x00,0x01,0xBE,0x00,0x01,0x00,0x00,0xC0},  
                                                        {0xAA,0x00,0x01,0xBE,0x00,0x01,0x00,0x01,0xC1}
                                                      };
//const uint8_t LaserONdata[] = {0xAA,0x00,0x01,0xBE,0x00,0x01,0x00,0x01,0xC1};
//const uint8_t LaserOFFdata[] ={0xAA,0x00,0x01,0xBE,0x00,0x01,0x00,0x00,0xC0};

uint8_t MeasResult[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

#define READ_BUFSIZE                    (20)

bool beacon_responce_flag = false;


char beacon_names[4][32] = { "LCbb_CBEACON_00","LCbb_CBEACON_01","LCbb_CBEACON_02","LCbb_CBEACON_03" };
byte cbeacon_sel = 0; //Corner Beacon Selected

#define CW 0 //ClockWise
#define ACW 1 //Anti ClockWise
#define SCP_1D    223 // Step Count per 1 Degree rotation
#define SCP_180D 40000 // Step Count Per 180 Degrees rotation
#define SCP_360D 80000 // Step Count Per 360 Degrees rotation
#define SCP_10D  6000 // Step Count Per 10 Degrees rotation
#define SCP_20D  12000 // Step Count Per 20 Degrees rotation
#endif //SCMS_MASTER_CONTROL_NRF52840_H
