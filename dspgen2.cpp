/*****************************************************
 * dspgen2 - DSP Signal Generator dual output
 * rev 1 - april 2022 - shabaz
 *
 * uses dspgen2.bin
 * first program the DSP board EEPROM using:
 * ./eeload -p dspgen2.bin
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

// consts used by dspgen2.bin
const int SINPHASE_ADDR1 = 0x0000;
const int SINPHASE_ADDR2 = 0x0005;
const int SINPHASE_ADDR[] = {SINPHASE_ADDR1, SINPHASE_ADDR2};

// ************* main program **********************
 int
 main(int argc, char **argv)
 {
    char* sw; // used for command-line arguments
    int i=0;
    int fhertz[2] = {0, 0};
    int ang[2] ={0, 0};
    double amp[2] = {0, 0};
    double a;

    char do_freq[2]={0, 0};
    char do_amp[2]={0, 0};
    char do_phase[2]={0, 0};

    // read in the command-line arguments
    sw = getCmdOption(argv, argv + argc, "-f1");
    if (sw) {
        sscanf(sw, "%d", &fhertz[0]);
        printf("Setting frequency1 to %d Hz\n", fhertz[0]);
        do_freq[0]=1;
    }
    sw = getCmdOption(argv, argv + argc, "-f2");
    if (sw) {
        sscanf(sw, "%d", &fhertz[1]);
        printf("Setting frequency2 to %d Hz\n", fhertz[1]);
        do_freq[1]=1;
    }
    sw = getCmdOption(argv, argv + argc, "-a1");
    if (sw) {
        sscanf(sw, "%lf", &amp[0]);
        printf("Setting amplitude1 to %f Vp-p\n", amp[0]);
        do_amp[0]=1;
    }
    sw = getCmdOption(argv, argv + argc, "-a2");
    if (sw) {
        sscanf(sw, "%lf", &amp[1]);
        printf("Setting amplitude2 to %f Vp-p\n", amp[1]);
        do_amp[1]=1;
    }
    sw = getCmdOption(argv, argv + argc, "-p1");
    if (sw) {
        sscanf(sw, "%d", &ang[0]);
        printf("Setting phase1 to %d deg\n", ang[0]);
        do_phase[0]=1;
    }
    sw = getCmdOption(argv, argv + argc, "-p2");
    if (sw) {
        sscanf(sw, "%d", &ang[1]);
        printf("Setting phase2 to %d deg\n", ang[1]);
        do_phase[1]=1;
    }

    dsp_open(); // create I2C handle for the DSP

    for (i=0; i<2; i++) {
        if (do_freq[i]) set_sinphase_freq(SINPHASE_ADDR[i], fhertz[i]);
        if (do_amp[i]) {
            // convert value between 0.0-2.6Vp-p into a value between 0-1
            a = amp[i] / 2.6;
            set_sinphase_gain(SINPHASE_ADDR[i], a);
        }
        if (do_phase[i]) set_sinphase_phase(SINPHASE_ADDR[i], ang[i]);
    }

    dsp_close(); // close the I2C resource for the DSP

    return(0);
 }


