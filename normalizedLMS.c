// Normalized LMS Filter for TIc61713 DSP
// Edwin Cordon

#include "dsk6713_aic23.h"

#define DSK6713_AIC23_INPUT_MIC 0x0015
#define DSK6713_AIC23_INPUT_LINEIN 0x0011

Uint32 fs = DSK6713_AIC23_FREQ_8KHZ; 
Uint16 inputsource = DSK6713_AIC23_INPUT_LINEIN; // 0x011

#define numTaps	50
float mu = 0.5;

// 32-bit register for stereo input
union{
	unsigned int Stereo;
	short channel[2];
}Data;

float xbuffer[numTaps];	// input noise sample buffer
float h[numTaps];		// filter coefficients
int i = 0;
float xf = 0.0f, df = 0.0f, yf = 0.0f, ef = 0.0f;
short x = 0, d = 0, y = 0, e = 0;

void main(){

	comm_intr();	// Enable interrupts

	DSK6713_LED_init();
	DSK6713_DIP_init();

	// Initialize filter coefficients and input buffer
	for (i = 0; i < numTaps; i++){
		xbuffer[i] = 0.0f;
		h[i] = 0.0f;
	}

	while(1);
}

interrupt void c_int11(){

	Data.Stereo = input_sample();
	
	d = Data.channel[0];	//signal + noise
	x = Data.channel[1];	//noise

	df = (float)d;
	df = df/32767;

	xf = (float) x;
	xf = xf/32767;

	// shift input buffer and store new noise sample
	for (i = numTaps-1; i>0; i--){
		xbuffer[i] = xbuffer[i-1];
	}
	xbuffer[0] = xf;
	yf = 0;

	// calculate output sample
	for (i = 0; i < numTaps; i++){
		yf += (h[i] * xbuffer[i]);
	}

	// caculate error
	ef = df - yf;

	// update filter coefficients
	for (i = 0; i < numTaps; i++){
		h[i] = h[i] + mu * ef * xbuffer[i];
	} 

	ef = 32767 * ef;
	e = (short) ef;

	yf = 32767 * yf;
	y = (short) yf;

	// if switch is not pressed, output caculated error on left channel
	if(DSK6713_DIP_get(0) == 0) {
		Data.channel[0] = e;
	}
	// else output original input on left channel
	else{
		Data.channel[0] = d;
	}
	
	Data.channel[1] = y;
	output_sample(Data.Stereo);
}

