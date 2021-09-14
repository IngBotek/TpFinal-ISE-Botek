//=====[Libraries]=============================================================

#include "mbed.h"
#include "arm_book_lib.h"

#include "fire_alarm.h"

#include "siren.h"
#include "strobe_light.h"
#include "user_interface.h"
#include "code.h"
#include "date_and_time.h"
#include "temperature_sensor.h"
#include "gas_sensor.h"
#include "matrix_keypad.h"
#include "pc_serial_com.h"

#include "battery_measure.h" //***********************************************

//=====[Declaration of private defines]======================================

#define TEMPERATURE_C_LIMIT_ALARM               50.0
#define STROBE_TIME_GAS               1000
#define STROBE_TIME_OVER_TEMP          500
#define STROBE_TIME_GAS_AND_OVER_TEMP  100

#define UMBRALPOWER						65       // % critico de bateria

//=====[Declaration of private data types]=====================================

//=====[Declaration and initialization of public global objects]===============

DigitalIn alarmTestButton(BUTTON1);

//=====[Declaration and initialization of private global variables]============

//=====[Declaration of external public global variables]=======================

//=====[Declaration and initialization of public global variables]=============

//=====[Declaration and initialization of private global variables]============

static bool gasDetected                  = OFF;
static bool overTemperatureDetected      = OFF;
static bool gasDetectorState             = OFF;
static bool overTemperatureDetectorState = OFF;
static bool lowPowerBatteryAlarm		 = OFF; //+++++++++++++++++++++++++++++
static bool lowPowerBatteryAlarmState	 = OFF; //+++++++++++++++++++++++++++++
int lowPowerCount						 = 1; //+++++++++++++++++++++++++++++++
int powerFlag							 = 1; //+++++++++++++++++++++++++++++++

//=====[Declarations (prototypes) of private functions]========================

static void fireAlarmActivationUpdate();
static void batteryReadingUpdate(); //+++++++++++++++++++++++++++++++++++++++++
static void fireAlarmDeactivationUpdate();
static void fireAlarmDeactivate();
static int fireAlarmStrobeTime();

//=====[Implementations of public functions]===================================

void fireAlarmInit()
{
    temperatureSensorInit();
    batteryMeasureInit(); //+++++++++++++++++++++++++++++++++++++++++++++++++++
    gasSensorInit();
    sirenInit();
    strobeLightInit();	
	
	alarmTestButton.mode(PullDown); 
}

void fireAlarmUpdate()
{
    fireAlarmActivationUpdate();
    batteryReadingUpdate(); //+++++++++++++++++++++++++++++++++++++++++++++++++
    fireAlarmDeactivationUpdate();
    sirenUpdate( fireAlarmStrobeTime() );
    strobeLightUpdate( fireAlarmStrobeTime() );	
}

bool gasDetectorStateRead()
{
    return gasDetectorState;
}

bool overTemperatureDetectorStateRead()
{
    return overTemperatureDetectorState;
}

bool gasDetectedRead()
{
    return gasDetected;
}

bool overTemperatureDetectedRead()
{
    return overTemperatureDetected;
}

bool lowPowerAlarmDetectorStateRead()
{
	return lowPowerBatteryAlarmState;
}

bool lowPowerAlarm()
{
	return lowPowerBatteryAlarm;
}

//=====[Implementations of private functions]==================================

static void fireAlarmActivationUpdate()
{
    temperatureSensorUpdate();
    gasSensorUpdate();
	batteryReadingUpdate();
    overTemperatureDetectorState = temperatureSensorReadCelsius() > 
                                   TEMPERATURE_C_LIMIT_ALARM;

    if ( overTemperatureDetectorState ) {
        overTemperatureDetected = ON;
        sirenStateWrite(ON);
        strobeLightStateWrite(ON);
    }

    gasDetectorState = !gasSensorRead();

    if ( gasDetectorState ) {
        gasDetected = ON;
        sirenStateWrite(ON);
        strobeLightStateWrite(ON);
    }
	
    if( alarmTestButton ) {             
        overTemperatureDetected = ON;
        gasDetected = ON;
        sirenStateWrite(ON);
        strobeLightStateWrite(ON);
    }
}

static void batteryReadingUpdate() //++++++++++++++++++++++++++++++++++++++++
{
	batteryMeasureUpdate();
	lowPowerBatteryAlarmState = batteryMeasureInPercent() < UMBRALPOWER;
	
	if ( lowPowerBatteryAlarmState ) {
        lowPowerBatteryAlarm  = ON;
        lowPowerCount = lowPowerCount+1;
        if ( lowPowerCount > 10 && powerFlag == 1) {
        	pcSerialComStringWrite("LOW POWER");
    		pcSerialComStringWrite("\r\n");
    		powerFlag = 0;
    	}
    }
}

static void fireAlarmDeactivationUpdate()
{
    if ( sirenStateRead() ) {
        if ( codeMatchFrom(CODE_KEYPAD) ||
             codeMatchFrom(CODE_PC_SERIAL) ) {
            fireAlarmDeactivate();
        }
    }
}

static void fireAlarmDeactivate()
{
    sirenStateWrite(OFF);
	strobeLightStateWrite(OFF);
    overTemperatureDetected = OFF;
    gasDetected             = OFF;   
}

static int fireAlarmStrobeTime()
{
    if( gasDetectedRead() && overTemperatureDetectedRead() ) {
        return STROBE_TIME_GAS_AND_OVER_TEMP;
    } else if ( gasDetectedRead() ) {
        return STROBE_TIME_GAS;
    } else if ( overTemperatureDetectedRead() ) {
        return STROBE_TIME_OVER_TEMP;
    } else {
        return 0;
    }
}
