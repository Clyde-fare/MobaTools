// MoToSyncStepper.cpp
//
// Syncronized stepper move with acceleration and deceleration
// 
// This software is part of the MobaTools library

#define debugPrint
#include <Arduino.h>
#include <MobaTools.h>

void printStepperChain();

MoToSyncStepper::MoToSyncStepper()
    : _numSteppers(0), _maxSpeed(5000), _rampLen(0)
{
}

boolean MoToSyncStepper::addStepper(MoToStepper& stepper)
{
    if (_numSteppers >= MAX_STEPPER) 	return false; // No room for more
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
	// TODO if a sync move is in motion, this sets the master speed immediately
	_maxSpeed = speed10;
	_rampLen = rampLen;
}

void MoToSyncStepper::_setStepData( long *absTarget, bool absValues  ) {
	// first find the stepper that has to move the longest distance
    _targets = absTarget;
	long maxDistance = 0;
	_masterSyncDataP = NULL; // There maybe no master if all distances are 0
	stepperSyncData_t *tempP = _stepperChain;		// start of chain -> pointer to first stepper
    uint8_t i;
	long thisDistance;
	//bool masterFound = false;
	DB_PRINT("finding longest distance...");
    for (i = 0; i < _numSteppers; i++) {
		if ( absValues ) {
			thisDistance =   absTarget[i] - tempP->syncStepper->currentPosition() ;
		} else{
			thisDistance =   absTarget[i] ;
		}
		tempP->stepsToMove = thisDistance;
		if ( maxDistance < abs(thisDistance)  ) {
			// new max
			maxDistance = abs(thisDistance);
			_masterSyncDataP = tempP;
		}
		tempP = tempP->nextSyncData;	// next stepper
       DB_PRINT("setTar: i=%d, maxD=%ld, thisD=%ld", i,maxDistance,thisDistance);
	}
	// set distance ratio to master
	tempP = _stepperChain;		// start of chain -> pointer to first stepper
	//DB_PRINT("compute ratio to master");
	do {
		// steptime ratio master -> slave
		if ( tempP->stepsToMove != 0 ) {
			// stepper must move
			tempP->ratioToMaster = (maxDistance * RATIOBASE) / abs(tempP->stepsToMove)  ;
			//DB_PRINT( "Ratio=%ld", tempP->ratioToMaster );
			if ( tempP == _masterSyncDataP) {
				tempP->ratioToMaster = 0; // This is the master	
				tempP->ratioCnt = 0; // This is the master	
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

bool MoToSyncStepper::move(long stepsToDo[]) {
	//Prüfen, ob ein sync move möglich ist ( Keiner der betroffenen Stepper darf in Bewegung sein )
	// relative move
    // First set all  syncData
	_setStepData( stepsToDo, false  ); // values are relative
	return _startMove();
} 	
bool MoToSyncStepper::moveTo(long absTarget[]) {
	//Prüfen, ob ein sync move möglich ist ( Keiner der betroffenen Stepper darf in Bewegung sein )
	// absolute move
    // First set all  syncData
	_setStepData( absTarget, true );	// values are absolute
	return _startMove();
} 	
	
bool MoToSyncStepper::_startMove() {	
	if ( moving() ) return false;					// none of the steppers must move
	stepperSyncData_t *tempP = _stepperChain;		// start of chain -> pointer to first stepper
	stepperData_t		*masterStepperDataP;					// Pointer to data of actual master
	stepperData_t		*tmpStepperDataP;
	if ( _masterSyncDataP == NULL ) return false; // no master - no movement
	printStepperChain();
	masterStepperDataP = _masterSyncDataP->stepperDataP;
	// now remove masterstepper from within the chain
	// search master in chain
	if ( stepperRootP != masterStepperDataP ) {
		// Master steht noch nicht am Anfang - verschieben
		_noStepIRQ();
		tmpStepperDataP = stepperRootP;
		while ( tmpStepperDataP->nextStepperDataP != masterStepperDataP ) tmpStepperDataP = tmpStepperDataP->nextStepperDataP;
		tmpStepperDataP->nextStepperDataP = masterStepperDataP->nextStepperDataP;

		// sort the master at the beginning of the chain of stepperdata. This ensures, that in IRQ all slaves 
		// are worked upon after the master.
		// set master at the beginning of the stepper chain
		masterStepperDataP->nextStepperDataP = stepperRootP;
		stepperRootP = masterStepperDataP;
		_stepIRQ();
	}
	printStepperChain();
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
				tempP->syncStepper->_stepperData.syncMode = syncStat::MASTER;
			} else {
				// no master, only set slavemode
				tempP->syncStepper->_stepperData.syncMode = syncStat::SLAVE;
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
	
#ifdef debugPrint
	// Ausgeben der Stepper-Datenchain für den IRQ
	void MoToSyncStepper::printStepperChain() {
		char txtBuf[80];
		stepperData_t	*tempP = stepperRootP;
		DB_PRINT("Master: 0x%08x ",(uintxx_t) _masterSyncDataP->stepperDataP );
		sprintf (txtBuf, "root: %08x, ", (uintxx_t)tempP );  Serial.print(txtBuf);
		while ( tempP != NULL ) {
			tempP = tempP-> nextStepperDataP;
			sprintf (txtBuf, "next: %08x, ", (uintxx_t)tempP );  Serial.print(txtBuf);
		}
		Serial.println();
	}
		
#else
	void MoToSyncStepper::printStepperChain() { // dummy
	}	
#endif