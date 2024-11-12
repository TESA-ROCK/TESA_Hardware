# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g
LIBS = -lm -lreadline -lpthread

# Target application name
TARGET = thr_app

# Source files and object files
SRCS = thr_app.c front_thr.c mid_thr.c end_thr.c
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Rule to link object/lib files
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# Rule to compile .c to .o files
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
