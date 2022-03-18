#include "MKL25Z4.h"                    // Device header
#include "buzzerControls.h"

#define MASK(x) (1<<(x))
#define PTA5_Pin 5
#define FREQ2MOD(x) 375000/x

int num_notes = 7;
int musical_notes[] = {262,294,330,349,392,440,494};

char notes[] = "EE E CE G g C g e a h bagEG AFG E CDh C g e a h bagEG AFG E CDh  GJFS E taC aCD GJFS E V VV  GJFS E taC aCD U D C  GJFS E taC aCD GJFS E V VV  GJFS E taC aCD U D C ";
int beats[] = {1,1,1,1,1,1,1,1,2,2,2,2,2,1,1,2,2,1,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,1,1,1,2,2,1,1,2,2,1,1,1,1,1,1,2,1,1,1,1,2,1,1,1,1,1,1,1,1,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,1,1,2,2,6,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,1,1,2,2,6};
int songLength = sizeof(notes);
int tempo = 0x7FFF;
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

int frequency(char note) 
{
  // This function takes a note character (a-g), and returns the
  // corresponding frequency in Hz for the tone() function.
  
  int i;
  const int numNotes = 19;  // number of notes we're storing
  
  // The following arrays hold the note characters and their
  // corresponding frequencies. The last "C" note is uppercase
  // to separate it from the first lowercase "c". If you want to
  // add more notes, you'll need to use unique characters.

  // For the "char" (character) type, we put single characters
  // in single quotes.

  char names[] = { 'c', 'd', 'e', 'f', 'g', 't', 'a', 'b', 'h', 'C' , 'D' , 'S' , 'E', 'F' , 'J', 'G' , 'A' , 'V' , 'U'};
  int frequencies[] = {262, 294, 330, 349, 392, 415 , 440, 466 , 494, 523, 587, 622 , 659, 698, 740, 784, 880 , 1047 , 622};
  
  // Now we'll search through the letters in the array, and if
  // we find it, we'll return the frequency for that note.
  
  for (i = 0; i < numNotes; i++)  // Step through the notes
  {
    if (names[i] == note)         // Is this the one?
    {
      return(frequencies[i]);     // Yes! Return the frequency
    }
  }
  return(0);  // We looked through everything and didn't find it,
              // but we still need to return a value, so return 0.
}

void playSong() {
	while (1) {
		for (int i = 0; i < num_notes; i++) {
			TPM0->MOD = FREQ2MOD(frequency(notes[i])); //need a function that converts freq to mod value
			TPM0_C2V = (FREQ2MOD(frequency(notes[i])))/2; 
			delay_mult100(beats[i] * tempo);
		}
		//TPM1_C0V++;
		//delay_mult100(0xffff);
	}
}