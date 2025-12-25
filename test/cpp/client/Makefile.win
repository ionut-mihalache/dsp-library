CC=cl
OPTIONS=/nologo /MD /W3 /Zi /EHsc /O2
INCLUDES=/I ../../../utils /I ../../../utils/macros /I ../../../utils/hashmap /I ../../../utils/exit /I ../../../utils/log /I ../../../utils/locking \
	/I ../../../include /I ../../../service/include /I ../../../client/include

all: build

build: client

client: main.obj
	$(CC) $(OPTIONS) $(INCLUDES) main.obj dsp.lib /Feclient

main.obj: main.cpp
	$(CC) $(OPTIONS) $(INCLUDES) /c main.cpp /Femain.obj

run:
	.\client

clean:
	del *.obj *.exe *.ilk *.pdb
