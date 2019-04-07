#include <fcntl.h>
#include <semaphore.h> 
#include <signal.h>
#include <stdio.h> 
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdlib.h> // Check if this is needed!
#include "application.h"

int hash_of(char * file_buffer, int * hash_result_pipe);
void clean(char * buffer);


int
main(int argc, char ** argv){

    if(argc==1){
        fprintf(stderr, "At least one file to process is needed\n");
        return(EXIT_ERROR);
    }

    // Set up shared memory here!
    key_t key = ftok("./application",1357); 
    if(key == -1){
        perror("Error when executing ftok");
        exit(1);
    }

    int shmid = shmget(key, SHARED_MEMORY_SIZE, IPC_CREAT | 0666);
    if(shmid<0){
        fprintf(stderr, "Error when creating shared memory segment\n");
        return(EXIT_ERROR);
    }

    char * shmadd = shmat(shmid, NULL, 0); // Default 0: read/write
    if( shmadd == (char *)-1 ){
        fprintf(stderr, "Error when attaching shared memory segment to address\n");
        return(EXIT_ERROR);
    }
    memset(shmadd, 0, SHARED_MEMORY_SIZE);


    // Create pipes
    int files_pipe[2];
    pipe(files_pipe);
    int hash_result_pipe[2];
    pipe(hash_result_pipe);


    // Create slaves semaphore
    sem_unlink("slaves_semaphore"); // Just in case...
    sem_t * slaves_semaphore;
    slaves_semaphore = sem_open("slaves_semaphore", O_CREAT, 0644, 1);
    if(slaves_semaphore == SEM_FAILED){
        fprintf(stderr, "Error when creating semaphore\n");
        return(EXIT_ERROR);
    }


    // Create slaves
    pid_t slave_pid[SLAVEQ];
  
    for(int i=0; i<SLAVEQ; i++){
        printf("Creating slave %d...\n",i);
    
        pid_t pid=fork();
        if(pid==-1){
            fprintf(stderr,"Error when executing fork");
            return(EXIT_ERROR);
        }
        if(pid==0){
            
            char * file_buffer=calloc(1,MAX_FILE_LENGTH);

            while(1){
                
                sem_wait(slaves_semaphore);
                read(files_pipe[0], file_buffer, MAX_FILE_LENGTH);
                sem_post(slaves_semaphore);
                
                if(hash_of(file_buffer, hash_result_pipe)==EXIT_ERROR){
                    fprintf(stderr,"Error when executing fork");
                    return(EXIT_ERROR);
                }

                clean(file_buffer);

            }
            return 0;
        
        }
        slave_pid[i]=pid;
    }


    // Distribute files
    int files_quantity=argc-1;
    int distributed=1;
    
    char * file_buffer=calloc(1,MAX_FILE_LENGTH);
    
    while(distributed <= files_quantity){
        strncpy(file_buffer,argv[distributed],MAX_FILE_LENGTH);
        write(files_pipe[1], file_buffer, MAX_FILE_LENGTH); // Files are sent in fixed length packages
        clean(file_buffer);
        distributed++;
    }


    // Create application semaphore
    sem_unlink("application_semaphore"); // Just in case...
    sem_t * application_semaphore;
    application_semaphore = sem_open("application_semaphore", O_CREAT, 0644, 1);
    if(application_semaphore == SEM_FAILED){
        fprintf(stderr, "Error when creating semaphore\n");
        return(EXIT_ERROR);
    }


    // Read hash results
    int remaining_files=files_quantity;
    char * memory_p=shmadd;
    while(remaining_files>0){
        

        read(hash_result_pipe[0], memory_p, 1024);
        sem_post(application_semaphore);

        while((*memory_p)!='\0'){
            if((*memory_p)=='\n')
                remaining_files--;
            memory_p++;
        }        
    }
    *memory_p=-1;


    // Kill slaves
    for(int i=0; i<SLAVEQ; i++){
        //printf("Killing %d child, pid: %d\n",i, slave_pid[i]);
        kill(slave_pid[i], SIGKILL);
    }


    // Close pipes



    // Close semaphore
    sem_close(slaves_semaphore);
    sem_unlink("slaves_semaphore");
    //sem_unlink("application_semaphore");

    // Return
    return 0;
}

int
hash_of(char * file_buffer, int * hash_result_pipe){

    pid_t pid=fork();
    if(pid==-1){
        fprintf(stderr,"Error when executing fork");
        return(EXIT_ERROR);
    }
    else if(pid==0){
        char * execv_arguments[3];

        execv_arguments[0]=SELF_PATH;
        execv_arguments[1]=file_buffer;
        execv_arguments[2]=NULL;

        dup2(hash_result_pipe[1],STDOUT_FILENO);
        close(hash_result_pipe[0]);
        close(hash_result_pipe[1]);

        execv(MD5_PATH,execv_arguments);
    }
    return 0;
}

void
clean(char * buffer){
    int i=0;
    while(buffer[i]!='\0')
        buffer[i++]='\0';
}
