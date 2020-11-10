#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <time.h>
#include <string.h>



int main(int argc, char **argv){

    if(argc!=4){
        fprintf (stderr, "Run as: %s p1 p2 p3\n", argv[0]);
        fprintf (stderr, "Where : p1 = graph file name (.gra)\n");
        fprintf (stderr, "        p2 = number of label pairs for each graph vertex\n");
        fprintf (stderr, "        p3 = query file name (.que)\n" );
        exit (1);
    }
  
    //char *str = "gcc -Wall -g -o main main.c -lm -lpthread; ./main xmark_dag_uniq.gra 3 my_xmark.que";
    char str[255];  
    sprintf(str, "./main %s %s %s", argv[1], argv[2], argv[3]);
  
    system(str); 
}