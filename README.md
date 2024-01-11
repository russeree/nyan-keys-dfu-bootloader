# Nyan Keys - STM32F723 DFU Bootloader (22KB)

## Overview
The STM32F723 can't use the USB2.0 High Speed PHY with the built in Bootloader _(Boot0 = High)_. This is very problematic becuase it forces the engineer to either use USB 2.0 FS to get a built in DFU mode or build a USB 2.0 High Speed compatible bootloader with DFU support.

This repo includes the source code to build the bootloader ```make``` or you can use the include binary in the ```binaries``` directory.

## Implementation
Fist to use this successfully you need to change the option bytes on your STM32F723. You will be pushing the BOOT0 (_LOW_) entry point up by 32KB. Then setting the BOOT0 (_HIGH_) entry point to the initial entry point of ```0x08000000```.
This is done because of the awkward layout of memory sectors on the STM32F723 [16KB,16KB,16KB,16KB,64KB,128KB,128KB,128KB]. The best optimization of these sectors is to place the 22KB bootloader in the first 32KB of space. This leaves 
480KB for the rest of the firmware. 

__DANGER__ These sectors are protected in DFU mode so it's very unlikely that you will be able to overwrite these sectors. However you can most certianly use the ST-LINK programmer and wipe out the entire memory contents of the MCU. 
I don't belive in locking down my products, so if you do this sorry. I can help you fix this if you still have the STM32 programmer or a STM32 Discovery board to bootstrap the ST-Link off of.

### Step 1 - A nice circuit

The schematic below links pin E0 to BOOT0 with a big 22uF capacitor on the line, the theory here is that you set EO to output mode and write a high bit. Then use current from that pin to charge the cape for ```HAL_Delay(1000)``` and then use a call to 
```NVIC_SystemReset()``` to reset the STM32, the resistor to ground limits the flow of current so that the line holds a logic high value for the time needed for BOOT0 to read high after the reset and activate the Nyan Keys DFU Bootloader.

![image](https://github.com/russeree/nyan-keys-dfu-bootloader/assets/3104223/854996d2-f43a-4cde-b5b9-398ca2b85385)

Now you need to setup your option bytes like such ```BOOT_ADDR0``` = ```0082``` and ```BOOT_ADDR1``` = ```0080``` 

![Screenshot from 2024-01-11 02-58-39](https://github.com/russeree/nyan-keys-dfu-bootloader/assets/3104223/3eba2ae2-3c05-48e2-bdc3-c7baeb3b5301)

After you are done with that step clone this repo - __NYAN KEYS Keyboards ONLY__
```
git clone https://github.com/russeree/nyan-keys-dfu-bootloader.git
cd nyan-keys-dfu-bootloader
make clean; make -j16; st-flash --connect-under-reset write build/nyan_keys_dfu.bin 0x08000000
cd ..
git clone https://github.com/russeree/nyan-keys-stm32-firmware
cd nyan-keys-stm32-firmware
make clean; make -j16; st-flash --connect-under-reset write build/nyan_keys.bin 0x08008000
```

Now you can visit [the Nyan Keys web configurator](https://russeree.github.io/)

connect to your Nyan Keys and then use the comman ```dfu-mode```

![image](https://github.com/russeree/nyan-keys-dfu-bootloader/assets/3104223/3d40bfd0-05a9-49ec-80c7-894d1a946f25)

Now you can use the DFU mode - Check with ```lsusb``` on linux you should see ```483:df11 STMicroelectronics STM Device in DFU Mode```

Now ...

![image](https://github.com/russeree/nyan-keys-dfu-bootloader/assets/3104223/c56c01a6-1d78-4bf8-ae57-59b5ff0fcbfe)





