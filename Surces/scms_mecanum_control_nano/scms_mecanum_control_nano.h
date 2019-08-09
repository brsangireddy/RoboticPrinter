#ifndef SCMS_MECANUM_CONTROL_NANO_H
#define SCMS_MECANUM_CONTROL_NANO_H

#define SPMM  10.19 //Steps Per MilliMeter @Wheel dia:10cm, pulses/revoluton:3200

void MoveForward();
void MoveBackward();
void MoveLeftForward();
void MoveRightForward();
void MoveLeftBackward();
void MoveRightBackward();
void MoveS1dewaysLeft();
void MoveS1dewaysRight();
void RotateLeft();
void RotateRight();
void (*moveMecanum[])(void) = {MoveForward, MoveBackward, MoveLeftForward, MoveRightForward, MoveLeftBackward, MoveRightBackward, MoveS1dewaysLeft, MoveS1dewaysRight, RotateLeft, RotateRight};


#endif //SCMS_MECANUM_CONTROL_NANO_H
