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

#define	SEMKEY1	((key_t) 7891) /* p2, ENC2 semaphore key */
#define	SEMKEY4	((key_t) 7894) /* ENC2, P2 semaphore key */

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
        process=2;
    else
        process=1;
   

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
        
        if(process==2){    /*P2->P1------------------------------------------------------*/
            
                /*GET MESSAGE*/
                printf("Enter some text: ");
                fgets(buffer, BUFSIZ, stdin);
                
                /*PROBABILITY*/
                shared_stuff->probability=20;
                shared_stuff->process=2;
                
                while(1){
                   
                    strncpy(shared_stuff->some_text, buffer, 2048);

                    /* Create semaphor P2-ENC2*/
                    if((sem_id = semget(SEMKEY1, 1, IPC_CREAT|0644)) == -1){
                        perror("semget()");
                        exit(EXIT_FAILURE);
                    }
                    semctl(sem_id, 0, SETVAL, 0);
                    
                
                    /*ENC2*/
                    pid = fork (); 
                  
                    if(pid<0){
                        perror("fork Faild");
                    }

                    if(pid==0){
                        char* args[]={"./ENC2",NULL};
                        execvp (args[0], args); 
                    }

                    struct sembuf enter, leave;
                    enter.sem_num = leave.sem_num = 0;
                    enter.sem_flg = leave.sem_flg = SEM_UNDO;
                    enter.sem_op = -1;              /* DOWN-Operation */
                    leave.sem_op = 1;               /* UP-Operation */
                    
                    semop(sem_id, &leave, 1);/*P2-ENC2 UP*/
                    
                    waitpid(pid, NULL, 0);

                    if(shared_stuff->flag!=0){
                        process=1;
                        shared_stuff->process=1;
                        semctl(sem_id, 0, IPC_RMID);/*P2-ENC2 RMV*/
                        break;
                    }    

                    semctl(sem_id, 0, IPC_RMID);/*P2-ENC2 RMV*/
                   
                }
                
            

        }else{ /*P1->P2----------------------------------------------*/


            /* Create semaphor ENC2-P2*/
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

            semop(sem_id4, &enter, 1); /*ENC2-P2 DOWN*/

            printf("PROCESS 2-> MESSAGE FROM PROCESS 1:  %s\n",shared_stuff->some_text);

            semop(sem_id4, &leave, 1); /*ENC2-P2 UP*/

            semctl(sem_id4, 0, IPC_RMID);/*ENC2-P2 RMV*/
            
            process=2;
            shared_stuff->process=2;
            
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