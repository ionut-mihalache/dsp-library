CLASSPATH=./src:../../jar/jna-5.15.0.jar
LIBPATH=java.library.path=/workspaces/ionut/Dev\ Containers/dsp-library

.SUFFIXES: .java .class

build:
	javac -cp $(CLASSPATH) src/Main.java

run:
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/workspaces/ionut/Dev\ Containers/dsp-library
	java -cp $(CLASSPATH) Main
