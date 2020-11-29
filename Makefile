target:
	gcc -Wall -g -o main main.c -lm -lpthread -lgomp 
	gcc -Wall -g -o main main2.c -lm -lpthread -lgomp 
	gcc -Wall -g -o main main3.c -lm -lpthread -lgomp 

	gcc -Wall -g -o runner runner.c -lm -lpthread
	


