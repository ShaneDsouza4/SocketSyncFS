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

// Utility Fucton to remove the \n and trim te strting end ending white spaces if presnt
void trimAndRemoveNewLine(char *input){
    if(input[0] != '\0' && input != NULL){ //Check if first character is not empty, n has characters

        //Index variables
        int startIndex = 0;
        int endIndex = strlen(input) - 1;

        //Replace New lin with Null and updting endIndex as can be --> "sample.c \n"
        if(input[endIndex] == '\n'){
            input[endIndex] = '\0';
            endIndex--;
        }

        // Identifying the spaces from index 0 and updating count
        while (input[startIndex] == ' ') {
            startIndex++;
        }

        // Identifying the spaces from last index nd updating count in revers
        while (input[endIndex] == ' ') {
            endIndex--;
        }
        
        //Shifting the start and end index values to the start
        int j = 0;
        int i = startIndex;
        while(i<= endIndex){
            input[j] = input[i];
            j++;
            i++;
        }
        input[j] = '\0'; //Null terminator mst be added to strings
    }
}  

// Function to check if file exists
bool checkIfFileExists(const char *filepath){
    struct stat fileInfo;
    int fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        return false;
    }

    close(fd);
    return true;
}

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

//Utility finction to check the file extensin of the path entered by user
bool checkFileExtension(const char *file){
    if(file == NULL){ //Check if user entered a file argument
        return false;
    }

    //Extract the Extension frim the path via a reverse loop
    const char *pathExt = NULL;
    for(int i = strlen(file); i>=0; i--){

        // Identifing the forst dot from reverse
        if(file[i] == '.'){ //abc.pdf => .pdf
            pathExt = &file[i];//Store in the extension
            break; // Exit the loop when found
        }
    }

    // If no dot is found or "." happens to be entered by the user like ~smain/.
    if (pathExt == NULL || pathExt == file) {
        return false; 
    }

    // Compare the extension with the allowed ones .txt .pdf .c
    return (strcmp(pathExt, ".txt") == 0 || strcmp(pathExt, ".pdf") == 0 || strcmp(pathExt, ".c") == 0) ? true : false;
}

//Utility funcrtion to check the path passed by the user
bool checkTildePath(const char *path){
    //Check if user has entred a path 
    if(!path) return false;

    //Check if path starts with ~smain
    if (strncmp(path, "~smain/", strlen("~smain/")) == 0 || strcmp(path, "~smain") == 0) {
        return true;  
    }

    return false;
}

