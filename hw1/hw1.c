#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <utime.h>

char help_msg[] = "\
cat {file}:              Display content of {file}.\n\
cd {dir}:                Switch current working directory to {dir}.\n\
chmod {mode} {file/dir}: Change the mode (permission) of a file or directory.\n\
                         {mode} is an octal number.\n\
                         Please do not follow symbolc links.\n\
echo {str} [filename]:   Display {str}. If [filename] is given,\n\
                         open [filename] and append {str} to the file.\n\
exit:                    Leave the shell.\n\
find [dir]:              List files/dirs in the current working directory\n\
                         or [dir] if it is given.\n\
                         Minimum outputs contatin file type, size, and name.\n\
help:                    Display help message.\n\
id:                      Show current euid and egid.\n\
mkdir {dir}:             Create a new directory {dir}.\n\
pwd:                     Print the current working directory.\n\
rm {file}:               Remove a file.\n\
rmdir {dir}:             Remove an empty directory.\n\
stat {file/dir}:         Display detailed information of the given file/dir.\n\
touch {file}:            Create {file} if it does not exist,\n\
                         or update its access and modification timestamp.\n\
umask {mode}:            Change the umask of the current session.";

size_t n, inputsize = 0;
char *input = NULL, cwd[1000], fullpath[1500];
char *cmd[100];
int cmd_index;
mode_t current_mask = 0002;

void input_spilt_to_cmd();
void cmd_cat(char *filename);
void cmd_cd(char *dir);
void cmd_pwd();
void cmd_find();
void cmd_help();
void cmd_id();
void cmd_mkdir(char *newdir);
void cmd_rm(char *filename);
void cmd_stat(char *filename);
void cmd_echo(char *str);
void cmd_echo2(char *str, char *filename);
void cmd_rmdir(char *dir);
int isDirectoryEmpty(char *dirname);
void cmd_chmod(char *mode, char *file);
int check_mode(char *octnum);
void cmd_umask(char *octnum);
void listdir(char *name);
void cmd_find2(char *dir);
void print_file_type(struct stat statbuf);
void cmd_touch(char *file);
int is_legal(char *_cmd, int corret_index);

int main(int argc, char *argv[]){
    char *pch;

    if(argc == 3){
        setegid(atoi(argv[2]));
        seteuid(atoi(argv[1]));
    }
    else{
        printf("Error: please set euid and egid\n");
        return 0;
    }

    setvbuf(stdin, 0, 2, 0);
    setvbuf(stdout, 0, 2, 0);
    setvbuf(stderr, 0, 2, 0);
    
    umask(0002);

    while(1){
        //get current working directory
        getcwd(cwd, sizeof(cwd));

        // read input
        printf("> ");
        n = getline(&input, &inputsize, stdin);
        if(n == 1) continue;
        else input[n-1] = '\0';

        // spilt input
        input_spilt_to_cmd();

        //check cmd
        if(strcmp(cmd[0], "cat") == 0){
            if(is_legal("cat", 2)){
                cmd_cat(cmd[1]);
            }
        }
        else if(strcmp(cmd[0], "cd") == 0){
            if(is_legal("cd", 2)){
                cmd_cd(cmd[1]);
            }
        }
        else if(strcmp(cmd[0], "exit") == 0){
            if(is_legal("exit", 1)){
                return 0;
            }
        }
        else if(strcmp(cmd[0], "help") == 0){
            if(is_legal("help", 1)){
                cmd_help();
            }
        }
        else if(strcmp(cmd[0], "mkdir") == 0){
            if(is_legal("mkdir", 2)){
                cmd_mkdir(cmd[1]);
            }
        }
        else if(strcmp(cmd[0], "pwd") == 0){
            if(is_legal("pwd", 1)){
                cmd_pwd();
            }
        }
        else if(strcmp(cmd[0], "rm") == 0){
            if(is_legal("rm", 2)){
                cmd_rm(cmd[1]);
            }
        }
        else if(strcmp(cmd[0], "echo") == 0){
            if(cmd_index == 2){ //echo str
                cmd_echo(cmd[1]);
            }
            else if(cmd_index == 3){ //append str with file
                cmd_echo2(cmd[1], cmd[2]);
            }
            else{
                printf("Error: wrong usage of 'echo'\n");
                printf("Use 'help' for more information\n");
            }
        }
        else if(strcmp(cmd[0], "rmdir") == 0){
            if(is_legal("rmdir", 2)){
                cmd_rmdir(cmd[1]);
            }
        }
        else if(strcmp(cmd[0], "stat") == 0){
            if(is_legal("stat", 2)){
                cmd_stat(cmd[1]);
            }
        }
        else if(strcmp(cmd[0], "id") == 0){
            if(is_legal("id", 1)){
                cmd_id();
            }
        }
        else if(strcmp(cmd[0], "umask") == 0){
            if(cmd_index == 1){
                printf("%#04o\n", current_mask);
            }
            else if(is_legal("umask", 2)){
                cmd_umask(cmd[1]);
            }
        }
        else if(strcmp(cmd[0], "chmod") == 0){
            if(is_legal("chmod", 3)){
                cmd_chmod(cmd[1], cmd[2]);
            }
        }
        else if(strcmp(cmd[0], "find") == 0){
            if(cmd_index == 1){
                cmd_find();
            }
            else if(cmd_index == 2){
                cmd_find2(cmd[1]);
            }
            else{
                printf("Error: wrong usage of 'find'\n");
                printf("Use 'help' for more information\n");
            }
        }
        else if(strcmp(cmd[0], "touch") == 0){
            if(is_legal("touch", 2)){
                cmd_touch(cmd[1]);
            }
        }
        else{
            printf("ERROR: unkown command\n");
        }
    }

    free(input);

    return 0;
}

