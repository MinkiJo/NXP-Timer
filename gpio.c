/*
 * main implementation: use this 'C' sample to create your own application
 *
 */
#include "S32K144.h"
#include "device_registers.h"
#define PTD15 15
#define PTD16 16
#define PTD0 0

#define PTC12 12
#define PTC13 13

int main(void){


	//////////////////////////////////////Switch////////////////////////////////
PCC-> PCCn[PCC_PORTC_INDEX] |= PCC_PCCn_CGC_MASK;

PORTC->PCR[12] &= ~PORT_PCR_MUX_MASK;
PORTC->PCR[12] |= PORT_PCR_MUX(1);
PTC->PDDR &= ~(1<<PTC12);



PORTC->PCR[13] &= ~PORT_PCR_MUX_MASK;
PORTC->PCR[13] |= PORT_PCR_MUX(1);
PTC->PDDR &= ~(1<<PTC13);



//////////////////////////////////////LED////////////////////////////////

PCC-> PCCn[PCC_PORTD_INDEX] |= PCC_PCCn_CGC_MASK;

PORTD->PCR[0] &= ~PORT_PCR_MUX_MASK;
PORTD->PCR[0] |= PORT_PCR_MUX(1);

PTD->PDDR |= 1<<PTD0; /////Blue

PORTD->PCR[15] &= ~PORT_PCR_MUX_MASK;
PORTD->PCR[15] |= PORT_PCR_MUX(1);

PTD->PDDR |= 1<<PTD15; /////Red

PORTD->PCR[16] &= ~PORT_PCR_MUX_MASK;
PORTD->PCR[16] |= PORT_PCR_MUX(1);

PTD->PDDR |= 1<<PTD16;  /////Green


for(;;){
	 if((PTC->PDIR & (1<<PTC12)) &&(PTC->PDIR & (1<<PTC13)) ){
				PTD-> PCOR |= 1<<PTD16;
	}else{
		PTD-> PSOR |= 1<<PTD16;

	}

	 if((PTC->PDIR & (1<<PTC12)) && !(PTC->PDIR & (1<<PTC13)) ){

		PTD-> PCOR |= 1<<PTD0;
	}else{
		PTD-> PSOR |= 1<<PTD0;
	}


	if(!(PTC->PDIR & (1<<PTC12)) && (PTC->PDIR & (1<<PTC13)) ){
			PTD-> PCOR |= 1<<PTD15;
		}else{
			PTD-> PSOR |= 1<<PTD15;
		}



}


}
