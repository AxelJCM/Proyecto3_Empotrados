# Makefile for V4L2 webcam capture program

# Compiler and flags
CC = gcc
CFLAGS = -Wall -O2 -Iinclude

# Target executable
TARGET = stream

# Source files
SRCS = server.c webcam.c filtros.c

# Output object files
OBJS = $(SRCS:.c=.o)

# Libraries
LIBS = -ljpeg -lm -fopenmp

# Compile target
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# Compile .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LIBS)

# Clean up build files
clean:
	rm -f $(TARGET) $(OBJS)


.PHONY: all clean
