#ifndef APPLICATION_H

#define EXIT_ERROR 1
#define FILES_PER_SLAVE 10
#define MD5_PATH "/usr/bin/md5sum"
#define MAX_FILE_LENGTH 50
#define SELF_PATH "./application"
#define SEPARATOR '\0'
#define SHARED_MEMORY_SIZE 90*150 // 90 equals to max length of <hash : file_name > , 150 max amount of files
#define SLAVE_PATH "./slave"
#define SLAVEQ 5
#define TOLERANCE 25 // If file quantity >= tolerance, files are released in a batch

#endif // APPLICATION_H
