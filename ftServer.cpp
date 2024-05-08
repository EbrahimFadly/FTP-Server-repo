#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
// #include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <iostream>
#include <fstream>
// #include <string>
using namespace std;

/* Definations */
#define DEFAULT_BUFLEN 1024
// #define PORT 4321

static int unum = 1;

typedef struct User {
    char username[200];
    char password[200];
    int usernum;
}User;

bool authUser();
void listFiles();
FILE getFile(FILE f);
int putFile();
int deleteFile(string filename, string dir);
void quit();


int main(int argc, char *argv[])
{
    if(argc != 6) return 1;
    int port, opt;
    string passfile, dir;

     while ((opt = getopt(argc, argv, "d:p:u:")) != -1) {
        switch (opt) {
            case 'd':
                dir = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'u':
                passfile = optarg;
                break;
            default:
                printf("Incorrect foramt: %s -d directory -p port -u password\n", argv[0]);
                return 1;
        }
    }
    if ((dir[0] == '\0' || passfile[0] == '\0')) return 1;
    
    return 0;
}

bool authUser(string file, string username, string pass){
    ifstream f(file);

    if (!f) {
        printf("Error in the password file");
        return false;
    }

    string line, pass;
    int pos;

    while (getline(f, line))
    {
        pos = line.find(':');
        if (line.substr(0, pos) == username){
            if (line.substr(pos+1) == pass) return true;
        }
    }
    return false;
}

void listFiles(string dir){

}

FILE getFile(string filename){

}

int putFile(FILE f){

}

int deleteFile(string filename, string dir){ // what if file has different extension other then .txt??
    string filetodelete = dir + '/' + filename + ".txt";
    if (remove(filetodelete.c_str()) != 0) return 1; 
    return 0;
}

void quit(){

}