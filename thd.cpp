/*****************************************************
 * thd - Total Harmonic Distortion Tool
 * rev 1 - april 2022 - shabaz
 *
 * uses thd.bin
 * supports THD measurements at
 * 20, 50, 100, 200, 500 and 1000 Hz.
 * first program the DSP board EEPROM using:
 * ./eeload -p thd.bin
 * then power-cycle the DSP board for the EEPROM
 * to be read by the DSP.
 *
 * The parameter -i is an index into the allowed frequencies.
 * the value is between 1 and 6, for
 * 20, 50, 100, 200, 500 and 1000 Hz
 * Example to just display the allowed index values:
 *      ./thd -q
 * Example to generate 1000 Hz and read the THD in dB:
 *      ./thd -i 6 -d
 * Example to set the source amplitude and read the THD in percent:
 *      ./thd -i 6 -a 0.5 -c
 * Example to use M2M mode (less verbose output):
 *      ./freqrest -i 6 -a 1.0 -c -m
 *****************************************************/

// includes
 #include <stdio.h>
 #include <math.h>
 #include "options.h"
 #include "dsputil.h"
 #include "i2cfunc.h" // so we can use the delay_ms function
 #include "thd.h"

// defines

// consts used by thd.bin
const int SIN_ADDR = 0x0000;
const int AMP_ADDR = 0x0003;
const int LEVEL_ADDR = 0x081a; // level_addr should be 0x081a or 0x081b for ADAU1401 DSP
const int LEVEL_NODE = 0x019e; // node is a 16-bit value
const int LEVEL_NODE2 = 0x01da; // node is a 16-bit value
const int FILTER_NODE = 0x0004; // address of first filter node

const int freqidx[] = {20, 50, 100, 200, 500, 1000, 0};
const int TOTHARM = 7; // total of 7 measurements including fundamental

