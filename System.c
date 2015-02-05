/*
 * System.c
 *
 * Created: 12/7/2014 3:22:20 AM
 *  Author: Disgust
 */ 

#include "System.h"


void prepareSystem()
{
    WDTCSR|= (1 << WDE)|(1 << WDIE);
    //WDTCSR|=(0b111 << WDP0);
	initOsc();
    setPowerReduction();
}

 void setPowerReduction() {
     PRR|= (1 << PRTIM0)|(1 << PRTIM2)|(1 << PRSPI);
 }
 
 void prepareCompass() {
	 
 }

 void prepareSonar() {
	 sonarInit(USE_INTERRUPT);
 }

 void prepareRF() {
	 
 }

 void prepareBarometer() {
	 BMP085Calibration();
 }
 
 void initOsc() {
 	
}
 