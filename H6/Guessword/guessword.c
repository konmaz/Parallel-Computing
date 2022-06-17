#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>
#include "linkedlist.h"
#include "hashtable.h"
#include "stringmanipulation.h"
#include "stringlist.h"
#include "pw_helpers.h"

/**
 *
 * @return The number of lines a file has
 */
int countLines(char *filename)
{
    FILE *fp = fopen(filename, "r");
    int ch = 0;
    int lines = 0;
    while (!feof(fp))
    {
        ch = fgetc(fp);
        if (ch == '\n')
        {
            lines++;
        }
    }
    fclose(fp);
    return lines;
}

/**
 * Splits a text file into 'numberOfParts' parts each part has a name t0, t1, ... ,tn
 * @param filename
 * @param numberOfParts
 */
void splitter(char *filename, int numberOfParts)
{
    FILE *fp = fopen(filename, "r");
    int ch = 0;
    int lines = 0;
    while (!feof(fp))
    {
        ch = fgetc(fp);
        if (ch == '\n')
        {
            lines++;
        }
    }
    fclose(fp);
}

/**
 * Our entrypoint. We require two arguments to our program: the paths to a passwd and
 * shadow file. The number of threads/processes is dictated by MPI, and is out of our
 * control at this point.
 *
 * Run like: mpiexec -n <threads> ./guessword <passwd> <shadow>
 */
int main(int argc, char **argv)
{
    // Check arguments
    if (argc != 3)
    {
        fprintf(stderr, "Usage: ./guessword <passwd> <shadow>");
        return EXIT_FAILURE;
    }

    MPI_Init(&argc, &argv);

    ///////////////////////////////////////////////////////////////////
    // We now set up the local environment
    ///////////////////////////////////////////////////////////////////

    // Read the password/shadow files and parse all input
    char *passwdPath = argv[1];
    char *shadowPath = argv[2];

    struct users users = parseInput(passwdPath, shadowPath, false);

    // Read precomputed guess list
    struct stringList *pwListMain = readStringsFile("Files/top250.txt", MAX_PW_LENGTH);

    ///////////////////////////////////////////////////////////////////
    // We will now start to do the real work
    ///////////////////////////////////////////////////////////////////

    // We will try the provided list of passwords and all usernames appended
    // with 00.

    tryPasswords(pwListMain, users.passwords, users.hashSetting);
    

    struct stringList *appendedPasswords = manipulateList(users.usernames, '\0', "00", 1);
    tryPasswords(appendedPasswords, users.passwords, users.hashSetting);
    freeStringList(appendedPasswords);
    
    struct stringList *pwListUpdated = uppercaseList(pwListMain);
    tryPasswords(pwListUpdated, users.passwords, users.hashSetting);
    freeStringList(pwListUpdated);
    
    pwListUpdated = capitalList(pwListMain);
    tryPasswords(pwListUpdated, users.passwords, users.hashSetting);
    
    pwListUpdated = combinationList(pwListMain, pwListUpdated);
    tryPasswords(pwListUpdated, users.passwords, users.hashSetting);
    freeStringList(pwListUpdated);

    ///////////////////////////////////////////////////////////////////
    // Cleanup
    ///////////////////////////////////////////////////////////////////

    // Clean password list
    freeStringList(pwListMain);

    // Free users struct/information
    freeUserData(users);

    MPI_Finalize();
    return EXIT_SUCCESS;

}
