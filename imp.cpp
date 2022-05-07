/*****************************************************
 * imp - DSP LCR Measurement
 * rev 1 - may 2022 - shabaz
 *
 * uses imp.bin
 * first program the DSP board EEPROM using:
 * ./eeload -p imp.bin
 * then power-cycle the DSP board for the EEPROM
 * to be read by the DSP.
 *
 * Frequency is set using -i to one of these values:
 * 1: 100 Hz
 * 2: 120 Hz
 * 3: 1000 Hz
 * 4: 10000 Hz
 * Display format is set using -d to one of these values:
 * 1: magnitude and phase
 * Example to perform mag and phase measurement at 1000 Hz:
 *      ./imp -i 3 -d 1 
 * Example for M2M mode:
 *      ./imp -i 3 -d 1 -m
 * Example to calibrate short:
 *      ./imp -c 0
 * Example to calibrate with 10.0 ohm resistance:
 *      ./imp -c 10.0
 *****************************************************/

// includes
 #include <stdio.h>
 #include "options.h"
 #include "dsputil.h"
 #include "math.h"
#include "i2cfunc.h" // so we can use the delay_ms function

// defines
#define FTABLE_SIZE 4

// consts used by imp.bin
const int DC_SELECT = 0x0028; // Source selector 0-3
const int LEVEL_ADDR = 0x081a; // level_addr should be 0x081a or 0x081b for ADAU1401 DSP
const int LEVEL_X100_I = 0x0626; // node is a 16-bit value
const int LEVEL_X100_Q = 0x061a; // node is a 16-bit value
const int LEVEL_X100_I_ZOOM = 0x662; // node is a 16-bit value
const int LEVEL_X100_Q_ZOOM = 0x066e; // node is a 16-bit value
const int LEVEL_TOP = 0x03d2; // node is a 16-bit value

const double ftable[] = {100.0, 120.0, 1000.0, 10000.0};

// externs
extern char do_log;

