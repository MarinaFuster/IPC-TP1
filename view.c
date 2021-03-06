#include <fcntl.h> 
#include <semaphore.h>
#include <signal.h>
#include <stdio.h> 
#include <stdlib.h>
#include <sys/ipc.h> 
#include <sys/select.h>
#include <sys/shm.h> 
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h> 

#define EXIT_ERROR 1
#define PIDLENGTH 10
#define SHARED_MEMORY_SIZE 50*150
#define TIME_LAPSE 7


int
main(int argc, char ** argv){


	char pid[PIDLENGTH];
	sleep(TIME_LAPSE);                      //Set timer of 3 seconds in order to wait for user to enter PID
	fgets(pid, PIDLENGTH, stdin);

    int applicationPID = atoi(pid); 
    if (applicationPID <= 0){
        fprintf(stderr, "Invalid PID\n");
        return(EXIT_ERROR);
    }

	key_t key = ftok("./application",1356);

	int shmid = shmget(key, 1024, 0666|IPC_CREAT);
    if(shmid<0){
        fprintf(stderr, "Error when creating shared memory segment\n");
        return(EXIT_ERROR);
    }

    char * shmadd = shmat(shmid, NULL, 0);
    if( shmadd == (char *)-1 ){
        fprintf(stderr, "Error when attaching shared memory segment to address\n");
        return(EXIT_ERROR);
    }


    // Open application semaphore
    sem_t * application_semaphore=sem_open("application_semaphore", O_CREAT, 0644, 1);


    // Open files semaphore
    sem_t * files_semaphore=sem_open("files_semaphore", O_CREAT, 0644, 1);

    
    // Printf hash results for files
    int i=0;
    int sem_value=0;
    sem_getvalue(files_semaphore,&sem_value);
    while(sem_value){
        sem_wait(application_semaphore);
        while(shmadd[i]!='\0'){
            if(shmadd[i]=='\n')
                sem_value--;
            putchar(shmadd[i++]);
        }
        sem_post(application_semaphore);
    }


    // Close semaphore
    sem_close(application_semaphore);
    sem_unlink("application_semaphore");
    sem_close(files_semaphore);
    sem_unlink("files_semaphore");

    
    // Free shared memory
    shmdt(shmadd);
    
    
    // Return
    return 0;
}