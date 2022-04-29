
Wave Miner
==========

This repository contains the schematic, PCB Gerber files, bill of materials and code for the Wave Miner DSP board, which connects to the Raspberry Pi (Pi 4 was tested; other versions can work with a small code change).

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

Nearly all parts are through-hole, so it is easy to solder. The few surface mount parts are large and mostly optional. The ferrite choke is hand-wound, but if you don't have the parts (ferrite core and ~ 0.2mm diameter enamelled copper wire) then you could just put a wire short on the PCB.

Plug four jumpers to allow the two channels of input and two channels of output signals to get to the connectors. Plug a fifth jumper to select the power source (for instance select the Pi setting, so that the Pi powers the Wave Miner board).

Connecting the Wave Miner to the Pi
-----------------------------------

With both the Wave Miner board and the Pi Unpowered, connect the ribbon cable between the Wave Miner and the Pi. You can plug the jumper into the Wave Miner so that the board will be powered from the Pi. Now you can power up the Pi.

Pi Pre-Requisites
-----------------

If you don't have a user account called **pi** (some recent Pi OS builds no longer have a pi user by default) then create a pi user account before proceeding, because some paths in the code are hard-coded for the pi user. You can manually modify the paths, but it is easier to just use a pi user for now.

Ensure, by typing **raspi-config** in a command prompt, that the **I2C** capability on the Pi is enabled. 

Installing the Code
-------------------

The **waveminer** software contains lots of utilities/applications. Type the following on the Pi in a command prompt to install it:

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

