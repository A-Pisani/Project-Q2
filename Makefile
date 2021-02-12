target:
	gcc -Wall -g -o main main.c -lm -lpthread -lgomp 
	gcc -Wall -g -o main2 main2.c -lm -lpthread -lgomp 
	gcc -Wall -g -o main3 main3.c -lm -lpthread -lgomp 

	gcc -Wall -g -o runner runner.c -lm -lpthread
	


