#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
using namespace std;

#define DEFAULT_BUFLEN 1024

string passfile, dir;

typedef struct User {
    int client_sock;
    bool loggedin = false;
}User;

bool authUser(string file, string username, string pass);
string listFiles(string dir);
char* getFile(string filename, string dir);
bool deleteFile(string filename, string dir);
void quit();
void* client(void *arg);

int main(int argc, char *argv[])
{
    if(argc != 7) return 1;
    int port, opt;
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
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(port); // little endian to big endian
    if ( bind(server, (struct sockaddr*)&local_addr, sizeof(local_addr)) != 0 ){
        perror("Error binding");
        close(server);
        return 1;
    }
    if ( listen(server, 1) != 0 ){
        perror("Could not listen");
        close(server);
        return 1;
    }
    printf("FTP server now listening on port:");
    printf("%d\n", port);
    
    socklen_t remote_addrlen = sizeof(local_addr);

    while (true)
    {
         // ----------- aceepting ----------- 
        pthread_t clientt;
        User* user = new User;

        user->client_sock = accept(server, (struct sockaddr*)&local_addr, &remote_addrlen);
        printf("Connected: %s:%d\n", inet_ntoa(local_addr.sin_addr), ntohs(local_addr.sin_port));

        // ----------- creating a client thread ----------- 
        if ( pthread_create(&clientt, NULL, client, user) != 0 ){
            perror("Client thread created");
        }
        else{
            pthread_detach(clientt);
        }
    }
    return 0;
}

