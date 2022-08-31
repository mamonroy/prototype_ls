#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#include <dirent.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <libgen.h>
#include <unistd.h>
#include "myls.h"

#define MAX_LEN_CHARS 256

// tracks which argument seperates options and path names
int globalcutOffArguments = 0;

bool isPathExists(char *path)
{
    DIR *directory = opendir(path);
    if(directory == NULL) {
        printf("myls: cannot access '%s': No such file or directory\n", path);
        return false;
    }
    closedir(directory);
    return true;
}

char** sortInput(char* args[], int numArgs) 
{
    char temp[MAX_LEN_CHARS];

    char **sortedArgs = malloc(numArgs * sizeof(char *)); 
    for(int i = 0; i < numArgs; i++) {
      sortedArgs[i] = malloc(MAX_LEN_CHARS * sizeof(char));  
    }

    for(int i=0;i<numArgs;i++) {
        strcpy(sortedArgs[i], args[i]);
    }

    for(int i=globalcutOffArguments;i < numArgs;i++) {
        for(int j=i+1;j < numArgs; j++){
            if(strcmp(sortedArgs[i],sortedArgs[j])>0){
                strcpy(temp,sortedArgs[i]);
                strcpy(sortedArgs[i],sortedArgs[j]);
                strcpy(sortedArgs[j],temp);
            }
        }
    }
    return sortedArgs;
}

void getAndPrintUserName(uid_t uid)
{
    struct passwd *pw = getpwuid(uid);

    if (pw) {
        printf("%10s", pw->pw_name);
    } 
    else {
        printf("No name found for %u\n", uid);
    }
}

void getAndPrintGroup(gid_t grpNum)
{
    struct group *grp = getgrgid(grpNum);

    if (grp) {
        printf("%14s", grp->gr_name);
    } 
    else {
        printf("No group name for %u found\n", grpNum);
    }
}

bool isFile(const char *path)
{
    struct stat path_info;
    if(lstat(path, &path_info)!=0) {
        printf("myls: cannot access '%s': No such file or directory\n", path);
        exit(1);
    }
    return S_ISREG(path_info.st_mode);
}

bool isDirectory(const char *path) {
   struct stat path_info;
   lstat(path, &path_info);
   return S_ISDIR(path_info.st_mode);
}

void printOneFileName(char* path, bool i, bool l, bool r)
{
    struct stat fileStat;
    lstat(path, &fileStat);

    int length = strlen(path) + 2;
    char localname[length];

    if(containsSpecialSymbols(path)) {
        strcpy(localname,"'");
        strcat(localname, path);
        strcat(localname, "'");
    }
    else {
        strcpy(localname, path);
    }

    if((!i && !l)) {
        printf("%s\n",localname);
    } 

    else if(i && l) {
        printf("%4lu", fileStat.st_ino);
        printPermissions(fileStat, i);
        printf( "%4lu" ,fileStat.st_nlink);
        getAndPrintUserName(fileStat.st_uid);
        getAndPrintGroup(fileStat.st_gid);
        printf( "\t%-10lu" , fileStat.st_size);
        getDateTime(fileStat.st_mtime);
        printf("%s\n", localname);
    }

    else if(i) {
        printf("%4lu %4s\n", fileStat.st_ino, localname);
    }

    else if(l) {
        printPermissions(fileStat, i);
        printf( "%4lu" ,fileStat.st_nlink);
        getAndPrintUserName(fileStat.st_uid);
        getAndPrintGroup(fileStat.st_gid);
        printf( "\t%-10lu" , fileStat.st_size);
        getDateTime(fileStat.st_mtime);
        printf("%s\n", localname);
    }
}

void basicPrint(char *path, bool i, bool l, bool r)
{
    struct dirent **fileslist;
    struct stat fileStat;
    int n = scandir(path, &fileslist, NULL, alphasort); 

    for(int tracker = 0; tracker < n; tracker++) {
        if(fileslist[tracker] -> d_name[0] == '.' || strcmp(fileslist[tracker] -> d_name, "..") == 0) {
            free(fileslist[tracker]); 
            continue;
        }
        lstat(fileslist[tracker] -> d_name, &fileStat);
        printFileInfo(fileslist[tracker] -> d_name, path, fileStat.st_ino, fileStat, i, l, r);
        free(fileslist[tracker]); 
        
    }
    free(fileslist);
}

