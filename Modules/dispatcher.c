#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdbool.h>
#include "../Includes/client.h"
#include "../Includes/dispatcher.h"

void* sm1_shared_memory;
void* sm2_shared_memory;

int main(int argc, char *argv[]) {

    if(argc < 2){
        perror("No file detected\n");
        exit(EXIT_FAILURE);
    }

        /*Shared data between dispatcher - client*/

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

    sm1_data->next = false;
    if(sem_init(&(sm1_data->cl_turn), 1, 1) == -1){             //We need a client to start..
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    if(sem_init(&(sm1_data->d_turn), 1, 0) == -1){              //..so the dispatcher wait for the first client
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    if(sem_init(&(sm1_data->queue_sem), 1, 1) == -1){           //In the beginning the queue is open
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    sm1_data->found = false;
    sm1_data->end = false;

        /*Shared data between dispatcher - server*/

    int fdSM2 = shm_open(SHM_NAME_SM2, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fdSM2 == -1) {
        perror("shm_open for SM2");
        exit(EXIT_FAILURE);
    }

    sm2_shared_memory = mmap(NULL, sizeof(DispatcherData), PROT_READ | PROT_WRITE, MAP_SHARED, fdSM2, 0);
    if (sm2_shared_memory == MAP_FAILED) {
        perror("mmap for SM2");
        exit(EXIT_FAILURE);
    }

    ftruncate(fdSM2, sizeof(DispatcherData));

    sm2_shared_memory = mmap(NULL, sizeof(DispatcherData), PROT_READ | PROT_WRITE, MAP_SHARED, fdSM2, 0);
    if (sm2_shared_memory == MAP_FAILED) {
        perror("mmap for SM2");
        exit(EXIT_FAILURE);
    }

    DispatcherData *sm2_data;
    sm2_data = (DispatcherData*)sm2_shared_memory;

    sm2_data->flag = false;

    if(sem_init(&(sm2_data->s_turn), 1, 0) == -1){      //Server will wait for the input
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    if(sem_init(&(sm2_data->d_turn), 1, 0) == -1){      //Dispatcher will wait for server response
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    //Determine the file name to open
    if (argc > 1) {
        strcpy(sm2_data->file_name, argv[argc - 1]);
    }

    sm2_data->end = false;                      //Informs server when to end


    //We make 2 proccesses one for the dispatcher and one for the server
    int server_id = fork();

    if (server_id == 0) {
        //Replace the current process with the "server" executable
        execlp("./server", "server", NULL);
        //If execlp() fails, process will continue at this line
        perror("exec failed");
        sm2_data->end = true;                       //Informs the dispatcher and clients to end
        exit(EXIT_FAILURE);
    }
    else if (server_id > 0) {
        while (true) {

            sem_wait(&(sm1_data->d_turn));
            
            if(sm2_data->end == true){              //If any error occurred in server we check to end the processes    
                sm1_data->end = true;               //Inform the clients to end
                sem_post(&(sm1_data->cl_turn));     //Makes sure that there will be no stuck clients
                break;
            }

            if(sm1_data->end == true){
                sm2_data->end = true;
                sem_post(&(sm2_data->s_turn));      //Inform server to end
                break;
            }

            if(sm1_data->next ){                    //If the client who was in line left,
                sm1_data->next = false;             //return next to false for the rest of the clients,
                sem_post(&(sm1_data->cl_turn));     //let the next client to get input,
                continue;                           //and continue to the next one
            }

            strcpy(sm2_data->line, sm1_data->line);

            sem_post(&(sm2_data->s_turn));          //AKA make server to try find the line
                /*during this interval 
                the server is running*/
            sem_wait(&(sm2_data->d_turn));          //Only after the server finished..

            //..the dispatcher update the sm1
            if(sm2_data->flag){
                sm1_data->found = false;
            }
            else{
                strcpy(sm1_data->message, sm2_data->message);
                sm1_data->found = true;
            }
            //After the updated sm1, client gets their results
            sem_post(&(sm1_data->cl_turn));         //One post to print the results
            sem_wait(&(sm1_data->d_turn));          //Wait for the print
            sem_post(&(sm1_data->cl_turn));         //And one to allow the client to take input
        }
    }
    else{
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    
    //free all memory
    munmap(sm1_shared_memory, sizeof(SharedMemoryData));
    munmap(sm2_shared_memory, sizeof(SharedMemoryData));
    shm_unlink(SHM_NAME_SM1);
    shm_unlink(SHM_NAME_SM2);

    return 0;
}
