#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h> 
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <fcntl.h>           
#include <sys/stat.h>        
#include <semaphore.h>
#include <signal.h>


#define PIDLENGTH 10
#define EXIT_ERROR 1
#define SHARED_MEMORY_SIZE 50*150

int main(int argc, char ** argv){

    // Adapted code from https://stackoverflow.com/questions/3711830/set-a-timeout-for-reading-stdin
	char pid[PIDLENGTH];
	sleep(3);                      //Set timer of 10 seconds
	fgets(pid, PIDLENGTH, stdin);

    int applicationPID = atoi(pid); 
    if (applicationPID <= 0 || kill(applicationPID,0) < 0){
        fprintf(stderr, "Invalid PID\n");
        return(EXIT_ERROR);
    }

	key_t key = ftok("./application",1356);

	int shmid = shmget(key,1024,0666|IPC_CREAT);
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
    sem_t * application_semaphore=sem_open("application_semaphore", O_CREAT, 0777, 0);
    
    // Create view semaphore
    sem_unlink("view_semaphore"); // Just in case...
    sem_t * view_semaphore;
    view_semaphore = sem_open("view_semaphore", O_CREAT, 0666, 1);
    if(view_semaphore == SEM_FAILED){
        fprintf(stderr, "Error when creating semaphore\n");
        return(EXIT_ERROR);
    }
    
    // Printf hash results for files

    int i=0;

    while(kill(applicationPID,0)>=0){
        int sem_value=0;
        sem_getvalue(application_semaphore,&sem_value);
        //printf("El valor del semaforo es%d\n",sem_value);
        while(shmadd[i]!='\0'){
            putchar(shmadd[i++]);
        }
    }
    
    while(shmadd[i]!='\0'){
        putchar(shmadd[i++]);
    }
    

    // Close semaphore
    sem_close(application_semaphore);
    sem_unlink("application_semaphore");


    // Free shared memory
    shmdt(shmadd);

    return 0;
}