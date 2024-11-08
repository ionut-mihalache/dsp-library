CC=gcc
DEBUG_DEFINES=-D__COMPILE_MODE_DEBUG__
INCLUDES=

build: dsp.o
	$(CC) $(DEBUG_DEFINES) $(INCLUDES) dsp.o -shared -o libdsp.so

dsp.o: dsp.c dsp.h dsp-service.h dsp-client.h log/log.h
	$(CC) $(DEBUG_DEFINES) $(INCLUDES) -c dsp.c -fPIC

clean:
	rm *.o *.so
