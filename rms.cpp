/*****************************************************
 * rms - RMS Tool
 * rev 1 - april 2022 - shabaz
 *
 * uses rms.bin
 * The rms tool can be used for AC RMS
 * measurements with optional filters applied.
 * The available high pass filters are:
 * 0=Bypass, 1=10 Hz, 2=20 Hz, 3=100 Hz 
 * The available low pass filters are:
 * 1=100 Hz, 2=1000 Hz, 3=10000 Hz, 4=20000 Hz, 0=Bypass.
 * First program the DSP board EEPROM using:
 * ./eeload -p rms.bin
 * then power-cycle the DSP board for the EEPROM
 * to be read by the DSP.
 *
 * Example to configure high-pass at 10 Hz, and low-pass at 10000 Hz:
 *      ./rms -h 1 -l 3
 * Example to read the RMS value in mV:
 *      ./rms -v
 * Example to read the RMS value in mV in M2M mode (less output):
 *      ./rms -v -m
 * Example to read the unfiltered RMS value in mV:
 *      ./rms -u
 *****************************************************/

// includes
 #include <stdio.h>
 #include "options.h"
 #include "dsputil.h"

// defines
#define LOW 0
#define HIGH 1

// consts used by rms.bin
// const int SIN_ADDR = 0x0000; // future option to adjust the frequency
const int DFILTER_NODE1 = 0x0006; // address of first filter node
const int DFILTER_NODE2 = 0x0015; // address of first filter node
const int LEVEL_ADDR = 0x081a; // level_addr should be 0x081a or 0x081b for ADAU1401 DSP
const int LEVEL_NODE = 0x028a; // node is a 16-bit value
const int LEVEL_NODE_AMP = 0x031e; // node is a 16-bit value

// each row contains 15 coeff.
const double buttercoeff[7][15] = {
    { 0.99873673915863, -1.99747359752655, 0.99873673915863, 1.99747264385223, -0.997474431991577, 0.999074816703796, -1.99814963340759, 0.999074816703796, 1.99814879894257, -0.99815046787262, 0.999660849571228, -1.99932181835175, 0.999660849571228, 1.99932098388672, -0.999322652816772 }, /* 10 Hz HP */
    { 0.997475862503052, -1.9949517250061, 0.997475862503052, 1.994948387146, -0.9949551820755, 0.99815046787262, -1.99630105495453, 0.99815046787262, 1.99629759788513, -0.996304392814636, 0.999321103096008, -1.99864232540131, 0.999321103096008, 1.99863886833191, -0.998645782470703 }, /* 20 Hz HP */
    { 0.98747193813324, -1.97494399547577, 0.98747193813324, 1.97485935688019, -0.975028514862061, 0.990786671638489, -1.98157334327698, 0.990786671638489, 1.98148846626282, -0.981658339500427, 0.996580958366394, -1.9931617975235, 0.996580958366394, 1.99307644367218, -0.993247151374817 }, /* 100 Hz HP */
    { 4.23192977905273E-05, 8.46385955810547E-05, 4.23192977905273E-05, 1.97485935688019, -0.975028514862061, 4.24385070800781E-05, 8.48770141601563E-05, 4.24385070800781E-05, 1.98148846626282, -0.98165833950042, 4.26769256591797E-05, 8.53538513183594E-05, 4.26769256591797E-05, 1.99307644367218, -0.993247151374817 }, /* 100 Hz LP */
    { 0.00379860401153564, 0.00759732723236084, 0.00379860401153564, 1.76088035106659, -0.776074886322021, 0.00391614437103271, 0.00783228874206543, 0.00391614437103271, 1.81534111499786, -0.831005573272705, 0.00413775444030762, 0.00827550888061523, 0.00413775444030762, 1.91809153556824, -0.934642672538757 }, /* 1000 Hz LP */
    { 0.191716551780701, 0.383433103561401, 0.191716551780701, 0.267788290977478, -0.0346543788909912, 0.220194697380066, 0.440389394760132, 0.220194697380066, 0.307566404342651, -0.188345193862915, 0.296472430229187, 0.592944741249084, 0.296472430229187, 0.414110422134399, -0.600000023841858 }, /* 10000 Hz LP */
    { 0.629154443740845, 1.25830888748169, 0.629154443740845, -1.16796636581421, -0.34865140914917, 0.689306139945984, 1.37861227989197, 0.689306139945984, -1.27963244915009, -0.47759222984314, 0.826106667518616, 1.65221321582794, 0.826106667518616, -1.53358972072601, -0.77083683013916} /* 20000 Hz LP */
};

const int htable[]={0, 10, 20, 100};
const int ltable[]={0, 100, 1000, 10000, 20000};

// externs
extern char do_log;

// ************* functions *************************
void
error_oor(void)
{
    dsp_close(); // close the I2C resource for the DSP
    printf("error - parameter(s) out of range\n");
    exit(1);
}

// ************* main program **********************
 int
 main(int argc, char **argv)
 {
    char* sw; // used for command-line arguments

    int fhertz1idx = 0;
    int fhertz2idx = 0;
    char do_freq1=0;
    char do_freq2=0;
    double v, converted;

    if (cmdOptionExists(argv, argv + argc, "-m")) {
        do_log = 0; // M2M mode
    }

    dsp_open(); // create I2C handle for the DSP

    if (cmdOptionExists(argv, argv + argc, "-v")) {
        if (do_log) printf("RMS requested\n");
        // read channel, divide by square of gain
        v = readback(LEVEL_ADDR, LEVEL_NODE_AMP) / (100.0*100.0);
        converted = ms_to_rms(v);
        if (converted<0.06) {
            // low value
        } else { // need to use the larger value
            v = readback(LEVEL_ADDR, LEVEL_NODE);
            converted = ms_to_rms(v);
        }

        // any adjustment
        converted = converted/1.07491;
        // print the values after converting from V to mV
        if (do_log) {
            printf("value (RMS) is %.2lf\n", 
                    converted*1000);
        } else {
            // m2m mode
            printf("%.2lf\n", converted*1000);
        }
        dsp_close(); // close the I2C resource for the DSP
        return(0);
    }

    // read in the command-line arguments
    sw = getCmdOption(argv, argv + argc, "-h");
    if (sw) {
        sscanf(sw, "%d", &fhertz1idx);
        if ((fhertz1idx>3) || (fhertz1idx<0)) error_oor();
        if (do_log) printf("Setting highpass to %d Hz\n", htable[fhertz1idx]);
        do_freq1=1;
    }

    sw = getCmdOption(argv, argv + argc, "-l");
    if (sw) {
        sscanf(sw, "%d", &fhertz2idx);
        if ((fhertz2idx>4) || (fhertz2idx<0)) error_oor();
        if (do_log) printf("Setting lowpass to %d Hz\n", ltable[fhertz2idx]);
        do_freq2=1;
    }

    if (do_freq1) {
        if (fhertz1idx>0) {
            set_dfilter6(DFILTER_NODE1, (double*)(&buttercoeff[fhertz1idx-1][0]));  
        } else { // bypass
            set_dfilter6_bypass(DFILTER_NODE1);
        }
    }

    if (do_freq2) {
        if (fhertz2idx>0) {
            set_dfilter6(DFILTER_NODE2, (double*)(&buttercoeff[fhertz2idx+2][0]));  
        } else { // bypass
            set_dfilter6_bypass(DFILTER_NODE2);
        }
    }

    dsp_close(); // close the I2C resource for the DSP

    if (do_log) printf("Applied.\n");

    return(0);
 }

