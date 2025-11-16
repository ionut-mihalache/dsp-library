all: build

CLASSPATH=../../lib/jeromq-0.6.0.jar:../../lib/saxon-he-12.5.jar:../../lib/xmlresolver-6.0.11.jar

build:
	javac -cp $(CLASSPATH) -d . Main.java

run:
	java -cp ".:../../lib/jeromq-0.6.0.jar:../../lib/saxon-he-12.5.jar:../../lib/xmlresolver-6.0.11.jar" zeromq.Main

clean:
	rm zeromq/*.class