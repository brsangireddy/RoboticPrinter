#ifndef SCMS_MECANUM_CONTROL_NANO_H
#define SCMS_MECANUM_CONTROL_NANO_H

#define SPMM  255 //Steps Per MilliMeter (25.477 steps for centimeter)

void MoveBackward();
void MoveLeftForward();
void MoveLeftForward();
void MoveRightForward();
void MoveLeftBackward();
void MoveRightBackward();
void MoveS1dewaysLeft();
void MoveS1dewaysRight();
void RotateLeft();
void RotateRight();
void (*moveMecanum[])(void) = {MoveBackward, MoveLeftForward, MoveLeftForward, MoveRightForward, MoveLeftBackward, MoveRightBackward, MoveS1dewaysLeft, MoveS1dewaysRight, RotateLeft, RotateRight};


#endif //SCMS_MECANUM_CONTROL_NANO_H
