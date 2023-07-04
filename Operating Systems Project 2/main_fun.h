#include <stdlib.h>
#pragma once

typedef struct Node {
    char* p_num;
    char* of;
    char* r_w;
    int dirty_bit;
    struct Node *next;
} Node;

int hash_function (int p_num, int N);
int hextodec(char* hexVal);

char* GET_PAGE_NUM(char** str );
char* GET_OFFSET(char** str  );

int search_insert(Node* head, char** page_num, char** of, char** r_w, int max_frames);
int search_delete(Node* head, char** victim_num, int* read_counter, int* write_counter);

int LRU_Victim(int time[], int frame_num);

typedef struct SC_Node {
    char* p_num;
    int bit;
    int process;
    struct SC_Node *next;
    struct SC_Node *prev;
} SC_Node;

int push(SC_Node** first,SC_Node** last ,char** p_num,int process);
int delete_victim(SC_Node** first, SC_Node** last, char** victim_page_num,int victim);

char* SECOND_CHANCE(SC_Node** first,SC_Node** last,int* victim_process);