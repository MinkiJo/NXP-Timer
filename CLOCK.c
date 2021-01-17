#include "device_registers.h"
#include "S32K144.h"
#define DIG1 1
#define DIG2 2
#define DIG3 6
#define DIG4 8

#define SEG1 14
#define SEG2 4
#define SEG3 13
#define SEG4 3
#define SEG5 5
#define SEG6 11
#define SEG7 10
#define SEG8 7 //dot

#define DOT 17

#define PTC12 12
#define PTC13 13
#define PTD15 15
#define PTD16 16
#define PTD0 0

volatile int timer = 0;
volatile int setmode = 0;
int digpin[4] = {DIG1,DIG2,DIG3,DIG4};
int segpin[8] = {SEG1,SEG2,SEG3,SEG4,SEG5,SEG6,SEG7,SEG8};

void SOSC_init_8MHz(void){
	SCG->SOSCDIV=SCG_SOSCDIV_SOSCDIV1(1)|
			SCG_SOSCDIV_SOSCDIV2(1);
	SCG->SOSCCFG = SCG_SOSCCFG_RANGE(2)|
			SCG_SOSCCFG_EREFS_MASK;

	while(SCG->SOSCCSR&SCG_SOSCCSR_LK_MASK);
	SCG->SOSCCSR=SCG_SOSCCSR_SOSCEN_MASK;

	while(!(SCG->SOSCCSR & SCG_SOSCCSR_SOSCVLD_MASK));

}

void SPLL_init_160MHz(void){
	while(SCG->SPLLCSR &SCG_SPLLCSR_LK_MASK);
	SCG->SPLLCSR &= ~SCG_SPLLCSR_SPLLEN_MASK;
	SCG->SPLLDIV |= SCG_SPLLDIV_SPLLDIV1(2)|
			SCG_SPLLDIV_SPLLDIV2(3);
	SCG->SPLLCFG = SCG_SPLLCFG_MULT(24);

	while(SCG->SPLLCSR & SCG_SPLLCSR_LK_MASK);
	SCG->SPLLCSR |=SCG_SPLLCSR_SPLLEN_MASK;

	while(!(SCG->SPLLCSR &SCG_SPLLCSR_SPLLVLD_MASK));
}


void NormalRUNmode_80MHz(void){
	SCG->SIRCDIV = SCG_SIRCDIV_SIRCDIV1(1)
			| SCG_SIRCDIV_SIRCDIV2(1);

	SCG->RCCR=SCG_RCCR_SCS(6)
			|SCG_RCCR_DIVCORE(0b01)
			|SCG_RCCR_DIVBUS(0b01)
			|SCG_RCCR_DIVSLOW(0b10);

	while(((SCG->CSR & SCG_CSR_SCS_MASK) >>SCG_CSR_SCS_SHIFT) != 6){}
}

void NVIC_init_IRQs(void){
	   S32_NVIC -> ICPR[1] |= 1 <<(61 %32);
	   S32_NVIC -> ISER[1] |= 1 <<(61 %32);
	   S32_NVIC -> IP[61] = 0xB;

	   S32_NVIC -> ICPR[1] |= 1 <<(48%32);
	   S32_NVIC -> ISER[1] |= 1 <<(48%32);
	   S32_NVIC -> IP[48] = 0xA;
}

void LPIT0_init(void){
	PCC->PCCn[PCC_LPIT_INDEX] &= ~PCC_PCCn_PCS_MASK;
	PCC->PCCn[PCC_LPIT_INDEX]|=PCC_PCCn_PCS(6);
	PCC->PCCn[PCC_LPIT_INDEX] |= PCC_PCCn_CGC_MASK;

	LPIT0->MCR |= LPIT_MCR_M_CEN_MASK;

	LPIT0->MIER |=LPIT_MIER_TIE0_MASK;
	LPIT0->TMR[0].TVAL =40000000;
	LPIT0->TMR[0].TCTRL |=LPIT_TMR_TCTRL_T_EN_MASK;
}

void PORT_init(void){

   PCC -> PCCn[PCC_PORTC_INDEX] = PCC_PCCn_CGC_MASK;
   PORTC ->PCR[12] = PORT_PCR_MUX(1);
   PTC ->PDDR &= ~(1<<PTC12);
   PORTC->PCR[12] |= (9<<16);

   PCC->PCCn[PCC_PORTD_INDEX] |=PCC_PCCn_CGC_MASK;

	for(int i=0;i<4;i++){
			PORTD->PCR[digpin[i]] &= ~PORT_PCR_MUX_MASK;
			PORTD->PCR[digpin[i]]|= PORT_PCR_MUX(1);
			PTD->PDDR |= 1<<digpin[i];
		}

	for(int i=0;i<8;i++){
			PORTD->PCR[segpin[i]] &= ~PORT_PCR_MUX_MASK;
			PORTD->PCR[segpin[i]]|= PORT_PCR_MUX(1);
			PTD->PDDR |= 1<<segpin[i];
		}

		PORTD->PCR[17] &= ~PORT_PCR_MUX_MASK;
		PORTD->PCR[17]|= PORT_PCR_MUX(1);
		PTD->PDDR |= 1<<DOT;

		PORTD->PCR[0] &= ~PORT_PCR_MUX_MASK;
		PORTD->PCR[0] |= PORT_PCR_MUX(1);
		PTD->PDDR |= 1<<PTD0; /////Blue
}

