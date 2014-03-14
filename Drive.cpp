#include "Arduino.h"
#include "Drive.h"

Drive::Drive(int basespeed, int dir1, int e1, int dir2, int e2){
	_basespeed = basespeed;
	pinMode(dir1, OUTPUT);
	pinMode(e1, OUTPUT);
	pinMode(dir2, OUTPUT);
	pinMode(e2, OUTPUT);
	_dir1 = dir1;
	_e1 = e1;
	_dir2 = dir2;
	_e2 = e2;

	_motorOffset = 0;
}

void Drive::init(int motor1Forward, int motor2Forward){
	_motor1Forward = motor1Forward;
	_motor2Forward = motor2Forward;
	this->stop();
}


/* Function: offset(int motorOffset)
 * ---------------------------------
 * adds an offset to motor drive functions to account for
 * motors spinning at different speed/tolerances
 */

void Drive::offset(int motorOffset) {
	_motorOffset = motorOffset;
}

void Drive::drive(int leftSpeed, int rightSpeed){
	
	//leftSpeed += _motorOffset;
	//rightSpeed -= _motorOffset;

	int leftRate = (double)leftSpeed/10 * _basespeed;
	int rightRate = (double)rightSpeed/10 * _basespeed;

	if (leftRate > 0){
		digitalWrite(_dir1, _motor1Forward);
	}
	else if (leftRate < 0){
		leftRate = -leftRate;
		digitalWrite(_dir1, _motor2Forward);
	} else{
		digitalWrite(_dir1, _motor2Forward);
	}

	if (rightRate > 0){
		digitalWrite(_dir2, _motor2Forward);
	}
	else if (rightRate < 0){
		rightRate = -rightRate;
		digitalWrite(_dir2, _motor1Forward);
	}
	else{
		digitalWrite(_dir2, _motor1Forward);
	}

	analogWrite(_e1, leftRate);
	analogWrite(_e2, rightRate);
}

void Drive::driveForward(int speed){
	drive(speed, speed);
}

void Drive::driveBackward(int speed){
	drive(-speed, -speed);
}

void Drive::rotateLeft(int speed){
	drive(-speed, speed);
}

void Drive::rotateRight(int speed){
	drive(speed, -speed);
}

void Drive::stop(){
	drive(0,0);
}

void Drive::writeDir(int dir1, int dir2){
	digitalWrite(_dir1, dir1);
	digitalWrite(_dir2, dir2);
}

void Drive::writeSpeed(int speed1, int speed2){
	analogWrite(_e1, speed1);
	analogWrite(_e2, speed2);
}