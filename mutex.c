#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>
#include <pthread.h>

#define upper 65535
#define lower 0
#define op_upper 1000
#define op_lower 1
#define n 1000
#define m 10000


struct list_node_s {
    int data;
    struct list_node_s* next;
};

struct list_node_s* ll_head;

int operations[m];
int thread_count, iterations;
double p_member, p_insert, p_delete;

pthread_mutex_t list_mutex;

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

    const int w_member = (int) (p_member*op_upper); 
    const int w_insert = (int) (p_insert*op_upper); 
    const int w_delete = (int) (p_delete*op_upper);
    
    int i = 0;
    while ((m_member + m_insert + m_delete) != 0)
    {
        int operation = (rand() % (op_upper - op_lower + 1)) + op_lower;

        if ((w_member >= operation) && (m_member != 0)){
            m_member--;
            operations[i] = 0; // 0 for member function
            i++;
        }
        else if ((w_member < operation) && 
                    (w_insert + w_member >= operation) && 
                    (m_insert != 0)){
            m_insert--;
            operations[i] = 1; // 1 for insert function
            i++;
        }
        else if (((w_insert + w_member) < operation) && 
                    (w_delete + w_insert + w_member >= operation) && 
                    (m_delete != 0)){
            m_delete--;
            operations[i] = 2; // 2 for delete function
            i++;
        }
    }

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

        if (operations[i] == 0){         
            pthread_mutex_lock(&list_mutex);
            Member(rand_numb, ll_head);
            pthread_mutex_unlock(&list_mutex);
        }
        else if (operations[i] == 1) {
            pthread_mutex_lock(&list_mutex);
            Insert(rand_numb, &ll_head);
            pthread_mutex_unlock(&list_mutex);
        } else {
            pthread_mutex_lock(&list_mutex);
            Delete(rand_numb, &ll_head);
            pthread_mutex_unlock(&list_mutex);
        }
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    thread_count = atoi(argv[5]);
    iterations = atoi(argv[4]);

    p_member = (double) atof(argv[1]);
    p_insert = (double) atof(argv[2]);
    p_delete = (double) atof(argv[3]);

    double execution_times[iterations];
    double average=0;
    double std=0;
    double total_exe_times=0;
    double sqrd_error_sum=0;

    int iter = 0;
    while (iter<iterations){
        ll_head = NULL;
        srand(time(0));

        pthread_mutex_init(&list_mutex, NULL);

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
        pthread_mutex_destroy(&list_mutex);

        clock_gettime(CLOCK_REALTIME, &finish);
        double time_spent = (finish.tv_sec - start.tv_sec) + ((finish.tv_nsec - start.tv_nsec) / 1000000000.0);

        // printf("round: %d time: %f \n", iter, time_spent);

        total_exe_times+=time_spent;
        execution_times[iter]=time_spent;
        iter++;
    }
    average = total_exe_times/iterations;
    
    int i;
    for ( i = 0; i < iterations; i++)
    {
        sqrd_error_sum += pow(execution_times[i]-average, 2);
    }
    std = sqrt(sqrd_error_sum/(iterations-1));

    printf("MUT avg: %f std: %f \n", average, std);

    printf("n: %f\n", (100*1.96*std)/(5*average));

    return 0;
}