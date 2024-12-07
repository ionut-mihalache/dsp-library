CLASSPATH=./src:../../jar/jna-5.15.0.jar:../../jar/saxon-he-12.5.jar:../../jar/xmlresolver-6.0.11.jar

.SUFFIXES: .java .class

all: build

build:
	javac -cp $(CLASSPATH) src/Main.java

run:
	java -cp $(CLASSPATH) Main

clean:
	rm src/*.class

clean-log:
	rm *.log

# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/user/dsp-library
