#**********************************************************************
# File: Makefile
#
# Description: Generic C project Makefile for linux applications.
#              Assumes a flat directory structure and only one 
#              main function. Change BIN to desired executable name.
#
# Author: Jonathan Lunt (jml6757@rit.edu)
#**********************************************************************

# Local Files
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
BIN  = out

# Externals
GL_LIBS = -lglut -lGL -lGLU -lGLEW
CL_LIBS = -lOpenCL
JP_LIBS = -ljpeg

# Flags
CFLAGS  = -g -Wall

# Rules
all: $(BIN)

clean:
	rm -f *.o $(BIN)

%.o: %.c
	$(CC) $(INC) -c $(CFLAGS) $< -o $@

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(CL_LIBS) $(GL_LIBS) $(JP_LIBS)