int numFiles(char *path)
{
    int count = 0;
    DIR *directory = opendir(path);
    struct dirent *dirFiles;

    while ((dirFiles = readdir(directory)) != NULL) {
        if(dirFiles -> d_name[0] == '.' || strcmp(dirFiles -> d_name, "..") == 0) {
            continue;
        }
        count++;
    }

    closedir(directory);

    return count;
}

void scanPath(char *path, bool i, bool l, bool r) 
{

    if(!(isPathExists(path))) {
        exit(1);
    }

    char modifiedPath[MAX_LEN_CHARS];
    strcpy(modifiedPath, path);

    int length = strlen(modifiedPath);
    if(modifiedPath[length - 1] != '/') {
        strcat(modifiedPath,"/");
    }

    struct dirent **fileslist;
    struct stat fileStat;
    int n = scandir(path, &fileslist, NULL, alphasort); 

    for(int tracker = 0; tracker < n; tracker++) {
        if(fileslist[tracker] -> d_name[0] == '.' || strcmp(fileslist[tracker] -> d_name, "..") == 0) {
            free(fileslist[tracker]); 
            continue;
        }

        char filePath[MAX_LEN_CHARS];
        strcpy(filePath, modifiedPath);
        strcat(filePath, fileslist[tracker] -> d_name);
        lstat(filePath, &fileStat);
        printFileInfo(fileslist[tracker] -> d_name, filePath, fileStat.st_ino, fileStat, i, l, r);
        free(fileslist[tracker]); 
    }
    free(fileslist);
}

void recursiveScan(char *path, bool i, bool l, bool r)
{
    if(!(isPathExists(path))) {
        exit(1);
    }

    printf("%s:\n",path);
    char modifiedPath[MAX_LEN_CHARS];
    strcpy(modifiedPath, path);

    int length = strlen(modifiedPath);
    if(modifiedPath[length - 1] != '/') {
        strcat(modifiedPath,"/");
    }

    scanPath(modifiedPath, i, l , r);
    struct dirent **fileslist;
    int n = scandir(modifiedPath, &fileslist, NULL, alphasort); 

    for(int tracker = 0; tracker < n; tracker++) {
        if(fileslist[tracker] -> d_name[0] == '.' || strcmp(fileslist[tracker] -> d_name, "..") == 0) {
            free(fileslist[tracker]); 
            continue;
        }

        char filePath[MAX_LEN_CHARS];
        strcpy(filePath, modifiedPath);
        strcat(filePath, fileslist[tracker] -> d_name);

        if(isDirectory(filePath)) {
            printf("\n");
            recursiveScan(filePath, i, l, r);
        }
        free(fileslist[tracker]); 
    }
    free(fileslist);
}

void printPermissions(struct stat fileStat, bool i) 
{
    // For padding purposes if -i is present
    if(i) {
        printf("\t");
    }

    // Check if symbolic link or a normal directory
    if(S_ISDIR(fileStat.st_mode)) {
        printf("d");
    }
    else if(S_ISLNK(fileStat.st_mode)) {
        printf("l");
    }
    else {
        printf("-");
    }
    // For users
    printf((fileStat.st_mode & S_IRUSR) ? "r" : "-");
    printf((fileStat.st_mode & S_IWUSR) ? "w" : "-");
    printf((fileStat.st_mode & S_IXUSR) ? "x" : "-");
    // For Groups
    printf((fileStat.st_mode & S_IRGRP) ? "r" : "-");
    printf((fileStat.st_mode & S_IWGRP) ? "w" : "-");
    printf((fileStat.st_mode & S_IXGRP) ? "x" : "-");
    // For Others
    printf((fileStat.st_mode & S_IROTH) ? "r" : "-");
    printf((fileStat.st_mode & S_IWOTH) ? "w" : "-");
    printf((fileStat.st_mode & S_IXOTH) ? "x" : "-");
}

