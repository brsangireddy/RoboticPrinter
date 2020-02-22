#ifndef SCMS_MECANUM_CONTROL_NANO_H
#define SCMS_MECANUM_CONTROL_NANO_H

#define SPMM  10.19 //Steps Per MilliMeter @Wheel dia:10cm, pulses/revoluton:3200
#define SPD   48.35 //Steps per Degree rotation

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

void MoveForward();
void MoveBackward();
void MoveLeftForward();
void MoveRightForward();
void MoveLeftBackward();
void MoveRightBackward();
void MoveSidewaysLeft();
void MoveSidewaysRight();
void RotateLeft();
void RotateRight();
void (*moveMecanum[])(void) = {RotateLeft,RotateRight , MoveLeftForward, MoveRightForward, MoveLeftBackward, MoveRightBackward, MoveSidewaysLeft, MoveSidewaysRight, MoveForward, MoveBackward };

union 
{
  uint32_t i;
  float f;
}uData;

#endif //SCMS_MECANUM_CONTROL_NANO_H
