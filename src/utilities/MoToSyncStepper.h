/* MoToSyncStepper V3.0
** class to syncronise the movement of up to MAXSTEPPER steppers with acceleration and deceleration
** The stepper with the longest distance to move is the master. This movement is controlled in the
** ISR like a singel stepper. All other steppers that are closer to the target (slaves) 
** are controlled by the master, which sets the time for the next step of the slave steppers.
** During the synced movement no changes to the stepper objects are allowed.
**
*/

#pragma once


class MoToSyncStepper
{
public:
    MoToSyncStepper();

    boolean addStepper(MoToStepper& stepper);  	// Add a stepper to group of synced steppers
	void setMaxSpeedSteps( uintxx_t speed10 );		// fastet speed of the motors ( in steps/10sec )
    void moveTo(long absolute[]);  				// define the target positions of all steppers
												// in group and start the move
												
	void setTargets(long absolute[]); 			// define targets (and set stepper speeds) , 
												// but don't start movement
	void startSyncMove();			// start the move and wait until it is finished
	
	bool moving();				    // true until all steppers reached their targeet.
     
private:
    // all steppers that will run in sync are connected via a circular pointerchain.
	// 
    //stepperSyncData_t _stepperSyncData[MAX_STEPPER];
    stepperSyncData_t *stepperChain;	// pointer chain of steppers in sync

    // Number of steppers we are controlling and the number
    // of steppers in _steppers[]
    uint8_t		_num_steppers;
	uintxx_t 	_maxSpeed;		// speed of the stepper with longest distance
	uintxx_t 	_rampLen;		// ramp length of the stepper with longest distance
	long *_targets;      // pointer to the targets			
};