void printFileInfo(char *name, char *path, long int d_ino, struct stat fileStat, bool i, bool l, bool r)
{

    int length = strlen(name) + 2;
    char localname[length];

    if(containsSpecialSymbols(name)) {
        strcpy(localname,"'");
        strcat(localname, name);
        strcat(localname, "'");
    }
    else {
        strcpy(localname, name);
    }

    if((!i && !l)) {
        printf("%s\n",localname);
    } 

    else if(i && l) {
        printf("%lu", d_ino);
        printPermissions(fileStat, i);
        printf( "%4lu" ,fileStat.st_nlink);
        getAndPrintUserName(fileStat.st_uid);
        getAndPrintGroup(fileStat.st_gid);
        printf( "\t%-10lu" , fileStat.st_size);
        getDateTime(fileStat.st_mtime);
        // Soft Link
        char buf[MAX_LEN_CHARS];
        int length = readlink(path, buf, sizeof(buf)-1);
        if (length != -1) {
            buf[length] = '\0';
            printf("%s -> %s\n",localname, buf);
        }
        else {
            printf("%s\n", localname);
        }
    }

    else if(i) {
        printf("%4lu %4s\n", d_ino, localname);
    }

    else if(l) {
        printPermissions(fileStat, i);
        printf( "%4ld" , fileStat.st_nlink);
        getAndPrintUserName(fileStat.st_uid);
        getAndPrintGroup(fileStat.st_gid);
        printf( "\t%-10lu" , fileStat.st_size);
        getDateTime(fileStat.st_mtime);
        // Soft Link
        char buf[MAX_LEN_CHARS];
        int length = readlink(path, buf, sizeof(buf)-1);
        if (length != -1) {
            buf[length] = '\0';
            printf("%s -> %s\n",localname,buf);
        }
        else {
            printf("%s\n",localname);
        }
    }
}

void getDateTime(time_t dateTime)
{
    char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"}; 
    struct tm *temp = localtime(&dateTime);
    int month_number = temp -> tm_mon;
    int day = temp -> tm_mday;
    int year = temp -> tm_year;
    int hour = temp -> tm_hour;
    int minute = temp -> tm_min;

    printf("%s %2d %d %02d:%02d\t", months[month_number], day, 1900 + year, hour, minute);
}

void freeMalloc(char* sortedArgs[], int numArgs)
{
    for(int i = 0; i < numArgs; i++) {
        free(sortedArgs[i]);
    }
    free(sortedArgs);
}

bool isInvalidOption(char *option)
{
    int correctLetters = 0;
    int numOfLetters = 0;
    for(int i = 0; option[i] !='\0'; i++) {
        numOfLetters++;
        if(option[i] == 'i' || option[i] == 'l' || option[i] == 'R') {
            correctLetters++;
        }
    }
    if(numOfLetters != correctLetters) {
        return true;
    }
    return false;
}

bool checkI(char* sortedArgs[], int numArgs) 
{
    bool flag = false;
    for(int i = 1; i < numArgs; i++) {
        // If it's not an option, skip
        if(sortedArgs[i][0] != '-') {
            globalcutOffArguments = i;
            break;
        }
        char* chopped_arg = sortedArgs[i] + 1;
        if(*chopped_arg == '\0' || isInvalidOption(chopped_arg)) {
            printf("Invalid option or Unsupported Option!\n");
            exit(1);
        }
        for(int j = 0; chopped_arg[j] != '\0'; j++) {
            if(chopped_arg[j] == 'i') {
                flag = true;
            } 
        }
    }
    return flag;
}

bool checkL(char* sortedArgs[], int numArgs) 
{
    bool flag = false;
    for(int i = 1; i < numArgs; i++) {
        // If it's not an option, skip
        if(sortedArgs[i][0] != '-') {
            globalcutOffArguments = i;
            break;
        }
        char* chopped_arg = sortedArgs[i] + 1;
        if(*chopped_arg == '\0' || isInvalidOption(chopped_arg)) {
            printf("Invalid option!\n");
            exit(1);
        }
        for(int j = 0; chopped_arg[j] != '\0'; j++) {
            if(chopped_arg[j] == 'l') {
                flag = true;
            } 
        }
    }
    return flag;
}

