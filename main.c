#define _GNU_SOURCE  // for strsep
#define _XOPEN_SOURCE 500  // for nftw
#include <stdio.h> 
#include <stdlib.h> 
#include <ftw.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <unistd.h>  
#include <string.h> 
#include <stdint.h> 
#include <limits.h> 
#include <fcntl.h>
#include <errno.h> 
#include <stdbool.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <sys/socket.h>
#include <netdb.h>
#include <regex.h>
#include <time.h>
#include <dirent.h>
#define MAX_LEN 1024

//Glbal Variabe to keep track if fles exists in the swrver directiry
bool cFilesExist = false;

// Utility funcion to brek down comands into indivydal commsnds to prepare for execution
void commandSplitter(char *input, char **commandArgv, int *commandArgc) {
    int index = 0;
    char *currentCmd;

    //Iterates and seperate the commands based on " " identified
    while ((currentCmd = strsep(&input, " ")) != NULL) { 
        if (*currentCmd != '\0') { //Empty check
            commandArgv[index] = currentCmd; //Adds cCarrent cmmand in argv
            //printf("Arg Index---> %s\n", argv[index]);
            index++;
        }
    }

    commandArgv[index] = NULL; // Addin Null terminate the last index is necessaru
    *commandArgc = index; //Assigning the value of index as count ref
}

//Utility Functon to chck if a specifc file existss in the directory
bool checkIfFileExists(const char *filepath){
    struct stat fileInfo; // Stat func gets directory info

    // S_ISREG macro chek the file mode to determinee if it's a file
    return (stat(filepath, &fileInfo) == 0 && S_ISREG(fileInfo.st_mode)) ? true : false;
}

//Utility function to eztract a file name from a path
const char* extractFileName(const char* path){
    const char *fileName = NULL;
    for(int i = strlen(path); i>=0; i--){
        if(path[i] == '/'){ //abc.pdf => .pdf
            fileName = &path[i]; //Store in the pathExt variable
            break; // Exit the loop
        }
    }
    return fileName;
}

//Utiity function to constuct the ful absolute pth
char *constructFullPath(const char *path){
    
    //Get the curent pwd
    char pwd[MAX_LEN];
    if (getcwd(pwd, sizeof(pwd)) != NULL) {

        //Constructig the full absolute path
        static char fullPath[200];
        strcpy(fullPath, pwd);
        strcat(fullPath, path + 6); //Add after ~smain
        return fullPath;
    }else{
        printf("Current Working Directory can not be determined.\n");  
    }
}

//Utilit funcion to get the extwnsion of a file by checkong it backwards
const char* getFileExtension(const char *fullPath) {
    const char *pathExt = NULL;
    for(int i = strlen(fullPath) - 1; i >= 0; i--) {
        if(fullPath[i] == '.') { // Find the lst dot in the path
            pathExt = &fullPath[i];
            break; // Exit thr loop oce the extension is found
        }
    }
    return pathExt;
}


int main(int argc, char *argv[]) {

    int sd, client, portNumber;
    struct sockaddr_in servAdd;

    // Check if the correct arguments number
    if(argc != 2){
        fprintf(stderr,"Call model: %s <Port#>\n",argv[0]);
        exit(0);
    }

    // Create socket
    if((sd = socket(AF_INET, SOCK_STREAM, 0))<0){
        fprintf(stderr, "Could not create socket\n");
        exit(1);
    }

    // Initialize the server address structure
    servAdd.sin_family = AF_INET;
    servAdd.sin_addr.s_addr = htonl(INADDR_ANY);
    sscanf(argv[1], "%d", &portNumber);
    servAdd.sin_port = htons((uint16_t)portNumber);

    // Bind socket to server address
    bind(sd, (struct sockaddr *) &servAdd,sizeof(servAdd));

    // Listen for incoming connections
    if (listen(sd, 5) < 0) {
        perror("listen failed");
        exit(1);
    }

    // To accept a new client and process the command in a new process
    while(1) {
        // Accept a connection
        client=accept(sd,(struct sockaddr*)NULL,NULL);//accept()
        if(client < 0) {
            perror("Error accepting connection");
            continue;
        }
        // Create a new process where client is serviced
        int pid = fork();
        // Child process handles the client
        if (pid == 0) {
            char buff1[MAX_LEN];
            int bytes_read = read(client, buff1, MAX_LEN); // Get command from client
            if (bytes_read < 0) {
                printf("Server: read() failure\n");
                exit(3);
            }

            int success = prcclient(buff1, client);  // Process client's command


            close(client);
            printf("\n");
            exit(0);
        } else if (pid > 0) { // Parent continues to accept new connection
            close(client);
        }
    }
}
