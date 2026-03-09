all: build throughput-build

CLASSPATH=../../lib/jeromq-0.6.0.jar:../../lib/saxon-he-12.5.jar:../../lib/xmlresolver-6.0.11.jar

build:
	javac -cp $(CLASSPATH) -d . Main.java

throughput-build:
	javac -cp $(CLASSPATH) -d . throughput/Main.java

run:
	exec java -cp ".:$(CLASSPATH)" zeromq.Main $(PAYLOAD_TYPE) $(MSG_NUMBER)

throughput-run:
	exec java -cp ".:$(CLASSPATH)" zeromq.throughput.Main $(PAYLOAD_TYPE) $(MSG_NUMBER)

clean:
	rm zeromq/throughput/*.class
	rm zeromq/*.class
