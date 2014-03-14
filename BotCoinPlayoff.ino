// BotCoinPlayoff.ino

#include "Timer.h"
#include "Drive.h"
#include "NewPing.h"
#include "Servo.h"


//STATES
#define SCANNING_FIRST_BEACON 1
#define SPINNING_FIRST_BEACON 2
#define HIT_FIRST_VINYL 3
#define SCANNING_SECOND_BEACON 4
#define SPINNING_SECOND_BEACON 5
#define HIT_SECOND_VINYL 6
#define SCANNING_SERVER 7
#define SPINNING_SERVER 8
#define HIT_SERVER 9
#define MINING 10
#define SCANNING_EXCHANGE_LEFT 11
#define SPINNING_EXCHANGE_LEFT 12
#define SCANNING_EXCHANGE_MIDDLE 13
#define SPINNING_EXCHANGE_MIDDLE 14
#define SCANNING_EXCHANGE_RIGHT 15
#define SPINNING_EXCHANGE_RIGHT 16
#define HIT_EXCHANGE_VINYL 17
#define DUMP_COINS 18
#define DEAD 19
#define HIT_RIGHT_WALL 20
#define AT_MIDDLE_GOING_TO_SERVER 21
#define AT_RIGHT_GOING_TO_SERVER 22
#define HIT_LEFT_WALL 23
#define AT_LEFT_GOING_TO_SERVER 24

//PINS
#define LEFT A5
#define MIDDLE A6
#define RIGHT A7

#define MIDDLE_BACK A10

//BEACON READ VALUES
#define BEACON_CENTER 0
#define BEACON_LEFT 1
#define BEACON_RIGHT 2
#define NO_BEACON 3
#define AVG_MINIMUM 140

#define VINYL A0
#define MIN_DIST 17

#define NUM_TURNS 5
#define LEFT_SIDE 0
#define RIGHT_SIDE 1

#define BASE_SPEED 200

#define SCORING_MIDDLE 1
#define SCORING_LEFT 2
#define SCORING_RIGHT 3

int currentTarget = SCORING_RIGHT;

bool scoredOnce = false;

bool middlScored = false;
bool leftScored = false;
bool rightScored = false;

Timer spinTimer(250);
Timer scanTimer(450);
Timer arcTimer(350);
Timer firstBeaconTimer(1500);
Timer missedFirstBeaconTimer(2000);
Timer ignoreVinylTimer(400);
Timer shortIgnoreVinyl(200);
Timer longScanTimer(500);
Timer foundServerTimer(5000);
Timer rightBeaconTimer(2000);
Timer endOfGameTimer(120000);

NewPing sonar(5, 6, 100);

Drive drive(210, 13, 12, 11, 3);
//Actuator miner(9);

Servo miner;
Servo rightMiner;
Servo dumper;

int state = SCANNING_FIRST_BEACON;
int fieldSide = -1;
int fieldSideCounter = 0;
int approxVinyl = -1;
int buttonPushedCount = 0;

int hitCounter = 0;

bool initialExchangeSeek = false;
bool hitWall = false;

void setup() {
	//Serial.begin(9600);

	miner.attach(9);
	rightMiner.attach(2);
	pullBackMiners();

	dumper.attach(7);
	dumper.write(120);

	drive.init(LOW, HIGH);
	scanTimer.start();
	pinMode(A0, INPUT);
	pinMode(LEFT, INPUT);
	pinMode(MIDDLE, INPUT);
	pinMode(RIGHT, INPUT);
	approxVinyl = analogRead(VINYL) - 200;
	//drive.offset(1);
	endOfGameTimer.start();
}


