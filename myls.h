#ifndef _MYLS_H_
#define _MYLS_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#define MAX_LEN_CHARS 256

// For padding purposes
int getPadforNumbers(long inode);

// Sorts the user input into following: ./myls [options] [path]
char** sortInput(char* args[], int numArgs);

// Checks if option -i is present
bool checkI(char* sortedArgs[], int numArgs);

// Checks if option -l is present
bool checkL(char* sortedArgs[], int numArgs);

// Checks if option -R is present 
bool checkR(char* sortedArgs[], int numArgs); 

// Checks if the options entered by user are valid
bool isInvalidOption(char *option);

// Checks if path exists
bool isPathExists(char *path);

// Checks if the specified path is a file
bool isFile(const char *path);

// Checks if the specified path is a directory
bool isDirectory(const char *path);

// Checks how many files are within a directory
int numFiles(char *path);

// Prints which the username  the file belongs to
void getAndPrintUserName(uid_t uid);

// Prints which group the file belongs to
void getAndPrintGroup(gid_t grpNum);

// Prints specifically about a file
void printOneFileName(char* path, bool i, bool l, bool r);

// Prints information about a file
void printFileInfo(char *name, char *path, long int d_ino, struct stat fileStat, bool i, bool l, bool r);

// Prints the permission of the given files
void printPermissions(struct stat fileStat, bool i);

// Prints all the files when ./myls or ./myls . are used
void basicPrint(char *path, bool i, bool l, bool r);

// Scans the given path
void scanPath(char *path, bool i, bool l, bool r); 

// Recursively scans the specified path; if -R is present
void recursiveScan(char *path, bool i, bool l, bool r);

// Prints in the following format [mmm dd yyyy hh:mm] about the file
void getDateTime(time_t dateTime);

// Remove all malloc memory
void freeMalloc(char* sortedArgs[], int numArgs);

// Checks to see if a file contains special symbols !$^&*()["?;|<>=
bool containsSpecialSymbols(char *name);

#endif