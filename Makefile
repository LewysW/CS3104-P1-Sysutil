CFLAGS = -std=gnu99

myls: myls.o
	gcc -std=gnu99 -Wall -Wextra -g myls.c -o myls

clean:
	rm myls *.o
