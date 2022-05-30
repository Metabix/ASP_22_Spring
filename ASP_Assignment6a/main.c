/*
=========================================================================
=													                    =
=				Name: Mohit Sathyaseelan								=
=				contact: Mohitsathyaseelan@gmail.com			        =
=													    	            =
=========================================================================
*/

#include<unistd.h>
#include<stdio.h>
#include<sys/mman.h>
#include<pthread.h>
#include<semaphore.h>
#include<stdlib.h>
#include<ctype.h>
#include<stdbool.h>
#include <sys/stat.h>
#include <fcntl.h>

int irqpipe[2];
int ackpipe[2];
int ctl_pipe[2];
int* ledbuff;
int ostatus=0;

pthread_t corekbdirq;
pthread_t corekbdctl;
sem_t slock;

struct myurbkbd;
typedef void* (*handler_routine)(void* urb);

struct myurbkbd{
    char* data;
    int fdpipe;
    handler_routine handle;
    char* old;
};


struct mydevkbd{
    struct myurbkbd* urbirq;
    struct myurbkbd* urbled;
    sem_t lockledurb;
    int uppercase;                                                  //  UPPERCASE
    bool urdirq_done,urdled_done;
    int cmdled_wait;
    sem_t led_done;
};

struct mydevkbd* mydev;
int urbdone(struct myurbkbd* myurbirq, struct myurbkbd* myurbled);


void* usbfunc(){
	sem_wait(&mydev->lockledurb);
    if(mydev->urdled_done==true){
        mydev->cmdled_wait+=1;
        sem_post(&mydev->lockledurb);
        return;
    }
    *ledbuff=(*ledbuff+1)%2;
    sem_post(&mydev->lockledurb);
    urbdone(NULL,mydev->urbled);
    return;
}

void inputkey(char key){
    if(isalnum(key) || (key!='#' && key!='@' && key!='&')){    //The isalnum() function checks whether the argument passed is an alphanumeric character (alphabet or number) or not.
        if(*(mydev->urbirq->old)=='@'){
            printf("%c",*mydev->urbirq->old);
            *mydev->urbirq->old='\0';
        }
        if(mydev->uppercase==1){
            printf("%c",toupper(key));
            return;
        }
        printf("%c",key);
        return;
    }
    if(key=='#'){
        if(*mydev->urbirq->old=='@'){
            printf("%c",*mydev->urbirq->old);
            mydev->urbirq->old='\0';
        }
        return;
    }
    if(key=='@'){
        if(*mydev->urbirq->old=='@'){
            printf("%c",*mydev->urbirq->old);
            mydev->urbirq->old="\0";
        }
        *mydev->urbirq->old='@';
        return;
    }
    if(key=='&'){
        if(*mydev->urbirq->old=='@'){
            mydev->uppercase=(mydev->uppercase+1)%2;
            *mydev->urbirq->old='\0';
            pthread_t usbevent;
            if(pthread_create(&usbevent,NULL,usbfunc,NULL)<0){
                printf("usb event error\n");
            }
            return;
        }
        printf("%c\n",key);
    }
    return; 
}


void* irqfunc(void*myurbirq){                                                               //usbfunction
    struct myurbkbd* my_urbirq1=(struct myurbkbd*)myurbirq;
    pthread_t irq_thread;
    while(read(my_urbirq1->fdpipe,my_urbirq1->data,1)>0){
        if(pthread_create(&irq_thread,NULL,my_urbirq1->handle,(void*)myurbirq)<0){
            printf("irq_thread error\n");
            return 0;
        }
        pthread_join(irq_thread,NULL);
    }
    close(my_urbirq1->fdpipe);
    printf("\n");
    sleep(0.5);
    exit(0);
}



void* ackfunc(void*myurbled){                                                              //usbfunction
    struct myurbkbd* myurbled1=(struct myurbkbd*)myurbled;
    char c='C';
    char* alert=&c;
    while(1){
        sem_wait(&(mydev->led_done));
        write(ctl_pipe[1],alert,1);
        pthread_t ledthread;
        if(read(myurbled1->fdpipe,myurbled1->data,1)>0){
            if(pthread_create(&ledthread,NULL,myurbled1->handle,(void*)myurbled)<0){
                printf("ledthread error\n");
                return 0;
            }
            pthread_join(ledthread,NULL);
        }
        else{
            break;
        }
    }
    close(ctl_pipe[1]);
    close(myurbled1->fdpipe);
    return 0;
}





int urbdone(struct myurbkbd* myurbirq, struct myurbkbd* myurbled){                          //Creates polling threads for the first call
    sem_wait(&slock);
    if(ostatus==0){
        ostatus=1;
        sem_post(&slock);
        if(pthread_create(&corekbdirq,NULL,irqfunc,(void*)myurbirq)<0){
            printf("irq function error\n");
            return 0;
        }
        if(pthread_create(&corekbdctl,NULL,ackfunc,(void*)myurbled)<0){
            printf("ackknowledgment function error\n");
            return 0;
        }
        pthread_join(corekbdirq,NULL);
        pthread_join(corekbdctl,NULL);
        exit(0);
    }
    sem_post(&slock);
    sem_wait(&mydev->lockledurb);
    mydev->urdled_done=true;
    sem_post(&mydev->lockledurb);
    sem_post(&mydev->led_done);
    return 0;
}



