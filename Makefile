TARGET = dellfancontrol
SRC = main.c dell-bios-fan-control.c
OBJ = $(SRC:.c=.o)

CC = gcc
CFLAGS = -std=gnu11 -pedantic -Wall -Wextra -Wdouble-promotion\
				 -Wfloat-conversion -Wno-error=unused-function\
				 -Wno-error=unused-parameter -Wno-error=unused-variable
LFLAGS =


$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

.PHONY: clean
clean:
	rm $(OBJ) $(TARGET)

