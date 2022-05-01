NAME = eeload dspgen dspgen2 level freqresp thd notch pitch filter rms
LIB_PATH = /usr/local/lib
OBJ = i2cfunc.o options.o dsputil.o
EXTENSION = .cpp
CC = g++
CFLAGS = -Wall -I/usr/local/include -g
LIBS = -lwiringPi

all: $(NAME)

eeload: eeload.cpp

dspgen: dspgen.cpp

dspgen2: dspgen2.cpp

level: level.cpp

freqresp: freqresp.cpp

thd: thd.cpp

notch: notch.cpp

pitch: pitch.cpp

filter: filter.cpp

rms: rms.cpp

%.o: %$(EXTENSION) $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(NAME): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -rf *.o $(NAME)


