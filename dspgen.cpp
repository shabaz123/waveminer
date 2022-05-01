/*****************************************************
 * dspgen - DSP Signal Generator
 * rev 2 - april 2022 - shabaz
 *
 * uses tone_app.bin
 * first program the DSP board EEPROM using:
 * ./eeload -p tone_app.bin
 * then power-cycle the DSP board for the EEPROM
 * to be read by the DSP.
 *
 * Examples to set frequency and amplitude:
 *      ./dspgen -f 1000
 *      ./dspgen -a 0.5
 *****************************************************/

// includes
 #include <stdio.h>
 #include "options.h"
 #include "dsputil.h"

// defines

// consts used by tone_app.bin
const int SIN_ADDR = 0x0000;
const int AMP_ADDR = 0x0003;


// ************* main program **********************
 int
 main(int argc, char **argv)
 {
    char* sw; // used for command-line arguments
    int fhertz = 0;
    double amp = 0;
    char do_freq=0;
    char do_amp=0;

    // read in the command-line arguments
    sw = getCmdOption(argv, argv + argc, "-f");
    if (sw) {
        sscanf(sw, "%d", &fhertz);
        printf("Setting frequency to %d Hz\n", fhertz);
        do_freq=1;
    }

    sw = getCmdOption(argv, argv + argc, "-a");
    if (sw) {
        sscanf(sw, "%lf", &amp);
        printf("Setting amplitude to %f\n", amp);
        do_amp=1;
    }


    dsp_open(); // create I2C handle for the DSP

    if (do_freq) set_freq(SIN_ADDR, fhertz);
    if (do_amp) set_amp(AMP_ADDR, amp);

    dsp_close(); // close the I2C resource for the DSP

    return(0);
 }


