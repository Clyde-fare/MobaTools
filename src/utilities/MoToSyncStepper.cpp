// MoToSyncStepper.cpp
//
// Syncronized stepper move with acceleration and deceleration
// 
// This software is part of the MobaTools library

//#define debugPrint
#include <Arduino.h>
#include <MobaTools.h>

MoToSyncStepper::MoToSyncStepper()
    : _num_steppers(0), _maxSpeed(10000)
{
}

boolean MoToSyncStepper::addStepper(MoToStepper& stepper)
{
    if (_num_steppers >= MAX_STEPPER)
	return false; // No room for more
	// Add stepper to sync-chain
    //TODO
    return true;
}

void MoToSyncStepper::setMaxSpeedSteps( uintxx_t speed10 ) {
	_maxSpeed = speed10;
}

void MoToSyncStepper::setTargets( long *absTarget ) {
	// first find the stepper that has to move the longest distance
    _targets = absTarget;
	long maxDistance = 0;
    uint8_t i;
	/*
    for (i = 0; i < _num_steppers; i++) {
		long thisDistance = abs(  absTarget[i] - _stepperSyncData[i].syncStepper->currentPosition() );
		if ( maxDistance <= thisDistance ) {
			// new max
			maxDistance = thisDistance;
		}
       DB_PRINT("setTar: i=%d, maxD=%ld, thisD=%ld", i,maxDistance,thisDistance);
	}
	// set Speed for each stepper with no ramp
    for (i = 0; i < _num_steppers; i++) {
		long thisDistance = abs( absTarget[i] - _steppers[i]->currentPosition() );
		_steppers[i]->setSpeedSteps( (_maxSpeed * thisDistance) / maxDistance , 0 ); 
	}
	*/
}	
	
void MoToSyncStepper::moveTo(long absTarget[])
{
    // First set all stepperspeeds
	setTargets( absTarget );
	/*
	// now start all steppers
    for ( uint8_t i = 0; i < _num_steppers; i++) {
        DB_PRINT("stepper %d moving from %ld to %ld", i,_steppers[i]->currentPosition()  , absTarget[i] );
		_steppers[i]->moveTo( absTarget[i] );
    }
	*/
}

bool MoToSyncStepper::moving() {
	int allMoving=0;
	/*
    for ( uint8_t i = 0; i < _num_steppers; i++) {
		allMoving += _stepperSyncData[i].syncStepper->moving();
    }
	*/
	return ( allMoving > 0 );
	
}

void MoToSyncStepper::startSyncMove() {
	// start the move and wait until it is finished
	// now start all steppers
	/*
    for ( uint8_t i = 0; i < _num_steppers; i++) {
		_steppers[i]->moveTo( _targets[i] );
    }
	// and wait ...
	while ( moving() );
	*/
}	
