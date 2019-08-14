#ifndef SCMS_MECANUM_CONTROL_NANO_H
#define SCMS_MECANUM_CONTROL_NANO_H

#define SPMM  10.19 //Steps Per MilliMeter @Wheel dia:10cm, pulses/revoluton:3200
#define SPD   48.35 //Steps per Degree rotation
//chassis direction constants
#define CHAS_DIR_CW 8
#define CHAS_DIR_ACW 9

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
void (*moveMecanum[])(void) = {RotateRight, RotateLeft, MoveLeftForward, MoveRightForward, MoveLeftBackward, MoveRightBackward, MoveSidewaysLeft, MoveSidewaysRight, MoveForward, MoveBackward };


#endif //SCMS_MECANUM_CONTROL_NANO_H
