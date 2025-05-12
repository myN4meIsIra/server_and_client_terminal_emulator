//TCP server

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <pwd.h>

#include "connectionSettings.c"


//initial/global variables
char tokenizedString[MAX][MAX];

struct sockaddr_in   server_addr, client_addr;
int                  mysock, csock;         // socket descriptors
int                  r, len, n;             // help variables

/////////////////////////////////execute command///////////////////////////////
int runCommand(char **command)
{
    //create strings
    char src[255];
    char executablePath[255];

    //copy strings to the src and executablePath
    strcpy(executablePath,"/bin/");
    strcpy(src,  command[0]);
    strcat(executablePath,src);

    // child process identification number
    pid_t  pid;
    int status;

    printf("executable path: %s", executablePath);
    //fork the process
    pid = fork();

    //parent
    if ( pid )
    {
        pid = wait( &status );
        //printf("child killed\n");

    }

        //child
    else
    {
        char* envp[] = { NULL };
        //printf("running command %s", executablePath);
        if (execve(executablePath, command, envp) == -1)
            printf("%s: command not found\n", command[0]);

        exit(1);
    }

    //printf("end of runCommand\n");
    return 1;
}


/////////////////////////////////tokenizeString/////////////////////////////////
char **tokenizeString(const char *PATHNAME, const char *KEY)
{
    //clear the tokanized string
    bzero( tokenizedString, MAX );


    //initial variables
    int i;
    char s [256];

    //allocate space for the array of strings for the path
    char **LocaltokenizedString = malloc(sizeof(char) * 255);

    //tokenize the string PATHNAME based on the key KEY
    strcpy(s, PATHNAME);
    char* token = strtok(s, KEY);
    i = 0;

    //while there are still characters left
    while (token != NULL)
    {
        LocaltokenizedString[i++] = token;
        //printf("token: %s\n", token);
        token = strtok(NULL, KEY);
    }

    //ensure that the tokenizedString is null-terminated
    LocaltokenizedString[sizeof(LocaltokenizedString)] = NULL;


    for(int i = 0; LocaltokenizedString[i] != NULL; i++)
    {
        //printf("(tokenizeString) Term %d of tokenizedString (which has size %ld): %s \n",i,sizeof(LocaltokenizedString[i]),LocaltokenizedString[i]);
        strcpy(tokenizedString[i], LocaltokenizedString[i]);
    }

    runCommand(LocaltokenizedString);

    return LocaltokenizedString;
}



//initiate the server
int server_init()
{
    printf("------------Starting Server------------\n");
    // create a TCP socket by socket() syscall

    mysock = socket( AF_INET, SOCK_STREAM, 0 );
    if ( mysock < 0 ) {
        printf( "   Error: socket call failed\n" );
        exit( -1 );
    }

    // initialize the server_addr structure
    server_addr.sin_family = AF_INET;                   // for TCP/IP
    server_addr.sin_addr.s_addr = htonl( INADDR_ANY );  // This HOST IP
    server_addr.sin_port = htons( SERVER_PORT );        // port number 1234

    r = bind( mysock,(struct sockaddr*)&server_addr, sizeof(server_addr) );
    if ( r < 0 ) {
        printf( "   Error: Bind failed\n" );
        exit( -1 );
    }
    printf( "   Hostname = %s port = %d\n", SERVER_HOST, SERVER_PORT );

    listen( mysock, 5 );                               // queue length = 5
    printf("------------Server Ready------------\n");
}


//function for forked process
void connection(int csock)
{
    char  line[MAX];

    while ( 1 ) {
        n = read(csock, line, MAX);
        if (n == 0) {
            printf("Client died\n");
            close(csock);
            exit(0);
            break;
        }

        // show the line string
        printf("Message Recieved: \"%s\"\n", line);

        //send the client information about the filesystem
        if (!strcmp(line, "returnServerFilesystemInformation")) {

            printf("\tDetected request for server information\n");

            char cwd[MAX];
            char * pwd = getcwd(cwd, MAX);

            //build the server output string: concatenate the various elements of information to the output string
            char serverInformation[MAX] = "server:";
            strcat(serverInformation,cwd);
            //strcat(serverInformation, ":");

            strcpy(line, serverInformation);
            n = write(csock, serverInformation, MAX);
            printf("\tSent client server information\n\n");
        }

            // echo line to client
        else
        {

            //execute the command from client
            tokenizeString(line, " ");


            n = write(csock, line, MAX);

            printf("Sent message : \"%s\"\n", line);
            printf("\nReady\n");
        }
    }

}

//main function
int main()
{
    char  line[MAX];
    server_init();

    while ( 1 ) {                         // Try to accept a client request
        // Try to accept a client connection as descriptor newsock
        len = sizeof(client_addr);
        csock = accept( mysock, (struct sockaddr *)&client_addr, &len );
        if ( csock < 0 ) {
            printf( "Error: error in accepting connection\n" );
            exit( -1 );
        }
        printf( "Connection accepted\nClinet: IP = %s port = %d\n\n", inet_ntoa( client_addr.sin_addr ), ntohs( client_addr.sin_port )  );



        ////fork the system when a new user connects/////////////////////////////////

        // child process identification number
        pid_t  pid;
        int status;

        //printf("executable path: %s\n\n\n", executablePath);
        //fork the process
        pid = fork();

        //parent
        if ( pid )
        {
            //do nothing... except for prepare for another user
        }

        //child
        else
        {

            // Processing loop: client_sock <== data ==> client
            connection(csock);

    }
}
