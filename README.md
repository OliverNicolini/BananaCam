# BananaCam

BananaCam is a server wich aim to control camera remotly by using a socket communication.
BananaCam is free of interface and only a CL version controllable through UNIX socket is available.
BananaCam has been written in C and it wrap gphoto2 with new functionnalities and socket control.
An interface can easily control the BananaCam local/remote server by sending specific messages.

## Socket

The UNIX socket are located in /tmp/ under the name of:

camera_control.sock 	    ==> communication socket
camera_control_data.sock    ==> data socket (liveview)

Those path can be easily changed in the file ./headers/camera_control.h

## Protocol Messages

The messages can have the following values:

OK		1
KO		2
WRONG_COMMAND	4
BAD_PARAMETERS	8
WAIT_RESPONSE	16
COMPLETE	32
EXEC		64
INFO		128
VALUE		256

The messages available are composed by:

- Message value
- Command
- Info message
- Parameter(s)

All the differents elements of a messages are separated by a pipe `|`.

example:

64|capture|capture a photo|1|/tmp/|

64		=	EXEC
capture		=	command to execute
capture a photo =	info message to log
1 	  	= 	number of shoots
/tmp/ 	      	=	directory where the image will be saved.

The number of argument can be NULL.

The list of the preimplemented commands and theyr options are:

capture ==> (int)numberOfShots(optional) | (string)pathWhereToSave(optional)
Trigguer the capture and save the photos in a directory.

auto_focus ==> NULL
Trigguer the autofocus if the camera is in AutoFocus mode

liveview ==> NULL
Trigguer (activate/deactivate) the liveview mode sent through the datacomm

liveviewfps ==> (int)numberOfFPS
Set the number of FPS (Frame per Second) of the liveview.

get_liveviewfps ==> NULL
Get the number of FPS of the liveview

defaultpath ==> (string)path
Set the default path of the application where the images will be saved.

get_defaultpath ==> NULL
Get the default path of the application where the images will be saved.

All the others possible messages may depends on the camera, a list of those messages with theyr optionals values are sent at the first connection.
All the messages if non preappended with `get_` are used to set a value, with the exeption of the previous list.
In order to get the values the message command need to be prefixed wit `get_`

example:

64|iso|setting iso|200 ==> setting iso
64|get_iso|getting iso|NULL ==> getting iso

## Compilation

This software can be compiled under Linux or Mac OSX 

make			==> create the executable
make fclean		==> clean the directory and the remove the executable
make clean		==> clean the directory
make debug		==> create a debug build
make re			==> clean and compile again

## Dependency

This software depends on the followin libraries:

for Linux and Mac OSX

libgphoto2
libpthread

for Mac OSX

IOKit framework
CoreFundation framework


This software has only been tested with D800E Nikon camera.
This software is under GPL licence and it is provided as it is without any warranty or support.
