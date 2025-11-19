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
    : _numSteppers(0), _maxSpeed(5000), _rampLen(0), _stepperChainP(NULL) {
}

boolean MoToSyncStepper::addStepper(MoToStepper& stepper) {
    if (_numSteppers >= MAX_STEPPER) 	return false; // No room for more
	// Add stepper to sync-chain
	// create new struct for new stepper at heap
	stepperSyncData_t *tempP = new stepperSyncData_t( );
	if ( _stepperChainP == NULL ) {
		// this is the first stepper
		_stepperChainP = tempP;
	} else {
		// search end of chain (points to first )
		stepperSyncData_t *lastEntryP = _stepperChainP;
		while( lastEntryP->nextSyncData != _stepperChainP ) lastEntryP = lastEntryP->nextSyncData;
		lastEntryP->nextSyncData = tempP;
	}
	tempP->nextSyncData = _stepperChainP;
	tempP->syncStepper = &stepper;
	tempP->stepperDataP = &stepper._stepperData; // MoToSyncStepper must be friend class of MoToStepper!!
	_numSteppers++;
    //
    return true;
}

void MoToSyncStepper::setMaxSpeedSteps( uintxx_t speed10, uintxx_t rampLen ) {
	// TODO Check limits
	// TODO if a sync move is in motion, this sets the master speed immediately
	_maxSpeed = speed10;
	_rampLen = rampLen;
}

void MoToSyncStepper::_setStepData( long *absTarget, bool absValues  ) {
	// first find the stepper that has to move the longest distance
	long maxDistance = 0;
	_masterSyncDataP = NULL; // There maybe no master if all distances are 0
	stepperSyncData_t *tempP = _stepperChainP;		// start of chain -> pointer to first stepper
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
	tempP = _stepperChainP;		// start of chain -> pointer to first stepper
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
	} while ( tempP != _stepperChainP ); // Until first stepper reached again.
	
	#ifdef debugPrint
		// Print syncdata of all steppers
		i=0;
		tempP = _stepperChainP;		// start of chain -> pointer to first stepper
		DB_PRINT("Syncdata of steppers:");
		do {
			DB_PRINT("N0:%d, stepstoMove=%ld, ratio=%lu, ratioCnt=%lu", i,tempP->stepsToMove,tempP->ratioToMaster,tempP->ratioCnt); 
			tempP = tempP->nextSyncData;
			i++;
		} while ( tempP != _stepperChainP ); // Until first stepper reached again.
	
	#endif
}	

bool MoToSyncStepper::move(long stepsToDo[]) {
	if ( _stepperChainP == NULL ) return false; // there is no stepper yet
	//Prüfen, ob ein sync move möglich ist ( Keiner der betroffenen Stepper darf in Bewegung sein )
	// relative move
    // First set all  syncData
	_setStepData( stepsToDo, false  ); // values are relative
	return _startMove();
} 	
bool MoToSyncStepper::moveTo(long absTarget[]) {
	if ( _stepperChainP == NULL ) return false; // there is no stepper yet
	//Prüfen, ob ein sync move möglich ist ( Keiner der betroffenen Stepper darf in Bewegung sein )
	// absolute move
    // First set all  syncData
	_setStepData( absTarget, true );	// values are absolute
	return _startMove();
} 	
	
