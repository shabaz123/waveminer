/*****************************************************
 * pitch - DSP Pitch Transposer
 * rev 2 - april 2022 - shabaz
 *
 * uses speed.bin
 * first program the DSP board EEPROM using:
 * ./eeload -p speed.bin
 * then power-cycle the DSP board for the EEPROM
 * to be read by the DSP.
 *
 * Example to increase the pitch:
 *      ./pitch -v +0.01
 * Example to make the unmodified signal available on
 * the second channel:
 *      ./pitch -u 1
 * Example to disable the second channel:
 *      ./pitch -u 0
 *****************************************************/

// includes
 #include <stdio.h>
 #include "options.h"
 #include "dsputil.h"

// defines

// consts used by speed.bin
const int PITCH_ADDR = 0x0001;
const int MUTE_ADDR = 0x0004;

// ************* main program **********************
 int
 main(int argc, char **argv)
 {
    char* sw; // used for command-line arguments
    double pval = 0;
    char mute = 0;
    int uval=0;
    char do_pitch=0;
    char do_mutechange=0;

    // read in the command-line arguments
    sw = getCmdOption(argv, argv + argc, "-v");
    if (sw) {
        sscanf(sw, "%lf", &pval);
        printf("Setting pitch trim to %lf\n", pval);
        do_pitch=1;
    }

    sw = getCmdOption(argv, argv + argc, "-u");
    if (sw) {
        sscanf(sw, "%d", &uval);
        if (uval==0) {
            printf("Disabling unmodified output\n");
            mute=1;
            do_mutechange=1;
        } else {
            printf("Enabling unmodified output\n");
            mute=0;
            do_mutechange=1;
        }
    }


    dsp_open(); // create I2C handle for the DSP

    if (do_pitch) set_pitch(PITCH_ADDR, pval);
    if (do_mutechange) set_mute(MUTE_ADDR, mute);

    dsp_close(); // close the I2C resource for the DSP

    return(0);
 }


