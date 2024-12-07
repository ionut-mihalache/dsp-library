CC=gcc
DEBUG_DEFINES=-D__COMPILE_MODE_DEBUG__
INCLUDES=
OPTIONS=-Wall -Wextra

build: dsp.o dsp-service.o dsp-client.o commons.o
	$(CC) $(DEBUG_DEFINES) $(INCLUDES) dsp.o dsp-service.o dsp-client.o commons.o -shared -o libdsp.so

dsp.o: dsp.c dsp.h utils/log/log.h utils/commons.h utils/hashmap/hashmap.h utils/macros/macros.h
	$(CC) $(OPTIONS) $(DEBUG_DEFINES) $(INCLUDES) -c dsp.c -fPIC

dsp-service.o: dsp-service.c dsp-service.h dsp.h utils/commons.h utils/hashmap/hashmap.h utils/macros/macros.h
	$(CC) $(OPTIONS) $(DEBUG_DEFINES) $(INCLUDES) -c dsp-service.c -fPIC

dsp-client.o: dsp-client.c dsp-client.h dsp.h utils/commons.h utils/hashmap/hashmap.h utils/macros/macros.h
	$(CC) $(OPTIONS) $(DEBUG_DEFINES) $(INCLUDES) -c dsp-client.c -fPIC

commons.o: utils/commons.c utils/commons.h dsp.h utils/macros/macros.h
	$(CC) -c utils/commons.c -o commons.o -fPIC

clean:
	rm *.o *.so

# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/user/dsp-library
