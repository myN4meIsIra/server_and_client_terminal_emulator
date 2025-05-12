//#include <strdef.h>
#include <string.h>
#include <stdlib.h>


//tokenizeString
char **tokenizeString(const char *PATHNAME, const char *KEY)
{
    //initial variables
    int i;
    char s [256];

    //allocate space for the array of strings for the path
    char **tokenizedString = malloc(sizeof(char) * 255);

    //tokenize the string PATHNAME based on the key KEY
    strcpy(s, PATHNAME);
    char* token = strtok(s, KEY);
    i = 0;

    //while there are still characters left
    while (token != NULL)
    {

        tokenizedString[i++] = token;
        //printf("token: %s\n", token);
        token = strtok(NULL, KEY);
    }

    //ensure that the tokenizedString is null-terminated
    tokenizedString[sizeof(tokenizedString)] = NULL;

    //printf all the tokenized strings of the path

    for(int i = 0; tokenizedString[i] != NULL; i++)
    {
        printf("(tokenizeString) Term %d of tokenizedString (which has size %ld): %s \n",i,sizeof(tokenizedString[i]),tokenizedString[i]);
    }


    return tokenizedString;
}
