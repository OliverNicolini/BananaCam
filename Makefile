##
## This file is part of BananaCam.
##
## BananaCam is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## BananaCam is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with BananaCam.  If not, see <http://www.gnu.org/licenses/>.
##

SRC_DIR		=	./src/

NAME		= 	camera_control_server

UNAME		:=	$(shell uname)

SRC		= 	$(SRC_DIR)main.c			\
			$(SRC_DIR)comm.c			\
			$(SRC_DIR)focus.c			\
			$(SRC_DIR)utils.c			\
			$(SRC_DIR)comm_data.c			\
			$(SRC_DIR)liveview.c			\
			$(SRC_DIR)capture.c			\
			$(SRC_DIR)set_get_localconfig.c		\
			$(SRC_DIR)set_get_remoteconfig.c	\
			$(SRC_DIR)eject.c

ifeq ($(UNAME), Darwin)
SRC		+=	$(SRC_DIR)USBPrivateDataSample.c
endif

ifeq ($(UNAME), Linux)
SRC		+=	$(SRC_DIR)linux_usb.c
endif

INC		=	-I. -I./headers/

OBJ		=	$(SRC:.c=.o)
CC		=	gcc

CFLAGS		=	-O3 -W -Werror

ifeq ($(UNAME), Linux)
CFLAGS          +=      -ansi -pedantic -D_BSD_SOURCE -D_GNU_SOURCE -Wno-unused-result
endif

CDEBUG          =       -g -ggdb
RM		=	rm -Rf
ECHO		=	echo -e

LIB		=	-lgphoto2 -lpthread -ldl -lgphoto2_port

ifeq ($(UNAME), Linux)
LIB		+=	-ludev
endif

ifeq ($(UNAME), Darwin)
LIB		+=	-framework IOKit -framework CoreFoundation
endif

$(NAME)		:	$(OBJ)
			@$(CC) $(INC) $(CFLAGS) -o $(NAME) $(OBJ) $(LIB)
			@$(ECHO) '\033[0;32m> Compiled\033[0m'

clean		:
			-@$(RM) $(OBJ)
			-@$(RM) *~ $(SRC_DIR)*~
			-@$(RM) \#*\#
			@$(ECHO) '\033[0;35m> Directory cleaned\033[0m'

all		:	$(NAME)

fclean		:	clean
			-@$(RM) $(NAME)
			@$(ECHO) '\033[0;35m> Remove executable\033[0m'

re		:	fclean all

debug		:	CFLAGS += $(CDEBUG)

debug		:	fclean $(OBJ)
			@$(CC) $(INC) $(CDEBUG) -o $(NAME) $(OBJ) $(LIB)
			@$(ECHO) '\033[0;32m> Debug compilation: done\033[0m'

.PHONY		:	all clean re fclean debug

.c.o		:	
			@$(CC) $(INC) $(CFLAGS) -o $@ -c $<
			@$(ECHO) 'Compiling '$<