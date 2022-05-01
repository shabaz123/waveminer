/**********************************************************
 * dsputil.c - Functions to simplify using the DSP chip
 * rev 1 - april 2022 - shabaz
 **********************************************************/

// includes
 #include <stdio.h>
 #include <string.h>
 #include <stdint.h>
 #include <stdlib.h>
 //extern "C" {
 #include <wiringPi.h>
 //}
 #include "dsputil.h"
 #include "i2cfunc.h"
 #include <math.h>

 // globals
 int dsp_handle; // I2C handle for DSP chip
 char do_log = 1;

// function to open I2C communication with the DSP
void dsp_open(void) {
     dsp_handle = i2c_open(I2CBUS, DSP_ADDR);
}

// function to close I2C communication with the DSP
void dsp_close(void) {
    i2c_close(dsp_handle);
}

// put integer into a char array, with the correct order
// (most significant byte first)
void swap_order(char* res, int n)
{
  *res = *(((char*)&n)+1);
  *(res+1) = *((char*)&n);
}

// program the EEPROM on the DSP board with a .bin application file
int ee_prog(char* fname) {
    FILE *fptr = NULL;
    char fbuf[BLOCKSIZE+2];
    size_t bytesRead = 0;
    unsigned int addr = 0x0000;
    int ee_handle;


    if ((fptr = fopen(fname, "rb")) == NULL) {
        if (do_log) {
            printf("file '%s' not found!\n", fname);
        } else {
            printf("error\n");
        }
        return(1);
    }

    if (do_log) printf("setting WP low\n");
    wiringPiSetupGpio();
    pinMode(WPGPIO, OUTPUT);
    digitalWrite(WPGPIO, 0);

    ee_handle = i2c_open(I2CBUS, EE_ADDR);

    memset(&fbuf[2], 0xff, BLOCKSIZE);
    while ((bytesRead = fread(&fbuf[2], 1, BLOCKSIZE, fptr)) > 0)
    {
        if (do_log) printf("writing block to addr 0x%04x\n", addr);
        fbuf[0]=(char)((addr>>8) & 0x00ff);
        fbuf[1]=(char)(addr & 0x00ff);
        i2c_write(ee_handle, (unsigned char*)fbuf, BLOCKSIZE+2);
        delay_ms(200);
        memset(fbuf, 0xff, BLOCKSIZE);
        addr = addr + BLOCKSIZE;
    }
    i2c_close(ee_handle);
    pinMode(WPGPIO, INPUT);
    return(0);
}

// 5.23 format used by DSP
// parameters: v is the decimal input, bytes is a 4-byte array
void
double_to_5_23_format(double v, char* bytes) {
    int decscaled;
    char tb;

    decscaled = (int)( v * TWOPOW23 );
    memcpy((void*)bytes, (const void*)&decscaled, 4);
    bytes[3] &= 0x0f; // we only want 28 bits, not 32
    // flip the bytes ordering
    tb = bytes[0];
    bytes[0]=bytes[3];
    bytes[3]=tb;
    tb=bytes[1];
    bytes[1]=bytes[2];
    bytes[2]=tb;
}

// 5.19 format used by DSP (e.g. for Readback)
// parameters: v is the decimal result, bytes is the 3-byte array to be converted
void
dsp_5_19_format_to_double(double *v, char* bytes) {
  int unscaled;
  char* iptr = (char*)&unscaled;

  *iptr = 0x00; // we only have 24 bits
  *(iptr+1) = bytes[2];
  *(iptr+2) = bytes[1];
  *(iptr+3) = bytes[0];
  unscaled = unscaled >> 4;

  *v = ((double)unscaled) / TWOPOW23;
}

