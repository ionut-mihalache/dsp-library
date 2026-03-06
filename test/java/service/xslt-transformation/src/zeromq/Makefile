all: build

CLASSPATH=../../lib/jeromq-0.6.0.jar:../../lib/saxon-he-12.5.jar:../../lib/xmlresolver-6.0.11.jar

build:
	javac -cp $(CLASSPATH) -d . Main.java

run:
	exec java -cp ".:$(CLASSPATH)" zeromq.Main $(PAYLOAD_TYPE) $(MSG_NUMBER)

clean:
	rm zeromq/*.class