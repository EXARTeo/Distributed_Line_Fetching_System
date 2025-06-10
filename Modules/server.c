#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdbool.h>
#include "../Includes/dispatcher.h"

void* sm2_shared_memory;

int main() {

    printf("Server is starting...\n");
    //Gaining access to the sm2 shared memory
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
        perror("mmap for SM1");
        exit(EXIT_FAILURE);
    }

    DispatcherData *sm2_data;
    sm2_data = (DispatcherData*)sm2_shared_memory;

    char *file_name = sm2_data->file_name;

    FILE *file = fopen(file_name, "r");
    if (file == NULL) {
        perror("Unable to open the file");
        sm2_data->end = true;               //If there is a problem with the file end every process
        exit(EXIT_FAILURE);
    }

    printf("Server has started.\n\n");

    while(true){

        sem_wait(&(sm2_data->s_turn));      //Wait for an input

        if(sm2_data->end){
            //End the process
            break;
        }
        sm2_data->flag = true;              //Informs dispatcher if no line was found with the given input

        int line_number = atoi(sm2_data->line);
        char *buffer = NULL;
        size_t len = 0;
        ssize_t read;
        int current_line = 1;

        //Read through the file and find the right line if exists
        while((read = getline(&buffer, &len, file)) != -1){
            if (current_line == line_number) {
                //When line is found, send it back to the dispatcher
                strcpy(sm2_data->message, buffer);
                sm2_data->flag = false;
                break;
            }
            current_line++;
        }

        if (sm2_data->flag) {
            printf("Error: The file has fewer lines than expected or the input was not positive integer.\n");
        }
        //Inform the dispatcher that result is found
        sem_post(&(sm2_data->d_turn));
        //Rewind the file to the beginning for each new attempt
        rewind(file);
    }

    fclose(file);

    munmap(sm2_shared_memory, sizeof(DispatcherData));

    return 0;
}
