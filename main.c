#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <omp.h>

#define MAX_LINE 100
#define NUM_T 8    

enum{WHITE, GREY, BLACK};

typedef struct graph_s graph_t;
typedef struct vertex_s vertex_t;
typedef struct edge_s edge_t;

/* graph wrapper*/
struct graph_s{
    vertex_t* g;
    int nv;
};

struct edge_s{
    int weight;
    vertex_t *dst;
    edge_t *next;
};

struct vertex_s{
    int id;
    int *color;
    int dist;
    int disc_time;
    int endp_time;
    int scc;
    vertex_t *pred;
    edge_t *head;
    vertex_t *next;
    int *left_label;   
    int *right_label;
    //if already visited set to 1 (initialized to -1)
    int visited;
    int tmpColor;
};

typedef struct threadData{
    pthread_t threadID;
    int ID;
    int labelNum; 
    FILE *fp;
    graph_t *g;
    vertex_t *n;
}threadD;

int *post_order_index; 
int choice;
pthread_mutex_t sem;

static vertex_t *new_node(vertex_t *g, int id, int labelNum);
graph_t *graph_load(char *filename, int labelNum);
static void new_edge( graph_t *g, int i, int j);
void graph_attribute_init(graph_t *g, int index);
vertex_t *graph_find(graph_t *g, int id);
void graph_dispose(graph_t*g);

void *queries_checker(void *);                                         
void queriesDispose(int **mat, int size);
          
void *graph_dfs(void *);                                              
int graph_dfs_r(graph_t *g, vertex_t *n, int currTime, int index);

int Randoms(int lower, int upper, int count);
int isReachableDFS(vertex_t *v, vertex_t *u, graph_t *g, int d);
int isContained(vertex_t *v, vertex_t *u, int d);
int menu(void);

