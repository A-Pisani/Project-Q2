#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <time.h>
#include <string.h>

int menu(void);

int main(int argc, char **argv){

    if(argc!=4){
        fprintf (stderr, "Run as: %s p1 p2 p3\n", argv[0]);
        fprintf (stderr, "Where : p1 = graph file name (.gra)\n");
        fprintf (stderr, "        p2 = number of label pairs for each graph vertex\n");
        fprintf (stderr, "        p3 = query file name (.que)\n" );
        exit (1);
    }

    int choice = menu();

    //char *str = "gcc -Wall -g -o main main.c -lm -lpthread; ./main xmark_dag_uniq.gra 3 my_xmark.que";
    char str[255]; 
    if(choice==1){
       sprintf(str, "./main %s %s %s", argv[1], argv[2], argv[3]);
       system(str);  
    } else if(choice ==2 ){
        sprintf(str, "./main2 %s %s %s", argv[1], argv[2], argv[3]);
        system(str);
    }else if(choice ==3){
        sprintf(str, "./main3 %s %s %s", argv[1], argv[2], argv[3]);
        system(str);
    }else{
        sprintf(str, "./main %s %s %s", argv[1], argv[2], argv[3]);
        system(str);
        sprintf(str, "./main2 %s %s %s", argv[1], argv[2], argv[3]);
        system(str);
        sprintf(str, "./main3 %s %s %s", argv[1], argv[2], argv[3]);
        system(str);
    }
    

     

     
}

int menu(void){
    int choice;

    do {
        printf("Welcome to our implementation of GRAIL, choose the mode you want to execute.");
        printf("\n  - SEQUENTIAL    enter 1");
        printf("\n  - PARALLEL v1   enter 2");
        printf("\n  - PARALLEL v2   enter 3");
        printf("\n  - COMPARE ALL   enter 4");
        printf("\nEnter your choice: ");
        scanf("%d", &choice);
        (void) getchar(); //clean the input of newline
        if(choice != 1 && choice != 2 && choice != 3 && choice != 4){
            printf("ERROR: please enter a valid alternative\n");
        }

        printf("\n****************************************************\n");
    }while(choice != 1 && choice != 2 && choice != 3 && choice != 4);

  return choice;
    
}