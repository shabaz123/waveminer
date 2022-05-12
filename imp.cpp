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
#define GAIN_ARRAY_SIZE 4

#define PAUSE_LOGGING do_log_store=do_log; do_log=0;
#define RESUME_LOGGING do_log=do_log_store;

// consts used by imp.bin
const int DC_SELECT = 0x0028; // Source selector 0-3
const int DC_HOLD_MEAS = 0x002b; // node for ref level of potential divider
const int DC_SUB_I = 0x0029; // Select value to subtract from I measurement
const int DC_SUB_Q = 0x002a; // Select value to subtract from Q measurement
const int LEVEL_ADDR = 0x081a; // level_addr should be 0x081a or 0x081b for ADAU1401 DSP
const int LEVEL_I = 0x068a; // node for I measurement (vreal)
const int LEVEL_I_X10 = 0x06d2;
const int LEVEL_I_X100 = 0x06f6;
const int LEVEL_Q = 0x06a2; // node for Q measurement (vimag)
const int LEVEL_Q_X10 = 0x06de;
const int LEVEL_Q_X100 = 0x06ea;
const int LEVEL_TOP = 0x03ea; // node for ref level of potential divider 
const int GAIN_READ_BLOCK[] = {0x0047, 0x0049, 0x0048, 0x004a};


const double ftable[] = {100.0, 120.0, 1000.0, 10000.0};

// externs
extern char do_log;

// globals
char do_log_store; // use to reduce logging for part of the code

// ************* functions *************************

void
freeze_meas(void) {
    // the 'value hold' register uses safeload otherwise there are strange values
    set_dc_float_safeload(DC_HOLD_MEAS, 0.0);    // freeze the measurement registers
}

void
unfreeze_meas(void) {
    // the 'value hold' register uses safeload otherwise there are strange values
    set_dc_float_safeload(DC_HOLD_MEAS, 1.0);    // freeze the measurement registers
}

// reset_dsp_settings
// try to set all configuration to a default
void
reset_dsp_settings(void)
{
    PAUSE_LOGGING;
    unfreeze_meas();
    //for (i=0; i<GAIN_ARRAY_SIZE; i++) {
    //    set_amp(GAIN_READ_BLOCK[i], 1);
    //}

    set_dc_float_safeload(DC_SUB_I, 0.0);    // subtract zero from the measurements
    set_dc_float_safeload(DC_SUB_Q, 0.0);
    RESUME_LOGGING;
}