//Utility function to check the user commands wrt operation commands
bool checkInput(char **commandArgv, int commandArgc){

    //Upon presing Enter wil not di anything
    if(commandArgc < 1){
        return false;
    }

    //Validity Checks
    if(strcmp(commandArgv[0], "ufile") == 0){ //Check validity based on operation command entered
       
        //Check length as 3 allowed "ufile sample.c  ~smain/folder1/sample.c" 
        if(commandArgc > 3){
            printf("Only 3 arguments are allowed.\n");
            return false;
        }

        //Check the secomd arg is a file and has correct file extension
        if(!checkFileExtension(commandArgv[1])){
            printf("Second command must be a file with extension, like sample.c\n");
            printf("Note: Only .c .pdf .txt files allowed\n");
            return false;
        }

        //Check the tilde expansion path
        if(!checkTildePath(commandArgv[2])){
            printf("Second command must be a path, like ~smain/folder1/folder2\n");
            printf("Note: path must begin with ~smain \n");
            return false;
        }

        if(!checkIfFileExists(commandArgv[1])){
            printf("File does not exist on client\n");
            return false;
        }

    } 
    else if(strcmp(commandArgv[0], "dfile") == 0 || strcmp(commandArgv[0], "rmfile") == 0){ //Validity check for dfile or rmfile command

        //Check length as 2 allowed "dfile ~smain/sample.c" or "rm ~smain/sample.c
        if(commandArgc > 2){
            printf("Only 2 arguments are allowed.\n");
            return false;
        }

        //Check the tilde expansion path
        if(!checkTildePath(commandArgv[1])){
            printf("Second command must be a path, like ~smain/folder1/folder2/sample.c\n");
            printf("Note: path must begin with ~smain \n");
            return false;
        }

        //Check if the last path is a file with the valid extension
        if(!checkFileExtension(commandArgv[1])){
            printf("Second command must be a file with extension, like sample.c\n");
            printf("Note: Only .c .pdf .txt files allowed\n");
            return false;
        }

    }  
    else if(strcmp(commandArgv[0], "display") == 0 ){ //Validity check for display command
        //Check length as 2 allowed "display ~smain/folder1"
        if(commandArgc > 2){
            printf("Only 2 arguments are allowed.\n");
            return false;
        }

        //Check the tilde expansion path
        if(!checkTildePath(commandArgv[1])){
            printf("Second command must be a path, like ~smain/folder1/folder2\n");
            printf("Note: path must begin with ~smain \n");
            return false;
        }
    } 
    else if(strcmp(commandArgv[0], "dtar") == 0){ //Validity check for dtar command

        //Check length as 2 allowed "dfile ~smain/sample.c" or "rm ~smain/sample.c
        if(commandArgc > 2){
            printf("Only 2 arguments are allowed.\n");
            return false;
        }

        //Check if valid exension is provided
        if (strcmp(commandArgv[1], ".c") != 0 && strcmp(commandArgv[1], ".pdf") != 0 && strcmp(commandArgv[1], ".txt") != 0) {
            printf("Invalid extension '%s'. Please enter '.txt', '.pdf', or '.c'.\n", commandArgv[1]);
            return false;
        }
    }else{ //If no command matches
        printf("Invalid command passed.\n");
        return false;
    }

    return true;
}

//Utility function to extrsct the file name
const char* extractFileName(const char* path){
    const char *fileName = NULL;
    for(int i = strlen(path); i>=0; i--){
        if(path[i] == '/'){ //abc.pdf => .pdf
            fileName = &path[i];//Store in the pathExt variable
            break; // Exit the loop
        }
    }
    return fileName;
}

//Function to create destination path where the file hs to be saved
char* createDestinationPath(const char *filePath) {

    //Get the current pwd
    char pwd[1024];
    if (getcwd(pwd, sizeof(pwd)) == NULL) {
        printf("Current Working Directory can not be determined.\n"); 
    }

    //Extract the file name
    const char *fileName = extractFileName(filePath);

    //Constructing the full destination path
    static char destinationPath[MAXSIZE];
    strcpy(destinationPath, pwd);
    strcat(destinationPath, "/");
    strcat(destinationPath, fileName + 1);

    return destinationPath;
}

//Function to download a fil received from the server onto the client pwd
void downloadingFile(int server, const char *filePath){

    //Constructng the destinatin directory
    char *destinationPath = createDestinationPath(filePath);
    if (destinationPath == NULL) {
        printf("Download path could not be created.\n");
        return;
    }
    
    char buffer[MAXSIZE];
    long int bytesRead;

    // Opens File Descrptor in Read Only from the soufce file
    umask(0000);
    int fdDest = open(destinationPath, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if (fdDest < 0) { //Error Check
        printf("Error occured while creating file.\n");
        return;
    }

    // Rece9ive data and write into a destination file
    int chunksSent = 0;
    while ( ( bytesRead = recv(server, buffer, sizeof(buffer), 0)) > 0) { //Read up to 1024 bytes in chunks from the server
        long int bytesWrite = write(fdDest, buffer, bytesRead); //Writing to the dest file
        if (bytesWrite < 0) { //Iff err occurs during write
            printf("Error occured when writing to destination file");
            close(fdDest);
            return;
        }else{
            //Checking file size based on chinks
            chunksSent = chunksSent + 1;
            if(chunksSent > 16000){ //16mb
                //printf("File size exceeds 16mb\n");
            }
        }
        // Clear buffer
        memset(buffer, 0, sizeof(buffer));    
    }
    //If errr occus when readong from the data sent by server
    (bytesRead < 0) ? printf("Error occured while receiving file from server\n") : printf("File transfered successfully.\n");

    close(fdDest);
}

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