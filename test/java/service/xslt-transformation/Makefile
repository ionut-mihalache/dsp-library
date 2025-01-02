SOURCE_DIR=src
MAIN_PACKAGE=main
OBJECTS_DIR=bin

CLASSPATH=./$(SOURCE_DIR):./$(OBJECTS_DIR):../../jar/jna-5.15.0.jar:../../jar/saxon-he-12.5.jar:../../jar/xmlresolver-6.0.11.jar

.SUFFIXES: .java .class

all: $(OBJECTS_DIR) build

$(OBJECTS_DIR):
	mkdir $(OBJECTS_DIR)

build:
	javac -cp $(CLASSPATH) -d $(OBJECTS_DIR) $(SOURCE_DIR)/$(MAIN_PACKAGE)/Main.java

run:
	cd $(OBJECTS_DIR)
	java -cp $(CLASSPATH) $(MAIN_PACKAGE)/Main

clean:
	rm -rf $(OBJECTS_DIR)

clean-log:
	rm *.log

# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/user/dsp-library
