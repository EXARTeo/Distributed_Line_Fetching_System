#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <sys/time.h>
#include "../Includes/client.h"

void* sm1_shared_memory;


int main() {
    
    //Gaining access to the sm1 shared memory
    int fdSM1 = shm_open(SHM_NAME_SM1, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fdSM1 == -1) {
        perror("shm_open for SM1");
        exit(EXIT_FAILURE);
    }

    ftruncate(fdSM1, sizeof(SharedMemoryData));

    sm1_shared_memory = mmap(NULL, sizeof(SharedMemoryData), PROT_READ | PROT_WRITE, MAP_SHARED, fdSM1, 0);
    if (sm1_shared_memory == MAP_FAILED) {
        perror("mmap for SM1");
        exit(EXIT_FAILURE);
    }

    SharedMemoryData *sm1_data;
    sm1_data = (SharedMemoryData*)sm1_shared_memory;

    int value;
    //If the semaphore cannot be reached..
    if(sem_getvalue(&(sm1_data->queue_sem), &value) == -1){
        //..it means that the dispatcher - server has not started yet
        printf("Server is closed. Please try again later.\n");
        //free all memory
        munmap(sm1_shared_memory, sizeof(SharedMemoryData));
        return 0;
    }

    struct timeval start;
    struct timeval end;

    while(true){

        printf("Client PID: %d waiting in queue \n", getpid());

        sem_wait(&(sm1_data->queue_sem));                           //Each client wait for their turn
        
        if(sm1_data->end){
            printf("Server is closed\n");
            sem_post(&(sm1_data->queue_sem));                       //Let the next client in queue
            break;
        }

        printf("The server is busy, please wait...\n\n");
        sem_wait(&(sm1_data->cl_turn));

        char line[256];

        printf("Enter a line number to request from the server (or '#exit#' to quit or '#end#' to end everthing): ");
        if(fgets(line, 256, stdin) == NULL){
            strcpy(line, "#exit#");                                 //It means that we have to end the client
        }

        gettimeofday(&start, NULL);                                 //After we read we get the current time

        if(((strcmp(line, "#exit#\n") == 0) || (strcmp(line, "#exit#") == 0))){
            sm1_data->next = true;                                  //Inform the dispatcher to continue to an other client
            sem_post(&(sm1_data->queue_sem));
            sem_post(&(sm1_data->d_turn));
            break;                                                  //and go to end this client
        }

        if(((strcmp(line, "#end#\n") == 0) || (strcmp(line, "#end#") == 0))){
            sm1_data->end = true;                                   //Inform all clients and dispatcher-server to end
            sem_post(&(sm1_data->d_turn));
            sem_post(&(sm1_data->queue_sem));
            break;                                                  //and go to end this client
        }

        strcpy(sm1_data->line, line);

        sem_post(&(sm1_data->d_turn));
        sem_wait(&(sm1_data->cl_turn));                             //Make the client wait until the dispatcher inform the shared memory
        if(sm1_data->found){
            printf("The %d line : %s \n",atoi(sm1_data->line) ,sm1_data->message);
        }
        else{
            printf("Error: The file has fewer lines than expected or the input was not positive integer.\n");
        }
        gettimeofday(&end, NULL);                                   //After we get the result we get the current time again
        printf("Microseconds needed to get the result : %ld\n\n\n\n", end.tv_usec - start.tv_usec);
        sem_post(&(sm1_data->d_turn));
        sem_post(&(sm1_data->queue_sem));                           //Let the next client in queue
    }


    //free all memory
    munmap(sm1_shared_memory, sizeof(SharedMemoryData));

    return 0;
}
