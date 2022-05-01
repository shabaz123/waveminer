/*****************************************************
 * freqresp - Frequency Response Tool
 * rev 1 - april 2022 - shabaz
 *
 * uses freqresp.bin
 * first program the DSP board EEPROM using:
 * ./eeload -p freqresp.bin
 * then power-cycle the DSP board for the EEPROM
 * to be read by the DSP.
 *
 * Example to generate 1000 Hz and read the peak level:
 *      ./freqresp -f 1000 -p
 * Example to also set the level to 0.5:
 *      ./freqresp -f 1000 -a 0.5 -p
 * Example to use M2M mode (less verbose output):
 *      ./freqrest -f 1000 -a 0.5 -p -m
 *****************************************************/

// includes
 #include <stdio.h>
 #include "options.h"
 #include "dsputil.h"
 #include "i2cfunc.h" // so we can use the delay_ms function

// defines

// consts used by freqresp.bin
const int SIN_ADDR = 0x0000;
const int AMP_ADDR = 0x0003;
const int LEVEL_ADDR = 0x081a; // level_addr should be 0x081a or 0x081b for ADAU1401 DSP
const int LEVEL_NODE = 0x00fe; // node is a 16-bit value

// externs
extern char do_log;

// ************* main program **********************
 int
 main(int argc, char **argv)
 {
    char* sw; // used for command-line arguments
    char do_peak = 0;
    char do_rms = 0;
    char do_dbu = 0;
    double v;
    int fhertz = 0;
    double amp = 0;
    char do_amp=0;
    char do_freq=0;
    double converted;

    // read in the command-line arguments
    if (cmdOptionExists(argv, argv + argc, "-m")) {
        // m2m mode
        do_log = 0;
    }

    sw = getCmdOption(argv, argv + argc, "-f");
    if (sw) {
        sscanf(sw, "%d", &fhertz);
        if (do_log) printf("Setting frequency to %d Hz\n", fhertz);
        do_freq=1;
    }

    sw = getCmdOption(argv, argv + argc, "-a");
    if (sw) {
        sscanf(sw, "%lf", &amp);
        if (do_log) printf("Setting amplitude to %f\n", amp);
        do_amp=1;
    }

    if (cmdOptionExists(argv, argv + argc, "-p")) {
        if (do_log) printf("Peak-to-Peak level requested\n");
        do_peak=1;
    }

    if (cmdOptionExists(argv, argv + argc, "-r")) {
        if (do_log) printf("RMS level requested\n");
        do_rms=1;
    }

    if (cmdOptionExists(argv, argv + argc, "-u")) {
        if (do_log) printf("dBu level requested\n");
        do_dbu=1;
    }

    dsp_open(); // create I2C handle for the DSP

    // set up the tone generation
    if (do_amp) set_amp(AMP_ADDR, amp);
    if (do_freq) set_freq(SIN_ADDR, fhertz);

    delay_ms(600); // wait 100 msec before we take the level reading

    if (do_peak || do_rms || do_dbu) {
        v = readback(LEVEL_ADDR, LEVEL_NODE);
        if (do_peak) {
            converted = ms_to_pp(v);
        } else if (do_rms) {
            converted = ms_to_rms(v);
        } else if (do_dbu) {
            converted = ms_to_dbu(v);
        }
        if (do_log) {
            printf("value is %lf\n", converted);
        } else {
            // m2m mode
            printf("%lf\n", converted);
        }
    } 

    dsp_close(); // close the I2C resource for the DSP

    return(0);
 }


