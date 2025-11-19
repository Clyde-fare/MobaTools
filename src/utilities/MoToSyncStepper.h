/* MoToSyncStepper V3.0
** class to syncronise the movement of up to MAXSTEPPER steppers with acceleration and deceleration
** The stepper with the longest distance to move is the master. This movement is controlled in the
** ISR like a singel stepper. All other steppers that are closer to the target (slaves) 
** are controlled by the master, which sets the time for the next step of the slave steppers.
** During the synced movement no changes to the stepper objects are allowed.
**
*/

#pragma once

// stepperSyncData_t is declared in MoToStepper.h
class MoToSyncStepper
{
public:
    MoToSyncStepper();

    boolean addStepper(MoToStepper& stepper);  		// Add a stepper to group of synced steppers
	void setMaxSpeedSteps( uintxx_t speed10, uintxx_t rampLen = 0 );		// fastet speed of the motors ( in steps/10sec )
    bool moveTo(long absolute[]);  					// define the target positions of all steppers in group and 
													// start the move
    bool move(long stepsToDo[]);  					// define the target positions of all steppers in group and 
													//start the move
	bool moving();				    				// true if any of the steppers in the group is moving.
	void stop(bool emergency = true );				// emergency stop af all steppers ( no ramping )
													// if emergency == false it will ramp down
	bool syncMoveActive();							// returns true if a syncronous move is aktive
     
private:
	void _setStepData(long absolute[], bool absValues = true ); 	// set stepsToDo in stepperSyncData_t
	bool _startMove();

    // all steppers that will run in sync are connected via a circular pointerchain.
    stepperSyncData_t *_stepperChainP;		// pointer chain of steppers in sync
	stepperSyncData_t *_masterSyncDataP;	// Sync Data of masterstepper ( set by 'setTargets' )

    uint8_t		_numSteppers;				// Number of steppers we are controlling
	uintxx_t 	_maxSpeed;					// speed of the stepper with longest distance
	uintxx_t 	_rampLen;					// ramp length of the stepper with longest distance

	void printStepperChain();				// only for debugging

};