void input_spilt_to_cmd(){
    cmd_index = 0;

    cmd[cmd_index] = strtok(input, " ");
    while(cmd[cmd_index] != NULL){
        cmd[++cmd_index] = strtok(NULL, " ");
    }
    //printf("token number %d\n", cmd_index);
}

void cmd_cat(char *filename){
    FILE *fp;
    char line[100];

    fp = fopen(filename, "r");
    if(fp == NULL){
        printf("ERROR: file open failure\n");
    }
    else{
        while(fgets(line, sizeof(line), fp) != NULL){
            printf("%s", line);
        }
        fclose(fp);
    }
}

void cmd_cd(char *dir){
    sprintf(fullpath,"%s/%s", cwd, dir);
    int ret = chdir(dir);
    if(ret != 0){
        printf("ERROR: can't change to the dir\n");
    }
}

void cmd_pwd(){
    printf("%s\n", cwd);
}

void cmd_find(){
    DIR *d;
    struct dirent *file;
    struct stat statbuf;
    char buf[512];

    d = opendir(".");
    if(d){
        while((file = readdir(d)) != NULL){
            if((strcmp(file->d_name, ".") == 0) || (strcmp(file->d_name, "..") == 0))
                continue;
            sprintf(buf, "%s/%s", cwd, file->d_name);
            stat(buf, &statbuf);
            printf("%ld\t", statbuf.st_size);
            print_file_type(statbuf);
            printf("\t%s\n", file->d_name);
        }
        closedir(d);
    }
    else{
        printf("ERROR: find failure\n");
    }
}

void listdir(char *name){
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;

    if(!(dir = opendir(name))){
        printf("Error: cannot find the directory\n");
        return;
    }

    while((entry = readdir(dir)) != NULL){
        if(entry->d_type == DT_DIR){
            char path[1024];
            if((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0))
                continue;
            
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            stat(path, &statbuf);
            
            printf("%ld\t", statbuf.st_size);
            print_file_type(statbuf);
            printf("\t%s/%s\n", name, entry->d_name);
            
            listdir(path);
        }
        else{
            char path[1024];

            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            stat(path, &statbuf);
            
            printf("%ld\t", statbuf.st_size);
            print_file_type(statbuf);
            printf("\t%s/%s\n", name, entry->d_name);
        }
    }
    closedir(dir);
}

void cmd_find2(char *dir){
    DIR *d;
    struct dirent *file;
    struct stat statbuf;
    char buf[512];
    char path[1024];

    d = opendir(dir);
    if(d){
        while((file = readdir(d)) != NULL){
            if((strcmp(file->d_name, ".") == 0) || (strcmp(file->d_name, "..") == 0))
                continue;
            
            snprintf(path, sizeof(path), "%s/%s", dir, file->d_name);
            stat(path, &statbuf);

            printf("%ld\t", statbuf.st_size);
            print_file_type(statbuf);
            printf("\t%s\n", file->d_name);
        }
        closedir(d);
    }
    else{
        printf("ERROR: find failure\n");
    }
}

void cmd_help(){
    printf("%s\n", help_msg);
}

void cmd_id(){
    int euid = geteuid();
    int egid = getegid();
    printf("euid: %d\n", euid);
    printf("egid: %d\n", egid);
}

void cmd_mkdir(char *newdir){
    struct stat st = {0};

    sprintf(fullpath, "%s/%s", cwd, newdir);
    if(stat(fullpath, &st) == -1){
        mkdir(fullpath, 0777);
    }
    else{
        printf("ERROR: cannot create the directory\n");
    }
}

void cmd_rm(char *filename){
    if(remove(filename) != 0){
        printf("ERROR: cannot remove the file\n");
    }
}

