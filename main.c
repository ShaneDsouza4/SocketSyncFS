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

//Function to cgeck if a file is present as only .c files wil exist on the msin server
int containsCFiles(const char *filePath, const struct stat *FileInfo, int flag, struct FTW *ftwInfo){
    
    if(flag == FTW_F){ // Checks if is a File
        //printf("File Path Found: %s\n", filePath);

        //Extrct extension frm psth and deteminw if is a .c file
        const char *pathExt = getFileExtension(filePath); 
        if(pathExt != NULL && strcmp(pathExt, ".c") == 0){
            cFilesExist = true;
            return 1;
        }else{
            return 0;
        }
    }else{
        return 0;
    }
}

//Function that rwads a C fils and trnsfer it to the client
void downloadCFiles(const char *fullPath, int client){

    //Frst check if file eists in the direvtory
    if(!checkIfFileExists(fullPath)){ 
        printf("File does not exist on the server.\n");
        char *errmsg = "File does not exist on the server.";
        write(client, errmsg, strlen(errmsg) + 1);
        return;
    }else{
        char fileBuffer[MAX_LEN];
        long int bytesRead;
        long int bytesSent;

        //Reading File
        // Opens File Descrptor in Read Only permsion from the souce file
        int fdSrc = open(fullPath, O_RDONLY); 

        if (fdSrc == -1) { // If can not opn file send error mesage to client
            printf("Error whille opening the file.\n");
            char *errmsg = "File not found on server.";
            write(client, errmsg, strlen(errmsg) + 1); 
            close(fdSrc);
            return;
        }

        //Sening dfile to client, so clint can move to funvtion for proceing the chunkds it receives.
        write(client, "dfile", strlen("dfile") + 1);

        // Wqait for acknowledgmenwt from the client
        char ackBuffer[9];
        if (recv(client, ackBuffer, sizeof(ackBuffer) - 1, 0) < 0) {
            printf("Client acknowledgement failed\n");
            close(fdSrc);
            return;
        }

        ackBuffer[8] = '\0'; // Nul terwminate the acknowleddgment
        if (strcmp(ackBuffer, "SendFile") != 0) {
            printf("Invalid acknwledgment sent by client\n");
            close(fdSrc);
            return;
        }
        printf("Server allowed to send C File\n");

        //Readong file in chnks and sending to client
        int chunksSent = 0;
        while( ( bytesRead = read(fdSrc, fileBuffer, 1024) ) > 0 ){ //Read up to 1024 bytes in chunks from the server
            bytesSent = send(client, fileBuffer, bytesRead, 0); //Writing to the dest file
            if (bytesSent < 0) { //Iff err occurs during send
                printf("Error in sending file\n");
                close(fdSrc);
                return;    
            }else{
                //Checking file size based on chinks
                chunksSent = chunksSent + 1;
                if(chunksSent > 16000){
                    //printf("File size exceeds 5kb");
                }
            }

            // Clear buffer
            memset(fileBuffer, 0, sizeof(fileBuffer));
        }

        //Error check
        (bytesRead < 0) ? printf("Error occured while receiving file from server\n") : printf("File transfered successfully.\n");
        close(fdSrc);
    }
}

// Function to initiate downloading a file from the respevted serer and transfred to the client
void downloadHandler(char **commandArgv, int commandArgc, int client){
    
    //Convrrting argv into a single command with spaces after each command
    char buffer[MAX_LEN];
    buffer[0] = '\0';
    for (int i = 0; i < commandArgc; i++) {
        strcat(buffer, commandArgv[i]);  
        if (i < commandArgc - 1) { //Add space after evey commnd
            strcat(buffer, " "); 
        }
    }
    
    //Construting the full path
    const char *fullPath = constructFullPath(commandArgv[1]);

    //Extracting file extension
    const char *pathExt = getFileExtension(fullPath);

    //File extenson check to see which server to send to
    if (strcmp(pathExt, ".c") == 0) {
        downloadCFiles(fullPath, client);
    } 
    
}