// ************* main program **********************
int
main(int argc, char **argv)
{
    char* sw; // used for command-line arguments
    int fidx=0;
    double freqhz= 0;
    int dformat=0;
    char do_freq=0;
    char do_meas=0;
    double v100[2];
    double v1[2];
    double ph100 = 0.0;
    double ph1 = 0.0;
    double mag100 = 0.0;
    double mag1 = 0.0;
    double cart100[2];
    double cart1[2];
    double restop = 1000; // 1000 ohm top resistor
    double vstimpeak = 1.16587766; // stimulus voltage, measure 0.8244V RMS from op amp output to vmid. This is 1.16587766 V peak.
    double vinternalpeak = 1.217; // internally generated to go to multiplier
    double vtop = 0.0; // voltage across top resistor
    double itop = 0.0; // current through resistor (also through DUT)
    double impdut = 0.0;
    double resdut = 0.0;
    double reactdut = 0.0;
    double capdut = 0.0;
    double inddut = 0.0;

    // read in the command-line arguments
    if (cmdOptionExists(argv, argv + argc, "-m")) {
        // m2m mode
        do_log = 0;
    }

    sw = getCmdOption(argv, argv + argc, "-i");
    if (sw) {
        sscanf(sw, "%d", &fidx);
        if ((fidx > FTABLE_SIZE) || (fidx<1)) {
            printf("error, frequency index is out of range!\n");
            exit(1);
        }
        freqhz = ftable[fidx-1];
        if (do_log) printf("Selecting frequency %lf Hz\n", freqhz);
        do_freq=1;
    }

    sw = getCmdOption(argv, argv + argc, "-d");
    if (sw) {
        sscanf(sw, "%d", &dformat);
        if ((dformat > 3) || (dformat<1)) {
            printf("error, display format selection is invalid!\n");
            exit(1);
        }
        if (do_log) printf("Selecting display format #%d\n", dformat);
        do_meas=1;
    }

    dsp_open(); // create I2C handle for the DSP

    if (do_freq) {  // program the quadrature source selection
        set_dc_int(DC_SELECT, fidx-1);
    }

    if (do_meas) {
        delay_ms(900);

        v100[0] = readback(LEVEL_ADDR, LEVEL_X100_I); 
        v100[1] = readback(LEVEL_ADDR, LEVEL_X100_Q); 

        if ((v100[0]<0.1) && (v100[1]<0.1)) {
            // amplitudes are low, we can use the zoomed values
            printf("using zoom\n");
            v100[0] = readback(LEVEL_ADDR, LEVEL_X100_I_ZOOM) / 100;
            v100[1] = readback(LEVEL_ADDR, LEVEL_X100_Q_ZOOM) / 100;
        }
       
        v100[0] = ms_to_pp(v100[0]);
        v100[1] = ms_to_pp(v100[1]);

        vstimpeak = readback(LEVEL_ADDR, LEVEL_TOP);
        vstimpeak = ms_to_pp(vstimpeak);

        printf("X100:\n");
        printf("Source voltage: %lf V peak\n", vstimpeak);
        printf("Raw vreal, vimag values (Vpp): [%lf, %lf]\n", v100[0], v100[1]);

        // correction to scale the cartesian values
        v100[0]=v100[0]/28.3;
        v100[1]=v100[1]/28.3;

        printf("Scaled vreal, vimag values: [%lf, %lf]\n", v100[0], v100[1]);

        // raw phase:
        mag100 = sqrt( pow(v100[0], 2) + pow(v100[1], 2) );
        if (v100[1]>=0) {
            ph100 = (PI/2) - atan(v100[0]/v100[1]);
        } else {
            ph100 = (0.0 - (PI/2)) - atan(v100[0]/v100[1]);
        }
        printf("mag, phase (rad) is [%lf, %lf]\n", mag100, ph100);
        // phase and mag correction done by using a known 10 ohm resistor
        // i.e. assume it has pure 10 ohm resistance and no reactance
        // phase correction
        ph100 = ph100 - 0.391420;
        ph100 = 0 - ph100;
        // mag correction
        //mag100 = mag100 / 45.5;
        printf("Corrected mag, phase (rad) is [%lf, %lf]\n", mag100, ph100);
        cart100[0] = mag100 * cos(ph100);
        cart100[1] = mag100 * sin(ph100);
        printf("Corrected real, imag is [%lf, %lf]\n", cart100[0], cart100[1]);

        // aim: find current through the circuit.
        // current through top resistor (phasor subtraction then magnitude via pythag):
        vtop = sqrt(pow(vstimpeak-cart100[0], 2) + pow(cart100[1], 2));
        printf("voltage across source resistor is %lf Vrms\n", vtop/SQROOT2);
        itop = vtop/restop;
        printf("current through circuit is %lf mA peak (%lf mA RMS)\n", itop*1000.0, (itop/SQROOT2)*1000.0);
        // impedance Z of DUT; use the same current. We don't need this to calculate the resistance and reactance.
        impdut = mag100 / itop;
        printf("DUT impedance (Z) at %lf Hz: %lf ohm\n", freqhz, impdut);
        // DUT resistance; use magnitude
        resdut = mag100 / (itop * cos(ph100));
        reactdut = mag100 / (itop * sin(ph100));
        
        // print out reactance and RCL values
        printf("DUT reactance: %lf ohm\n", reactdut);
        printf("DUT resistance: %lf ohm\n", resdut);

        if (reactdut<=0) {  // capacitance
            capdut = 1/(2*PI*freqhz*reactdut);
            capdut = 0-capdut;
            printf("DUT capacitance: %lf nF\n", capdut * 1e9);
        } else { // inductance
            inddut = reactdut / (2*PI*freqhz);
            printf("DUT inductance: %lf uH\n", inddut * 1e6);
        }


        //printf("capacitance is %lf nF\n", (1.0/(cart100[1]*2*PI*1000))*1000000000  );


    } 

    dsp_close(); // close the I2C resource for the DSP

    return(0);
}