void print_file_type(struct stat statbuf){
    switch(statbuf.st_mode & S_IFMT){
        case S_IFBLK:  printf("block device");            break;
        case S_IFCHR:  printf("character device");        break;
        case S_IFDIR:  printf("directory");               break;
        case S_IFIFO:  printf("FIFO/pipe");               break;
        case S_IFLNK:  printf("symlink");                 break;
        case S_IFREG:  printf("regular file");            break;
        case S_IFSOCK: printf("socket");                  break;
        default:       printf("unknown?");                break;
    }
}

void cmd_stat(char *filename){
    struct stat statbuf;
    sprintf(fullpath, "%s/%s", cwd, filename);
    if(stat(fullpath, &statbuf) == 0){
        printf("File type:                ");
        print_file_type(statbuf);
        printf("\n");
        printf("I-node number:            %ld\n",               (long) statbuf.st_ino);
        printf("Mode:                     %lo (octal)\n",       (unsigned long) statbuf.st_mode);
        printf("Link count:               %ld\n",               (long) statbuf.st_nlink);
        printf("Ownership:                UID=%ld   GID=%ld\n", (long) statbuf.st_uid, (long) statbuf.st_gid);
        printf("Preferred I/O block size: %ld bytes\n",         (long) statbuf.st_blksize);
        printf("File size:                %lld bytes\n",        (long long) statbuf.st_size);
        printf("Blocks allocated:         %lld\n",              (long long) statbuf.st_blocks);
        printf("Last status change:       %s",                  ctime(&statbuf.st_ctime));
        printf("Last file access:         %s",                  ctime(&statbuf.st_atime));
        printf("Last file modification:   %s",                  ctime(&statbuf.st_mtime));
    }
    else{
        printf("ERROR: cannot get file's information\n");
    }
}

void cmd_echo(char *str){
    printf("%s\n", str);
}

void cmd_echo2(char *str, char *filename){
    FILE *fp;

    fp = fopen(filename, "a");
    if(fp == NULL){
        printf("ERROR: file open failure\n");
    }
    else{
        fwrite(str , sizeof(char), strlen(str), fp);
        fprintf(fp, "\n");
        fclose(fp);
    }
}

void cmd_rmdir(char *dir){
    int check = isDirectoryEmpty(dir);

    if(check == 0){
        printf("ERROR: cannot remove non empty directory\n");
    }
    else{
        sprintf(fullpath, "%s/%s", cwd, dir);
        if(rmdir(fullpath) != 0){
            printf("ERROR: remove directory failure\n");
        }
    }
}

int isDirectoryEmpty(char *dirname){
    int n = 0;
    struct dirent *d;
    DIR *dir;

    dir = opendir(dirname);
    if(dir == NULL){ //Not a directory or doesn't exist
        return 1;
    }

    while((d = readdir(dir)) != NULL){
        if(++n > 2) break;
    }
    closedir(dir);

    if(n <= 2){ //Directory Empty
        return 1;
    }
    else{
        return 0;
    }
}

void cmd_chmod(char *octnum, char *file){
    if(check_mode(octnum)){
        mode_t mode = strtol(octnum, NULL, 8);
        if(chmod(file, mode) != 0){
            printf("ERROR: chmod failure\n");
        }
    }
    else{
        printf("Error: wrong mode\n");
    }
}

int check_mode(char *octnum){
    if((strlen(octnum) == 4) && 
       ('0'<=octnum[0]) && (octnum[0]<='7') &&
       ('0'<=octnum[1]) && (octnum[1]<='7') &&
       ('0'<=octnum[2]) && (octnum[2]<='7') &&
       ('0'<=octnum[3]) && (octnum[3]<='7'))
    {
        return 1;
    }
    else if((strlen(octnum) == 3) && 
            ('0'<=octnum[0]) && (octnum[0]<='7') &&
            ('0'<=octnum[1]) && (octnum[1]<='7') &&
            ('0'<=octnum[2]) && (octnum[2]<='7'))
    {
        return 1;
    }
    else{
        return 0;
    }
}

void cmd_umask(char *octnum){
    if(check_mode(octnum)){
        mode_t mode = strtol(octnum, NULL, 8);
        umask(mode);
        current_mask = mode;
    }
    else{
        printf("Error: wrong mode\n");
    }
}

void cmd_touch(char *file){
    if(access(file, F_OK) != -1){
        // file exists
        // update its access and modification timestamp
        struct stat statbuf;
        struct utimbuf new_times;

        stat(file, &statbuf);
        new_times.actime = time(NULL);
        new_times.modtime = time(NULL);
        utime(file, &new_times);
    }
    else{
        // file doesn't exist
        FILE *fp;
        fp = fopen(file, "w");
        fclose(fp);
    }
}

int is_legal(char *_cmd, int corret_index){
    if(cmd_index != corret_index){
        printf("Error: wrong usage of '%s'\n", _cmd);
        printf("Use 'help' for more information\n");
        return 0;
    }
    else
        return 1;
}
