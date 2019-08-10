#ifndef SCMS_ONCHASSIS_SM_CONTROL_NANO_H
#define SCMS_ONCHASSIS_SM_CONTROL_NANO_H

String ver = "Laser Beacon FW Ver 1.0";

#define FSM '1' //Front Stepper Motor
#define RSM '2' //Rear Stepper Motor
#define SMR 'M' //Stepper Motor Rotation
//#define GetDegrees 'G'
#define VERSION 'V' //Version

#define CW 1 //ClockWise
#define ACW 0 //Anti ClockWise
#define SPD 222.222     //steps per one degree rotation of stepper motor

#define SCP_180D 40000 // Sttep Count Per 180 Degrees rotation
#define SCP_360D 80000 // Sttep Count Per 360 Degrees rotation
#define SCP_10D  6000 // Sttep Count Per 10 Degrees rotation
#define SCP_20D  12000 // Sttep Count Per 20 Degrees rotation

const byte SmFEn=9;
const byte SmFPulse=11;
const byte SmFDir=10;
const byte SmFHD=3;

const byte SmBEn=5;
const byte SmBPulse=7;
const byte SmBDir=6;
const byte SmBHD=4;

#endif //SCMS_ONCHASSIS_SM_CONTROL_NANO_H
