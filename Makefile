CFLAGS = -std=c99

myls: myls.o
	gcc -std=gnu99 -Wall -Wextra -g myls.c -o myls -std=c99

clean:
	rm myls *.o
