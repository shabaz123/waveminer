dspgen
======

This repository contains software for operating a Digital Signal Processor (DSP) board connected to a Raspberry Pi 4.

The repository contains code for running on the Pi, as well as binary applications for installing on the DSP board.

Typically, the procedure will be:

(1) Build the Pi code by simply typing **make**

(2) Decide on what application you wish to run on the DSP board, depending on your use-case. For instance, if you wish to generate tones, then select the **tone_app.bin** application. 

(2) Transfer the selected application to the DSP board by using the **eeload** program described below, for instance by typing **./eeload -p tone_app.bin**

(3) Control the selected application by running the associated program on the Pi. For instance, the **tone_app.bin** application is controlled by the Pi using a Pi program called **dspgen** as described further below. For instance, to set the output frequency to 1000 Hz, type **dspgen -f 1000**

Applications List
-----------------

This is a brief list of the current applications. Each one is described in more detail further below.


    DSP Application     Associated Pi Software
    ----------------    ----------------------
    tone_app.bin        dspgen


Building the Software
---------------------

To build the software, type:

    make

The following software items are available:

eeload
------

Use the **eeload** program to transfer DSP binary applicatons to the DSP board EEPROM chip.

Example:

    ./eeload -p dsp_app_name.bin

After the eeload program has been used, power-cycle the DSP board for the EEPROM to be read by the DSP.

dspgen
------

Use the dspgen program to control the **tone_app.bin** DSP application.

Example:

    ./eeload -p tone_app.bin
    <<power-cycle the DSP board>>
    ./dspgen -f 1000
    ./dspgen -a 0.5


The above commands will upload the tone_app.bin application to the DSP, and will then control the tone frequency and amplitude, by setting them to 1000 Hz and 0.5 (the range is 0 to 1) respectively.



