build:
	gcc -c dsp.c -fPIC
	gcc dsp.o -shared -o libdsp.so