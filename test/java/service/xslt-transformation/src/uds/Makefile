all: build

CLASSPATH=../../lib/saxon-he-12.5.jar:../../lib/xmlresolver-6.0.11.jar

build:
	javac -cp $(CLASSPATH) -d . Main.java

run:
	java -cp ".:../../lib/saxon-he-12.5.jar:../../lib/xmlresolver-6.0.11.jar" uds.Main

clean:
	rm uds/*.class