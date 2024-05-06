#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <pthread.h>

/* Definations */
#define DEFAULT_BUFLEN 1024
// #define PORT 4321

static int unum = 1;

typedef struct User {
    char username[200];
    char password[200];
    int usernum;
}User;

User createUser();
void listFiles();
FILE getFile(char filename[]);
void putFile();
void deleteFile();
void quit();


int main(int argc, char *argv[])
{
    if(argc != 6) return 1;
    int port, opt;
    char passfile[200] = "";
    char dir[200] = "";

     while ((opt = getopt(argc, argv, "d:p:u:")) != -1) {
        switch (opt) {
            case 'd':
                strncpy(dir, optarg, sizeof(dir) - 1);
                dir[sizeof(dir) - 1] = '\0';
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'u':
                strncpy(passfile, optarg, sizeof(passfile) - 1);
                passfile[sizeof(passfile) - 1] = '\0';
                break;
            default:
                printf("Incorrect foramt: %s -d directory -p port -u password\n", argv[0]);
                return 1;
        }
    }
    if ((dir[0] == '\0' || passfile[0] == '\0')) return 1;
    
    return 0;
}


User createUser(){

}

void listFiles(){

}

FILE getFile(char filename[]){

}

void putFile(FILE f){

}

void deleteFile(char filename[]){

}

void quit(){

}