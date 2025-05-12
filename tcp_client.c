//TCP client

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


// files containing functions
#include "connectionSettings.c"

// initial (global) variables
struct  sockaddr_in  server_addr, client_addr;
int socketID, r;
char *serverReturn;
//char ** tokenizedString;
char tokenizedString[MAX][MAX];
char programName[MAX];

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

    return LocaltokenizedString;
}



////////////////////////////////initialize the client/////////////////////////////////////
int initiateClient()
{
    printf("------------Client Start------------\n");

    //create a socket
    socketID = socket(AF_INET, SOCK_STREAM, 0);
    //error check of socket
    if(socketID < 0){
        printf("Error: failure in creating socket.\n");
        exit(-1);
    }

    //bind socket to an IP address and port number
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl( INADDR_ANY );
    server_addr.sin_port = htons( SERVER_PORT );

    //send a datagram to destination host
    r = connect( socketID, (struct sockaddr*)&server_addr, sizeof(server_addr) );
    //error check of connect request
    if(r < 0)
    {
        printf("Error: failure in requesting a connection\n");
        exit(-1);
    }

    printf("Connection to IP: %s on port: %d\n", SERVER_HOST, SERVER_PORT);

    return(0);
}



////////////////////////////////get server information/////////////////////////////////////
void getServerInfo()
{
    char serverReturn[MAX];
    int requestData;

    // send data request to server
    //printf( "Sending server data request\n" );
    requestData = write( socketID, "returnServerFilesystemInformation", MAX );

    // get the data
    requestData = read( socketID, serverReturn, MAX );
    //printf( "Server data reply: %s\n", serverReturn);

    tokenizeString(serverReturn, ":");
}



////////////////////////////////command loop/////////////////////////////////////
int commandLoop()
{
    int  n;
    char line[MAX];
    char serverReturn[MAX];

    while ( 1 ) {
        //prompt user to input line
        getServerInfo();
        printf( "\nclient:%s:~%s:", inet_ntoa( client_addr.sin_addr ), tokenizedString[1]);
        bzero( line, MAX );                     // zero out line[ ]
        fgets( line, MAX, stdin );              // get a line from stdin
        line[strlen( line ) - 1] = 0;           // kill the new-line character \n at end of string


        if ( line[0] == 0)                      // exit if NULL line
            exit( 0 );

        //lcat  lls  lcd  lpwd  lmkdir  lrmdir  lrm   // executed LOACLLY

        char commandsList[7][10] = {"lcat","lls","lcd","lpwd","lmkdir","lrm", "\n"};

        //tokenize the inputted line with " " as the key
        tokenizeString(line, " ");
        //printf("tokenized string[0]: key ' ': %s\n",tokenizedString[0]);

        for(int i = 0; i<7; i++)
        {

            //create temporary string to hold a substring of the string
            char * tempCommand;
            memcpy(tempCommand, commandsList[i]+1, sizeof(line));

            if(!strcmp(commandsList[i], tokenizedString[0])) {

                //remove the first letter from the string
                memcpy(line, line + 1, sizeof(line));

                //reset the tokenized string and then re-write it
                bzero(tokenizedString, sizeof(tokenizedString));
                tokenizeString(line, " ");

                printf("Executing command \"%s\" locally\n",tokenizedString[0]);
                //printf("line tokenized: \"%s\"\n", line);


                //if it's cd, then run its command and break
                if(i == 2)
                {
                    //get HOME directory location
                    struct passwd *pw = getpwuid(getuid());
                    const char *HOME = pw->pw_dir;

                    //initialize variables
                    int r = 0;
                    char *s;
                    char buf[256];

                    //if the user typed in a path
                    if(tokenizedString[1]!=NULL && strcmp(tokenizedString[1]," ") && strstr(tokenizedString[1], "/") != NULL)
                    {
                        //change directory
                        r = chdir(tokenizedString[1]);
                    }
                        //if the user didn't type in a path
                    else
                    {
                        //change directory to home
                        r = chdir(HOME);
                    }

                    //cleanup
                    bzero(tokenizedString, sizeof(char) * 255);
                    break;
                }

                //create the path to the executable file
                char executablePath[255];
                strcpy(executablePath, "/bin/");
                strcat(executablePath, tokenizedString[0]);



                //printf("executable Path %s\n", executablePath);


                //initialize another tokenized string and allocate memory for it and its elements
                char **tokString = malloc(sizeof(char) * MAX);

                //clear the tokString data
                bzero(tokString, sizeof(char) * MAX);

                //make the first term the program name

                for (int i = 0; strcmp(tokenizedString[i], "") && i < MAX; i++) {
                    tokString[i] = malloc(sizeof(char) * MAX);

                    //("c1\n");
                    //copy the tokenized string term
                    strcpy(tokString[i],  tokenizedString[i]);

                    //printf("executablePath: \"%s\"\ttokString[%d]: = \"%s\"\n",executablePath, i,tokString[i]);
                }
                //copy the last term
                tokString[i+1] = malloc(sizeof(char) * MAX);
                strcpy(tokString[i+1],  tokenizedString[i]);

                // add a null termination
                tokString[i+2] = NULL;



                ////fork/////////////////////////////////

                // child process identification number
                pid_t  pid;
                int status;

                //printf("executable path: %s\n\n\n", executablePath);
                //fork the process
                pid = fork();

                //parent
                if ( pid )
                {
                    pid = wait( &status );
                    //printf("child killed\n\n");
                }
                    //child
                else
                {
                    char* envp[] = { NULL };

                    /*
                     for(int i = 0; i < 5; i++)
                    {
                        printf("ExecutablePath: \"%s\"\ttokString[%d]: = \"%s\"\t\tenvp[%d]: %s\n",executablePath, i,tokString[i], 0,envp[0]);
                    }
                     */

                    if ( execve(executablePath, tokString, envp) == -1)
                        printf("\"%s\": command not found\n", tokenizedString[0]);

                    exit(1);
                }

                break;
            }


        }
        //send message to server
        char serverCommandsList[7][10] = {"cat","ls","cd","pwd","mkdir","rm", "\n"};
        for(int i = 0; i < 6; i++) {
            if (!strcmp(tokenizedString[0], serverCommandsList[i])) {
                // Send line to server
                n = write(socketID, line, MAX);
                printf("Executing command \"%s\" on server\n", line);

                // Read a line from sock and show it (for confirmation message ecieved)
                n = read(socketID, serverReturn, MAX);
                //printf( "Received message : \"%s\"\n", serverReturn);

            }
        }

    }
    return 0;
}



////////////////////////////////main function/////////////////////////////////////
int main(int argc, char *argv[], char *env[ ])
{

    /*
    for(int i =0; argv[i] != NULL; i++)
        //printf("argv[%d]: \"%s\"\n",i,argv[i]);
    */


    for(int i = 0; env[i] != NULL; i++)
        printf("\"%s\"\n",env[i]);


    //printf("words");
    //strcpy(programName,argv[0]);

    initiateClient();
    commandLoop();
}
