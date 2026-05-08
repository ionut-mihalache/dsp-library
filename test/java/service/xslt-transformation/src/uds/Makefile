all: build throughput-build

CLASSPATH=../../lib/saxon-he-12.5.jar:../../lib/xmlresolver-6.0.11.jar

build:
	javac -cp $(CLASSPATH) -d . Main.java

throughput-build:
	javac -cp $(CLASSPATH) -d . throughput/Main.java

run:
	exec java -cp ".:$(CLASSPATH)" uds.Main $(PAYLOAD_TYPE)

throughput-run:
	exec java -cp ".:$(CLASSPATH)" uds.throughput.Main $(PAYLOAD_TYPE) $(MSG_NUMBER)

clean:
	rm uds/throughput/*.class
	rm uds/*.class
