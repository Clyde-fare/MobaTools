// MoToSyncStepper.cpp
//
// Syncronized stepper move with acceleration and deceleration
// 
// This software is part of the MobaTools library

#define debugPrint
#include <Arduino.h>
#include <MobaTools.h>

MoToSyncStepper::MoToSyncStepper()
    : _numSteppers(0), _maxSpeed(10000)
{
}

boolean MoToSyncStepper::addStepper(MoToStepper& stepper)
{
    //if (_num_steppers >= MAX_STEPPER) 	return false; // No room for more
	// Add stepper to sync-chain
	// create new struct for new stepper at heap
	stepperSyncData_t *tempP = new stepperSyncData_t( );
	if ( _stepperChain == NULL ) {
		// this is the first stepper
		_stepperChain = tempP;
	} else {
		// search end of chain (points to first )
		stepperSyncData_t *lastEntryP = _stepperChain;
		while( lastEntryP->nextSyncData != _stepperChain ) lastEntryP = lastEntryP->nextSyncData;
		lastEntryP->nextSyncData = tempP;
	}
	tempP->nextSyncData = _stepperChain;
	tempP->syncStepper = &stepper;
	tempP->stepperDataP = &stepper._stepperData; // MoToSyncStepper must be friend class of MoToStepper!!
	_numSteppers++;
    //
    return true;
}

void MoToSyncStepper::setMaxSpeedSteps( uintxx_t speed10, uintxx_t rampLen ) {
	_maxSpeed = speed10;
	_rampLen = rampLen;
}

void MoToSyncStepper::setTargets( long *absTarget, bool absValues  ) {
	// first find the stepper that has to move the longest distance
    _targets = absTarget;
	long maxDistance = 0;
	stepperSyncData_t *tempP = _stepperChain;		// start of chain -> pointer to first stepper
    uint8_t i;
	DB_PRINT("finding longest distance...");
    for (i = 0; i < _numSteppers; i++) {
		long thisDistance =   absTarget[i] - tempP->syncStepper->currentPosition() ;
		tempP->stepsToMove = thisDistance;
		if ( maxDistance <= abs(thisDistance) ) {
			// new max
			maxDistance = abs(thisDistance);
			_masterDataP = tempP;
		}
		tempP = tempP->nextSyncData;	// next stepper
       DB_PRINT("setTar: i=%d, maxD=%ld, thisD=%ld", i,maxDistance,thisDistance);
	}
	// set distance ratio to master
	tempP = _stepperChain;		// start of chain -> pointer to first stepper
	//DB_PRINT("compute ratio to master");
	bool masterFound = false;	// there can only be one master
	do {
		// steptime ratio master -> slave
		if ( tempP->stepsToMove != 0 ) {
			// stepper must move
			tempP->ratioToMaster = (maxDistance * RATIOBASE) / abs(tempP->stepsToMove)  ;
			//DB_PRINT( "Ratio=%ld", tempP->ratioToMaster );
			if ( tempP->ratioToMaster == RATIOBASE && !masterFound) {
				tempP->ratioToMaster = 0; // This is the master	
				tempP->ratioCnt = 0; // This is the master	
				masterFound = true;
			} else {
				tempP->ratioCnt = tempP->ratioToMaster / 2;
			}
		} else {
			tempP->ratioToMaster = ULONG_MAX;
			tempP->ratioCnt = ULONG_MAX;
		}
		tempP = tempP->nextSyncData;
	} while ( tempP != _stepperChain ); // Until first stepper reached again.
	
	#ifdef debugPrint
		// Print syncdata of all steppers
		i=0;
		tempP = _stepperChain;		// start of chain -> pointer to first stepper
		DB_PRINT("Syncdata of steppers:");
		do {
			DB_PRINT("N0:%d, stepstoMove=%ld, ratio=%lu, ratioCnt=%lu", i,tempP->stepsToMove,tempP->ratioToMaster,tempP->ratioCnt); 
			tempP = tempP->nextSyncData;
			i++;
		} while ( tempP != _stepperChain ); // Until first stepper reached again.
	
	#endif
}	
	
uint8_t MoToSyncStepper::moveTo(long absTarget[]) {
	//TODO: Prüfen, ob ein sync move möglich ist ( Keiner der betroffenen Stepper darf in Bewegung sein )
	stepperSyncData_t *tempP = _stepperChain;		// start of chain -> pointer to first stepper

    // First set all  syncData
	setTargets( absTarget );
	// now start all steppers
	tempP = _stepperChain;		// start of chain -> pointer to first stepper
	DB_PRINT("Starting steppers");
	do {
		//DB_PRINT("N0:%d, stepstoMove=%ld, ratio=%lu, ratioCnt=%lu", i,tempP->stepsToMove,tempP->ratioToMaster,tempP->ratioCnt); 
		tempP = tempP->nextSyncData;
		if ( tempP->stepsToMove != 0 ) {
			// only steppers that really need to move
			// if it is the master stepper set speed and ramp
			if (tempP->ratioToMaster == 0 ) {
				tempP->syncStepper->setSpeedSteps( _maxSpeed, _rampLen );
			}
			tempP->syncStepper->_stepperData.syncDataP = tempP;	// set pointer für sync data for use in ISR
			tempP->syncStepper->move(tempP->stepsToMove);
		}
	} while ( tempP != _stepperChain ); // Until first stepper reached again.
	
	return true;
	
}

bool MoToSyncStepper::moving() {
	int allMoving=0;
	stepperSyncData_t *tempP = _stepperChain;		// start of chain -> pointer to first stepper
	do {
		allMoving += tempP->syncStepper->moving();	
		tempP = tempP->nextSyncData;
	} while ( tempP != _stepperChain ); // Until first stepper reached again.

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