// each row contains coeff b0, b1, b2, a1, a2:
const double peakfiltcoeff[6][7*5] = {
    {    /* 20 Hz and harmonics: */
        3.739851385908821E-05, 0.000000000000000E+00, -3.739851385908821E-05, -1.999918349340577E+00, 9.999252029722818E-01,
        7.479423062972668E-05, 0.000000000000000E+00, -7.479423062972668E-05, -1.999822998084116E+00, 9.998504115387405E-01,
        1.121871507301364E-04, 0.000000000000000E+00, -1.121871507301364E-04, -1.999713947908351E+00, 9.997756256985397E-01,
        1.495772745788715E-04, 0.000000000000000E+00, -1.495772745788715E-04, -1.999591200584700E+00, 9.997008454508423E-01,
        1.869646025939309E-04, 0.000000000000000E+00, -1.869646025939309E-04, -1.999454757978248E+00, 9.996260707948121E-01,
        2.243491351938687E-04, 0.000000000000000E+00, -2.243491351938687E-04, -1.999304622047726E+00, 9.995513017296123E-01,
        2.617308727964618E-04, 0.000000000000000E+00, -2.617308727964618E-04, -1.999140794845490E+00, 9.994765382544071E-01
    },
    {    /* 50 Hz and harmonics: */
        9.349104023748112E-05, 0.000000000000000E+00, -9.349104023748112E-05, -1.999770185252628E+00, 9.998130179195250E-01,
        1.869646025939309E-04, 0.000000000000000E+00, -1.869646025939309E-04, -1.999454757978248E+00, 9.996260707948121E-01,
        2.804206936044551E-04, 0.000000000000000E+00, -2.804206936044551E-04, -1.999053747684436E+00, 9.994391586127911E-01,
        3.738593197999407E-04, 0.000000000000000E+00, -3.738593197999407E-04, -1.998567187536018E+00, 9.992522813604001E-01,
        4.672804877093872E-04, 0.000000000000000E+00, -4.672804877093872E-04, -1.997995114352963E+00, 9.990654390245812E-01,
        5.606842038575754E-04, 0.000000000000000E+00, -5.606842038575754E-04, -1.997337568608134E+00, 9.988786315922848E-01,
        6.540704747675097E-04, 0.000000000000000E+00, -6.540704747675097E-04, -1.996594594424869E+00, 9.986918590504650E-01
    },
    {    /* 100 Hz and harmonics: */
        1.869646025939309E-04, 0.000000000000000E+00, -1.869646025939309E-04, -1.999454757978248E+00, 9.996260707948121E-01,
        3.738593197999407E-04, 0.000000000000000E+00, -3.738593197999407E-04, -1.998567187536018E+00, 9.992522813604001E-01,
        5.606842038575754E-04, 0.000000000000000E+00, -5.606842038575754E-04, -1.997337568608134E+00, 9.988786315922848E-01,
        7.474393069581975E-04, 0.000000000000000E+00, -7.474393069581975E-04, -1.995766239574418E+00, 9.985051213860836E-01,
        9.341246812443194E-04, 0.000000000000000E+00, -9.341246812443194E-04, -1.993853597180013E+00, 9.981317506375114E-01,
        1.120740378809382E-03, 0.000000000000000E+00, -1.120740378809382E-03, -1.991600096445757E+00, 9.977585192423812E-01,
        1.307286451698753E-03, 0.000000000000000E+00, -1.307286451698753E-03, -1.989006250568617E+00, 9.973854270966025E-01
    },
    {    /* 200 Hz and harmonics: */
        3.738593197999407E-04, 0.000000000000000E+00, -3.738593197999407E-04, -1.998567187536018E+00, 9.992522813604001E-01,
        7.474393069581975E-04, 0.000000000000000E+00, -7.474393069581975E-04, -1.995766239574418E+00, 9.985051213860836E-01,
        1.120740378809382E-03, 0.000000000000000E+00, -1.120740378809382E-03, -1.991600096445757E+00, 9.977585192423812E-01,
        1.493762951909172E-03, 0.000000000000000E+00, -1.493762951909172E-03, -1.986072630812219E+00, 9.970124740961817E-01,
        1.866507442035759E-03, 0.000000000000000E+00, -1.866507442035759E-03, -1.979188644323376E+00, 9.962669851159285E-01,
        2.238974264193061E-03, 0.000000000000000E+00, -2.238974264193061E-03, -1.970953863639003E+00, 9.955220514716139E-01,
        2.611163832610841E-03, 0.000000000000000E+00, -2.611163832610841E-03, -1.961374935822173E+00, 9.947776723347783E-01
    },
    {    /* 500 Hz and harmonics: */
        9.341246812443194E-04, 0.000000000000000E+00, -9.341246812443194E-04, -1.993853597180013E+00, 9.981317506375114E-01,
        1.866507442035759E-03, 0.000000000000000E+00, -1.866507442035759E-03, -1.979188644323376E+00, 9.962669851159285E-01,
        2.797154775903699E-03, 0.000000000000000E+00, -2.797154775903699E-03, -1.956083744344029E+00, 9.944056904481926E-01,
        3.726073146169351E-03, 0.000000000000000E+00, -3.726073146169351E-03, -1.924653432013082E+00, 9.925478537076613E-01,
        4.653268986126946E-03, 0.000000000000000E+00, -4.653268986126946E-03, -1.885047617782994E+00, 9.906934620277461E-01,
        5.578748699221037E-03, 0.000000000000000E+00, -5.578748699221037E-03, -1.837450881542105E+00, 9.888425026015579E-01,
        6.502518659224243E-03, 0.000000000000000E+00, -6.502518659224243E-03, -1.782081619591845E+00, 9.869949626815515E-01
    },
    {    /* 1000 Hz and harmonics: */
        1.866507442035759E-03, 0.000000000000000E+00, -1.866507442035759E-03, -1.979188644323376E+00, 9.962669851159285E-01,
        3.726073146169351E-03, 0.000000000000000E+00, -3.726073146169351E-03, -1.924653432013082E+00, 9.925478537076613E-01,
        5.578748699221037E-03, 0.000000000000000E+00, -5.578748699221037E-03, -1.837450881542105E+00, 9.888425026015579E-01,
        7.424585210413115E-03, 0.000000000000000E+00, -7.424585210413115E-03, -1.719191048759317E+00, 9.851508295791738E-01,
        9.263633317018072E-03, 0.000000000000000E+00, -9.263633317018072E-03, -1.572008011711891E+00, 9.814727333659639E-01,
        1.109594318993079E-02, 0.000000000000000E+00, -1.109594318993079E-02, -1.398521529026574E+00, 9.778081136201384E-01,
        1.292156453915949E-02, 0.000000000000000E+00, -1.292156453915949E-02, -1.201790557829667E+00, 9.741568709216810E-01
    }
};




// externs
extern char do_log;

