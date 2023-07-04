#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <openssl/md5.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define	SEMKEY1	((key_t) 7891) /* P2, ENC2 semaphore key */
#define	SEMKEY2	((key_t) 7892) /* ENC2, CHAN semaphore key */
#define	SEMKEY3	((key_t) 7893) /* CHAN, ENC2 semaphore key */
#define	SEMKEY4	((key_t) 7894) /* ENC2, P2 semaphore key */

struct shared_use_st {
	char some_text[2048];
    char checksum[2048];
    int probability;
    int flag;
    int process;
};

int main(int argc, char *argv[]){
    int shm_id, sem_id, sem_id2,sem_id3,sem_id4;
    char *shar_mem;;
    int pid2;
    
	struct shared_use_st *shared_stuff;
	char buffer[BUFSIZ];

    /*Shared memory */
    if((shm_id = shmget((key_t)1234, sizeof(struct shared_use_st) , IPC_CREAT|0600)) == -1){
        perror("shmget()");
            exit(EXIT_FAILURE);
    }

    if((shar_mem = shmat(shm_id,(void *) 0, 0)) == (void *) -1){
        perror("shmat()");
        exit(EXIT_FAILURE);
    }

    shared_stuff = (struct shared_use_st *)shar_mem;

    if(shared_stuff->process==2){ /*P2->P1-----------------------------------*/
    
        /* Create semaphor P2-ENC2*/
        if((sem_id = semget(SEMKEY1, 1, IPC_CREAT|0644)) == -1){
            perror("semget()");
            exit(EXIT_FAILURE);
        }

        /* Create semaphor ENC2-CHAN*/
        if((sem_id2 = semget(SEMKEY2, 1, IPC_CREAT|0644)) == -1){
            perror("semget()");
            exit(EXIT_FAILURE);
        }
        semctl(sem_id2, 0, SETVAL, 0);

        /* Structs for semaphor */
        struct sembuf enter, leave;
        enter.sem_num = leave.sem_num = 0;
        enter.sem_flg = leave.sem_flg = SEM_UNDO;
        enter.sem_op = -1;              /* DOWN-Operation */
        leave.sem_op = 1;               /* UP-Operation */


        /* Join critical area*/

        semop(sem_id, &enter, 1); /*P2-ENC2 DOWN*/

        char hash[MD5_DIGEST_LENGTH];
        MD5(shared_stuff->some_text, sizeof(shared_stuff->some_text), hash);
        strcpy(shared_stuff->checksum,hash);
        
        /*CHAN*/
        pid2 = fork (); 
        if(pid2<0){
            perror("fork Faild");
        }
        if(pid2==0){
            char* args[]={"./CHAN",NULL};
            execvp (args[0], args); 
        }

        semop(sem_id2, &leave, 1);/*ENC2-CHAN UP*/
        waitpid(pid2, NULL, 0);/*WAIT FOR CHAN*/

        /*RESEND MESSAGE*/
        if(shared_stuff->flag==0){
            semctl(sem_id2, 0, IPC_RMID);/*ENC2-CHAN RMV*/
            semctl(sem_id, 0, IPC_RMID);/*P2-ENC2 RMV*/
            exit(0);

        }

        semop(sem_id, &leave, 1); /*P2-ENC2 UP*/

        semctl(sem_id2, 0, IPC_RMID);/*ENC2-CHAN RMV*/
        semctl(sem_id, 0, IPC_RMID);/*P2-ENC2 RMV*/

        shmdt(shar_mem);/*DELETE SHARED_MEMORY*/
        shmctl(shm_id, IPC_RMID, 0);
        exit(0);

    }else{ /*P1->P2--------------------------------------------------------*/

        /* Create semaphor CHAN-ENC2*/
        if((sem_id3 = semget(SEMKEY3, 1, IPC_CREAT|0644)) == -1){
            perror("semget()");
            exit(EXIT_FAILURE);
        } 

        /* Create semaphor ENC2-P2*/
        if((sem_id4 = semget(SEMKEY4, 1, IPC_CREAT|0644)) == -1){
            perror("semget()");
            exit(EXIT_FAILURE);
        }
        semctl(sem_id4, 0, SETVAL, 0);  

        /* Structs for semaphor */
        struct sembuf enter, leave;
        enter.sem_num = leave.sem_num = 0;
        enter.sem_flg = leave.sem_flg = SEM_UNDO;
        enter.sem_op = -1;              /* DOWN-Operation */
        leave.sem_op = 1;               /* UP-Operation */

        /* Join critical area*/ 

        semop(sem_id3, &enter, 1); /*CHAN-ENC2 DOWN*/

        shared_stuff = (struct shared_use_st *)shar_mem;
      
        char hash[MD5_DIGEST_LENGTH];
        MD5(shared_stuff->some_text, sizeof(shared_stuff->some_text), hash);
        
        /*MEMORY COMPARE BETWEEN ENC1 AND ENC2 CHECKSUM*/ 
        if(memcmp(hash,shared_stuff->checksum,MD5_DIGEST_LENGTH)==0){/*SAME*/
            
            shared_stuff->flag=1;
        }
        else {/*RESEND MESSAGE*/
            shared_stuff->flag=0;
            semop(sem_id3, &leave, 1); /*CHAN-ENC2 UP*/
            exit(0);
        }

        semop(sem_id4, &leave, 1);/*ENC2-P2 UP*/

        semop(sem_id3, &leave, 1); /*CHAN-ENC2 UP*/
        
        semctl(sem_id4, 0, IPC_RMID);/*ENC2-P2 RMV*/
        semctl(sem_id3, 0, IPC_RMID);/*CHAN-ENC2 RMV*/

        shmdt(shar_mem);/*DELETE SHARED_MEMORY */
        shmctl(shm_id, IPC_RMID, 0);
        exit(0);
    
    }
    
    

}