void* irq_handler(void* myurbirq){                                                          //transfers key to input_report_key
    struct myurbkbd* my_urbirq1=(struct myurbkbd*)myurbirq;
    inputkey(*my_urbirq1->data);
    return 0;
}


void* led_handler(void* myurbled){
    sem_wait(&mydev->lockledurb);
    if(mydev->cmdled_wait==0){
        mydev->urdled_done=false;
        sem_post(&mydev->lockledurb);
    }
    else{
        *ledbuff=(*ledbuff+1)%2;
        mydev->cmdled_wait--;
        sem_post(&mydev->lockledurb);
        urbdone(NULL,mydev->urbled);
    }
    return 0;
}


int openmykbd(){                                                //initialize device data structure
    mydev=(struct mydevkbd*)malloc(sizeof(struct mydevkbd));
    mydev->urbirq=(struct myurbkbd*)malloc(sizeof(struct myurbkbd));
    mydev->urbled=(struct myurbkbd*)malloc(sizeof(struct myurbkbd));
    mydev->urbirq->data=(char*)malloc(sizeof(char));
    mydev->urbled->data=(char*)malloc(sizeof(char));

    *mydev->urbirq->data='m';
    mydev->urbirq->handle=&irq_handler;
    mydev->urbirq->old=(char*)malloc(sizeof(char));

    *mydev->urbirq->old='\0';
    mydev->urbirq->fdpipe=irqpipe[0];
    mydev->urbled->fdpipe=ackpipe[0];
    mydev->urbled->handle=&led_handler;
    mydev->urbled->old=(char*)malloc(sizeof(char));

    *mydev->urbled->old='\0';
    mydev->uppercase=0;
    mydev->cmdled_wait=0;
    sem_init(&(mydev->lockledurb),0,1);
    mydev->urdirq_done=true;
    mydev->urdled_done=false;

    if(sem_init(&(mydev->led_done),0,0)==0){
    	sem_post(&(mydev->led_done));
    	sem_wait(&(mydev->led_done));
    	
    }
    urbdone(mydev->urbirq, mydev->urbled);
    return 0;
}

void* irq_func(){                                                                       //endpoint
    int fd=open("input.txt",O_RDONLY);
    char* buf=(char*)malloc(sizeof(char));
    while(read(STDIN_FILENO,buf,1)>0){
        write(irqpipe[1],buf,1);
        sleep(0.5);
    }
    close(irqpipe[1]);
    close(fd);
    return 0;
}

void* ctl_func(){                                                                       //endpoint
    int fd=open("input.txt",O_RDONLY);
    char* uppercaseled=(char*)malloc(sizeof(char));
    char* bufread=(char*)malloc(sizeof(char));
    char bufwrite='A';
    int count=1;
    while(read(ctl_pipe[0],bufread,1)>0){
        uppercaseled=(char*)realloc(uppercaseled,count*sizeof(char));
        if(*ledbuff==0){
            uppercaseled[count-1]='0';
        }
        else{
            uppercaseled[count-1]='1';
        }
        write(ackpipe[1],&bufwrite,1);
        count++;
    }
    close(ackpipe[1]);
    close(ctl_pipe[0]);
    for(int i=0;i<count-1;i++){
        if(uppercaseled[i]=='0'){
            printf("OFF ");
        }
        else{
            printf("ON ");
        }
    }
    printf("\n");
    exit(0);
}


int main(){
    ledbuff=(int*)mmap(NULL,1,PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS,0,0);
    *ledbuff=0;
    sem_init(&slock,0,1);
    if(pipe(irqpipe)<0 || pipe(ackpipe)<0 || pipe(ctl_pipe)<0){
        printf("pipe error\n");
        return 0;
    }
    switch(fork()){
        case -1:
            printf("child error\n");
            return 0;
        case 0:
            close(irqpipe[0]);
            close(ackpipe[0]);
            close(ctl_pipe[1]);
            pthread_t kbdirq_endpoint;
            pthread_t kbdctl_endpoint;
            if(!pthread_create(&kbdirq_endpoint,NULL,irq_func,NULL)<0){
                printf("kbd irq error \n");
                return 0;
            }
            if(pthread_create(&kbdctl_endpoint,NULL,ctl_func,NULL)<0){
                printf("kbd ctl error \n");
                return 0;
            }
            pthread_join(kbdirq_endpoint,NULL);
            pthread_join(kbdctl_endpoint,NULL);
            return 0;
        default:
            close(irqpipe[1]);
            close(ackpipe[1]);
            close(ctl_pipe[0]);
            openmykbd();
    }
    return 0;
}
