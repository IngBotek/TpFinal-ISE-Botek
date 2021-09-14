//=====[Libraries]=============================================================

#include "mbed.h"

#include "battery_measure.h"
#include "fire_alarm.h"
#include "smart_home_system.h"

//=====[Declaration of private defines]======================================

#define     R1      (26800.0f)   // target 27K Ohm
#define     R2      (100000.0f)  // target 100K Ohm
#define     VCC_MAX (4.4f)       // VCC maxima que entrega la bateria 18650
#define     BATTERY_NUMBER_OF_SAMPLES    10 // Cantidad de mediciones para promediar

//=====[Declaration of private data types]=====================================

//=====[Declaration and initialization of public global objects]===============

AnalogIn    v_input(A3);         // Pin de medicion A3
AnalogIn    vref(ADC_VREF);      // Tension de referencia

//=====[Declaration of external public global variables]=======================


//=====[Declaration of external public global variables]=======================

//=====[Declaration and initialization of public global variables]=============

//=====[Declaration and initialization of private global variables]============

float batteryM = 0.0;
float batteryReadingsArray[BATTERY_NUMBER_OF_SAMPLES];

//=====[Implementations of public functions]===================================

void batteryMeasureInit()  //Inicializa el arreglo con 0
{
    int i;
    
    for( i=0; i<BATTERY_NUMBER_OF_SAMPLES ; i++ ) {
        batteryReadingsArray[i] = 0;
    }
}

void batteryMeasureUpdate()
{
    static int batterySampleIndex = 0;
    float batteryReadingsSum = 0.0;
    float batteryReadingsAverage = 0.0;
    double v_in, vdd;
    double v_in_actual;

    int i = 0;
    
    vdd  = (1.224f) / vref.read();
    v_in = v_input.read();  
    v_in_actual = v_in * vdd * ( R1 + R2) / R2;
    v_in_actual = (v_in_actual / VCC_MAX);
    
    batteryReadingsArray[batterySampleIndex] = v_in_actual*100.0f;
    batterySampleIndex++;
    if ( batterySampleIndex >= BATTERY_NUMBER_OF_SAMPLES) {
        batterySampleIndex = 0;
    }
    
   batteryReadingsSum = 0.0;
   for (i = 0; i < BATTERY_NUMBER_OF_SAMPLES; i++) {
        batteryReadingsSum = batteryReadingsSum + batteryReadingsArray[i];
   }
   batteryReadingsAverage = batteryReadingsSum / BATTERY_NUMBER_OF_SAMPLES;
   batteryM = batteryReadingsAverage;   
}


float batteryMeasureInPercent()
{
    return batteryM;
}