// ************* main program **********************
int
main(int argc, char **argv)
{
    char* sw; // used for command-line arguments
    int fidx=0;
    int intportion_real;
    int intportion_imag;
    double freqhz= 0;
    int dformat=0;
    char do_dsp_reset=0;
    char do_freq=0;
    char do_meas=0;
    double v_complex[2];    // real and imaginary voltage measurement results from the DSP
                            // vreal is v_complex[0] and vimag is v_complex[1]
    double phase = 0.0;     // phase and mag representation of v_complex
    double mag = 0.0;       //
    double restop = 1000; // resistance of top resistor in the potential divider
    double vstimpeak = 0.0; // stimulus voltage, measures approx 0.824V RMS (1.166 Vpeak) from op amp output to 2.5 V rail.
    double vtop = 0.0; // voltage across top resistor (i.e. vstimpeak minus the voltage at center of potential divider)
    double itop = 0.0; // current through resistor (also through DUT)
    double reactdut_parallel = 0.0;
    double impdut = 0.0;  // impedance Z
    double reactdut = 0.0; // reactance X
    double resdut = 0.0; // resistance R
    double resdut_parallel = 0.0;  // parallel resistance Rp
    double capdut_parallel = 0.0;  // parallel capacitance Cp
    double inddut_parallel = 0.0;  // parallel inductance Lp

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
        do_dsp_reset=1;
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
        do_dsp_reset=1;
    }

    do_log_store = do_log;

    dsp_open(); // create I2C handle for the DSP

    if (do_dsp_reset) {
        reset_dsp_settings();
        //delay_ms(500);
    }

    if (do_freq) {  // program the quadrature source selection
        PAUSE_LOGGING;
        set_dc_int(DC_SELECT, fidx-1);
        RESUME_LOGGING;
    }

    if (do_meas) {

        PAUSE_LOGGING;
        freeze_meas();

        v_complex[0] = readback(LEVEL_ADDR, LEVEL_I); 
        v_complex[1] = readback(LEVEL_ADDR, LEVEL_Q); 
        if (do_log) printf("frozen real, imag is [%.8lf, %.8lf]\n", v_complex[0], v_complex[1]);
        RESUME_LOGGING;

        // now we subtract the integer value, so that we can zoom into the fractional part
        intportion_real = (int)v_complex[0];
        intportion_imag = (int)v_complex[1];
        PAUSE_LOGGING;
        set_dc_float_safeload(DC_SUB_I, (double)intportion_real);
        set_dc_float_safeload(DC_SUB_Q, (double)intportion_imag);
        RESUME_LOGGING;
        //delay_ms(100);
        // now read the fractional part with the X10 registers
        PAUSE_LOGGING;
        v_complex[0] = readback(LEVEL_ADDR, LEVEL_I_X10) / 10;
        //delay_ms(100);
        v_complex[1] = readback(LEVEL_ADDR, LEVEL_Q_X10) / 10;
        //delay_ms(100);
        RESUME_LOGGING;

        // check if we can read the X10 registers for more resolution
        if (v_complex[0]<0.1) {
            PAUSE_LOGGING;
            v_complex[0] = readback(LEVEL_ADDR, LEVEL_I_X100) / 100;
            RESUME_LOGGING;
            //delay_ms(100);
        }

        if (v_complex[1]<0.1) {
            PAUSE_LOGGING;
            v_complex[1] = readback(LEVEL_ADDR, LEVEL_Q_X100) / 100;
            RESUME_LOGGING;
            //delay_ms(100);
        }

        //printf("enhanced fractions are [%.8lf, %.8lf]\n", v_complex[0], v_complex[1]);
        v_complex[0] = v_complex[0] + (double)intportion_real;
        v_complex[1] = v_complex[1] + (double)intportion_imag;
        if (do_log) printf("hi-res real, imag are now [%.8lf, %.8lf]\n", v_complex[0], v_complex[1]);

        v_complex[0] = ms_to_pp(v_complex[0]);
        v_complex[1] = ms_to_pp(v_complex[1]);

        PAUSE_LOGGING;
        vstimpeak = readback(LEVEL_ADDR, LEVEL_TOP);
        RESUME_LOGGING;

        vstimpeak = ms_to_pp(vstimpeak);

        if (do_log) printf("Raw vreal, vimag values (Vpp): [%lf, %lf]\n", v_complex[0], v_complex[1]);

        // correction to scale the cartesian values
        v_complex[0]=v_complex[0]/70.45;
        v_complex[1]=v_complex[1]/70.45;

        if (do_log) printf("Scaled vreal, vimag values: [%lf, %lf]\n", v_complex[0], v_complex[1]);

        // raw phase:
        mag = sqrt( pow(v_complex[0], 2) + pow(v_complex[1], 2) );
        if (v_complex[1]>=0) {
            phase = (PI/2) - atan(v_complex[0]/v_complex[1]);
        } else {
            phase = (0.0 - (PI/2)) - atan(v_complex[0]/v_complex[1]);
        }
        if (do_log) printf("mag, phase (rad) is [%lf, %lf]\n", mag, phase);
        // phase and mag correction done by using a known 10 ohm resistor
        // i.e. assume it has pure 10 ohm resistance and no reactance
        // phase correction
        phase = phase - 0.391466;
        phase = 0 - phase;
        // mag correction
        //mag = mag / 45.5;
        if (do_log) printf("Corrected mag, phase (rad) is [%lf, %lf]\n", mag, phase);
        v_complex[0] = mag * cos(phase);
        v_complex[1] = mag * sin(phase);
        if (do_log) printf("Corrected vreal, vimag is [%lf, %lf]\n", v_complex[0], v_complex[1]);

        if (do_log) printf("Stim: %.2lf Hz source voltage: %lf V peak\n", freqhz, vstimpeak);

        // aim: find current through the circuit.
        // current through top resistor (phasor subtraction then magnitude via pythag):
        vtop = sqrt(pow(vstimpeak-v_complex[0], 2) + pow(v_complex[1], 2));
        if (do_log) printf("voltage across source resistor is %lf V peak (%lf V rms)\n", vtop, vtop/SQROOT2);
        itop = vtop/restop;
        if (do_log) printf("current through circuit is %lf mA peak (%lf mA RMS)\n", itop*1000.0, (itop/SQROOT2)*1000.0);
        reactdut_parallel = mag / (itop * sin(phase)); // use this to compute capacitance and inductance
        // DUT impedance (Z) use the same current. We don't need this to calculate the parallel resistance and parallel reactance.
        impdut = mag / itop;
        // DUT parallel resistance
        resdut_parallel = mag / (itop * cos(phase));
        // DUT reactance (X)
        //reactdut = sqrt(abs(pow(impdut, 2) - pow(resdut_parallel, 2)));
        reactdut = sqrt(pow(resdut_parallel, 2) - pow(impdut, 2));
        // DUT resistance (R)
        resdut = sqrt(pow(impdut, 2) - pow(reactdut, 2));
        
        if (reactdut_parallel<=0) {  // capacitance
            capdut_parallel = 1/(2*PI*freqhz*reactdut_parallel);
            capdut_parallel = 0-capdut_parallel;
            reactdut = 0-reactdut;
        } else { // inductance
            inddut_parallel = reactdut_parallel / (2*PI*freqhz);
        }

        // print out results
        printf("Phase               (phi) : %.3lf rad\n", phase);
        printf("Impedance             (Z) : %.3lf ohm\n", impdut);
        printf("Reactance             (X) : %.3lf ohm\n", reactdut);
        printf("Resistance            (R) : %.3lf ohm\n", resdut);
        printf("Parallel Resistance  (Rp) : %.3lf ohm\n", resdut_parallel);
        if (reactdut_parallel<=0) {  // capacitance
            if (capdut_parallel >= 1e-6) {
                printf("Parallel Capacitance (Cp) : %.3lf uF\n", capdut_parallel * 1e6);
            } else {
                printf("Parallel Capacitance (Cp) : %.3lf nF\n", capdut_parallel * 1e9);
            }
        } else { // inductance
            printf("Parallel Inductance  (Lp) : %.3lf uH\n", inddut_parallel * 1e6);
        }


        //reset_dsp_settings();


    } 

    dsp_close(); // close the I2C resource for the DSP

    return(0);
}


