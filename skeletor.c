#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>

#define SLAVEQ 5
#define SIZE 100
#define MD5SUMPATH "/usr/bin/md5sum"
#define BUFFER_SIZE 256


int hashFunction(char * buffer, char * fileName);


int main(int argc, char** argv) {
    // PRIMERO CREO TODAS LAS VARIABLES Y LAS COSAS QUE NECESITE
    sem_t * pipeSemaphore = sem_open("pipeSemaphore", O_CREAT); 
    int keepProcessing=1; int fileNumber=1;

    char * buffer=calloc(BUFFER_SIZE,BUFFER_SIZE);
    char* fileName;

    while(keepProcessing){
      fileName=argv[fileNumber];

      hashFunction(buffer, fileName);
      write(1, buffer,BUFFER_SIZE);
      free(buffer);
      free(fileName);

      fileNumber++;

      if (fileNumber>argc){
        keepProcessing=0;
      }
    }


    return 1;
}

int hashFunction(char* buffer, char* fileName){
  int p[2];
  pipe(p);
  int pid;

  char* execv_arguments[2];
  execv_arguments[0]=fileName;
  execv_arguments[1]=NULL;

  if((pid=fork())==-1){
    perror("Error al crear el proceso\n");
    exit(EXIT_FAILURE);
  }

  else if(pid==0){

    dup2(p[1], STDOUT_FILENO);
    //close(p[1]);
    //close(p[0]);
    execv(MD5SUMPATH, execv_arguments);
  }

    read(p[0], buffer, BUFFER_SIZE);
    close(p[0]);
    close(p[1]);
    return 1;
}