void loop() {

	int avg = 0;
	int pos = NO_BEACON;
	int wallDist = 0;

	if (endOfGameTimer.isExpired()) {
		state = DEAD;
	}

	switch (state){


		case SCANNING_FIRST_BEACON:
			//drive.stop();
			
			avg = getAverageBeaconValue();
			wallDist = sonar.ping_in();
			if (avg > 70 && wallDist > 0 && wallDist < 30){
				state = HIT_FIRST_VINYL;
				ignoreVinylTimer.start();
				firstBeaconTimer.start();
				break;
			}
			if (scanTimer.isExpired()){
				state = SPINNING_FIRST_BEACON;
				spinTimer.start();
				drive.rotateRight(8);
				break;
			}

		break;

		case SPINNING_FIRST_BEACON:
			//rotateRight(8);
			if (spinTimer.isExpired()){
				state = SCANNING_FIRST_BEACON;
				scanTimer.start();
				drive.stop();
				break;
			}

		break;

		case HIT_FIRST_VINYL:

			if (firstBeaconTimer.isExpired()) {
				stopAndReverse(700);
				numbSpin(9, 400);
				state = SPINNING_SECOND_BEACON;
				drive.rotateRight(10); //NOTE FASTER SPEED FOR THIS INITIAL SPIN. SHOULD REMOVE ONE STATE
				spinTimer.start();
				break;
			}

			pos = getBeaconPosition();
			if (ignoreVinylTimer.isExpired() && analogRead(VINYL) < approxVinyl) {
				state = SPINNING_SECOND_BEACON;
				stopAndReverse(700); //this needs to be adjusted
				numbSpin(9, 500);
				drive.rotateRight(10); //NOTE FASTER SPEED FOR THIS INITIAL SPIN. SHOULD REMOVE ONE STATE
				spinTimer.start();
				break;
			} else if (pos == BEACON_CENTER){
				drive.driveForward(10);
			}
			else if (pos == BEACON_LEFT){
				drive.drive(9, 10);
			}
			else if (pos == BEACON_RIGHT){
				drive.drive(10, 9);
			}
			else if (pos == NO_BEACON) {
				if (!firstBeaconTimer.isExpired()) {
					drive. driveForward(9);
					break;
				}
				stopAndReverse(700);
				numbSpin(9, 500); //this  might need to be lowered for new batteries
				state = SPINNING_SECOND_BEACON;
				drive.rotateRight(10); //NOTE FASTER SPEED FOR THIS INITIAL SPIN. SHOULD REMOVE ONE STATE
				spinTimer.start();
				break;
			}

		break;

		case SPINNING_SECOND_BEACON:
			if (spinTimer.isExpired()){
				state = SCANNING_SECOND_BEACON;
				scanTimer.start();
				drive.stop();
				fieldSideCounter++;
				break;
			}

		break;

		case SCANNING_SECOND_BEACON:
			avg = getAverageBeaconValue();
			wallDist = sonar.ping_in();
			if (avg > 60 && avg < 200 && wallDist == 0){ //need to change case
				state = HIT_SECOND_VINYL;
				ignoreVinylTimer.start();
				//state = DEAD;
				//break;
				if (fieldSideCounter > NUM_TURNS){
					fieldSide = LEFT_SIDE;
					drive.drive(6,7);
				}
				else {
					fieldSide = RIGHT_SIDE;
					drive.drive(7,6);
				}
				arcTimer.start();

				break;
			}
			if (scanTimer.isExpired()){
				state = SPINNING_SECOND_BEACON;
				spinTimer.start();
				drive.rotateRight(8);
			}

		break;

		case HIT_SECOND_VINYL:
			//WAIT UNTIL INITIAL ARC COMPLETE
			if (!arcTimer.isExpired()){
				break;
			}
			//IF WE HIT THE VINYL, NEXT STATE
			if (ignoreVinylTimer.isExpired() && analogRead(VINYL) < approxVinyl){
				
				state = SPINNING_SERVER;
				if (fieldSide == LEFT_SIDE){
					drive.rotateLeft(8);
				}
				if (fieldSide == RIGHT_SIDE){
					drive.rotateRight(8);
				}
				spinTimer.start();
				break;
			}
			//NO VINYL? KEEP GOING, JUST DON'T HIT THE WALL
			wallDist = sonar.ping_in();
			if ( (fieldSide == RIGHT_SIDE && wallDist > 0 && wallDist < MIN_DIST + 2) || (wallDist < MIN_DIST && wallDist > 0)) {
				if (fieldSide == LEFT_SIDE){
					//drive.rotateRight(10);
					drive.stop();
					delay(500);
					drive.rotateRight(9);
					delay(550); //this number was originally 450
					drive.stop();
					delay(500);
				}
				if (fieldSide == RIGHT_SIDE){
					drive.rotateLeft(7);
				}
				break;
			}
			drive.driveForward(8);

		break;

		case SPINNING_SERVER:
			if (spinTimer.isExpired()){
				state = SCANNING_SERVER;
				drive.stop();
				scanTimer.start();
				break;
			}

		break;

		case SCANNING_SERVER:
			avg = getAverageBeaconValue();
			if (avg > AVG_MINIMUM){
				foundServerTimer.start();
				state = HIT_SERVER;
				drive.driveForward(8);
				break;
			}

			if (scanTimer.isExpired()){
				state = SPINNING_SERVER;
				if (fieldSide == LEFT_SIDE){
					drive.rotateLeft(7);
				}
				else {
					drive.rotateRight(7);
				}
				spinTimer.start();
				break;
			}

		break;

		case HIT_SERVER:
			if (getAverageBeaconValue() > AVG_MINIMUM && sonar.convert_in(sonar.ping_median(7)) <= 2 && sonar.ping_in() > 0){
				state = MINING;
				break;
			} else if (foundServerTimer.isExpired()) {
				state = MINING;
				break;
			}

		break;

		case MINING:
			if ((!scoredOnce && buttonPushedCount > 18) || buttonPushedCount > 40) { //40 for 8 coins
				stopAndReverse(1300);
				pullBackMiners();
				buttonPushedCount = 0;
				if (currentTarget == SCORING_MIDDLE) state = SCANNING_EXCHANGE_MIDDLE;
				if (currentTarget == SCORING_LEFT) state = SCANNING_EXCHANGE_LEFT;
				if (currentTarget == SCORING_RIGHT) state = SCANNING_EXCHANGE_RIGHT;
				initialExchangeSeek = true;
				//numbSpin(8, 600); //direction and magnitude needs to be different for different exchanges
				if (currentTarget == SCORING_LEFT || currentTarget == SCORING_MIDDLE) numbSpin(8, 600); //left exchange
				else numbLeftSpin(8, 600);
				
				break;
			}
			drive.stop();
			mineCoin();
			//extendTimer.start();

		break;

		case SCANNING_EXCHANGE_MIDDLE:
			avg = getAverageBeaconValue();
			wallDist = sonar.ping_in();
			if (initialExchangeSeek && avg > 250 && avg < 500  && wallDist > 0 && wallDist < 23){ //middle beacon
				initialExchangeSeek = false;
				state = HIT_EXCHANGE_VINYL; //MOVE TO EXCHANGE SET
				shortIgnoreVinyl.start();
				missedFirstBeaconTimer.start();
				break;
			} else if (avg > 200 && avg < 500 && wallDist > 0) {
				state = HIT_EXCHANGE_VINYL; //MOVE TO EXCHANGE SET
				shortIgnoreVinyl.start();
				missedFirstBeaconTimer.start();
				break;
			}
			if (longScanTimer.isExpired()){
				state = SPINNING_EXCHANGE_MIDDLE;
				spinTimer.start();
				drive.rotateRight(8);
				break;
			}
		break;

		case SPINNING_EXCHANGE_MIDDLE:
			if (spinTimer.isExpired()){
				state = SCANNING_EXCHANGE_MIDDLE;
				longScanTimer.start();
				drive.stop();
				break;
			}

		break;

		case SCANNING_EXCHANGE_LEFT:
			avg = getAverageBeaconValue();
			wallDist = sonar.ping_in();
			if (avg > 60 && avg < 500  && wallDist > 15 && wallDist < 32){ //left beacon
				numbLeftSpin(8, 300);
				state = HIT_LEFT_WALL; //MOVE TO EXCHANGE SET
				//shortIgnoreVinyl.start();
				rightBeaconTimer.start();
				drive.driveForward(8);
				break;
			}
			if (hitWall){
				if (avg > 60 && avg < 500){
					state = HIT_EXCHANGE_VINYL;
					shortIgnoreVinyl.start();
					missedFirstBeaconTimer.start();
					break;
				}
			}
			if (longScanTimer.isExpired()){
				state = SPINNING_EXCHANGE_LEFT;
				spinTimer.start();
				drive.rotateRight(8);
				break;
			}
		break;

		case SPINNING_EXCHANGE_LEFT:
			if (spinTimer.isExpired()){
				state = SCANNING_EXCHANGE_LEFT;
				longScanTimer.start();
				drive.stop();
				break;
			}

		break;

		case SCANNING_EXCHANGE_RIGHT:
			avg = getAverageBeaconValue();
			wallDist = sonar.ping_in();
			if (avg > 70 && avg < 500  && wallDist > 15 && wallDist < 32){ //right beacon, weaker signal
				numbSpin(8, 400);
				state = HIT_RIGHT_WALL;//HIT_EXCHANGE_VINYL; //MOVE TO EXCHANGE SET
				drive.driveForward(8);
				rightBeaconTimer.start();
				//shortIgnoreVinyl.start();
				//missedFirstBeaconTimer.start();
				break;
			}
			if (hitWall){
				if (avg > 70 && avg < 500){
					state = HIT_EXCHANGE_VINYL;
					shortIgnoreVinyl.start();
					missedFirstBeaconTimer.start();
					break;
				}
			}
			if (longScanTimer.isExpired()){
				state = SPINNING_EXCHANGE_RIGHT;
				spinTimer.start();
				drive.rotateLeft(8);
				break;
			}
		break;

		case SPINNING_EXCHANGE_RIGHT:
			if (spinTimer.isExpired()){
				state = SCANNING_EXCHANGE_RIGHT;
				longScanTimer.start();
				drive.stop();
				break;
			}

		break;

		case HIT_RIGHT_WALL:
			wallDist = sonar.convert_in(sonar.ping_median(7));
			if (rightBeaconTimer.isExpired() || (wallDist <= 20 && wallDist > 0)) {
				hitWall = true;
				state = SCANNING_EXCHANGE_RIGHT;
				longScanTimer.start();
				drive.stop();
			}
		break;

		case HIT_LEFT_WALL:
			wallDist = sonar.convert_in(sonar.ping_median(7));
			if (rightBeaconTimer.isExpired() || (wallDist <= 22 && wallDist > 0)) {
				hitWall = true;
				state = SCANNING_EXCHANGE_LEFT;
				longScanTimer.start();
				drive.stop();
			}
		break;

		case HIT_EXCHANGE_VINYL:
			pos = getBeaconPosition();
			wallDist = sonar.convert_in(sonar.ping_median(7));
			if (shortIgnoreVinyl.isExpired() && wallDist <= 3 && wallDist > 0) {

				state = DUMP_COINS;
				break;
			} else if (pos == BEACON_CENTER){
				missedFirstBeaconTimer.start();
				drive.driveForward(9);
			}
			else if (pos == BEACON_LEFT){
				missedFirstBeaconTimer.start();
				drive.driveForward(9);
			}
			else if (pos == BEACON_RIGHT){
				missedFirstBeaconTimer.start();
				drive.driveForward(9);
			}
			else if (pos == NO_BEACON) {
				if (!missedFirstBeaconTimer.isExpired()) {
					drive.driveForward(9);
					break;
				}
				stopAndReverse(400);
				if (currentTarget == SCORING_MIDDLE) state = SCANNING_EXCHANGE_MIDDLE;
				if (currentTarget == SCORING_LEFT) state = SCANNING_EXCHANGE_LEFT;
				if (currentTarget == SCORING_RIGHT) state = SCANNING_EXCHANGE_RIGHT;
				drive.rotateRight(9); //NOTE FASTER SPEED FOR THIS INITIAL SPIN. SHOULD REMOVE ONE STATE
				spinTimer.start();
				break;
			}
		break;

		case DUMP_COINS:
			dumpCoins();
			dumpCoins();
			//dumpCoins();
			if (currentTarget == SCORING_MIDDLE) state = AT_MIDDLE_GOING_TO_SERVER;
			if (currentTarget == SCORING_RIGHT) state = AT_RIGHT_GOING_TO_SERVER;
			if (currentTarget == SCORING_LEFT) state = AT_LEFT_GOING_TO_SERVER;


		break;

		case AT_MIDDLE_GOING_TO_SERVER:
			stopAndReverse(700);
			numbSpin(8, 600);
			drive.driveForward(9);
			delay(1100);
			drive.stop();
			state = SCANNING_SECOND_BEACON;
			middlScored = true;
			currentTarget = SCORING_RIGHT;

		break;

		case AT_RIGHT_GOING_TO_SERVER:
			stopAndReverse(700);
			numbSpin(8, 600);
			drive.stop();
			state = SCANNING_SECOND_BEACON;
			middlScored = true;
			currentTarget = SCORING_LEFT;

		break;

		case AT_LEFT_GOING_TO_SERVER:
			stopAndReverse(700);
			numbLeftSpin(8, 600);
			drive.stop();
			state = SCANNING_SECOND_BEACON;
			middlScored = true;
			currentTarget = SCORING_MIDDLE;
		break;

		case DEAD:
			drive.stop();

		break;
	}
}

