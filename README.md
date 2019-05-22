# MouseGes


Steps to install and run **MouseGes** on your machine.

Prerequisites - 
1. Linux Kernel (tested on 4.4.0).
2. Sudo access to kernel.
3. Python 2.7 

To install MouseGes, download and unzip the folder.

Open `MouseGes.c` and do the following changes - 

* In line 39, change `/home/abhigyan/projects/` to the path of your MouseGes folder directory.
* In line 49, change `/home/abhigyan/` to your `HOME` path.

Then follow the given commands -
 
 ```
 make
 sudo insmod MouseGes.ko
 ```

### Usage

There are 5 gestures that are currently supported - 
* Horizontal Swipe
* Vertical Swipe
* Right Diagonal
* Left Diagonal
* V swipe

To set a gesture to opening a parituclar application - open `configuration` file. Each line contains the name of the executable of the application. The gesture order mentioned above is the corresponds to the line order of executables in the `configuration` file.

**Note** - Make sure the executables are present in `/usr/bin`.

After saving the `configuration` file, run `python config.py`.

To start recording of gesture, do three left click taps anywhere and then start the trackpad movement. Click anywhere to end gesture recording. Once completed, corresponding application to gesture would open.


The module would be removed by default on rebooting.

To load module on boot 

1. Edit `/etc/modules` file to add our module name i.e MouseGes.
2. Copy *MouseGes.ko* to `/lib/modules/$(uname -r)/kernel/drivers/input/mouse/`
3. To check if the module loaded on reboot run `lsmod | grep MouseGes`

