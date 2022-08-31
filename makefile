makeall:
	gcc -std=gnu99 -o myls myls.c -Wall -Werror 

valgrind: 
	valgrind --leak-check=full --show-leak-kinds=all ./myls

clean:
	rm myls