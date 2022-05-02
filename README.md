
Wave Miner
==========

This repository contains the schematic, PCB Gerber files, bill of materials and code for the Wave Miner Digital Signal Processor (DSP) board, which connects to the Raspberry Pi (Pi 4 was tested; other versions can work with a small code change). 

For simplicity this project uses a ready-made low-cost (about $20 USD) DSP module from AliExpress, it is the green board on the left in the photo below. The remainder parts and blue circuit board should cost less than $30, and assembly time is just a few hours.

<img width="100%" align="left" src="assets\waveminer-pi-annotated.jpg">



What can you do with the Wave Miner?
------------------------------------

Here are some examples; all of these are possible with the hardware and software. Reboot the Wave Miner card (toggle the power button left and then right) after using the **eeload** command:

(1) Sine Wave Generation at (say) 1000 Hz, with an amplitude of 1.0 (1.0 corresponds to 2.6 Vp-p):

    ./eeload -p tone_app.bin
    ./dspgen -f 1000 -a 1.0

(2) Notch filter at (say) 50 Hz:

    ./eeload -p notch4.bin
    ./notch -n 50

(3) Bandpass filter a signal, between 300 Hz and 3000 Hz:

    ./eeload -p filter6.bin
    ./filter -b1 300 -b2 3000

(4) High-pass filter a signal, at 60 Hz:

    ./eeload -p filter6.bin
    ./filter -h 60

(5) Low-pass filter a signal, at 500 Hz:

    ./eeload -p filter6.bin
    ./filter -l 500

(6) Very experimental lock-in amplifier, to identify weak signals

    ./eeload -p lia.bin

The stimulus signal is generated at Output channel 1, and the input is at the channel 1 input connection. The lock-in amplifier result is a tone output for now, audible on output channel 2. The tone increases as the signal is detected by the lock-in amplifier.

(7) Frequency response and RMS measurements:

    ./eeload -p freqresp.bin
    
and

    ./eeload -p rms.bin


See the webdsp project which uses the Wave Miner hardware. It describes how to view the frequency response and RMS values on a web page.


Ordering the PCB
----------------

The zip file prefixed **export-gerber-pi-dsp** can be uploaded to popular PCB manufacturer sites such as JLC PCB or Elecrow, for an instant quote. It shouldn't cost more than about $5 (not including shipping charges).

Ordering the Components
-----------------------

See the wave-miner **bill-of-materials** PDF file for the parts list. A few parts (particularly the DSP board) are available from AliExpress, and the remainder can be obtained from sources such as Farnell/Newark. It is recommended that a ribbon cable with connectors pre-assembled for the Raspberry Pi is purchased from (say) Adafruit, unless you're familiar with how to attach insulation displacement connectors (IDC) to the ribbon cable. Note that the cable is essential, it is not possible to plug the board directly on top of the Pi like 'Pi HAT' cards.

It is also useful to have one BNC-to-BNC cable, and one cable with stereo 3.5mm plug on each end, for experimenting with the Wave Miner.

You will also need a Raspberry Pi. A Pi 4 was tested. It is possible to use a Pi 3 variant, but a code change will be required. No code change is needed if a Pi 4 is being used.

Building the Wave Miner
-----------------------

Nearly all parts are through-hole, so it is easy to solder, since the main part (the DSP module) is ready-made. The few surface mount parts are large and mostly optional. The ferrite choke is hand-wound, but if you don't have the parts (ferrite core and ~ 0.2mm diameter enamelled copper wire) then you could just put a wire short on the PCB.

Plug four jumpers to allow the two channels of input and two channels of output signals to get to the connectors. Plug a fifth jumper to select the power source (for instance select the Pi setting, so that the Pi powers the Wave Miner board).

<img width="100%" align="left" src="assets\pi-dsp-jumpers.png">

Connecting the Wave Miner to the Pi
-----------------------------------

With both the Wave Miner board and the Pi Unpowered, connect the ribbon cable between the Wave Miner and the Pi. You can plug the jumper into the Wave Miner so that the board will be powered from the Pi. Now you can power up the Pi.

Pi Pre-Requisites
-----------------

If you don't have a user account called **pi** (some recent Pi OS builds no longer have a pi user by default) then create a pi user account before proceeding, because some paths in the code are hard-coded for the pi user. You can manually modify the paths, but it is easier to just use a pi user for now.

Ensure, by typing **raspi-config** in a command prompt, that the **I2C** capability on the Pi is enabled. 

Installing the Code
-------------------

The **waveminer** software contains lots of utilities/applications.

Once you have completed the **Pi Pre-Requisites** steps above, type the following on the Pi in a command prompt to install the waveminer software:

    mkdir -p ~/development
    cd ~/development
    git clone https://github.com/shabaz123/waveminer.git
    cd waveminer
    make

Testing It
----------

You can test that the setup is working fine by running an example application:

Switch on the Wave Miner by flicking the toggle switch to the right, which is the **on** position. Then plug in headphones into the front 3.5mm stereo connector.

Type the following in a command prompt on the Pi:

    cd ~development/waveminer
    ./eeload -p tone_app.bin

Reboot the Wave Miner for the code to take effect. To perform the reboot, flick the toggle switch off and on.

Now type the following, and you should hear a continuous 220 Hz tone from the left earpiece:

    ./dspgen -a 0.1 -f 220

If you don't hear anything, you can increase the volume by increasing the **-a** parameter:

    ./dspgen -a 0.4 -f 220

Next Steps
----------

There are some partner web apps for the Wave Miner, to allow you to operate the Wave Miner directly from a web browser.

To install and use the web apps, visit https://github.com/shabaz123/webdsp

When run, the web apps will automatically call the Wave Miner code.

Creating Custom Apps
--------------------

To create your own custom DSP apps, install free-to-use SigmaStudio software. 

The SigmaStudio Automotive and Generic Release SigmaStudio 64 Bit-OS Rev 4.6 is available from the [Analog Devices](https://www.analog.com/en/design-center/evaluation-hardware-and-software/software/ss_sigst_02.html#software-overview) website.