// ************* main program **********************
 int
 main(int argc, char **argv)
 {
    char* sw; // used for command-line arguments
    char do_db = 0;
    char do_percent = 0;
    double v[TOTHARM];
    int fhertz = 0;
    int fidx = 0;
    double amp = 0;
    char do_amp=0;
    char do_freq=0;
    char do_test=0;
    int testharmonic=0;
    double converted[TOTHARM];
    int i, j;
    double thd_result;
    double thd_result_db, thd_result_percent;
    double s;
    int invalid=0;
    int k;
    char logstate;

    // read in the command-line arguments
    if (cmdOptionExists(argv, argv + argc, "-m")) {
        // m2m mode
        do_log = 0;
    }

    if (cmdOptionExists(argv, argv + argc, "-q")) {
        i=0;
        printf("Use parameter -i to specify a frequency:\n");
        while (freqidx[i]!=0) {
            printf("    -i %d : %d Hz\n", i+1, freqidx[i]);
            i++;
        }
    }

    sw = getCmdOption(argv, argv + argc, "-t"); // test mode
    if (sw) {
        sscanf(sw, "%d", &testharmonic);
        testharmonic--;
        if (testharmonic>0) {
            printf("Test mode. Setting filter to harmonic # %d\n", testharmonic+1);
        } else {
            printf("Test mode. Setting filter to fundamental (harmonic # 1)\n");
        }
        
        do_test=1;
    }

    sw = getCmdOption(argv, argv + argc, "-i");
    if (sw) {
        sscanf(sw, "%d", &fidx);
        fidx--;
        fhertz = freqidx[fidx];
        if (do_log) printf("Setting frequency to %d Hz\n", fhertz);
        do_freq=1;
    }

    sw = getCmdOption(argv, argv + argc, "-a");
    if (sw) {
        sscanf(sw, "%lf", &amp);
        if (do_log) printf("Setting amplitude to %f\n", amp);
        do_amp=1;
    }

    if (cmdOptionExists(argv, argv + argc, "-d")) {
        if (do_log) printf("dB result requested\n");
        do_db=1;
    }

    if (cmdOptionExists(argv, argv + argc, "-c")) {
        if (do_log) printf("percent level requested\n");
        do_percent=1;
    }

    dsp_open(); // create I2C handle for the DSP

    // set up the tone generation
    if (do_amp) set_amp(AMP_ADDR, amp);
    if (do_freq) {
        set_freq(SIN_ADDR, fhertz);
    }

    if (do_test) {
        for (j=0; j<4; j++) { // there are 4 identical filters
            set_gen_2nd_order_filter(FILTER_NODE+(j*5), (double*)(&peakfiltcoeff[fidx][testharmonic*5]));
        }
        delay_ms(900); // wait some time before we take the level reading
        delay_ms(900); // wait some time before we take the level reading
        if (testharmonic==0) {
            v[testharmonic] = readback(LEVEL_ADDR, LEVEL_NODE);
        } else {
            v[testharmonic] = readback(LEVEL_ADDR, LEVEL_NODE2) / 10000.0;
        }
        converted[testharmonic] = ms_to_rms(v[testharmonic]);
        printf("value (RMS) of harmonic # %d is %lf\n", testharmonic+1, converted[testharmonic]);
        dsp_close(); // close the I2C resource for the DSP
        exit(0);
    }

    logstate = do_log;
    if (do_db || do_percent) {
        for (i=0; i<TOTHARM; i++) { // do fundamental and each harmonic
            for (j=0; j<4; j++) { // there are 4 identical filters
                if (j>0) do_log=0; // too much output so lets reduce it
                set_gen_2nd_order_filter(FILTER_NODE+(j*5), (double*)(&peakfiltcoeff[fidx][i*5]));
                do_log=logstate;
            }
            delay_ms(900); // wait some time before we take the level reading
            if (i==0) { 
                v[i] = readback(LEVEL_ADDR, LEVEL_NODE);
                converted[i] = ms_to_rms(v[i]); // store the RMS for fundamental
            } else {
                for (k=0; k<10; k++) {
                    v[i] = readback(LEVEL_ADDR, LEVEL_NODE2) / 10000.0;
                    converted[i] = ms_to_rms(v[i]); // store the RMS for each harmonic
                    if (converted[i]<0.07999) {
                        break;
                    } else {
                        if (do_log) printf("Out of range. Reattempt..");
                        delay_ms(900);
                    }
                }
            }
            if ((i>0) && (converted[i]>=0.07999)) {
                invalid = 1; // out of range
                printf("invalid!\n");
            }
        }

        s = 0;
        for (i=1; i<TOTHARM; i++) { // sum up the unwanted squares
            s = s + pow(converted[i], 2);
        }
        thd_result = sqrt(s) / converted[0]; // ratio result
        thd_result_db = 20 * log10(thd_result); // dB result
        thd_result_percent = thd_result * 100.0; // percent result

        if (do_log) {
            printf("values (RMS) are %lf, %lf, %lf, %lf, %lf, %lf, %lf\n", 
                    converted[0], converted[1], converted[2], converted[3], converted[4], converted[5], converted[6]);
            printf("thd is %lf percent (%lf dB)\n", thd_result_percent, thd_result_db);
        } else {
            // m2m mode
            if (do_db) {
                printf("%lf\n", thd_result_db);
            } else if (do_percent) {
                printf("%lf\n", thd_result_percent);
            }
        }
        if (invalid) {
            printf("*** ERROR - invalid results ***\n");
        }
    } 

    dsp_close(); // close the I2C resource for the DSP

    return(0);
 }


