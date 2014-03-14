#ifndef Drive_h
#define Drive_h

#include "Arduino.h"

class Drive{
public:
	Drive(int basespeed, int dir1, int e1, int dir2, int e2);
	void init(int motor1Forward, int motor2Forward);
	void offset(int motorOffset);
	void drive(int leftSpeed, int rightSpeed);
	void driveForward(int speed);
	void driveBackward(int speed);
	void rotateLeft(int speed);
	void rotateRight(int speed);
	void stop();
	void writeDir(int dir1, int dir2);
	void writeSpeed(int speed1, int speed2);


private:
	int _basespeed;
	int _dir1;
	int _e1;
	int _dir2;
	int _e2;
	int _motor1Forward;
	int _motor2Forward;
	int _motorOffset;
};


#endif