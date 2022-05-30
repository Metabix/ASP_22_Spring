/*
=========================================================================
=													                    =
=				Name: Mohit Sathyaseelan								=
=				contact: Mohitsathyaseelan@gmail.com			        =
=													    	            =
=========================================================================
*/
#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>
#include<errno.h>
#include<stdlib.h>
#include<limits.h>
#include<string.h>
#include<sys/types.h>
#include<stdbool.h>
#include <pthread.h>


#define len 100
#define NA 1000

struct account_name
{
    int account_number;
    int account_balance;
};
struct transfers
{
    int account1;
    int account2;
    int transfer_amount;
};


pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;
struct account_name *account_details = {0};
struct transfers *account_transfers = {0};
int * account_transaction = {0};
int threads;
int account_count = 0;
int transfer_count = 0;
int transfer_index = 0;
int transfer_done = 1;

static void *func(void *arg)
{
    int threadid;
    threadid = (int)arg;
    while(transfer_done ||  account_transaction[threadid])
    {
        if(pthread_mutex_lock(&m))
        {
            printf("Error");
            exit(-1);
        }
        while(transfer_done && ( account_transaction[threadid]==0))
        {
            if(pthread_cond_wait(&empty, &m))
            {
                printf("Error");
                exit(-1);
            }
        }
        if(pthread_mutex_unlock(&m))
        {
            printf("Error");
            exit(-1);
        }
        if( account_transaction[threadid])
        {
            for(int i = 0; i<account_count; i++)
            {
                if(account_details[i].account_number == account_transfers[threadid].account1)
                {
                    
                    account_details[i].account_balance -= account_transfers[threadid].transfer_amount;
                    
                }
                if(account_details[i].account_number == account_transfers[threadid].account2)
                {
                    account_details[i].account_balance += account_transfers[threadid].transfer_amount;
                    
                }
            }
            if(pthread_mutex_lock(&m))
            {
                printf("Error");
                exit(-1);
            }
             account_transaction[threadid] = 0;
            if(pthread_cond_signal(&full))
            {
                printf("Error");
                exit(-1);
            } 
            if(pthread_mutex_unlock(&m))
            {
                printf("Error");
                exit(-1);
            }
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{    
    setbuf(stdout, NULL);
    threads = atoi(argv[2]);
    account_details = (struct account_name *)malloc(NA*sizeof(struct account_name));
    account_transfers = (struct transfers *)malloc(threads*sizeof(struct transfers));
    account_transaction = (int *)malloc(threads*sizeof(int));
    pthread_t threadtid[threads];
    int threadids[threads];
    for(int i=0; i<threads; i++)
    {
        threadids[i]=i;
        if(pthread_create(&threadtid[i], NULL, func, (void *)threadids[i]))
        {
            printf("Error");
            exit(-1);
        }
    }
    FILE *f = fopen(argv[1], "r");
    char ch[len], arr[len];
    char x, y;
    x = getc(f);
    while(x != EOF)
    {
        fgets(ch, len, f);
        char temp[len] = {0};
        *temp = x;
        strcat(temp, ch);
        char* s1;
        char* s2 = temp;
        while (s1 = strtok_r(s2, "\n", &s2))
        {
            char* s3;
            char* s4 = s1;
            s3 = strtok_r(s4, " ", &s4);

            if(strcmp(s3,"Transfer")!=0)
            {
                account_details[account_count].account_number = atoi(s3);
                s3 = strtok_r(s4, " ", &s4);
                account_details[account_count].account_balance = atoi(s3);
                account_count += 1;
            }
            else
            {
                if(pthread_mutex_lock(&m))
                {
                    printf("Error");
                    exit(-1);
                }
                while( account_transaction[transfer_index])
                {
                    if(pthread_cond_wait(&full, &m))
                    {
                        printf("Error");
                        exit(-1);
                    }
                }
                s3 = strtok_r(s4, " ", &s4);
                account_transfers[transfer_index].account1 = atoi(s3);
                s3 = strtok_r(s4, " ", &s4);
                account_transfers[transfer_index].account2 = atoi(s3);
                s3 = strtok_r(s4, " ", &s4);
                account_transfers[transfer_index].transfer_amount = atoi(s3);
                 account_transaction[transfer_index] = 1;
                transfer_count += 1;
                transfer_index = transfer_count % threads;
                if(pthread_cond_broadcast(&empty))
                {
                    printf("Error");
                    exit(-1);
                }
                if(pthread_mutex_unlock(&m))
                {
                    printf("Error");
                    exit(-1);
                }
            }
        }
        x = getc(f);
    }
    if(pthread_mutex_lock(&m))
    {
        printf("Error");
        exit(-1);
    }
    transfer_done = 0;
    if(pthread_cond_broadcast(&empty))
    {
        printf("Error");
        exit(-1);
    }
    if(pthread_mutex_unlock(&m))
    {
        printf("Error");
        exit(-1);
    }
    for(int i=0; i<threads; i++)
    {
        if(pthread_join(threadtid[i], NULL))
        {
            printf("Error");
            exit(-1);
        }
    }
    FILE *g;
    g = fopen("Output.txt", "w");
    for(int i=0; i<account_count; i++)
    {
        printf("%d %d\n",account_details[i].account_number, account_details[i].account_balance);
        fprintf(g,"%d %d\n",account_details[i].account_number, account_details[i].account_balance );
    }
    fclose(f);
    pthread_exit(NULL);
}
