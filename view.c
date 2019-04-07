#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h> 
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <stdio.h> 

#define PIDLENGTH 10
#define EXIT_ERROR 1

int main(int argc, char ** argv){

	//char pid[PIDLENGTH];
	//sleep(2); //Set timer of 2 seconds
	//fgets(pid, PIDLENGTH, stdin);
	
  
	key_t key = ftok("./application",1111);

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

    printf("Hola");

    return 0;
}