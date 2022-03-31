#include "buzzerControls.h"

#define MASK(x) (1<<(x))
#define PTA5_Pin 5
#define FREQ2MOD(x) 375000/x

int num_notes = 7;
int musical_notes[] = {262,294,330,349,392,440,494};

char notes[] = "EE E CE G g C g e a h bagEG AFG E CDh C g e a h bagEG AFG E CDh  GJFS E taC aCD GJFS E V VV  GJFS E taC aCD U D C  GJFS E taC aCD GJFS E V VV  GJFS E taC aCD U D C ";
int beats[] = {1,1,1,1,1,1,1,1,2,2,2,2,2,1,1,2,2,1,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,1,1,1,2,2,1,1,2,2,1,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,1,1,1,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,1,1,2,2,6,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,1,1,2,2,6};
int songLength = sizeof(notes);
int tempo = 150;

void InitPWMBuzzer(){
	//Enable clock gating for PortA
	SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;
	
	//Enable clock gating for timer0
	SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK;
	
	//Select clock for TPM module
	SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);
	
	//modulo value
	TPM0->MOD = 7500;
	
	TPM0->SC &= ~((TPM_SC_CMOD_MASK | TPM_SC_PS_MASK));
	TPM0->SC |= (TPM_SC_CMOD(1) | TPM_SC_PS(7)); //Set CMOD to 01 to increment according to main clock
	TPM0->SC &= ~(TPM_SC_CPWMS_MASK); //upcounting
	
	TPM0_C2SC &= ~((TPM_CnSC_ELSB_MASK) | (TPM_CnSC_ELSA_MASK) | (TPM_CnSC_MSB_MASK) | (TPM_CnSC_MSA_MASK));
	TPM0_C2SC |= (TPM_CnSC_ELSB(1) | TPM_CnSC_MSB(1));
}

void InitGPIOBuzzer() {
	//Configure mode 3 for pwm pin
	PORTA->PCR[PTA5_Pin] &= ~PORT_PCR_MUX_MASK;
	PORTA->PCR[PTA5_Pin] |= PORT_PCR_MUX(3);
}

static void delay(volatile uint32_t nof) {
  while(nof!=0) {
    __asm("NOP");
    nof--;
  }
}

void delay_mult100(volatile uint32_t nof) {
	for (int i = 0; i < 100; i++) {
		delay(nof);
	}
}

int frequency(char note) {
  //int numNotes = 19; 
  char names[] = { 'c', 'd', 'e', 'f', 'g', 't', 'a', 'b', 'h', 'C' , 'D' , 'S' , 'E', 'F' , 'J', 'G' , 'A' , 'V' , 'U'};
  int frequencies[] = {262, 294, 330, 349, 392, 415 , 440, 466 , 494, 523, 587, 622 , 659, 698, 740, 784, 880 , 1047 , 622};
 
  for (int i = 0; i < sizeof(frequencies); i++) {
    if (names[i] == note) return(frequencies[i]);  
  }
  return 0;  
}

void playSong() {
	while (1) {
		for (int i = 0; i < songLength; i++) {
			osSemaphoreAcquire(buzzerSem, osWaitForever);
			TPM0->MOD = FREQ2MOD(frequency(notes[i])); //need a function that converts freq to mod value
			TPM0_C2V = (FREQ2MOD(frequency(notes[i])))/5; //20% Duty Cycle
			osSemaphoreRelease(buzzerSem);
			osDelay(beats[i] * tempo); //delay_mult100(beats[i] * tempo);
		}
		//TPM1_C0V++;
		//delay_mult100(0xffff);
	}
}

void playCompletedSong() {
	while (1) {
		for (int i = 0; i < num_notes; i++) {
			osSemaphoreAcquire(buzzerSem, osWaitForever);
			TPM0->MOD = FREQ2MOD(musical_notes[i]); //need a function that converts freq to mod value
			TPM0_C2V = (FREQ2MOD(musical_notes[i]))/5; //20% Duty Cycle
			osSemaphoreRelease(buzzerSem);
		}
	}
}