bool authUser(string file, string username, string pass){
    ifstream f(file);

    if (!f) {
        perror("Error in the password file");
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

    return filelist += "\n.\n";
}

char* getFile(string filename, string dir){
    string line, filepath = dir + '/' + filename;
    struct stat filestats;

    ifstream f(filepath);
    if (!f) return nullptr;

    lstat(filepath.c_str(), &filestats);
    char* filebytes = new char[filestats.st_size + 4];

    f.read(filebytes, filestats.st_size);
    // filebytes[filestats.st_size] = '\n';
    filebytes[filestats.st_size + 1] = '.';
    filebytes[filestats.st_size + 2] = '\n';
    f.close();

    return filebytes;
}

bool deleteFile(string filename, string dir){
    string filetodelete = dir + '/' + filename;
    if (remove(filetodelete.c_str()) != 0) return false; 
    return true;
}

void* client(void *arg){ // will take user struct as argument
    int bytes;
    User clientInfo = *(User *)arg; 
    string req, msg;

    do
    {
        char line[DEFAULT_BUFLEN];
        bytes = recv(clientInfo.client_sock, line, DEFAULT_BUFLEN, 0);
        if(bytes > 0){
            line[bytes] = '\0';
            req = strtok(line, " ");
            // handling requests
            //  ------------------------ Authentication ------------------------
            if(strncmp(req.c_str(), "USER", 4) == 0){
                    if (clientInfo.loggedin){
                        msg = "You are already logged in!\n";
                        if((bytes=send(clientInfo.client_sock, msg.c_str(), strlen(msg.c_str()), 0)) < 0){
                            printf("failed to send message\n");
                            break;
                        }
                    }else{
                    char *namet = strtok(NULL, " ");
                    char *passt = strtok(NULL, " ");
                    if (namet != nullptr && passt != nullptr)
                    {
                        string name(namet);
                        string pass(passt);
                        pass.pop_back();
                        clientInfo.loggedin = authUser(passfile, name, pass);
                        msg = "200 User " + name + " granted to access.\n";
                    }
                    if (clientInfo.loggedin){
                        if((bytes=send(clientInfo.client_sock, msg.c_str(), strlen(msg.c_str()), 0)) < 0){
                            printf("failed to send message\n");
                            break;
                        }
                    }else{
                        msg = "400 User not found. Please try with another user.\n";
                        if((bytes=send(clientInfo.client_sock, msg.c_str(), strlen(msg.c_str()), 0)) < 0){
                            printf("failed to send message\n");
                            break;
                        }
                    }
                }      
            }
            //  ------------------------ LIST DIR ------------------------
            else if (strncmp(req.c_str(), "LIST", 4) == 0){
                if (clientInfo.loggedin){
                    string filesstr = listFiles(dir);
                    if((bytes=send(clientInfo.client_sock, filesstr.c_str(), strlen(filesstr.c_str()), 0)) < 0){
                            printf("failed to send message: %s \n", msg);
                            break;
                    }
                }else{
                    msg = "You are not logged in!!\n";
                    if((bytes=send(clientInfo.client_sock, msg.c_str(), strlen(msg.c_str()), 0)) < 0){
                            printf("failed to send message: %s \n", msg);
                            break;
                    }
                }
            }
            //  ------------------------ GET FILE ------------------------
            else if (strncmp(req.c_str(), "GET", 3) == 0){
                if (clientInfo.loggedin){
                    char* filenamet = strtok(NULL, " ");
                    if (filenamet != nullptr){
                        string filename(filenamet);
                        filename.pop_back();
                        char* filedata = getFile(filename, dir);
                        if(filedata != nullptr){
                            if((bytes=send(clientInfo.client_sock, filedata, strlen(filedata), 0)) < 0){
                                printf("failed to send message: %s \n", msg);
                                break;
                            }
                        }else{
                            string msg = "404 File " + filename + " not found.\n";
                            if((bytes=send(clientInfo.client_sock, msg.c_str(), strlen(msg.c_str()), 0)) < 0){
                                printf("failed to send message: %s \n", msg);
                                break;
                            }
                        }
                    }
                }else{
                    msg = "You are not logged in!!\n";
                    if((bytes=send(clientInfo.client_sock, msg.c_str(), strlen(msg.c_str()), 0)) < 0){
                            printf("failed to send message: %s \n", msg);
                            break;
                    }
                }
            }
            //  ------------------------ PUT FILE------------------------
            else if (strncmp(req.c_str(), "PUT", 3) == 0){
                if (clientInfo.loggedin){
                    char* filenamet = strtok(NULL, " ");
                    if (filenamet != nullptr){
                        string filename(filenamet);
                        filename.pop_back();
                        char tmpline[DEFAULT_BUFLEN];
                        int tbytes;
                        int size = 0;
                        string filepath = dir + '/' + filename;
                        ofstream f(filepath, ios_base::binary);
                        if (f.is_open() == false){
                            msg = "400 File cannot save on server side.\n";
                            if((bytes=send(clientInfo.client_sock, msg.c_str(), strlen(msg.c_str()), 0)) < 0){
                                printf("failed to send message: %s \n", msg);
                            break;
                            }
                        }else{
                            do{
                                tbytes = recv(clientInfo.client_sock, tmpline, DEFAULT_BUFLEN, 0);
                                if (tbytes > 0) {
                                    f.write(tmpline, tbytes);
                                    size += tbytes;
                                } else if (tbytes == 0) {
                                    cout << "Connection closed by client" << endl;
                                    break;
                                } else {
                                    cout << "Problem with the connection with client " << clientInfo.client_sock << ", closing connection..." << endl;
                                    break;
                                }
                            }while((strncmp(tmpline, ".", 1) != 0) || tbytes > 2);
                            f.close();
                            msg = "200 " + to_string(size) + " Byte " + filename + " file retrieved by server and was saved.\n";
                            if((bytes=send(clientInfo.client_sock, msg.c_str(), strlen(msg.c_str()), 0)) < 0){
                                    printf("failed to send message: %s \n", msg);
                                    break;
                            }
                        }   
                    }
                }else{
                    msg = "You are not logged in!!\n";
                    if((bytes=send(clientInfo.client_sock, msg.c_str(), strlen(msg.c_str()), 0)) < 0){
                            printf("failed to send message: %s \n", msg);
                            break;
                    }
                }
            }
            //  ------------------------ Delete ------------------------
            else if (strncmp(req.c_str(), "DEL", 3) == 0){
                if (clientInfo.loggedin){
                    bool deleted = false;
                    char *filent = strtok(NULL, " ");
                    if (filent != nullptr){
                        string filename(filent);
                        filename.pop_back();
                        deleted = deleteFile(filename, dir);
                        if(deleted){
                            msg = "200 File " + filename + " deleted.\n";
                            if((bytes=send(clientInfo.client_sock, msg.c_str(), strlen(msg.c_str()), 0)) < 0){
                                printf("failed to send message: %s \n", msg);
                                break;
                            }
                        }else{
                            msg = "404 File " + filename + " is not on the server.\n";
                            if((bytes=send(clientInfo.client_sock, msg.c_str(), strlen(msg.c_str()), 0)) < 0){
                                printf("failed to send message: %s \n", msg);
                                break;
                            }
                        }
                    } 
                }else{
                    msg = "You are not logged in!!\n";
                    if((bytes=send(clientInfo.client_sock, msg.c_str(), strlen(msg.c_str()), 0)) < 0){
                            printf("failed to send message: %s \n", msg);
                            break;
                    }
                }
            }
            // ------------------------ QUIT ------------------------
            else if (strncmp(req.c_str(), "QUIT", 4) == 0){
                if (clientInfo.loggedin){
                    msg = "Goodbye!\n";
                    if((bytes=send(clientInfo.client_sock, msg.c_str(), strlen(msg.c_str()), 0)) < 0){
                        printf("failed to send message: %s \n", msg);
                    }
                    break;
                }
            }else{
                msg = "Incorrect command!!\n";
                if((bytes=send(clientInfo.client_sock, msg.c_str(), strlen(msg.c_str()), 0)) < 0){
                        printf("failed to send message: %s \n", msg);
                        break;
                }
            }
        }else if (bytes == 0 ) {
            printf("Connection closed by client\n");
            break;
        }else{
            printf("Problem with the connection with client %d, closing connection...\n", clientInfo.client_sock);
            break;
        }
    } while (bytes > 0);
    
    return 0;
}



