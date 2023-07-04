#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main_fun.h"




int main(int argc, char** argv) {

    int program;
    if(strcmp(argv[1],"lru") == 0 ) // LRU OR SC
        program=1;
    else if(strcmp(argv[1],"sc") == 0 )
        program=2;
    else {
        printf("ERROR WRONG INPUT\n");
        return -1;
    }
    
    
    int max=atoi(argv[2]);//MAX TRACE NUM
    int q =atoi(argv[4]); 
    int process=2;	// BZIP=1 OR GCC=2
    int N = 200;	// SIZE OF LINE
    int b=atoi(argv[3]); //NUMBER OF BUCKETS
    int flag;   
    char line[N] ;	
    int bucket_num;
    int frame_num=atoi(argv[3]);//NUMBER OF FRAMES
    int counter=0;
    int victim_position;
    
    if(max<1 || frame_num<1 || q<1){
        printf("ERROR WRONG INPUT\n");
        return -1;
    }

    int page_fault_counter = 0 ; //PAGE FAULT COUNTER
    int victim_read_counter = 0; 
    int victim_write_counter = 0;
    int bzip_records_counter = 0;
    int gcc_records_counter = 0;

    char* FRAMES[frame_num];
    int TIME[frame_num];
    int page_process[frame_num];

    Node * bzip_hash[b];
    Node * gcc_hash[b];
   
    Node * head;
    Node * head1;

    SC_Node* new=malloc(sizeof(SC_Node));
    SC_Node* first=new;
    SC_Node* last=new;
    new->next=last;
    new->prev=last;
    new->p_num= malloc(sizeof(char)*6);
    strcpy(new->p_num,"-1");
    new->bit=0;
    

    for(int i=0; i<frame_num; i++ ){ //FRAME,TIME,PROCESS INITIALIZATION FOR LRU
        FRAMES[i] = malloc(6*sizeof(char));
        FRAMES[i] = "-1";
        TIME[i] = -1;
        page_process[i]=-1;
    }

    for(int i=0; i<b; i++){//BZIP AND GCC HASHED PAGE TABLE INITIALIZATION
        bzip_hash[i] = malloc(sizeof(Node));
    
        bzip_hash[i]->p_num=malloc(6*sizeof(char));
        bzip_hash[i]->of=" ";
        bzip_hash[i]->dirty_bit=0;
        bzip_hash[i]->next=NULL;
        bzip_hash[i]->r_w=malloc(3*sizeof(char));

        gcc_hash[i] = malloc(sizeof(Node));
        gcc_hash[i]->p_num=malloc(6*sizeof(char));
        gcc_hash[i]->of=" ";
        gcc_hash[i]->dirty_bit=0;
        gcc_hash[i]->next=NULL;
        gcc_hash[i]->r_w=malloc(3*sizeof(char));
    }

    FILE* fd = fopen("bzip.trace", "r+");
    FILE* fd2 = fopen("gcc.trace","r+");

    for(int i=0; i<max;i++){ // MAIN LOOP
        
        if((i%q)==0 && i!=0) //CHANGE PROCESS
            if(process == 1)
                process = 2;
            else if(process == 2)
                process = 1;

        if(process == 1){ //READ LINE FROM FILE
            fgets(line, N, fd); 
            bzip_records_counter++;  
        }else if(process == 2){
            fgets(line, N, fd2);
            gcc_records_counter++;
        } 
        
        char* page_num = malloc(6*sizeof(char)) ; 
        char* offset = malloc(4*sizeof(char));
        
        char* logical_address = strtok(line, " ");
        char* RW= strtok(NULL, " ");
        
        page_num = GET_PAGE_NUM(&logical_address);//GET PAGE NUMBER
        offset = GET_OFFSET(&logical_address);//GET OFFSET

        
        int dec = hextodec(page_num);//CHANGE HEX PAGE NUMBER  TO DEC

        bucket_num = hash_function(dec,b);//FIND BUCKET POSITION
     
        if(process == 1)
            head = bzip_hash[bucket_num];
        else if(process == 2)
            head = gcc_hash[bucket_num];

        flag = search_insert(head,&page_num,&offset,&RW,frame_num);//SEARCH FOR THE PAGE NUMBER AND RETURN THE CURRENT SITUATION
        

        
        if(program == 1){////////////////////////////////////LRUL///////////////////////////////
            for(int j=0; j<frame_num; j++){
                
                if(flag == 1){// IF THE PAGE IS ALREADY IN MEM
                    
                    if(strcmp(FRAMES[j],page_num) == 0 && page_process[j]==process){// FIND THE PAGE IN FRAMES AND INCREESE THE TIME COUNTER
                        counter++;
                        TIME[j] = counter;
                        break;  
                    } 
                }

                if(flag == 2){// IF THE MEM IS NOT FULL

                    if(strcmp(FRAMES[j],"-1") == 0){//INSERT THE PAGE IN THE FRAME ARRAY AT THE POSITION WITH -1
                        page_fault_counter ++;
                        victim_read_counter++;
                        counter++;
                        TIME[j] = counter;
                        FRAMES[j] = page_num;
                        page_process[j] = process;
                        break;
                    }
                }

                if(flag == 3){// IF THE MEM IS FULL 

                    page_fault_counter++;

                    victim_position=LRU_Victim(TIME,frame_num);//FIND VICTIM WITH LRU

                    int dec1 = hextodec(FRAMES[victim_position]);//CHANGE VICTIM PAGE NUM FROM HEX TO DEC
                    int bucket_num1 = hash_function(dec1,b);//FIND BUCKET POSITION 
                
                    if(page_process[victim_position]==1)//DELETE THE VICTIM FROM HASH TABLE
                        search_delete(bzip_hash[bucket_num1],&FRAMES[victim_position],&victim_read_counter,&victim_write_counter);
                    if(page_process[victim_position]==2)
                        search_delete(gcc_hash[bucket_num1],&FRAMES[victim_position],&victim_read_counter,&victim_write_counter);
                        
                    search_insert(head,&page_num,&offset,&RW,frame_num);//INSERT THE NEW PAGE NUMBER IN THE HASH TABLE
                    

                    counter++;//INSERT THE PAGE IN THE FRAME ARRAY AT THE POSITION OF THE VICTIM
                    TIME[victim_position] = counter;
                    FRAMES[victim_position] = page_num;
                    page_process[victim_position] = process;
                    break;
                }
                 
            } 

        }else if(program == 2){//////////////////////////////////////////////////////SECOND CHANCE//////////////////////////////////////////

            

            if(flag == 1){// IF THE PAGE IS ALREADY IN MEM
                SC_Node* current =last;

                while(current!=first){//FIND THE PAGE IN QUEUE AND CHANGE THE BIT

                    if(strcmp(current->p_num,page_num)==0 && current->process == process)
                        current->bit=1;

                    current=current->next;
                }

                if(strcmp(current->p_num,page_num)==0 && current->process == process)
                    current->bit=1;
                
               
                              
            }

            if(flag == 2){// IF THE MEM IS FULL 

                page_fault_counter ++;
                victim_read_counter++;
                push(&first,&last,&page_num,process); //INSERT THE PAGE IN QUEUE
               
            }
            

            if(flag == 3){// IF THE MEM IS FULL 
                int victim_process=-1;
                page_fault_counter++;
          
                char* victim_page=SECOND_CHANCE(&first,&last,&victim_process);// FIND THE VICTIM WITH SECOND CHANCE
             
                
                int dec1 = hextodec(victim_page);//CHANGE VICTIM PAGE NUM FROM HEX TO DEC 
                int bucket_num1 = hash_function(dec1,b);//FIND BUCKET POSITION

                if(victim_process == 1)// DELETE VICTIM FROM HASH TABLE
                    search_delete(bzip_hash[bucket_num1],&victim_page,&victim_read_counter,&victim_write_counter);
                if(victim_process == 2)
                    search_delete(gcc_hash[bucket_num1],&victim_page,&victim_read_counter,&victim_write_counter);
                        
                search_insert(head,&page_num,&offset,&RW,frame_num);// INSERT NEW PAGE IN HASH TABLE
             
                delete_victim(&first,&last,&victim_page,victim_process);//DELETE VICTIM FROM QUEUE

                push(&first,&last,&page_num,process);//INSERT VICTIM FROM QUEQUE
            

            }

              
        } 
     
    }
      

    printf("NUMBER OF PAGE_FAULTS:\t%d\nNUMBER OF FRAMES:\t%d\nVICTIM_READ:\t%d\nVICTIM_WRITE:\t%d\nBZIP_COUNTER:\t%d\nGCC_COUNTER:\t%d\n",page_fault_counter,frame_num,victim_read_counter,victim_write_counter,bzip_records_counter,gcc_records_counter);


    return 0;
}