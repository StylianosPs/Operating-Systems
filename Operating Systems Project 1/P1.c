#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/sem.h>
#include <string.h>

#define	SEMKEY1	((key_t) 7891) /* p1, ENC1 semaphore key */
#define	SEMKEY4	((key_t) 7894) /* ENC1, P1 semaphore key */


struct shared_use_st {
	char some_text[2048];
    char checksum[2048];
    int probability;
    int flag;
    int process;
};

int main(int argc, char *argv[]){
    int shm_id, sem_id,sem_id4;
    char *shar_mem;
    int pid,process;
    char TERM[10]="TERM\n";
  
	struct shared_use_st *shared_stuff;
	char buffer[BUFSIZ];

    /*WHICH PROCESS FIRST*/
    if(argc!=1)
        process=1;
    else
        process=2;
    
   
    while(1){

        /* Create shared memory */
        if((shm_id = shmget((key_t)1234, sizeof (struct shared_use_st), IPC_CREAT|0600)) == -1){
            perror("shmget()");
                exit(EXIT_FAILURE);
        }
        if((shar_mem = shmat(shm_id,(void*) 0, 0)) == (void *) -1){
            perror("shmat()");
            exit(EXIT_FAILURE);
        }

        shared_stuff = (struct shared_use_st *)shar_mem;

        if(process==1){    /*P1->P2------------------------------------------------------*/

            /*GET MESSAGE*/
            printf("Enter some text: ");
            fgets(buffer, BUFSIZ, stdin);

            /*PROBABILITY*/
            shared_stuff->probability=20;
            shared_stuff->process=1;
            
            while(1){

                strncpy(shared_stuff->some_text, buffer, 2048);

                /* Create semaphor P1-ENC1*/
                if((sem_id = semget(SEMKEY1, 1, IPC_CREAT|0644)) == -1){
                    perror("semget()");
                    exit(EXIT_FAILURE);
                }
                semctl(sem_id, 0, SETVAL, 0);
            
                /*ENC1*/
                pid = fork (); 
            
                if(pid<0){
                    perror("fork Faild");
                }

                if(pid==0){
                    char* args[]={"./ENC1",NULL};
                    execvp (args[0], args); 
                }

                struct sembuf enter, leave;
                enter.sem_num = leave.sem_num = 0;
                enter.sem_flg = leave.sem_flg = SEM_UNDO;
                enter.sem_op = -1;              /* DOWN-Operation */
                leave.sem_op = 1;               /* UP-Operation */
                
                semop(sem_id, &leave, 1);/*P1-ENC1 UP*/
                waitpid(pid, NULL, 0);

                if(shared_stuff->flag!=0){
                    process=2;
                    shared_stuff->process=2;
                    semctl(sem_id, 0, IPC_RMID);/*P1-ENC1 RMV*/
                    break;
                }                    
            
                semctl(sem_id, 0, IPC_RMID);/*P1-ENC1 RMV*/

            }

            
            
        }else{ /*P2->P1----------------------------------------------*/

            /* Create semaphor ENC1-P1*/
            if((sem_id4 = semget(SEMKEY4, 1, IPC_CREAT|0644)) == -1){
                perror("semget()");
                exit(EXIT_FAILURE);
            }    

            /* Structs for semaphor */
            struct sembuf enter, leave;
            enter.sem_num = leave.sem_num = 0;
            enter.sem_flg = leave.sem_flg = SEM_UNDO;
            enter.sem_op = -1;              /* DOWN-Operation */
            leave.sem_op = 1;               /* UP-Operation */

            /* Join critical area */ 
            semop(sem_id4, &enter, 1); /*ENC1-P1 DOWN*/

            printf("PROCESS 1-> MESSAGE FROM PROCESS 2:  %s\n",shared_stuff->some_text);

            semop(sem_id4, &leave, 1); /*ENC1-P1 UP*/
            
            semctl(sem_id4, 0, IPC_RMID);/*ENC1-P1 RMV*/
            
            shared_stuff->process=1;
            process=1;

        
        }


        if(strcmp(shared_stuff->some_text,TERM)!=0){/*CONTINUE*/

            shmdt(shar_mem);/*DELETE SHARED_MEMORY*/
            shmctl(shm_id, IPC_RMID, 0);    
        
        }
        else{/*THE END*/
            
            shmdt(shar_mem);/*DELETE SHARED_MEMORY*/
            shmctl(shm_id, IPC_RMID, 0);
            break;
            
        }    
        
    }

exit(0);
return 1;
}