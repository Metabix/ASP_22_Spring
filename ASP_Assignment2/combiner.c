=========================================================================
=													                    =
=				Name: Mohit Sathyaseelan								=
=				contact: Mohitsathyaseelan@gmail.com			        =
=													    	            =
=========================================================================
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

pthread_mutex_t lock;
pthread_cond_t full;
pthread_cond_t empty;

//combiner
int bnum;
int bsize;
int B[7];
//mapper
int a=0;
int tip=0;
int Flag=0;

struct sample{
    int score;
    int uniq;
    char usid[5];
    char topic[16];
    char action;
};

FILE *fp1,*fp2,*fp3;

struct sample upd[100];
struct sample new[100];

void *mapper(void *t)
{ 
    int Call=*(int*)t;
    char c;
    
    while(1) {
        pthread_mutex_lock(&lock);
        if (Flag == 1) {
            break;
        }   
            while(scanf("( %[^,],%c,%[^)])%c", new[a].usid, &new[a].action, new[a].topic, &c) != EOF){
            Flag=0;      
            switch (new[a].action) {
                case 'P':
                    new[a].score = 50;
                    break;
                case 'L':
                    new[a].score = 20;
                    break;
                case 'D':
                    new[a].score = -10;
                    break;
                case 'C':
                   new[a].score = 30;
                    break;
                case 'S':
                    new[a].score = 40;
                    break;
                default:
                    printf("Default\n");
                    break;
            }
            a++;
            if (a == 1) // INITIAL VALUES COND
            {
                B[0] =  B[0] + 1;
                strncpy(upd[0].usid,new[0].usid,5); 
                new[0].uniq = 0;
                upd[0].uniq=1; 
            }
            else
            {
                int flag = 0;   
                for(int i=0;i<a-1;i++)
                {
                    if(strcmp(new[a-1].usid,upd[new[i].uniq].usid) == 0) //NEW ID
                    {
                        flag=1;                                         //OLD ID IDENT
                        B[new[i].uniq]++;
                        new[a-1].uniq=new[i].uniq;
                        break;
                    }
                }
                if(flag!=1)
                {
                    for (int i = 0; i < bnum; i++) 
                    {
                        if (upd[i].uniq == 0) 
                        {
                            upd[i].uniq=1;
                            new[a-1].uniq = i;
                            strncpy(upd[i].usid,new[a-1].usid,5);
                            B[i]++;
                            break;
                        }
                    }
                }
            }

        }
        Flag = 1;
        pthread_cond_broadcast(&empty);
        pthread_mutex_unlock(&lock);
        break;
    }
    pthread_cond_broadcast(&empty);
    pthread_cond_broadcast(&full);
    pthread_mutex_unlock(&lock);
    return 0;
}


void *reducer(void *t) {
    int tid=0;
    tid = *(int *) t; // unique t handling unique user
    if(tid >= bnum)
    {
        pthread_cond_broadcast(&full);
        pthread_cond_broadcast(&empty);
        pthread_mutex_unlock(&lock);
        tip = 0;
    }
    
    while (1) {
        pthread_mutex_lock(&lock);
        if(tip==tid)
        {
            if(Flag == 1 &&  B[tid] == 0) //IF MAPPER DONE
            {
                pthread_mutex_unlock(&lock);
                break;
            }
            while( B[tid] < bsize && Flag != 1) 
            { 
                pthread_cond_wait(&empty, &lock);
            }
            
        }
    
        pthread_cond_broadcast(&full);
        tip++;
        if(tip>=bnum)
        {
            tip=0;
        }
        break;
    }
    for (int i = 0; i <a; i++)                  //UPDATE SCORE
    {
    
        for (int k = i+1; k <a; k++) 
        {
            if (strcmp(new[i].usid,new[k].usid)==0)
            {
                if(strcmp(new[i].topic,new[k].topic)==0)
                {
                   new[i].score = new[i].score +new[k].score;
                   strcpy(new[k].usid,"done");
                }
            }
        }
    }
    pthread_cond_broadcast(&full);
    pthread_mutex_unlock(&lock);
    return t;
}



int main(int argc,char *argv[])
{
    bsize=atoi(argv[1]);
    bnum=atoi(argv[2]);
    
    pthread_t th[bnum+1];
    int t=0;
    int p=0;
    for(int i=0;i<bnum;i++)
    {
       B[i]=0;
    }

    if(argc<3)
    {
        printf("The Number of Arguments is Not Right\n");
        exit(EXIT_SUCCESS);
    }
    
    pthread_create(&th[0],NULL,mapper,(void *)&p);
    printf("mapper thread created\n");
    pthread_join(th[0],NULL);
    printf("access to mapper\n");  

    for(t=0;t<bnum;t++)
    {
        upd[t].score=t;
        pthread_create(&th[t+1],NULL,reducer,(void *)&t);
        printf("reducer thread [%d] created\n", t);
    }

    for(int i=0;i<bnum;i++)
    {
        pthread_join(th[i+1],NULL);
        printf("access to reducer%ld\n", i);
    }

    if ((fp3 = fopen("output.txt", "w")) == NULL) {
        printf("error with mapper output\n");
        exit(EXIT_SUCCESS);
    }
    printf("\n================================================== OUT PUT STARTS HERE =========================================================\n");
    for (int i = 0; i < a; i++) {
        if (strcmp(new[i].usid,"done") != 0)
        {
            printf("(%s,%s,%d)\n",new[i].usid,new[i].topic,new[i].score);
            //fprintf(fp3,"(%s,%s,%d)\n",new[i].usid,new[i].topic,new[i].score);
        }
    }

}