// set the frequency for the DSP Sine Tone object
void
set_freq(int addr, int f) {
    char buf[6];

    // sin_lookupAlg19401mask
    swap_order(buf, addr); // store addr into start of buffer
    buf[2] = 0x00;
    buf[3] = 0x00;
    buf[4] = 0x00;
    buf[5] = 0xff;
    if (do_log) printf("writing to address 0x%02x%02x\n", buf[0], buf[1]);
    i2c_write(dsp_handle, (unsigned char*)buf, 6);

    // sin_lookupAlg19401increment
    addr++;
    swap_order(buf, addr);
    double_to_5_23_format( ((double)f)/24000.0, &buf[2] );
    if (do_log) printf("writing to address 0x%02x%02x\n", buf[0], buf[1]);
    i2c_write(dsp_handle, (unsigned char*)buf, 6);
    
    // sin_lookupAlg19401ison
    addr++;
    swap_order(buf, addr);
    buf[2] = 0x00;
    buf[3] = 0x80;
    buf[4] = 0x00;
    buf[5] = 0x00;
    if (do_log) printf("writing to address 0x%02x%02x\n", buf[0], buf[1]);
    i2c_write(dsp_handle, (unsigned char*)buf, 6);

}

// set the frequency for the DSP Sine with Phase and Gain object
// (Sources->Oscillators->With Phase->Sine Tone with Phase and Gain)
void
set_sinphase_freq(int addr, int f) {
    char buf[6];

    // sin_lookupPhaseNincrement
    swap_order(buf, addr); // store addr into start of buffer
    double_to_5_23_format( ((double)f)/24000.0, &buf[2] );
    if (do_log) printf("writing to address 0x%02x%02x\n", buf[0], buf[1]);
    i2c_write(dsp_handle, (unsigned char*)buf, 6);
}
// set the gain value of the object
void
set_sinphase_gain(int addr, double amp)
{
    char buf[6];
    addr=addr+2;
    // sin_lookupPhaseNGain_0
    swap_order(buf, addr); // store addr into start of buffer
    double_to_5_23_format( amp, &buf[2] );
    if (do_log) printf("writing to address 0x%02x%02x\n", buf[0], buf[1]);
    i2c_write(dsp_handle, (unsigned char*)buf, 6);
}
// set the phase value of the object
void
set_sinphase_phase(int addr, int ang)
{
    char buf[6];
    double acalc;
    char angc;
    addr=addr+3;

    acalc = ((double)ang)/1.411764; // get 0-360 deg into 0-255 range
    acalc = roundf(acalc);
    angc=(char)acalc;
    // sin_lookupPhaseNGain_0
    swap_order(buf, addr); // store addr into start of buffer
    buf[2] = 0x00;
    buf[3] = 0x80;
    buf[4] = 0x00;
    buf[5] = angc;
    if (do_log) printf("writing to address 0x%02x%02x\n", buf[0], buf[1]);
    i2c_write(dsp_handle, (unsigned char*)buf, 6);
}