bool checkR(char* sortedArgs[], int numArgs) 
{
    bool flag = false;
    for(int i = 1; i < numArgs; i++) {
        // If it's not an option, skip
        if(sortedArgs[i][0] != '-') {
            globalcutOffArguments = i;
            break;
        }
        char* chopped_arg = sortedArgs[i] + 1;
        if(*chopped_arg == '\0' || isInvalidOption(chopped_arg)) {
            printf("Invalid option!\n");
            exit(1);
        }
        for(int j = 0; chopped_arg[j] != '\0'; j++) {
            if(chopped_arg[j] == 'R') {
                flag = true;
            } 
        }
    }
    return flag;
}

int getPadforNumbers(long number)
{
    int count = 0;
    do
    {
        count++;
        number /= 10;
    } while(number != 0);

    return count;
}

bool containsSpecialSymbols(char *name)
{
    int length = strlen(name);
    char symbols[16] = {' ','!', '$', '^', '&', '*' ,'(', ')' , '[' , '?' ,';'  ,'|'  ,'<' ,'>' ,'='}; 
    for(int i = 0; i < length; i++) {
        for(int j = 0; j < 16; j++)
             if(name[i] == symbols[j]) {
                return true;
             }
    }
    return false;
}

int main(int numArgs, char* args[])
{
    // If no arguments (E.g. just ./myl or ./myls . or ./myls path), only print current directory
    if(numArgs == 1 || (strcmp(args[numArgs-1],".") == 0 && numArgs == 2)) {
        basicPrint(".", false, false, false);
        exit(0);
    }

    bool is_i_present = checkI(args, numArgs);
    bool is_l_present = checkL(args, numArgs);
    bool is_R_present = checkR(args, numArgs);

    char** sortedInput = sortInput(args, numArgs);

    // No path is specified
    if(globalcutOffArguments == 0) {
        if(is_R_present) {
            recursiveScan(".", is_i_present, is_l_present, is_R_present);
            freeMalloc(sortedInput, numArgs);
            exit(0);
        }
        else {
            basicPrint(".", is_i_present, is_l_present, is_R_present);
            freeMalloc(sortedInput, numArgs);
            exit(0);
        }
    }

    int numFiles = 0;
    int numFolders = 0;

    for(int i = globalcutOffArguments; i < numArgs; i++) {
        if(isDirectory(sortedInput[i]))
            numFolders++;
        else if(isFile(sortedInput[i]))
            numFiles++;
    }
    // Allocate to store normal files only
    char **files = malloc(numFiles * sizeof(char *)); 
    for(int i = 0; i < numFiles; i++) {
        files[i] = malloc(MAX_LEN_CHARS * sizeof(char));  
    }

    // Allocate to store folder files only
    char **folders = malloc(numFolders * sizeof(char *));
    for(int i = 0; i < numFolders; i++) {
        folders[i] = malloc(MAX_LEN_CHARS * sizeof(char));  
    }

    int fileCounter = 0;
    int folderCounter = 0;
    for(int i = globalcutOffArguments; i < numArgs; i++) {
        if(isFile(sortedInput[i])) {
            strcpy(files[fileCounter],sortedInput[i]);
            fileCounter++;
        }
        else if(isDirectory(sortedInput[i])) {
            strcpy(folders[folderCounter],sortedInput[i]);
            folderCounter++;
        }
    }

    char **sortedfiles = sortInput(files, numFiles);
    char **sortedfolders = sortInput(folders, numFolders);

    freeMalloc(files, numFiles);
    freeMalloc(folders, numFolders);

    for(int i = 0; i < numFiles; i++) {
        printOneFileName(sortedfiles[i], is_i_present, is_l_present, is_R_present);
    }

    for(int i = 0; i < numFolders; i++) {
        if(is_R_present) {
            recursiveScan(sortedfolders[i], is_i_present, is_l_present, is_R_present);
        }
        else {
            if(numArgs - globalcutOffArguments > 1)
                printf("\n%s:\n", sortedfolders[i]);
            scanPath(sortedfolders[i], is_i_present, is_l_present, is_R_present);
        }
    }
    freeMalloc(sortedfiles, numFiles);
    freeMalloc(sortedfolders, numFolders);
    freeMalloc(sortedInput, numArgs);
    return 0;
}
