#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <regex.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PORT 3400
#define MAXSIZE 1024

int main(int argc, char *argv[]){


    //Check if User provided IP and PORT Number
    if(argc != 3){
        printf("ERROR: IP Address or PORT Number missing \n");
        printf("Please enter command line:%s <IP Address> <Port#>\n",argv[0]);
        exit(0);
    }


    //Display available commands
    printf("Select any of the commands to run on the smain server: \n");
    printf("1. Upload: ufile <filename> <destination_path>\n");
    printf("2. Download File: dfile <filename>\n");
    printf("3. Delete: rmfile <filename>\n");
    printf("4. Download Tar: dtar <file extesion>\n");
    printf("5. Display: display <directory_path>\n");
    printf("Please note 'file names' and 'paths' must be a tilde expansion path\n");

    while(1){ //Infinite loop start

        int server;
        int portNumber;
        char message[MAXSIZE];
        struct sockaddr_in servAdd;
        char userCommand[MAXSIZE];
        int commandArgc  = 0;
        char *commandArgv[200]; 


        //Converting individual args into a single buffer
        char buffer[MAXSIZE];
        buffer[0] = '\0';
        for (int i = 0; i < commandArgc; i++) {
            strcat(buffer, commandArgv[i]);  
            if (i < commandArgc - 1) {
                strcat(buffer, " "); 
            }
        }

        //Create Socket
        if( (server = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
            printf("Failed to create Socket.\n");
            exit(2);
        }

        //Connecting IP and PORT Number on Server Object.
        servAdd.sin_family = AF_INET; //Internet
        sscanf(argv[2], "%d", &portNumber);
        servAdd.sin_port = htons((uint16_t)portNumber);//Port number

        if( inet_pton(AF_INET, argv[1], &servAdd.sin_addr) < 0 ){ //IP Address Connection
            printf("inet_pton() failure.\n");
            exit(3);
        }

        //Connect System Call
        if(connect(server, (struct sockaddr *) &servAdd,sizeof(servAdd))<0){//Connect()
            printf("connect() failure.\n");
            exit(4);
        }

        //write(server, userCommand, strlen(userCommand) + 1); // Include null terminator in write
        long int bytesWrite = write(server, buffer, strlen(buffer) + 1); // Include null terminator in write
        if (bytesWrite < 0) {
            printf("Client: write() failure\n");
            exit(4);
        }

        
        //Read from pipe and display
        int bytes_read = read(server, message, MAXSIZE - 1);
            if (bytes_read < 0) {
                printf("Client: read() failure\n");
                exit(3);
            }
            message[bytes_read] = '\0';
            printf("Server: %s\n", message);


        close(server);
    } //Infinite loop end

    exit(0);
    return 0;
}