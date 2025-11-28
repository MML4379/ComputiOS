# ComputiOS
Computer Operating System, something I work on in the free time I have. 

## My goals for this OS
I want this operating system to be able to play Crab Rave on YouTube, whilst rendering a nice looking UI. Quite the ambitious goal but I will get there some day. It will be a 64 bit OS, targeted towards real, modern hardware.

## What I've done so far:
- Bootloader that essentially loads the kernel, sets up protected mode and long mode with paging, and passes control to the kernel. 
- A kernel with some basic interrupt and exception handling, and logging to a serial console.
- A PCI "driver" that prints all of the devices to the console and to the screen
- I've also got PIC, a timer system, and a super basic keyboard driver going.

There's much more to be done, obviously.