//Functio to create .c tar files and send to client
void tarCFiles(int client){

    //Get current working directory of server to visit the entre directory
    char pwd[MAX_LEN];
    if (getcwd(pwd, sizeof(pwd)) == NULL) {
        //printf("Current Working Directory %s\n", pwd);  
        char *errmsg = "Server directory error.";
        write(client, errmsg, strlen(errmsg) + 1); 
    }

    //Check if C Files exist on the smain server bt traversing using the nftw system call
    const char *path = pwd;
    if(nftw(path, containsCFiles, 50, FTW_PHYS ) == -1){ //If error occurs whilw traversng
        printf("Error occurred while visiting the directory '%s'\n", path);
        char *errmsg = "Server directory error.";
        write(client, errmsg, strlen(errmsg) + 1); 
        return; 
    }

    //Check if C files exist in the directory, if not then send messfe to client
    if(!cFilesExist){
        printf("C files do not exist!\n");
        char *errmsg = "C files not exist on the server.";
        write(client, errmsg, strlen(errmsg) + 1);
        return;
    }

    //Creating tar file name, adding a timestamo to it
    char tarFileName[MAX_LEN];
    time_t currentTime = time(NULL);
    snprintf(tarFileName, sizeof(tarFileName), "%s/cTar-%ld.tar",path, time(NULL));

    //Peparing Tar executable Shell Commans commqnd. writ to the directory getting only all .c fles
    char tarExe[MAX_LEN];
    if(tarFileName && path){
        snprintf(tarExe, sizeof(tarExe), "tar -czf %s -C %s $(find . -name '*.c')", tarFileName, path);
    }
    

    //Executing the tar command
    int result = system(tarExe);
    if (result < 0) { //Error check
        printf("Failed to create tar file\n");
        char *errmsg = "Failed to create backup.";
        write(client, errmsg, strlen(errmsg) + 1);
        return;
    }else{
        printf("Tar file creation success\n");
    }

    //Send to client
    //Open tar file, witg read permissions
    int tarfd = open(tarFileName, O_RDONLY);
    if (tarfd < 0) { //Erro check and send message t client
        perror("Failed to open tar file for sending");
        const char *errmsg = "Failed to access tar file.";
        write(client, errmsg, strlen(errmsg) + 1);
        return;
    }

    //File transfer indicator so client know tar file download is about to come in
    write(client, tarFileName, strlen(tarFileName) + 1);

    // Waitt for acknowledgent from the clent
    char ackBuffer[9];
    if (recv(client, ackBuffer, sizeof(ackBuffer) - 1, 0) < 0) {
        printf("Client acknowledgement failed\n");
        close(tarfd);
        return;
    }

    ackBuffer[8] = '\0'; // Null termisnate the acknowledgment
    if (strcmp(ackBuffer, "SendFile") != 0) {
        printf("Invalid acknwledgment sent by client\n");
        close(tarfd);
        return;
    }
    printf("Server allowed to send .c tar File\n");

    char tarBuffer[MAX_LEN];
    long int bytesRead;
    long int bytesSent;

    //Reading file in chunks and sending to client
    int chunksSent = 0;
    while( ( bytesRead = read(tarfd, tarBuffer, 1024) ) > 0 ){ //Read up to 1024 bytes in chunks from the server
        bytesSent = send(client, tarBuffer, bytesRead, 0); //send to the client
        if (bytesSent < 0) {  //Iff err occurs during send
            printf("Error in sending file\n");
            close(tarfd);
            return;    
        }else{
            //Checking file size based on chinks
            chunksSent = chunksSent + 1;
            if(chunksSent > 16000){ //16mb
                //printf("File size exceeds 16mb\n");
            }
        }
        // Clear buffer
        memset(tarBuffer, 0, sizeof(tarBuffer));
        
    }
    //Error check
    (bytesRead < 0) ? printf("Error occured while receiving file from server\n") : printf("File transfered successfully.\n");

    close(tarfd);
}

//Function to handle tar server distribution via extensions
void tarHandler(char **commandArgv, int commandArgc, int client){

    //Converting argv into a sinle command with spaces after each command
    char buffer[MAX_LEN];
    buffer[0] = '\0';
    for (int i = 0; i < commandArgc; i++) {
        strcat(buffer, commandArgv[i]);  
        if (i < commandArgc - 1) { //Add space after every command
            strcat(buffer, " "); 
        }
    }

    //Check the file extension to know on which server processing must be done
    if (strcmp(commandArgv[1], ".c") == 0) {
        //printf("Process for DTAR .c\n");
        tarCFiles(client);
    }else if (strcmp(commandArgv[1], ".txt") == 0) {
       // printf("Process for DTAR .txt\n");
        downloadFromServers(buffer, "txt" , client);
    } else if (strcmp(commandArgv[1], ".pdf") == 0) {
        //printf("Process for DTAR .pdf\n");
        downloadFromServers(buffer, "pdf" , client);
    }
}

// Function to process client command
int prcclient(char* userinput, int client) {
    char cmd[MAX_LEN];
    char filename[MAX_LEN];
    char dest[MAX_LEN];
    char* mainfolder = "~smain";
    char destpath[MAX_LEN];
    sscanf(userinput, "%s %s %s", cmd, filename, dest);
    printf("%s, %s, %s\n", cmd, filename, dest);

    //Splitting command into individual commands for executionn
    int commandArgc  = 0;
    char *commandArgv[200]; 
    commandSplitter(userinput, commandArgv, &commandArgc);

    //Command checks to perform respected operations
    if(strcmp(commandArgv[0], "dfile") == 0){ //If command starts with dfile, user wants to download a file
        //printf("Processing for dfile\n");
        downloadHandler(commandArgv, commandArgc, client);
    }
    else if(strcmp(commandArgv[0], "dtar") == 0){ //If command starts with dtar, user wants to download a tar file
        //printf("Processing for dtar\n");
        tarHandler(commandArgv, commandArgc, client);
    }

    return 0;
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
