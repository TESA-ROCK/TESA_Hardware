# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g
LIBS = -lm -lpthread -lcurl -lpaho-mqtt3c -lcjson

# Target application name
TARGET = mqtt_sub_app

# Source files and object files
SRCS = mqtt_sub_app.c mqtt_sub_thr.c
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
