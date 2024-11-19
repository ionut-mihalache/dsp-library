CC=gcc
DEBUG_DEFINES=-D__COMPILE_MODE_DEBUG__
INCLUDES=
OPTIONS=-Wall -Wextra

build: dsp.o dsp-service.o dsp-client.o
	$(CC) $(DEBUG_DEFINES) $(INCLUDES) dsp.o dsp-service.o dsp-client.o -shared -o libdsp.so

dsp.o: dsp.c dsp.h log/log.h
	$(CC) $(OPTIONS) $(DEBUG_DEFINES) $(INCLUDES) -c dsp.c -fPIC

dsp-service.o: dsp-service.c dsp-service.h
	$(CC) $(OPTIONS) $(DEBUG_DEFINES) $(INCLUDES) -c dsp-service.c -fPIC

dsp-client.o: dsp-client.c dsp-client.h
	$(CC) $(OPTIONS) $(DEBUG_DEFINES) $(INCLUDES) -c dsp-client.c -fPIC

clean:
	rm *.o *.so

# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/ionut/git-repos/dsp-library
