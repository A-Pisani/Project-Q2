#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#define MAX_LINE 100
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
    int color;
    int dist;
    int disc_time;
    int endp_time;
    int scc;
    vertex_t *pred;
    edge_t *head;
    vertex_t *next;
    // Labels
    int left_label;
    int right_label;
};

int post_order_index=1;

//LOAD GRAPH
static vertex_t *new_node(vertex_t *g, int id);
graph_t *graph_load(char *filename);
static void new_edge( graph_t *g, int i, int j);
void graph_attribute_init(graph_t *g);
vertex_t *graph_find(graph_t *g, int id);
void graph_dispose(graph_t*g);
// LOAD QUERIES
int **queries_load(char *filename, int *size);
void queriesDispose(int **mat, int size);
//DFS VISIT prototypes
void graph_dfs(graph_t *g, vertex_t *n);
int graph_dfs_r(graph_t *g, vertex_t *n, int currTime);

int main(int argc, char **argv){
    int i=0, queryNum=0;
    vertex_t *src;

    if(argc!=4){
        fprintf (stderr, "Run as: %s p1 p2 p3\n", argv[0]);
        fprintf (stderr, "Where : p1 = graph file name (.gra)\n");
        fprintf (stderr, "        p2 = number of label pairs for each graph vertex\n");
        fprintf (stderr, "        p3 = query file name (.que)\n" );
        exit (1);
    }

    printf("loading graph\n");
    graph_t *g = graph_load(argv[1]);

    int labelNum= atoi(argv[2]);
    printf("loading queries\nQueries are:\n");
    int **mat = queries_load(argv[3], &queryNum);
    for(i=0; i<queryNum;i++){
        printf("%d %d\n", mat[i][0], mat[i][1]);
    }
    printf("(DFS) Initial vertex? ");
    scanf("%d", &i);
    src= graph_find(g, i);
    graph_attribute_init(g);

    graph_dfs(g, src);

    graph_dispose(g);
    queriesDispose(mat, queryNum);

}

graph_t *graph_load(char *filename) {
    graph_t *g;
    int i, j, k, weight, dir;
    FILE *fp;
    char character;
    g = (graph_t*) calloc(1, sizeof(graph_t));
    fp = fopen(filename, "r");
    fscanf(fp, "%d", &g->nv);
    printf("Graph number of vertices: %d\n", g->nv);
    /* create initial structure for vertices */
    for (i=g->nv-1; i>=0; i--) {        /*Creates main list of vertices*/
        g->g = new_node(g->g, i);
    }

    /* load edges*/
    for (i=g->nv-1; i>=0; i--) {
        fscanf(fp, "%d: ", &k);
        //printf("%d\n", i);
        do{
            fscanf(fp, "%c ", &character);
            j = atoi(&character);
            if(character!='#')
                new_edge(g, k, j);
        }while(character!='#');
    }

    fclose(fp);

    return g;
}

static vertex_t *new_node(vertex_t *g, int id) { /*Add a new vertex node into main list*/
    vertex_t*v;
    v = (vertex_t*)malloc(sizeof(vertex_t));
    v->id = id;
    v->color = WHITE;
    v->dist= INT_MAX;
    v->scc = v->disc_time = v->endp_time = -1;
    v->pred= NULL; v->head = NULL;
    v->next= g;

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

    //DEBUG comment to not show
    printf("created edge %d -> %d\n", i, j);

    return;
}

void graph_attribute_init(graph_t *g) {
    vertex_t *v;
    v = g->g;
    while(v!=NULL) {
        v->color = WHITE;
        v->dist= INT_MAX;
        v->disc_time= -1;
        v->endp_time= -1;
        v->scc= -1;
        v->pred= NULL;
        v = v->next;
    }

    return;
}

vertex_t *graph_find(graph_t *g, int id) { /*It is often necessary to avoid linear searches(use pointers or efficient symbol tables)*/
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

void graph_dispose(graph_t*g) { /*Free list of lists*/
    vertex_t *v; edge_t *e;
    v = g->g;
    while(v != NULL) {
        while(v->head != NULL) {
            e = v->head;
            v->head = e->next;
            free(e);
        }
        v = v->next;
        free (v);
    }

    return;
}

void graph_dfs(graph_t *g, vertex_t *n) {
    int currTime=0;
    vertex_t *tmp, *tmp2;
    printf("List of edges:\n");
    currTime = graph_dfs_r (g, n, currTime);
    for (tmp=g->g; tmp!=NULL; tmp=tmp->next) {
        if(tmp->color == WHITE) {
            currTime= graph_dfs_r(g, tmp, currTime);
        }
    }
    printf("List of vertices:\n");
    for (tmp=g->g; tmp!=NULL; tmp=tmp->next) {
        tmp2 = tmp->pred;
        printf("%2d: %2d/%2d (%d)  labelL=%d       labelR=%d\n", tmp->id, tmp->disc_time, tmp->endp_time, (tmp2!=NULL) ? tmp->pred->id : -1, tmp->left_label, tmp->right_label);
    }

}

int graph_dfs_r(graph_t *g, vertex_t *n, int currTime) {
    vertex_t *tmp;
    edge_t *e;
    vertex_t *t;
    n->color = GREY;
    n->disc_time = ++currTime;
    e = n->head;
    while (e != NULL) {
        t = e->dst;
        switch (t->color) {
            case WHITE:
                printf("%d -> %d : T\n", n->id, t->id);
                break;
            case GREY :
                printf("%d -> %d : B\n", n->id, t->id);
                break;
            case BLACK:
                if (n->disc_time < t->disc_time) {
                    printf("%d -> %d : F\n", n->disc_time, t->disc_time);
                } else {
                    printf("%d -> %d : C\n", n->id, t->id);
                }
        }
        if (t->color == WHITE) {
            t->pred = n;
            currTime = graph_dfs_r(g, t, currTime);
        }
        e = e->next;
    }
    n->color = BLACK;
    n->endp_time = ++currTime;
    n->right_label= post_order_index++;
    n->left_label=50;
//    if(n->next==NULL)
//        n->left_label = n->right_label;
//    else{
//        for (tmp=n->next; tmp!=NULL; tmp=tmp->next) {
//            if(tmp->left_label<n->left_label)
//                n->left_label=tmp->left_label;
//        }
//    }


            //currTime = graph_dfs_r(g, t, currTime);

    if(n->head==NULL)
        n->left_label=n->right_label;
    else{
        for(e=n->head; e!=NULL; e=e->next){
            t = e->dst;
                if(t->left_label<n->left_label)
                    n->left_label=t->left_label;
        } 
    }


    return currTime;
}

int **queries_load(char *filename, int *size){
    int **mat;
    int num=0, tmp1, tmp2;
    FILE *fp2;
    //mat = (int**) calloc(1, sizeof(int*));
    fp2 = fopen(filename, "r");

    while(fscanf(fp2, "%d %d", &tmp1, &tmp2)!=EOF){
        num++;
    }
    mat = (int**) malloc(num*sizeof(int*));

    rewind(fp2);
    for(int i=0; i<num;i++){
        mat[i]=(int*) malloc(2*sizeof(int));
        fscanf(fp2, "%d %d", &mat[i][0], &mat[i][1]);
    }

    *size=num;
    fclose(fp2);

    return mat;
}

void queriesDispose(int **mat, int size){
    for(int i=0;i<size;i++){
        free(mat[i]);
    }
    free(mat);
}