int main(int argc, char **argv){
    int i=0, lower = 0, count = 1;
    vertex_t *src;
    threadD *td, *td2;
    // Use current time as seed for random generator
    srand(time(0));
    if(argc!=4){
        fprintf (stderr, "Run as: %s p1 p2 p3\n", argv[0]);
        fprintf (stderr, "Where : p1 = graph file name (.gra)\n");
        fprintf (stderr, "        p2 = number of label pairs for each graph vertex\n");
        fprintf (stderr, "        p3 = query file name (.que)\n" );
        exit (1);
    }
    int labelNum= atoi(argv[2]);
    printf("************ PARALLEL GRAIL VERSION **************\n");
    choice = menu();
    
    pthread_mutex_init(&sem, NULL); 
    td = (threadD *)malloc(labelNum* sizeof(threadD));
    if(td == NULL){
        fprintf (stderr, "Error allocating threads\n" );
        exit (1);
    }
    printf("************ LOADING GRAPH **************\n");
    clock_t begin = clock();
    graph_t *g = graph_load(argv[1], labelNum);
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    if(choice==1)
        printf("Graph Construction time (ms): %lf\n", time_spent);
    post_order_index=(int*)calloc(labelNum, sizeof(int));
    if(post_order_index==NULL){
        fprintf(stderr, "Error in array of indeces allocation\n");
        exit(1);
    }
    printf("************ RANDOMIZED LABELING ************\n");
    double start = omp_get_wtime();
    
    for(int j=0;j<labelNum;j++){        
        // Randomized traversal strategy (RandomizedLabeling)
        do{
            i = Randoms(lower, g->nv-1, count); // finds "count" (one) rand number between [0, nv-1]
            src= graph_find(g, i);             
        }while(src->visited!=-1);
        // TO AVOID PROBLEMS SET HERE src->visited = 1 AND NOT IN GRAPH_DFS
        src->visited = 1;
        td[j].ID=j;
        td[j].n=src;
        td[j].g=g;
        td[j].labelNum = labelNum;
        graph_attribute_init(g, j);
        pthread_create(&td[j].threadID, NULL, graph_dfs, (void *) &td[j]);
    }

    for(int i=0; i<labelNum; i++){
        pthread_join(td[i].threadID, NULL);
    }

    double end1 = omp_get_wtime();
    double final = end1 - start;
    if(choice==1)
        printf("Construction time (ms): %lf\n", final);
    printf("************ CHECKING QUERIES ************\n");
    td2 = (threadD *)malloc(NUM_T* sizeof(threadD));
    if(td2 == NULL){
        fprintf (stderr, "Error allocating second round of threads \n" );
        exit (1);
    }
    FILE *fp2= fopen(argv[3], "r");
    double start2 = omp_get_wtime();
    
    for(int j=0;j<NUM_T;j++){
        td2[j].ID=j;
        td2[j].g=g;
        td2[j].fp = fp2;
        td2[j].labelNum = labelNum;
        pthread_create(&td2[j].threadID, NULL, queries_checker, (void *) &td2[j]);
    }


    for(int i=0; i<NUM_T; i++){
        pthread_join(td2[i].threadID, NULL);
    }

    double end2 = omp_get_wtime();
    double final2 = end2 - start2;
    if(choice==1)
        printf("Query time (ms): %lf\n", final2);
    printf("************ END ************\n");
    fclose(fp2);
    free(td); free(td2);
    graph_dispose(g);
    free(post_order_index);
    pthread_mutex_destroy(&sem); 

}
graph_t *graph_load(char *filename, int labelNum) {
    graph_t *g;
    int i, j, k;
    FILE *fp;
    char character[100];
    g = (graph_t*) calloc(1, sizeof(graph_t));
    fp = fopen(filename, "r");
    if(fp == NULL){
        printf("error opening %s\n", filename);
        exit(0);
    }
    fscanf(fp, "%d", &g->nv);
    if(choice == 2)
        printf("Graph number of vertices: %d\n", g->nv);
    /* create initial structure for vertices */
    for (i=g->nv-1; i>=0; i--) {        /*Creates main list of vertices*/
        g->g = new_node(g->g, i, labelNum);
    }
    /* load edges*/
    for (i=g->nv-1; i>=0; i--) {
        fscanf(fp, "%d: ", &k);
        //printf("%d\n", i);
       do{
           fscanf(fp, "%s ", character);
           if(character[0]!='#'){
               j = atoi(character);
               new_edge(g, k, j);
           }
       }while(character[0]!='#');
    }
    fclose(fp);
    return g;
}
static vertex_t *new_node(vertex_t *g, int id, int labelNum) { /*Add a new vertex node into main list*/
    vertex_t*v;
    v = (vertex_t*)malloc(sizeof(vertex_t));
    v->id = id;
    v->dist= INT_MAX;
    
    v->disc_time = v->endp_time = -1;
    v->pred= NULL; v->head = NULL;
    v->next= g;
    v->tmpColor=WHITE;
    v->visited=-1;
    v->right_label=(int*)calloc(labelNum, sizeof(int));
    v->left_label=(int*)calloc(labelNum, sizeof(int));
    v->color =(int*)malloc(labelNum* sizeof(int));
    if(v->right_label==NULL || v->left_label==NULL || v->color==NULL){
        fprintf(stderr, "Error in array of lables allocation\n");
        exit(1);
    }
    for(int i=0; i<labelNum;i++){
        v->color[i]=WHITE;
    }
    return v;
}
static void new_edge( graph_t *g, int i, int j) { /*Add a new edge node into secondary list*/
    vertex_t *src, *dst;
    edge_t *e;
    src= graph_find(g, i);
    dst= graph_find(g, j);
    e = (edge_t*) malloc(sizeof(edge_t));
    e->dst= dst;
    e->next= src->head; src->head = e;
    //printf("created edge %d -> %d\n", i, j);
    return;
}
void graph_attribute_init(graph_t *g, int index) {
    vertex_t *v;
    v = g->g;
    while(v!=NULL) {
        v->color[index] = WHITE;
        v->tmpColor=WHITE;
        v->dist= INT_MAX;
        v->disc_time= -1;
        v->endp_time= -1;
        v->scc= -1;
        v->pred= NULL;
        v = v->next;
    }
    return;
}
vertex_t *graph_find(graph_t *g, int id) { 
    vertex_t *v;
    v = g->g;
    while(v != NULL) {
        if(v->id == id) {
            return v;
        }
        v = v->next;
    }
    return NULL;
}
void graph_dispose(graph_t *g) { /*Free list of lists*/
    vertex_t *v, *curr; edge_t *e;
    v = g->g;
    while(v != NULL) {
        curr=v;
        while(curr->head != NULL) {
            e = curr->head;
            curr->head = e->next;
            free(e);
        }
        free(curr->right_label);
        free(curr->left_label);
        free(curr->color);
        v = curr->next;
        free(curr);
    }
    free(g);
    return;
}
void *graph_dfs(void *param) {
    threadD *td ;
    td = (threadD *) param;
    if(choice==2)
        fprintf(stdout, "thread %d working on node %d\n", td->ID, td->n->id);
    int currTime=0;

    vertex_t *tmp, *tmp2;
    // printf("List of edges:\n");
    currTime = graph_dfs_r (td->g, td->n, currTime, td->ID);
    // PERFORMS A DFS ON ISLANDS
    for (tmp=td->g->g; tmp!=NULL; tmp=tmp->next) {
        if(tmp->color[td->ID] == WHITE) {
            // printf("Root of the DFS %2d\n", tmp->id);
            currTime= graph_dfs_r(td->g, tmp, currTime, td->ID);
        }
    }
    
    //Print labels
        if(choice==2){
           printf("List of vertices (and labels):\n"); 
           for (tmp=td->g->g; tmp!=NULL; tmp=tmp->next) {
                tmp2 = tmp->pred;
                printf("%2d: %2d/%2d (%d)  labelL=%d       labelR=%d\n", tmp->id, tmp->disc_time, tmp->endp_time,
                        (tmp2!=NULL) ? tmp->pred->id : -1, tmp->left_label[td->ID], tmp->right_label[td->ID]);
           }  
        }
    pthread_exit((void *) 1) ;
}
int graph_dfs_r(graph_t *g, vertex_t *n, int currTime, int index) {
    //vertex_t *tmp;
    edge_t *e;
    vertex_t *t;
    n->color[index] = GREY;
    n->disc_time = ++currTime;
    e = n->head;
    while (e != NULL) {
        t = e->dst;
        if (t->color[index] == WHITE) {
            t->pred = n;
            currTime = graph_dfs_r(g, t, currTime, index);
        }
        e = e->next;
    }
    n->color[index] = BLACK;
    n->endp_time = ++currTime;
    n->right_label[index]= ++post_order_index[index];
    n->left_label[index]=g->nv +1;

    // if so we are on a leaf
    if(n->head==NULL)
        n->left_label[index]=n->right_label[index];
    else{
        for(e=n->head; e!=NULL; e=e->next){
            t = e->dst;
                if(t->left_label[index]<n->left_label[index])
                    n->left_label[index]=t->left_label[index];
        }
    }
    return currTime;
}
void *queries_checker(void *param){
    threadD *td ;
    td = (threadD *) param;
    graph_t *g= td->g;
    int labelNum = td->labelNum;
    int readval;
    vertex_t *src, *dst;
    int tmp1, tmp2;
    
    do{
        pthread_mutex_lock(&sem);
            readval = fscanf(td->fp, "%d %d", &tmp1, &tmp2);
        pthread_mutex_unlock(&sem);
        if(readval != EOF){
            
            src= graph_find(g, tmp1);
            dst= graph_find(g, tmp2);           
            if(isReachableDFS(src, dst, g, labelNum)){
                if(choice==3)
                    printf("%d reaches %d\n", tmp1, tmp2);
            }else{
                if(choice==3)
                    printf("%d does not reach %d\n", tmp1, tmp2);
            }
        }
    }while(readval != EOF);
    return (int *)1;
}

