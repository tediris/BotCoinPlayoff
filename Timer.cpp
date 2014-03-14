#include "Arduino.h"
#include "Timer.h"

Timer::Timer(unsigned long ms){
	_msec = ms;
	_running = false;
}

bool Timer::isExpired(){
	unsigned long curTime = millis();
	if (_startTime + _msec <= curTime){
		_running = false;
		return true;
	}
	return false;
}

void Timer::start(){
	_startTime = millis();
	_running = true;
}

void Timer::stop(){
	if (!_running || isExpired()) return;
	unsigned long curTime = millis();
	_msec = _msec - (curTime - _startTime);
	_running = false;
}

void Timer::set(unsigned long newMS){
	_msec = newMS;
}

