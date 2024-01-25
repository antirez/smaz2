all: smaz2 fuzzing

smaz2: smaz2.c example.c
	$(CC) -O2 -Wall -W --pedantic smaz2.c example.c -o smaz2

fuzzing: smaz2.c fuzzing.c
	$(CC) -O2 -Wall -W --pedantic smaz2.c fuzzing.c -o fuzzing

clean:
	rm -f smaz2 fuzzing
