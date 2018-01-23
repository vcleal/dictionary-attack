all: wordharvest

wordharvest:
	gcc -Wall -o bin/wordharvest wordharvest.c

clean:
	rm bin/wordharvest