bool MoToSyncStepper::_startMove() {	
	if ( moving() ) return false;					// none of the steppers must move
	stepperSyncData_t *tempP = _stepperChainP;		// start of chain -> pointer to first stepper
	stepperData_t		*masterStepperDataP;		// Pointer to data of actual master
	stepperData_t		*tmpStepperDataP;
	if ( _masterSyncDataP == NULL ) return false; // no master - no movement
	
	printStepperChain(); // only debugging
	
	// sort the master at the beginning of the chain of stepperdata. This ensures, that in IRQ all slaves 
	// are worked upon after the master.
	masterStepperDataP = _masterSyncDataP->stepperDataP;
	// now remove masterstepper from within the chain
	// search master in chain
	if ( stepperRootP != masterStepperDataP ) {
		// Master not yet at the beginning, so move it
		_noStepIRQ();  // there must not happene a stepper IRQ while changing the stepper chain
		// search master in chain ...
		tmpStepperDataP = stepperRootP;
		while ( tmpStepperDataP->nextStepperDataP != masterStepperDataP ) tmpStepperDataP = tmpStepperDataP->nextStepperDataP;
		// ... and remove it there ...
		tmpStepperDataP->nextStepperDataP = masterStepperDataP->nextStepperDataP;
		// ... and move to the beginning of the stepper chain
		masterStepperDataP->nextStepperDataP = stepperRootP;
		stepperRootP = masterStepperDataP;
		_stepIRQ();
	}
	
	printStepperChain();	// only debugging
	// now start all steppers
	tempP = _stepperChainP;		// start of chain -> pointer to first stepper
	DB_PRINT("Starting steppers");
	do {
		//DB_PRINT("N0:%d, stepstoMove=%ld, ratio=%lu, ratioCnt=%lu", i,tempP->stepsToMove,tempP->ratioToMaster,tempP->ratioCnt); 
		tempP = tempP->nextSyncData;
		if ( tempP->stepsToMove != 0 ) {
			// only steppers that really need to move
			// if it is the master stepper set speed and ramp
			if (tempP->ratioToMaster == 0 ) {
				//tempP->syncStepper->setSpeedSteps( _maxSpeed, _rampLen );
				tempP->syncStepper->_stepperData.syncMode = syncStat::MASTER;
			} else {
				// no master, only set slavemode
				tempP->syncStepper->_stepperData.syncMode = syncStat::SLAVE;
			}
			tempP->syncStepper->_stepperData.syncDataP = tempP;	// set pointer für sync data for use in ISR
			// set speed of all steppers ( but only master will really move at that speed )
			// rampLen may be adjusted if its out of limits
			_rampLen = tempP->syncStepper->setSpeedSteps( _maxSpeed, _rampLen );
			tempP->syncStepper->move(tempP->stepsToMove);
		}
	} while ( tempP != _stepperChainP ); // Until first stepper reached again.
	
	return true;
	
}

bool MoToSyncStepper::moving() {
	if ( _stepperChainP == NULL ) return false; // there is no stepper yet
	int allMoving=0;
	stepperSyncData_t *tempP = _stepperChainP;		// start of chain -> pointer to first stepper
	do {
		allMoving += tempP->syncStepper->moving();	
		tempP = tempP->nextSyncData;
	} while ( tempP != _stepperChainP ); // Until first stepper reached again.

	return ( allMoving > 0 );
	
}

bool MoToSyncStepper::syncMoveActive() {
	bool syncMove = false;
	stepperSyncData_t *tempP = _stepperChainP;	// start of chain -> pointer to first stepper
	if ( _stepperChainP == NULL ) return false; // there is no stepper yet
	do {
		if ( tempP->syncStepper->_stepperData.syncDataP != NULL ) syncMove = true ;	
		tempP = tempP->nextSyncData;
	} while ( tempP != _stepperChainP ); // Until first stepper reached again.

	return syncMove;
}

void MoToSyncStepper::stop (bool emergency ) {
	if ( _stepperChainP == NULL ) return; 	// there is no stepper yet
	if ( !syncMoveActive() ) return ; 		// nothing to stop
	DB_PRINT("emergency stop %d" , emergency);
	//stop the move
	stepperSyncData_t *tempP = _stepperChainP;		// start of chain -> pointer to first stepper
	do {
		// set ratioCnt < RATIOBASE, so that master in any case creates a slave step
		if ( emergency || _rampLen == 0 ) { // emergency stop and stop without ramp is the same
			// set ratioCnt < RATIOBASE, so that master in any case creates the last slave step
			_noStepIRQ();
			if ( tempP->ratioCnt >= RATIOBASE ) tempP->ratioCnt = RATIOBASE-1;
			_stepIRQ();
			tempP->syncStepper->stop();	
		} else {
			// We want prematurely to ramp down, ramping down is controlled by master
			// set remaining steps
			if ( tempP->ratioToMaster == 0 ) {
				// its the master
				tempP->syncStepper->rotate(0);
			} else {
				// its a slave, set remaining steps
				// TODO funktioniert wohl nicht, wenn während der Beschleunigungsrampe abgebrochen wird,
				// da dann auch die Bremsrampe kürzer ist.
				_noStepIRQ();
				tempP->syncStepper->_stepperData.stepCnt = (_rampLen * RATIOBASE) / tempP->ratioToMaster;	
				//DB_PRINT("Rampe=%d, RATIO=%ld, ratioMaster=%ld, stepCnt=%ld",_rampLen, RATIOBASE, tempP->ratioToMaster,  tempP->syncStepper->_stepperData.stepCnt);
				//tempP->syncStepper->_stepperData.syncMode = syncStat::CANCELED;
				_stepIRQ();
			}
		}	
		tempP = tempP->nextSyncData;
	} while ( tempP != _stepperChainP ); // Until first stepper reached again.
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