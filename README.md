# Projects

Following are the instructions used to implement the real-time task program:

    • Download the following files from the directory
1. all.c
	2. Makefile
	3. Report


**Code is not using an Input file. As per our understanding that was not needed. We have hard-coded the inputs as per the assignment document. (More details around this in project report)

For Linux Host: make

For Galileo2 – make EXECUTION_ENV=GALILEO2

gcc cross-compiler path: /opt/iot-devkit/1.7.2/sysroots/x86_64-pokysdk-linux/usr/bin/i586-poky-linux/i586-poky-linux-gcc (it may differ on your system)

Before you run the program:

export MOUSE_INPUT=’/dev/input/mice’
If you have another mouse input file, please change accordingly

export LOG_PATH=‘/home’
The file will be created at path ‘/home/log.txt’ 

