/*
 * Function isReachableDFS(u, v, G):
 *  if Lv!⊆ Lu then
 *      return false; // u does not reach v
 *  end
 *
 *  foreach c ∈ Children(u) s.t. Lv ⊆ Lu do
 *      if isReachableDFS(c,v,G) then
 *          return true; // u reaches v
 *      end
 *  end
 *  return false; // u does not reach v
 * */
int isReachableDFS(vertex_t *u, vertex_t *v, graph_t *g, int d){
    if(u->id==v->id)
        return 1;
    if(!isContained(u, v, d)){
        return 0;
    }
        edge_t *e;
        vertex_t *t;
        e = u->head;
        while (e != NULL) {
            t = e->dst;
                if(isContained(t,v,d)){
                    if(isReachableDFS(t, v, g, d))
                        return 1;
                }
                 
            e = e->next;
        }
        return 0;
    
}

int isContained(vertex_t *u, vertex_t *v, int d){
    for(int i=0; i<d; i++){
        if(v->left_label[i] < u->left_label[i] || v->right_label[i] > u->right_label[i])
            return 0;
    }
    return 1;
}

// Generates and returns 'count' random numbers in range [lower, upper].
int Randoms(int lower, int upper, int count){
    int i, num;
    for (i = 0; i < count; i++) {
         num = (rand() % (upper - lower + 1)) + lower;
         if(choice==2)
            printf("Randomly selected node is:      %d \n", num);
    }
    return num;
}

void queriesDispose(int **mat, int size){
    for(int i=0;i<size;i++){
        free(mat[i]);
    }
    free(mat);
}

int menu(void){
    int choice;
    do {
        printf("\n******************  MENU   ************************");
        printf("\nEXECUTION TIME enter    1");
        printf("\nDEBUGGING INFORMATION enter     2");
        printf("\nQUERY RESULTS enter   3");
        printf("\nEnter your choice: ");
        scanf("%d", &choice);
        (void) getchar(); //clean the input of newline
        if(choice != 1 && choice != 2 && choice != 3){
            printf("ERROR: please enter a valid alternative\n");
        }
        printf("\n***************************************************\n");
    }while(choice != 1 && choice != 2 && choice != 3);
  return choice;
    
}
