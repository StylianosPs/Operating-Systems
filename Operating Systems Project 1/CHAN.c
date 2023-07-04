#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>

#define	SEMKEY2	((key_t) 7892) /* ENC1 OR ENC2, CHAN semaphore key */
#define	SEMKEY3	((key_t) 7893) /* CHAN, ENC2 OR ENC1 semaphore key */

struct shared_use_st {
	char some_text[2048];
    char checksum[2048];
    int probability;
    int flag;
    int process;
};

int main(int argc, char *argv[]){ 
    int pid3,shm_id, sem_id2,sem_id3;
    char *shar_mem;
    int i=0;
   
	struct shared_use_st *shared_stuff;
	char buffer[BUFSIZ];

    /* Create semaphor ENC1 OR ENC2-CHAN*/
    if((sem_id2 = semget(SEMKEY2, 1, IPC_CREAT|0644)) == -1){
        perror("semget()");
        exit(EXIT_FAILURE);
    }    

    /* Create semaphor CHAN-ENC2 OR ENC1*/
    if((sem_id3 = semget(SEMKEY3, 1, IPC_CREAT|0644)) == -1){
        perror("semget()");
        exit(EXIT_FAILURE);
    }
    semctl(sem_id3, 0, SETVAL, 0);

    /*Shared memory */
    if((shm_id = shmget((key_t)1234, sizeof(struct shared_use_st) , IPC_CREAT|0600)) == -1){
        perror("shmget()");
            exit(EXIT_FAILURE);
    }
    if((shar_mem = shmat(shm_id,(void *) 0, 0)) == (void *) -1){
        perror("shmat()");
        exit(EXIT_FAILURE);
    }
    
    /* Structs for semaphor */
    struct sembuf enter, leave;
    enter.sem_num = leave.sem_num = 0;
    enter.sem_flg = leave.sem_flg = SEM_UNDO;
    enter.sem_op = -1;              /* DOWN-Operation */
    leave.sem_op = 1;               /* UP-Operation */

    /* Join critical area*/
    
    semop(sem_id2, &enter, 1); /*ENC1 OR ENC2-CHAN DOWN*/

    shared_stuff = (struct shared_use_st *)shar_mem;
    
    /*CHANGE MESSAGE*/
    srand(time(0));
    while(shared_stuff->some_text[i]!='\0'){

        if((rand() % 100 +1)<shared_stuff->probability)
            shared_stuff->some_text[i]='*';
        i++;
    }
    
    /*ENC2 or ENC1*/
    pid3 = fork (); 
    if(pid3<0){
        perror("fork Faild");
    }
    /*ENC1 OR ENC2*/
    if(shared_stuff->process==1){
        if(pid3==0){
            char* args[]={"./ENC2",NULL};
            execvp (args[0], args); 
        }
    }else{
        if(pid3==0){
            char* args[]={"./ENC1",NULL};
            execvp (args[0], args); 
        }
    }

    semop(sem_id3, &leave, 1);/*CHAN-ENC2 OR ENC1 UP*/
    waitpid(pid3, NULL, 0);/*WAIT FOR ENC1 OR ENC2*/

    /*RESEND MESSAGE*/
    if(shared_stuff->flag==0){
        semctl(sem_id3, 0, IPC_RMID);/*CHAN-ENC2 OR ENC1 RMV*/
        semctl(sem_id2, 0, IPC_RMID);/*ENC1 OR ENC2-CHAN RMV*/
        exit (0); 

    }
    
    semop(sem_id2, &leave, 1); /*ENC1 OR ENC2-CHAN UP*/
    
    
    shmdt(shar_mem);/*DELETE SHARED MEMORY*/
    shmctl(shm_id, IPC_RMID, 0);

    semctl(sem_id3, 0, IPC_RMID);/*CHAN-ENC2 OR ENC1 RMV*/
    semctl(sem_id2, 0, IPC_RMID);/*ENC1 OR ENC2 -CHAN RMV*/
    exit (0); 


}