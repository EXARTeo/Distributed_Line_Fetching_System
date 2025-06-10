#pragma once //#include one time only

#include <semaphore.h>
#define SHM_NAME_SM2 "/SM2_shared_memory"

typedef struct DispatcherData{
    bool flag;                  //If true then there is no line found
    bool end;                   //If true we end ALL the processes
    sem_t d_turn;               //Make the dispatcher wait for the server outcome
    sem_t s_turn;               //Make the server wait for the dispatcher input
    char file_name[256];
    char line[256];             //Contains the line number
    char message[256];          //Contains the line's text
} DispatcherData;
