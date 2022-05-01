/*****************************************************
 * level - DSP Level Measurement
 * rev 1 - april 2022 - shabaz
 *
 * uses levelread.bin
 * first program the DSP board EEPROM using:
 * ./eeload -p levelread.bin
 * then power-cycle the DSP board for the EEPROM
 * to be read by the DSP.
 *
 * Examples to read the peak level:
 *      ./level -p
 *****************************************************/

// includes
 #include <stdio.h>
 #include "options.h"
 #include "dsputil.h"

// defines

// consts used by levelread.bin
const int LEVEL_ADDR = 0x081a; // level_addr should be 0x081a or 0x081b for ADAU1401 DSP
const int LEVEL_NODE = 0x010a; // node is a 16-bit value

// ************* main program **********************
 int
 main(int argc, char **argv)
 {
    char do_peak=0;
    double v;

    // read in the command-line arguments
    if (cmdOptionExists(argv, argv + argc, "-p")) {
        printf("Reading peak level\n");
        do_peak=1;
    }

    dsp_open(); // create I2C handle for the DSP

    if (do_peak) {
        v = readback(LEVEL_ADDR, LEVEL_NODE); 
        printf("read a value of %lf\n", v);
    } 

    dsp_close(); // close the I2C resource for the DSP

    return(0);
 }


