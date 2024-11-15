# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g

# Libraries to link
LIBS = -lm

# Target application name
TARGET = app

# Source files and object files
SRCS = 1_1_Sound_detection.c 1_2_segment_datasize.c 6_2_SendCodeToPi.c 7_3_Classify.c
OBJS = $(SRCS:.c=.o)

# Default target to build the application
all: $(TARGET)

# Rule to link object files and create the executable with libraries
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# Rule to compile .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f $(OBJS) $(TARGET)

# Run the application
run: $(TARGET)
	./$(TARGET)

# Phony targets
.PHONY: all clean run