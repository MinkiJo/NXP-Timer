#include "S32K144.h"

#define PTD0 0
#define PTC12 12

void NVIC_init_IRQs(void){
	S32_NVIC->ICPR[1] |= 1<<(61%32);
	S32_NVIC->ISER[1] |= 1<<(61%32);
	S32_NVIC->IP[61] = 0xB;
}


void PORT_init(void){
	PCC->PCCn[PCC_PORTD_INDEX] |=PCC_PCCn_CGC_MASK;
	PORTD->PCR[0]&=~PORT_PCR_MUX_MASK;
	PORTD->PCR[0] |= PORT_PCR_MUX(1);
	PTD->PDDR |=1<<0; //Blue

	/////////switch/////////
	PCC-> PCCn[PCC_PORTC_INDEX] |= PCC_PCCn_CGC_MASK;

	PORTC->PCR[12] &= ~PORT_PCR_MUX_MASK;
	PORTC->PCR[12] |= PORT_PCR_MUX(1);
	PTC->PDDR &= ~(1<<12);
	PORTC->PCR[12] |= (9<<16);


}

int idle_counter = 0;
int lpit0_ch0_flag_counter = 0;

void PORTC_IRQHandler(void){
	PTD->PTOR |= 1<<PTD0;
	PORTC->PCR[12] |= 0x01000000;
}
int main(void){
	PORT_init();
	NVIC_init_IRQs();

	for(;;){
		idle_counter++;
	}
	return 0;
}
