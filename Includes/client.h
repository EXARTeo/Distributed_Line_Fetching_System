#pragma once //#include one time only

#include <semaphore.h>
#define SHM_NAME_SM1 "/SM1_shared_memory"

typedef struct SharedMemoryData{
    bool next;                      //If true we end the current client process
    bool end;                       //If true we end ALL the processes
    bool found;                     //Informs client if the line is found
    sem_t d_turn;                   //Makes the dispatcher wait until an input number
    sem_t cl_turn;                  //Makes the client wait for the dispacher outcome
    sem_t queue_sem;                //Put the clients into a line to give one by one the inputs
    char line[256];                 //Contains the line number
    char message[256];              //Contains the line text
} SharedMemoryData;
