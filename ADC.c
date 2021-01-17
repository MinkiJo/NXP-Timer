#include "S32K144.h"
#define PTD0 0
#define PTD15 15
#define PTD16 16

void SOSC_init_8Mhz(void){
   SCG->SOSCDIV = SCG_SOSCDIV_SOSCDIV1(1)|SCG_SOSCDIV_SOSCDIV2(1);

   SCG->SOSCCFG = SCG_SOSCCFG_RANGE(2)|SCG_SOSCCFG_EREFS_MASK;

   while(SCG->SOSCCSR & SCG_SOSCCSR_LK_MASK);
   SCG->SOSCCSR = SCG_SOSCCSR_SOSCEN_MASK;

   while(!(SCG->SOSCCSR & SCG_SOSCCSR_SOSCVLD_MASK));
}

void SPLL_init_160Mhz(void){
   while(SCG->SPLLCSR & SCG_SPLLCSR_LK_MASK);
   SCG->SPLLCSR &= ~SCG_SPLLCSR_SPLLEN_MASK;

   SCG->SPLLDIV |= SCG_SPLLDIV_SPLLDIV1(2)|SCG_SPLLDIV_SPLLDIV2(3);
   SCG->SPLLCFG = SCG_SPLLCFG_MULT(24);

   while(SCG->SPLLCSR & SCG_SPLLCSR_LK_MASK);
   SCG->SPLLCSR |= SCG_SPLLCSR_SPLLEN_MASK;

   while(!(SCG->SPLLCSR & SCG_SPLLCSR_SPLLVLD_MASK));
}

void NormalRUNMode_80Mhz(void){
   SCG->SIRCDIV = SCG_SIRCDIV_SIRCDIV1(1)|SCG_SIRCDIV_SIRCDIV2(1);

   SCG->RCCR = SCG_RCCR_SCS(6)
         |SCG_RCCR_DIVCORE(0b01)
         |SCG_RCCR_DIVBUS(0b01)
         |SCG_RCCR_DIVSLOW(0b10);

   while(((SCG->CSR & SCG_CSR_SCS_MASK) >> SCG_CSR_SCS_SHIFT) != 6){}
}

void PORT_init(void){
   PCC->PCCn[PCC_PORTD_INDEX] |= PCC_PCCn_CGC_MASK;
   PORTD->PCR[PTD0] = PORT_PCR_MUX(1);
   PORTD->PCR[PTD15] = PORT_PCR_MUX(1);
   PORTD->PCR[PTD16] = PORT_PCR_MUX(1);

   PTD->PDDR |= 1<<PTD0
            |1<<PTD15
            |1<<PTD16;
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

int main(void){
   uint32_t adcResultInMv = 0;

   SOSC_init_8Mhz();
   SPLL_init_160Mhz();
   NormalRUNMode_80Mhz();
   PORT_init();
   ADC_init();

   for(;;){
      convertAdcChan(12);
      while(adc_complete()==0){}
      adcResultInMv = read_adc_chx();

      if(adcResultInMv > 3750){
         PTD->PSOR |= 1<<PTD0 | 1<<PTD16;
         PTD->PCOR |= 1<<PTD15;
      }
      else if(adcResultInMv > 2500){
         PTD->PSOR |= 1<<PTD0 | 1<<PTD15;
         PTD->PCOR |= 1<<PTD16;
      }
      else if(adcResultInMv > 1250){
         PTD->PSOR |= 1<<PTD15 | 1<<PTD16;
         PTD->PCOR |= 1<<PTD0;
      }
      else{
         PTD->PSOR |= 1<<PTD0 | 1<<PTD15 | 1<<PTD16;
      }
   }
}





