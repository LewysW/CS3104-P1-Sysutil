myls: myls.o
	gcc -Wall -Wextra -g myls.c -o myls

clean:
	rm myls *.o
