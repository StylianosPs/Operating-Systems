#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main_fun.h"

int hash_function (int p_num, int N) {
    return p_num % N;//FIND BUCKET POSITION
}

int hextodec(char* hexVal) { 
	int len = strlen(hexVal); 
	
	
	int base = 1; 
	
	int dec_val = 0; 
	
	// GET CHARACTERS AS A DIGITS
	for (int i=len-1; i>=0; i--) 
	{ 
	
        //IF CHARACTER IS 0-9
		if (hexVal[i]>='0' && hexVal[i]<='9') 
		{ 
			dec_val += (hexVal[i] - 48)*base; 
			base = base * 16; 
		} 

		// IF THE CHARACTER IS A-F
		else if (hexVal[i]>='A' && hexVal[i]<='F') 
		{ 
			dec_val += (hexVal[i] - 55)*base; 

			base = base*16; 
		}
        //IF CHARACTER IS a-f
        else if(hexVal[i] >= 'a' && hexVal[i] <= 'f')
        {
            dec_val += (hexVal[i] - 87) * base;
            base *= 16;
        }
	} 
	
	return dec_val; 
}




char* GET_PAGE_NUM(char** str  ){
    char str1[9];
	char page_number[6];
    char* pn = malloc(6*sizeof(char));
    
    strcpy(str1,*str);

    page_number[0]=str1[0];
    page_number[1]=str1[1];
    page_number[2]=str1[2];
    page_number[3]=str1[3];
    page_number[4]=str1[4];
    
    strcpy(pn,page_number);

    return pn;//RETURN PAGE NUM
    
}

char* GET_OFFSET(char** str  ){
    char str1[9];
	char offset[4];
    char* of = malloc(6*sizeof(char));
    
    strcpy(str1,*str);

    offset[0]=str1[5];
    offset[1]=str1[6];
    offset[2]=str1[7];
    
    
    strcpy(of,offset);

    return of;//RETURN OFFSET
}


////////////////// HASHED PAGE TABLES ///////////////////////////////////////

int search_insert(Node* head, char** page_num, char** of,char** r_w, int max_Frames) {
   
    
    static int counter=0;
    Node* prev = NULL;
    Node* current = head;  

    while (current != NULL) {//SEARCH FOR PAGE_NUM
        
        if (strcmp(current->p_num,*page_num)==0){ //IF THE PAGE_NUM EXIST

            if( strcmp(*r_w,"W\n")==0)
                current->dirty_bit=1;
              
            strcpy(current->r_w,*r_w);//CHANGE R/W
            
            return 1; 
        }
        
        prev=current;
        current = current->next; 
    } 
    
    
    if(counter!=max_Frames){//IF THE MEM IS NOT FULL CREATE NEW NODE IN THE HASH TABLE 
    
        Node* new = malloc(sizeof(Node));
        new->p_num=*page_num;
        new->of=*of;
        new->r_w=malloc(3*sizeof(char));
        strcpy(new->r_w,*r_w);

        if( strcmp(*r_w,"W\n")==0)
            new->dirty_bit=1;
        else
            new->dirty_bit=0;

        new->next=NULL;
        prev->next=new;
       
        counter++;
     
        return 2;

    }else{//IF THE MEM IS FULL

        counter--;
        
        return 3;
    }
  
    
} 
int search_delete(Node* head, char** victim_num, int* read_counter, int* write_counter) {

    Node* prev = NULL;
    Node* current = head;  
    
    while (current != NULL) {// SEARCH AND DELETE THE PAGE NUM
        
        
        if (strcmp(current->p_num,*victim_num)== 0){ 
            
            *read_counter+=1;

            if(current->dirty_bit == 1)
                 *write_counter+=1;

            prev->next=current->next;
            free(current->r_w);
            free(current);

            
            return 1; 
        }
        
        prev=current;
        current = current->next; 
    } 
    return -1;
   
    
} 


/////////////////////////////////// LRU //////////////////////////////////////////

int LRU_Victim(int time[], int frame_num){
	int minimum = time[0], victim_position = 0;
 
	for(int i = 1; i < frame_num; ++i){// FIND THE OLDEST(MINIMUM ARRAIVAL TIME) AND RETURN POSITION IN ARRAY
		if(time[i] < minimum){
			minimum = time[i];
			victim_position = i;
		}
	}

	return victim_position;
}



//////////////////// SECOND CHANCE //////////////////////////////////




int push(SC_Node** first,SC_Node** last ,char** p_num,int process) {
    
    if(strcmp((*first)->p_num,"-1")==0){// IF ITS THE FIRST PUSH IN QUEUE
          
        (*first)->bit=0;
        (*first)->process=process;
        strcpy((*first)->p_num,*p_num);
        return 1;
        
    }
   
    //NEW NODE
    SC_Node* new = malloc(sizeof(SC_Node));
    new->p_num=malloc(6*sizeof(char));
    new->bit=0;
    strcpy(new->p_num,*p_num);
    new->process = process;
    new->next=(*last);
    new->prev=(*first);
    
    (*first)->next=new;
    (*last)->prev=new;
    (*last)=new;
    return 2;   
} 
int delete_victim(SC_Node** first, SC_Node** last, char** victim_page_num,int victim_process){

    SC_Node* current =(*last);
   
    while(current!=(*first)){//FIND AND DELETE THE VICTIM IN QUEUE 
      
        if(strcmp(current->p_num,*victim_page_num)==0 && current->process==victim_process){
            
            
            (*last)=current->next;
            (*first)=current->prev;
            
            current->prev->next=current->next;
            current->next->prev=current->prev;

            free(current->p_num);
            free(current);
            return 1;
        }

        current=current->next;
    }
    

    if(strcmp(current->p_num,*victim_page_num)==0 && current->process==victim_process){
       
        (*last)=current->next;
        (*first)=current->prev;
        
        current->prev->next=current->next;
        current->next->prev=current->prev;
       
        free(current->p_num);
        free(current);
    }

}





char* SECOND_CHANCE(SC_Node** first,SC_Node** last,int* victim_process){

    SC_Node* current =(*first);
    
    while(1){// FIND THE FIRST PAGE WITH BIT 0
    
        if(current->bit==0){
            *victim_process = current->process;
            return current->p_num;
        }  
        
        current->bit=0; 
        current=current->prev;
    }
    

}