#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <linux/limits.h>

void noFlags (DIR* );
void flag_a_func (DIR* );
void flag_R_func (DIR* , char* );
void flag_l_func (DIR* , char* );

void checkFlags (const int flag_a, const int flag_R, const int flag_l, DIR* newDir, char* currDirName);
void printDirectory (const char* directoryName);
void printInf (const struct dirent* , int*, char*);

#define FILE_ID  33279
#define DIRECTORY 16895
#define SUBDIR 0

const int  MAX_NUMBER_SUBDIRS = 10;
const int MAX_SUBDIR_NAME = 256;
const int MONTHS = 12;
const int MAX_STR = 10;

const char Months[MONTHS][MAX_STR] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

int main(int argc, char **argv)
{
	const struct option long_option[] =
    {
        {"all", no_argument, NULL, 'a'},
        {"long", no_argument, NULL, 'l'}, 
        {"recursive", no_argument, NULL, 'R'},
        {NULL, no_argument, NULL, 0}
    };

	int long_index;
    int opt;
    const char short_option[] = "alR";
    
    int flag_a = 0, flag_l = 0, flag_R = 0;
    
    
    while((opt = getopt_long(argc, argv, short_option, long_option, &long_index)) != -1)
    {
        switch(opt)
        {
        case 'a': flag_a = 1;
            break;
        case 'l': flag_l = 1;
            break;
        case 'R': flag_R = 1;
            break;
        case '?': printf("Unknown parameter: -%c\n", optopt);
        }
    }
    
    if (argc == 1 + flag_a + flag_l + flag_R)
    {
        DIR* newDir = opendir(".");
        char dirName[2] = ".";
        
        checkFlags (flag_a, flag_R, flag_l, newDir, dirName);
    }

    else 
    {
        DIR* newDir = opendir(argv[optind]);

        checkFlags (flag_a, flag_R, flag_l, newDir, argv[optind]);
    }
    
	return 0;
}

/***********************************************************************/

void noFlags (DIR* newDir)
{
    struct dirent* newStruct;

    while (newStruct = readdir (newDir))
        if (strcmp (".", newStruct->d_name) != 0 && strcmp ("..", newStruct->d_name) != 0)
            printf ("%s ", newStruct->d_name);
}

/***********************************************************************/

void flag_a_func (DIR* newDir)
{
    struct dirent* newStruct;

    while (newStruct = readdir (newDir))
        printf ("%s ", newStruct->d_name);

    printf ("\n");
}

/***********************************************************************/

void flag_R_func (DIR* newDir, char* path)
{
    char** arrSubDir = (char**) calloc (MAX_NUMBER_SUBDIRS, sizeof(*arrSubDir));
    struct dirent* newStruct;
    
    int nSubDirs = 0;
    while (newStruct = readdir (newDir))
    {
        struct stat *buf = (struct stat* ) calloc (1, sizeof (*buf));

        char subFileName[PATH_MAX];
        sprintf (subFileName, "%s/%s", path, newStruct->d_name);
        
        stat (subFileName, buf);

        if (S_ISDIR(buf->st_mode) && strcmp ("..", newStruct->d_name) != 0
                                  && strcmp (".", newStruct->d_name)  != 0)
        {
                arrSubDir[nSubDirs++] = newStruct->d_name;
        }

        if (strcmp (".", newStruct->d_name) != 0 && strcmp ("..", newStruct->d_name) != 0)
            printf ("%s ", newStruct->d_name);
    }

    for (int j = 0; j < nSubDirs; ++j)
    {
        char subDirName[PATH_MAX];
        sprintf (subDirName, "%s/%s", path, arrSubDir[j]);
        
        DIR* dir = opendir (subDirName);

        printf ("\n\n%s:\n", subDirName);
        flag_R_func (dir, subDirName);
    }
}

void printDirectory (const char* directoryName)
{
    if (strcmp (".", directoryName) == 0)
        printf (".:\n");
    
    else 
        printf ("./%s:\n", directoryName);

    DIR* dir = opendir (directoryName);

    noFlags (dir);
    printf ("\n\n");
}

/***********************************************************************/

void flag_l_func (DIR* newDir, char* path)
{
    struct dirent* newStruct;
    int total = 0;
    while (newStruct = readdir (newDir))
        if (strcmp (".", newStruct->d_name) != 0 && strcmp ("..", newStruct->d_name) != 0)
            printInf (newStruct, &total, path);
    
    printf ("total %d\n", total);
}

/***********************************************************************/

// permission bits: https://www.gnu.org/software/libc/manual/html_node/Permission-Bits.html

void logPermissions (struct stat* buf)
{
    printf ("%c", (buf->st_mode & S_IRUSR) ? 'r' : '-');
    printf ("%c", (buf->st_mode & S_IWUSR) ? 'w' : '-');
    printf ("%c", (buf->st_mode & S_IXUSR) ? 'x' : '-');

    printf ("%c", (buf->st_mode & S_IRGRP) ? 'r' : '-');
    printf ("%c", (buf->st_mode & S_IWGRP) ? 'w' : '-');
    printf ("%c", (buf->st_mode & S_IXGRP) ? 'x' : '-');

    printf ("%c", (buf->st_mode & S_IROTH) ? 'r' : '-');
    printf ("%c", (buf->st_mode & S_IWOTH) ? 'w' : '-');
    printf ("%c", (buf->st_mode & S_IXOTH) ? 'x' : '-');
    printf (" ");
}

/***********************************************************************/

void printInf (const struct dirent* newStruct, int* total, char* path)
{
    struct stat *buf = (struct stat* ) calloc (1, sizeof (*buf));

    char subFileName[PATH_MAX];

    if (strcmp (path, ".") == 0)
        sprintf (subFileName, "./%s", newStruct->d_name);    

    else 
        sprintf (subFileName, "./%s/%s", path, newStruct->d_name);    
    
    stat (subFileName, buf);

    *total += buf->st_blocks;

    const char* username = getpwuid(getuid())->pw_name;
    const char* gr_name = getgrgid(getgid())->gr_name;
    
    if (buf->st_mode == FILE_ID)
        printf ("-");

    if (buf->st_mode == DIRECTORY)
        printf ("d");

    logPermissions (buf);
    struct tm* time = localtime(&buf->st_mtim.tv_sec);
    printf ("%ld %s %s %7ld ", buf->st_nlink, username, gr_name, buf->st_size);
    printf ("%s %d %02d:%02d ", Months[time->tm_mon], time->tm_mday, time->tm_hour, time->tm_min);
    printf ("\x1b[32m%s\x1b[0m\n", newStruct->d_name);
}

/***********************************************************************/

void checkFlags (const int flag_a, const int flag_R, const int flag_l, DIR* newDir, char* currDirName)
{
    if (flag_a)
        flag_a_func (newDir);
    
    if (flag_R)
        flag_R_func (newDir, currDirName);
    
    if (flag_l)
        flag_l_func (newDir, currDirName);

    if (!flag_a && !flag_l && !flag_R)
        noFlags (newDir);
}

/***********************************************************************/