// DSP General 2nd Order Filter
// (Filters->Second Order->Double Precision->1 Ch->General (2nd order))
// coeff is pointer array of five double values b0, b1, b2, a1, a2
void
set_gen_2nd_order_filter(int addr, double* coeff) {
    char buf[6];
    int i;
    double c;

    for (i=0; i<5; i++) {
        // EQ1940Singlex0b1 to EQ1940Singlex2a1
        swap_order(buf, addr+i); // store addr into start of buffer
        c = coeff[i];
        if (i>=3) {
            c = 0-c;  // a1 and a2 need opposite sign, don't know why
        }
        double_to_5_23_format( c, &buf[2] );
        if (do_log) printf("writing to address 0x%02x%02x coeff 0x%02x,%02x,%02x,%02x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
        i2c_write(dsp_handle, (unsigned char*)buf, 6);
    }

}

// set the amplitude for the DSP Single Volume object
void
set_amp(int addr, double a) {
    char buf[6];

    // Gain1940AlgNS1
    swap_order(buf, addr); // store addr into start of buffer
    double_to_5_23_format( a, &buf[2] );
    if (do_log) printf("writing to address 0x%02x%02x\n", buf[0], buf[1]);
    i2c_write(dsp_handle, (unsigned char*)buf, 6);
}

// enable or disable mute (set mute to 1 to mute the signal)
// (Volume Controls->Mute->No Slew (Standard)->Mute)
void
set_mute(int addr, char mute) {
    char buf[6];

    buf[2] = 0x00;
    buf[3] = 0x00;
    buf[4] = 0x00;
    buf[5] = 0x00;

    if (mute==0) buf[3] = 0x80;

    // MuteNoSlewAlg1mute
    swap_order(buf, addr); // store addr into start of buffer
    if (do_log) printf("writing to address 0x%02x%02x\n", buf[0], buf[1]);
    i2c_write(dsp_handle, (unsigned char*)buf, 6);
}

// set the pitch for the DSP Pitch Transposer object
// (ADI Algorithms->Pitch Modification->Pitch Transposer)
void
set_pitch(int addr, double p) {
    char buf[6];

    // PitchShiftsAlg1freq
    swap_order(buf, addr); // store addr into start of buffer
    double_to_5_23_format( p, &buf[2] );
    if (do_log) printf("writing to address 0x%02x%02x\n", buf[0], buf[1]);
    i2c_write(dsp_handle, (unsigned char*)buf, 6);
}

// set the Double Precision Nth Order Filter (2-Channel)
// (Filters->Nth Order->Double Precision->2 Channels->Nth Order Filter)
void
set_dfilter6(int addr, double* coeff) {
    char buf[6];
    int i;
    double c;

    for (i=0; i<15; i++) {
        // NthOrderDouble2xxxx
        swap_order(buf, addr+i); // store addr into start of buffer
        c = coeff[i];
        double_to_5_23_format( c, &buf[2] );
        if (do_log) printf("writing to address 0x%02x%02x coeff 0x%02x,%02x,%02x,%02x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
        i2c_write(dsp_handle, (unsigned char*)buf, 6);
    }
}

// bypass the Double Precision Nth Order Filter (2-Channel)
void
set_dfilter6_bypass (int addr) {
    char buf[6];
    int i;

    for (i=0; i<15; i++) {
        // NthOrderDouble2xxxx
        swap_order(buf, addr+i); // store addr into start of buffer
        buf[2]=0x00;
        buf[3]=0x00;
        buf[4]=0x00;
        buf[5]=0x00;
        if ((i==0) || (i==5) || (i==10)) {
            buf[3]=0x80;
        }
        if (do_log) printf("writing to address 0x%02x%02x coeff 0x%02x,%02x,%02x,%02x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
        i2c_write(dsp_handle, (unsigned char*)buf, 6);
    }
}

// performs DSP readback (data capture register)
// addr should be 0x081a or 0x081b for ADAU1401 DSP
// node is a 16-bit value
// The code returns a decimal number
double
readback(int addr, int node) {
    int ret;
    char buf[6];
    char r[3];
    double v;

    // ReadBackAlg
    swap_order(buf, addr); // store addr into start of buffer
    swap_order(&buf[2], node); // store node into buffer
    if (do_log) printf("writing to address 0x%02x%02x node 0x%02x%02x\n", buf[0], buf[1], buf[2], buf[3]);
    i2c_write(dsp_handle, (unsigned char*)buf, 4);
    delay_ms(100);
    ret=i2c_write_read(dsp_handle, DSP_ADDR, (unsigned char*)buf, 2, DSP_ADDR, (unsigned char*)r, 3);
    if (do_log) printf("read %d bytes: 0x%02x, %02x, %02x\n", ret, *r, *(r+1), *(r+2));
    dsp_5_19_format_to_double(&v, r);
    return(v);
}

// performs mean square to V RMS conversion
double
ms_to_rms(double ms) {
    return(sqrt(ms) * 2);
}

// performs mean square to V p-p conversion
double
ms_to_pp(double ms) {
    return( SQROOT2 * ms_to_rms(ms) );
}

// perform mean square to dBu conversion
double
ms_to_dbu(double ms) {
    return(log10(ms_to_rms(ms) / sqrt(0.001*600)) * 20);
}