void pullBackMiners() {
	miner.write(180);
	rightMiner.write(17);
}

void dumpCoins() {
	dumper.write(120); //STARTING POSITION OF DUMPER MECH
	delay(500);
	dumper.write(160);
	delay(1000);
	dumper.write(120);
	scoredOnce = true;
	hitWall= false; 
}

void mineCoin(){
	miner.write(160);
	rightMiner.write(37);
	delay(300);
	miner.write(107);
	rightMiner.write(80);
	delay(300);
	buttonPushedCount++;
}

void stopAndReverse(int time) {
	drive.stop();
	delay(500);
	drive.driveBackward(9);
	delay(time);
	drive.stop();
	delay(500);
}

void numbSpin(int speed, int time){
	drive.stop();
	delay(500);
	drive.rotateRight(speed);
	delay(time);
	drive.stop();
	delay(500);
}

void numbLeftSpin(int speed, int time){
	drive.stop();
	delay(500);
	drive.rotateLeft(speed);
	delay(time);
	drive.stop();
	delay(500);
}

int getBeaconPosition() {
	int leftRead = analogRead(LEFT);
	int middleRead = analogRead(MIDDLE);
	int rightRead = analogRead(RIGHT); 

	unsigned int avg = leftRead + rightRead + middleRead;
	avg = avg/3;
 
	if ((leftRead > 400 && middleRead > 400 && rightRead > 400) || (abs(leftRead - rightRead) < 50 && leftRead > 100)) {
		return BEACON_CENTER;
	} else if (middleRead > 200 && middleRead > leftRead) {
		return BEACON_RIGHT;
	} else if (middleRead > 200 && middleRead > rightRead) {
		return BEACON_LEFT;
	} else {
  		return NO_BEACON;
 	}
}

int getAverageBeaconValue() {
	int leftRead = analogRead(LEFT);
	int middleRead = analogRead(MIDDLE);
	int rightRead = analogRead(RIGHT); 

	unsigned int avg = leftRead + rightRead + middleRead;
	avg = avg/3;

	//Serial.print("Average Beacon Value: ");
	//Serial.println(avg);

	return avg;
}