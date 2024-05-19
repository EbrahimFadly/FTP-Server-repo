# FTP-Server-repo

a modified version of the File Transfer Protocol.

This FTP server program allows users to connect and perform various file operations over a network. Users can authenticate themselves using a password file and then list, upload, download, and delete files on the server.

## How to Compile

To compile the program on Linux, use the following command:
```sh
g++ ftServer.cpp -o ftServer
```

## How to Run

```sh 
./ftServer -d <directory> -p <port> -u <password_file>
```
- Replace <directory> with the path of the directory where your files are stored.
- Replace <port> with the port number you want the server to listen on.
- Replace <password_file> with the path to the password file.

## Connecting to the Server

To connect to the server, you can use netcat on Linux:
```sh
nc <host> <port>
```

## Notes!
- Ensure the server has appropriate permissions to access the specified directory and password file.
- The port number should be between 1024 and 65535.
- The password file should follow the format: username:password on each line.




