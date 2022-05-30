/*
=========================================================================
=													                    =
=				Name: Mohit Sathyaseelan								=
=				contact: Mohitsathyaseelan@gmail.com			        =
=													    	            =
=========================================================================
*/

#include <pthread.h> 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>

struct sample{    
    char usid[5];
    char action;
    char topic[16];
    int score;
    int uniq;
};
FILE *fp;
//synch for each reducer
struct red{     
    pthread_mutex_t lock;
    pthread_mutexattr_t mutex_attr;
    int noitems;
    pthread_cond_t full;
    pthread_cond_t empty;
    pthread_condattr_t cond_attr;
};
struct test
    {   
        char usid[5];
        int uniq;
    };
int f=0;
int mpid=0;
int main(int argc, char *argv[])
{     
    int bzie = atoi(argv[1]);
    int bnum = atoi(argv[2]);
    pid_t reducers[bnum];
    int i = 0, d, te = 0,flag = 0, a = 0, out = 0,j = 0, k = 0, scond = 0,lnum;
    char c;
    int in[bnum];
    for(j = 0; j < bnum; j++)
    {
        in[j] = 0;  
    }
    struct test list[bnum];
    struct sample new;
    struct sample consumer[100];  
    struct sample reducer[200];  
    struct sample **buf;
    if ((fp = fopen("output.txt", "w")) == NULL) {
        printf("error with output\n");
        exit(EXIT_SUCCESS);
    }
    
    struct red *reds = mmap(NULL, sizeof(struct red)*bnum, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); 
    int *done = mmap(NULL,sizeof(int),PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *done = 0;
    buf = mmap(NULL,bnum,PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); 

    if(f==0)
    {
        mpid = getpid();                
        f = 1;
    }
    printf("\n================================================== OUT PUT STARTS HERE =========================================================\n");
    //reduce 
    while(i < bnum && !(*done))         
    {
        pthread_mutexattr_init(&reds[i].mutex_attr);
        pthread_mutexattr_setpshared(&reds[i].mutex_attr,PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&reds[i].lock,&reds[i].mutex_attr);
        pthread_condattr_init(&reds[i].cond_attr);
        pthread_condattr_setpshared(&reds[i].cond_attr,PTHREAD_PROCESS_SHARED);
        pthread_cond_init(&reds[i].full,&reds[i].cond_attr);
        pthread_cond_init(&reds[i].empty,&reds[i].cond_attr);
        buf[i] = mmap(NULL,sizeof(struct sample)*bzie,PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        
        
        switch(reducers[i] = fork())
        {
        case -1:
        {
            perror("Error child process");
            break;
        }
 
        case 0:
        {
            out = 0;
            flag = 0;
            scond = 0;
            j = 0;
            k = 0;
            te = 0;
            reds[i].noitems = 0;
            while(1)
            {
                pthread_mutex_lock(&reds[i].lock);              
                while(reds[i].noitems == 0 && !(*done))
                {
                    pthread_cond_wait(&reds[i].empty, &reds[i].lock);
                }
             
                if(reds[i].noitems == 0 && (*done))
                {
                    for(j = 0; j <= scond; j++)
                    {
                       if(j == 0)
                       {
                        strcpy(reducer[k].usid, consumer[j].usid);
                        reducer[k].score = consumer[j].score;
                        strcpy(reducer[k].topic, consumer[j].topic);
                       }
                       else
                       {
                        for(te = 0; te <= k; te++)
                          {
                            if(strcmp(consumer[j].topic,reducer[te].topic) == 0)
                            {
                            reducer[te].score += consumer[j].score;
                            flag = 1;
                            }
                          }
                          if(flag == 0)
                          {
                            k++;
                            strcpy(reducer[k].usid, consumer[j].usid);
                            reducer[k].score = consumer[j].score;
                            strcpy(reducer[k].topic, consumer[j].topic);
                          }
                        flag = 0;
                       }
                    }
                    
                    for(j = 0; j < k; j++)
                    {
                       printf("(%s,%s,%d)\n", reducer[j].usid, reducer[j].topic, reducer[j].score);
                       fprintf(fp,"(%s,%s,%d)\n", reducer[j].usid, reducer[j].topic, reducer[j].score);
                    } 
                    pthread_mutex_unlock(&reds[i].lock);
                    break;  
                }

                strcpy(consumer[scond].usid, buf[i][out].usid);
                consumer[scond].score = buf[i][out].score;
                strcpy(consumer[scond].topic, buf[i][out].topic);
                scond++;
                reds[i].noitems--;
                out = (out + 1) % bzie; 
                pthread_cond_signal(&reds[i].full);
                pthread_mutex_unlock(&reds[i].lock);
             }  
             break;           
        } 
        default:
            { 
                i++;
                break;
            }
        } 
    } 
    
    //mapper
    while(d = scanf("(%[^,],%c,%[^)])%c", new.usid, &new.action, new.topic, &c)) 
    {
        if(d != 4)
        {
            break;
        }
        
        switch(new.action)
        {
        case 'P':
		    new.score = 50;
		    break;
	    case 'L':
		    new.score = 20;
		    break;
	    case 'D':
		    new.score = -10;
		    break;
	    case 'C':
		    new.score = 30;
		    break;
	    case 'S':
		    new.score = 40;
		    break;
        }
        
        if(getpid()==mpid)
        {
            if(lnum == 0 && f == 0)
            {
                strcpy(list[lnum].usid, new.usid);
                list[lnum].uniq = 0;
                new.uniq = list[lnum].uniq;
                f = 1;
            }
            else
            {
                for(a = 0; a <= lnum; a++)
                {
                    if(strcmp(list[a].usid, new.usid) == 0)
                    {
                        new.uniq = list[a].uniq;
                        flag = 1;          
                    }
                }   
                if(flag == 0)
                {
                    lnum++;
                    strcpy(list[lnum].usid, new.usid);
                    list[lnum].uniq = list[lnum-1].uniq + 1;
                    new.uniq = list[lnum].uniq;
                }
                flag = 0;
            }
            pthread_mutex_lock(&reds[new.uniq].lock);
            while(reds[new.uniq].noitems == bzie)
            {
                pthread_cond_broadcast(&reds[new.uniq].empty);
                pthread_cond_wait(&reds[new.uniq].full, &reds[new.uniq].lock);
            }
            strcpy(buf[new.uniq][in[new.uniq]].topic, new.topic);
            strcpy(buf[new.uniq][in[new.uniq]].usid, new.usid);
            buf[new.uniq][in[new.uniq]].score = new.score;
            in[new.uniq] = (in[new.uniq] + 1) % bzie;
            reds[new.uniq].noitems++;     
            pthread_mutex_unlock(&reds[new.uniq].lock);
            pthread_cond_broadcast(&reds[new.uniq].empty);
        }
    }
    
    *done = 1;
     for(a = 0; a < bnum; a++)
       pthread_cond_signal(&reds[a].empty);
       
    int dec = bnum;
    if(getpid()==mpid)
    {
        while(dec!=0)
        {
            wait(0);
            dec--;
        }
    }
    
    munmap(reds,sizeof(struct red)*bnum);
    munmap(done,sizeof(int));
    munmap(buf,bnum);
    exit(0);  
}