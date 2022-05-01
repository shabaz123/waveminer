/*****************************************************
 * eeload - DSP Board EEPROM Programmer
 * rev 2 - april 2022 - shabaz
 *
 * After use, remember to power-cycle the DSP board
 * for the EEPROM to be read by the DSP.
 *
 * Example:
 *      ./eeload -p tone_app.bin
 * (then power-cycle the DSP board)
 *****************************************************/

// includes
 #include <stdio.h>
 #include "options.h"
 #include "dsputil.h"
 #include <string.h>

// defines 


// ************* main program **********************
 int
 main(int argc, char **argv)
 {
    char* sw; // used for command-line arguments
    char fname[256];
    char do_load=0;


    // read in the command-line arguments
    sw = getCmdOption(argv, argv + argc, "-p");
    if (sw) {
        sscanf(sw, "%s", fname);
        if (strstr((const char*)fname, ".bin")==NULL) {
            strcat(fname, ".bin");
        }
        printf("Requested file to upload: '%s'\n", fname);
        do_load=1;
    }


    if (do_load) ee_prog(fname);

    return(0);
 }

