#ifndef __DSPUTIL_HEADER_FILE__
#define __DSPUTIL_HEADER_FILE__

#define DSP_ADDR 0x34
#define EE_ADDR 0x50

#define I2CBUS 1
#define WPGPIO 4
#define BLOCKSIZE 32
#define TWOPOW23 8388608.0
#define SQROOT2 1.4142135623731
#define PI 3.1415926535897932384626433832795

// ADAU1401 safeload addresses
#define SAFE_DATA0 0x0810
#define SAFE_ADDR0 0x0815
#define SAFE_INITIATE 0x081c
#define SAFE_SET_IST 0x003c


// functions to open and close I2C communication with the DSP
void dsp_open(void);
void dsp_close(void);

// puts a 16-bit integer into a 2-byte array, most significant byte first
void swap_order(char* res, int n);

// programs the .bin file into the EEPROM on the DSP board
// fname needs to be the complete filename (including any path if necessary)
int ee_prog(char* fname);

// 5.23 format used by DSP
// parameters: v is the decimal input, bytes is a 4-byte array
void double_to_5_23_format(double v, char* bytes);

// Sine Tone
// (Sources->Oscillators->Sine Tone)
// sets the frequency to an integer value
void set_freq(int addr, int f);

// enable or disable mute (set mute to 1 to mute the signal)
// (Volume Controls->Mute->No Slew (Standard)->Mute)
void set_mute(int addr, char mute);

// set the pitch for the DSP Pitch Transposer object
// (ADI Algorithms->Pitch Modification->Pitch Transposer)
void set_pitch(int addr, double p);

// DSP General 2nd Order Filter
// (Filters->Second Order->Double Precision->1 Ch->General (2nd order))
// coeff is pointer array of five double values b0, b1, b2, a1, a2
void set_gen_2nd_order_filter(int addr, double* coeff);

// Double Precision Nth Order Filter (2-Channel)
// (Filters->Nth Order->Double Precision->2 Channels->Nth Order Filter)
// coeff is a pointer to array of 15 double values generated by SigmaStudio
void set_dfilter6(int addr, double* coeff);

// bypass the Double Precision Nth Order Filter (2-Channel)
void set_dfilter6_bypass (int addr);

// Single Volume
// (Volume Controls->Adjustable Gain->Single/Multiple Controls->No Slew (Standard)->Single Volume)
// sets the amplitude between 0.0 and 1.0
void set_amp(int addr, double a);

// set the switch on or off
// (Sources->Switch(0,1)->28_0 Format->On/Off Switch)
// v=0 represents OFF, and v>0 represents ON.
void set_switch(int addr, int v);

// set the frequency for the DSP Sine with Phase and Gain object
// (Sources->Oscillators->With Phase->Sine Tone with Phase and Gain)
void set_sinphase_freq(int addr, int f);
// set the gain value of the object
void set_sinphase_gain(int addr, double amp);
// set the phase value of the object
void set_sinphase_phase(int addr, int ang);

// set the DC integer (28.0) value for the DSP DC Input Entry object
// (Sources->DC->DC Input Entry)
// v is a value 0 to 0xffff
void set_dc_int(int addr, int v);
// set the DC float (28.0) value for the DSP DC Input Entry object
// (Sources->DC->DC Input Entry)
// v is a double value
void set_dc_float(int addr, double v);
// safeload version of set_dc_float
void set_dc_float_safeload(int addr, double v);

// performs DSP readback (data capture register)
// addr should be 0x081a or 0x081b for ADAU1401 DSP
// node is a 16-bit value
// The code returns a decimal value based on the 5.19 format that was received
double readback(int addr, int node);

// performs mean square to V RMS conversion
double ms_to_rms(double ms);

// performs mean square to V p-p conversion
double ms_to_pp(double ms);

// perform mean square to dBu conversion
double ms_to_dbu(double ms);

#endif // __DSPUTIL_HEADER_FILE__

