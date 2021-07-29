#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>
#include <pthread.h>

int thread_count = 4;
int upper = 65535;
int lower = 0;
int op_upper = 1000;
int op_lower = 1;
double p_member = 0.99;
double p_insert = 0.005;
double p_delete = 0.005;
int n = 1000;
int m = 10000;
int iterations = 1;


struct list_node_s {
    int data;
    struct list_node_s* next;
};

struct list_node_s* ll_head;
int operations[10000];
pthread_rwlock_t rw_lock = PTHREAD_RWLOCK_INITIALIZER;

int Member(int value, struct list_node_s* head_p){
    struct list_node_s* curr_p = head_p;

    while (curr_p != NULL && curr_p->data < value){ 
        // printf("item: %d\n", curr_p->data);
        curr_p = curr_p->next;
    }

    if (curr_p == NULL || curr_p->data > value)
        return 0;
    else
        return 1;
}

int Insert(int value, struct list_node_s** head_pp){
    struct list_node_s* curr_p = *head_pp;
    struct list_node_s* pred_p = NULL;
    struct list_node_s* temp_p;

    while (curr_p != NULL && curr_p->data < value){
        pred_p = curr_p;
        curr_p = curr_p->next;
    }
    
    if (curr_p == NULL || curr_p->data > value){
        temp_p = malloc(sizeof(struct list_node_s));
        temp_p->data = value;
        temp_p->next = curr_p;
        if (pred_p == NULL) // newly adding the first node
            *head_pp = temp_p;
        else
            pred_p->next = temp_p;
        return 1;
    }
    else // value in list
        return 0;
        
}

int Delete(int value, struct list_node_s** head_pp){
    struct list_node_s* curr_p = *head_pp;
    struct list_node_s* pred_p = NULL;

    while (curr_p != NULL && curr_p->data < value){
        pred_p = curr_p;
        curr_p = curr_p->next;
    }

    if (curr_p != NULL && curr_p->data == value){
        if (pred_p == NULL){ // del first node
            *head_pp = curr_p->next;
            free(curr_p);
        }
        else{
            pred_p->next = curr_p->next;
            free(curr_p);
        }
        return 1;
    }
    else    
        return 0;
}

void generateRandomOperations() {

    int m_member = (int) (p_member*m); 
    int m_insert = (int) (p_insert*m); 
    int m_delete = (int) (p_delete*m);
    // printf("\n%d %d %d\n", m_member, m_insert, m_delete);

    const int w_member = (int) (p_member*op_upper); 
    const int w_insert = (int) (p_insert*op_upper); 
    const int w_delete = (int) (p_delete*op_upper);
    
    int i = 0;
    while ((m_member + m_insert + m_delete) != 0)
    {
        int operation = (rand() % (op_upper - op_lower + 1)) + op_lower;
        // unsigned int rand_numb = (rand() % (upper - lower)) + lower;

        if ((w_member >= operation) && (m_member != 0)){
            m_member--;
            // operations[i].function = Member;
            // operations[i].value = rand_numb;
            operations[i] = 0; // 0 for member function
            i++;
        }
        else if ((w_member < operation) && 
                    (w_insert + w_member >= operation) && 
                    (m_insert != 0)){
            m_insert--;
            // operations[i].function = Insert;
            // operations[i].value = rand_numb;
            operations[i] = 1; // 1 for insert function
            i++;
        }
        else if (((w_insert + w_member) < operation) && 
                    (w_delete + w_insert + w_member >= operation) && 
                    (m_delete != 0)){
            m_delete--;
            // operations[i].function = Delete;
            // operations[i].value = rand_numb;
            operations[i] = 2; // 2 for delete function
            i++;
        }
    }

    // printf("\n%d %d %d\n", w_member, w_insert, w_delete);
    return;
}

void *doRandomOperations(void *rank) {
    long my_rank = (long) rank;

    int local_m = m/thread_count;
    int my_first_row = my_rank * local_m;
    int my_last_row = (my_rank + 1) * local_m - 1;

    int i;
    for (i = my_first_row; i <= my_last_row; i++){
        int rand_numb = (rand() % (upper - lower)) + lower;
        // if (*Member == (operations[i].function)){       
        if (operations[i] == 0){         
            pthread_rwlock_rdlock(&rw_lock);
            // (*(operations[i].function))(operations[i].value, &ll_head);
            Member(rand_numb, ll_head);
            pthread_rwlock_unlock(&rw_lock);
        }
        else if (operations[i] == 1) {
            pthread_rwlock_wrlock(&rw_lock);
            // (*(operations[i].function))(operations[i].value, &ll_head);
            Insert(rand_numb, &ll_head);
            pthread_rwlock_unlock(&rw_lock);
        } else {
            pthread_rwlock_wrlock(&rw_lock);
            // (*(operations[i].function))(operations[i].value, &ll_head);
            Delete(rand_numb, &ll_head);
            pthread_rwlock_unlock(&rw_lock);
        }
    }

    return NULL;
}



int main() {

    ll_head = NULL;
    srand(time(0));
    
    int i = 0;
    while (i < n){            
        int rand_numb = (rand() % (upper - lower + 1)) + lower;
        int return_val = Insert(rand_numb, &ll_head);
        if (return_val == 1){
            i++;
        }
    }

    generateRandomOperations();

    long thread;
    pthread_t* thread_handles;

    struct timespec start, finish;    
    clock_gettime(CLOCK_REALTIME, &start);


    thread_handles = malloc(thread_count * sizeof(pthread_t));
    for (thread = 0; thread < thread_count; thread++){
        pthread_create(&thread_handles[thread], NULL, doRandomOperations, (void*) thread);
    }

    for (thread = 0; thread < thread_count; thread++){
        pthread_join(thread_handles[thread], NULL);
    }

    free(thread_handles);
    pthread_rwlock_destroy(&rw_lock);

    clock_gettime(CLOCK_REALTIME, &finish);
    double time_spent = (finish.tv_sec - start.tv_sec) + ((finish.tv_nsec - start.tv_nsec) / 1000000000.0);

    printf("RWL time spent = %f \n", time_spent);
    return 0;
}
