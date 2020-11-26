target:
	gcc -Wall -g -o main main.c -lm -lpthread -lgomp 
	gcc -Wall -g -o runner runner.c -lm -lpthread


