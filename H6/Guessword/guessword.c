#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "linkedlist.h"
#include "hashtable.h"
#include "stringmanipulation.h"
#include "stringlist.h"
#include "pw_helpers.h"

/**
 * Our entrypoint. We require two arguments to our program: the paths to a passwd and
 * shadow file. The number of threads/processes is dictated by MPI, and is out of our
 * control at this point.
 * 
 * Run like: mpiexec -n <threads> ./guessword <passwd> <shadow>
 */
int main(int argc, char **argv) {
    // Check arguments
    if(argc != 3) {
        fprintf(stderr, "Usage: ./guessword <passwd> <shadow>");
        return EXIT_FAILURE;
    }

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

    ///////////////////////////////////////////////////////////////////
    // Cleanup
    ///////////////////////////////////////////////////////////////////

    // Clean password list
    freeStringList(pwListMain);
    freeStringList(appendedPasswords);

    // Free users struct/information
    freeUserData(users);
}
