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
#include <sys/stat.h>
#include <dirent.h>
// #include <string>

using namespace std;

#define DEFAULT_BUFLEN 1024

static int unum = 1;

typedef struct User {
    int client_sock;
    char username[200];
    char password[200];
    struct sockaddr_in ip_addr;
    time_t conntime;
    int userid;
}User;



bool authUser();
string listFiles();
FILE getFile(FILE f);
int putFile();
int deleteFile(string filename, string dir);
void quit();
void createClient(int clientSocket);
void* client(void *arg);



int main(int argc, char *argv[])
{
    if(argc != 7) return 1;
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
    if (port < 1024 || port > 65535)
    {
        printf("Port can not be use, choose another port");
        return 1;
    }
    if((access(passfile.c_str(), R_OK)) < 0){
        perror("Error with the passwords file");
        return 1;
    }
    if((access(dir.c_str(), R_OK)) < 0){
        perror("Error with the directory");
        return 1;
    }
    
    
    // -----------  creating the socket ----------- 
    int server;
    if ((server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("Can't create socket!");
        return 1;
    }

    // -----------  binding ----------- 
    struct sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(port); // little endian to big endian

    if ( bind(server, (struct sockaddr*)&local_addr, sizeof(local_addr)) != 0 ){
        perror("Error binding");
        return 1;
    }
    if ( listen(server, 1) != 0 ){
        perror("Could not listen");
        return 1;
    }

    printf("FTP server now listening on port: %d", port);


    // struct sockaddr_in remote_addr;
    socklen_t remote_addrlen = sizeof(local_addr);

    while (true)
    {
         // ----------- aceepting ----------- 
        pthread_t clientt;
        User user;

        user.client_sock = accept(server, (struct sockaddr*)&local_addr, &remote_addrlen);
        printf("Connected: %s:%d\n", inet_ntoa(local_addr.sin_addr), ntohs(local_addr.sin_port));

        // ----------- creating a client thread ----------- 
        if ( pthread_create(&clientt, NULL, client, &user) != 0 )
            perror("Client thread created");
        else
            pthread_detach(clientt);
    }
    return 0;
}

bool authUser(string file, string username, string pass){
    ifstream f(file);

    if (!f) {
        printf("Error in the password file");
        return false;
    }

    string line;
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

string listFiles(string dir){
    string filelist = "";
    string fname, fsize, fpath;
    struct dirent *dirfile;
    struct stat filestats;
    DIR *directory = opendir(dir.c_str());

    while ((dirfile = readdir(directory)) != NULL)
    {
        fname = dirfile->d_name;
        fpath = dir + "/" + fname;
        lstat(fpath.c_str(), &filestats);
        fsize = to_string(filestats.st_size);
        filelist += "\n" + fname + " " + fsize;
    }

    return filelist += "\n.";
}

int putFile(FILE f){

}

int deleteFile(string filename, string dir){ 
    string filetodelete = dir + '/' + filename;
    if (remove(filetodelete.c_str()) != 0) return 1; 
    return 0;
}

void* client(void *arg){ // will take user struct as argument
    char line[DEFAULT_BUFLEN];
    int bytes;
    User clientInfo = *(User *)arg;
    bool loggedin = false;

    do
    {
        bytes = recv(clientInfo.client_sock, line, sizeof(line), 0);
        if(bytes > 0){
            // handling requests
        }else if (bytes == 0 ) {
            printf("Connection closed by client\n");
            break;
        }else{
            printf("Problem with the connection with client %s, closing connection...\n");
            break;
        }
    } while (bytes > 0);
    

    printf(""); // print username exit message, session duration, connection time
    return;
}