void ADC_init(void){
   PCC->PCCn[PCC_ADC0_INDEX] &= ~PCC_PCCn_CGC_MASK;
   PCC->PCCn[PCC_ADC0_INDEX] |= PCC_PCCn_PCS(1);
   PCC->PCCn[PCC_ADC0_INDEX] |= PCC_PCCn_CGC_MASK;

   ADC0->SC1[0] |= ADC_SC1_ADCH_MASK;

   ADC0->CFG1 &= ~ADC_CFG1_ADIV_MASK;

   ADC0->CFG1 |= ADC_CFG1_MODE(1);

   ADC0->CFG2 = ADC_CFG2_SMPLTS(12);

   ADC0->SC2 = 0x00000000;
   ADC0->SC3 = 0x00000000;
}


void convertAdcChan(uint16_t adcChan){
   ADC0->SC1[0] &= ~ADC_SC1_ADCH_MASK;
   ADC0->SC1[0] |= ADC_SC1_ADCH(adcChan);
}

uint8_t adc_complete(void){
   return ((ADC0->SC1[0] & ADC_SC1_COCO_MASK) >>ADC_SC1_COCO_SHIFT);
}

uint32_t read_adc_chx(void){
   uint16_t adc_result = 0;
   adc_result = ADC0->R[0];

   return (uint32_t)((5000*adc_result)/0xFFF);
}


void setTime(int dig_num, int n){
	for(int i=0;i<8;i++){
		PTD->PSOR |= 1<<segpin[i]; //pin clear
	}

	PTD-> PSOR |= 1<<digpin[dig_num];

	switch(n){
	case 0:
		PTD->PCOR |= 1<<SEG1;
		PTD->PCOR |= 1<<SEG2;
		PTD->PCOR |= 1<<SEG3;
		PTD->PCOR |= 1<<SEG4;
		PTD->PCOR |= 1<<SEG5;
		PTD->PCOR |= 1<<SEG6;
		break;
	case 1:
		PTD->PCOR |= 1<<SEG2;
		PTD->PCOR |= 1<<SEG3;
		break;
	case 2:
		PTD->PCOR |= 1<<SEG1;
		PTD->PCOR |= 1<<SEG2;
		PTD->PCOR |= 1<<SEG4;
		PTD->PCOR |= 1<<SEG5;
		PTD->PCOR |= 1<<SEG7;
		break;
	case 3:
		PTD->PCOR |= 1<<SEG1;
		PTD->PCOR |= 1<<SEG2;
		PTD->PCOR |= 1<<SEG3;
		PTD->PCOR |= 1<<SEG4;
		PTD->PCOR |= 1<<SEG7;
		break;
	case 4:
		PTD->PCOR |= 1<<SEG2;
		PTD->PCOR |= 1<<SEG3;
		PTD->PCOR |= 1<<SEG6;
		PTD->PCOR |= 1<<SEG7;
		break;
	case 5:
		PTD->PCOR |= 1<<SEG1;
		PTD->PCOR |= 1<<SEG3;
		PTD->PCOR |= 1<<SEG4;
		PTD->PCOR |= 1<<SEG6;
		PTD->PCOR |= 1<<SEG7;
		break;
	case 6:
		PTD->PCOR |= 1<<SEG1;
		PTD->PCOR |= 1<<SEG3;
		PTD->PCOR |= 1<<SEG4;
		PTD->PCOR |= 1<<SEG5;
		PTD->PCOR |= 1<<SEG6;
		PTD->PCOR |= 1<<SEG7;
		break;
	case 7:
		PTD->PCOR |= 1<<SEG1;
		PTD->PCOR |= 1<<SEG2;
		PTD->PCOR |= 1<<SEG3;
		PTD->PCOR |= 1<<SEG6;
		break;
	case 8:
		PTD->PCOR |= 1<<SEG1;
		PTD->PCOR |= 1<<SEG2;
		PTD->PCOR |= 1<<SEG3;
		PTD->PCOR |= 1<<SEG4;
		PTD->PCOR |= 1<<SEG5;
		PTD->PCOR |= 1<<SEG6;
		PTD->PCOR |= 1<<SEG7;
		break;
	case 9:
		PTD->PCOR |= 1<<SEG1;
		PTD->PCOR |= 1<<SEG2;
		PTD->PCOR |= 1<<SEG3;
		PTD->PCOR |= 1<<SEG4;
		PTD->PCOR |= 1<<SEG6;
		PTD->PCOR |= 1<<SEG7;
		break;
	}
	PTD-> PCOR |= 1<<digpin[dig_num];
}

int main(void){
	PORT_init();
	SOSC_init_8MHz();
	SPLL_init_160MHz();
	NormalRUNmode_80MHz();
	NVIC_init_IRQs();
	ADC_init();
	LPIT0_init();
	int hour, min, sec, adc_num;
		for(int i=0;i<8;i++){
			PTD->PSOR |= 1<<segpin[i]; //pin clear
		}

		for(;;){


		convertAdcChan(12);
		if(setmode){
			while(adc_complete()==0){}
			timer = read_adc_chx()*(17.28);
		}
		if(timer == 86400)
			timer = 0;

		if(((timer >=0) && (timer <5) )|| ((timer >= 43200) && (timer < 43205)))
			PTD-> PCOR |= 1<<PTD0;
		else PTD-> PSOR |= 1<<PTD0;

		if((timer % 2) == 0)
			PTD->PCOR |= 1<<DOT;
		else PTD->PSOR |= 1<<DOT;


		hour = timer/3600;
		min = (timer%3600)/60;
		sec = ((timer%3600)%60)%60;


		setTime(0,hour/10);
		setTime(1,hour%10);
		setTime(2,min/10);
		setTime(3,min%10);
		}


}

void LPIT0_Ch0_IRQHandler(void){
	timer++;
	LPIT0->MSR |= LPIT_MSR_TIF0_MASK;
}

void PORTC_IRQHandler(void){

   if(setmode == 1)
	   setmode = 0;
   else setmode = 1;
   PORTC -> PCR[12] |= 0x01000000;
}
