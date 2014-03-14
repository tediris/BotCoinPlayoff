#ifndef Timer_h
#define Timer_h

#include "Arduino.h"

class Timer{
	public:
		Timer(unsigned long ms);
		void start();
		void stop();
		bool isExpired();
		void set(unsigned long newMS);
	private:
		unsigned long _msec;
		unsigned long _startTime;
		bool _running;
};

#endif