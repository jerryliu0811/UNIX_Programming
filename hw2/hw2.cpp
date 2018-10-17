#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>

#include <iostream>
#include <algorithm>
#include <map>
#include <vector>
#include <string>

using namespace std;

typedef struct proc_struct{
    unsigned uid;
    unsigned gid;
    int      pid;
    int      ppid;
    int      pgid;
    int      sid;
    int      tty;
    char     cmd[1024];
    char     state;
    char     cmdline[4096];
    char     ttyname[512];
} myproc_struct;

map<int, string> devmap;
map<int, string>::iterator iter;
vector<struct proc_struct> myproc;

bool pid_compare(myproc_struct lhs, myproc_struct rhs){
    return lhs.pid < rhs.pid;
}

bool ppid_compare(myproc_struct lhs, myproc_struct rhs){
    return lhs.ppid < rhs.ppid;
}

bool pgid_compare(myproc_struct lhs, myproc_struct rhs){
    return lhs.pgid < rhs.pgid;
}

bool sid_compare(myproc_struct lhs, myproc_struct rhs){
    return lhs.sid < rhs.sid;
}

void get_devmap(char *name){
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;

    if(!(dir = opendir(name))){
        return;
    }

    while((entry = readdir(dir)) != NULL){
        if(entry->d_type == DT_DIR){ //recursively enter another dir
            char path[1024];
            if((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0))
                continue;
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            get_devmap(path);
        }
        else{ //get file and set to devmap
            char path[1024];
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            lstat(path, &statbuf);
            switch (statbuf.st_mode & S_IFMT){
                case S_IFCHR: //character device
                    devmap.insert(pair<int, string>(statbuf.st_rdev, path+5));
                    break;
                default:
                    break;
            }
        }
    }
    closedir(dir);
}

void get_proc_info(char *file){
    FILE *fp;
    struct stat st;
    char buffer[4096], piddir[512], statfile[512], cmdlinefile[512];
    char *s;
    size_t n;
    myproc_struct proc;

    
    //----------/proc/pid/------------------------
    sprintf(piddir, "/proc/%s", file);

    if(stat(piddir, &st) != -1){
        proc.uid = st.st_uid;
        proc.gid = st.st_gid;
    }
    else{
        proc.uid = -1;
        proc.gid = -1;
    }

    //----------/proc/pid/stat--------------------
    sprintf(statfile, "/proc/%s/stat", file);

    if((fp = fopen(statfile, "r")) == NULL){
        printf("cannot open %s\n", statfile);
        return;
    }

    if((s = fgets(buffer, 4096, fp)) == NULL){
        fclose(fp);
        printf("stat fgets error\n");
        return;
    }

    sscanf(buffer, "%d %s %c %d %d %d %d", 
        &proc.pid, proc.cmd, &proc.state, 
        &proc.ppid, &proc.pgid, &proc.sid, &proc.tty);

    iter = devmap.find(proc.tty);
    if(iter != devmap.end())
        strcpy(proc.ttyname, iter->second.c_str());
    else
        strcpy(proc.ttyname, "-");

    fclose(fp);

    //----------/proc/pid/cmdline-----------------
    sprintf(cmdlinefile, "/proc/%s/cmdline", file);

    if((fp = fopen(cmdlinefile, "r")) == NULL){
        printf("cannot open %s\n", cmdlinefile);
        return;
    }

    n = fread(buffer, 1, 4096, fp);
    if(n != 0){
        int i,j;
        
        if(buffer[n-1] != '\0'){
            n++;
        }

        for(i = 0; i < n-1; i++){
            if(buffer[i] == '\0'){
                buffer[i] = ' ';
            }
        }

        for(j = n-2; j > 0; j--){
            if(buffer[j] == ' '){
                continue;
            }
            else{
                buffer[++j] = '\0';
                break;
            }
        }

        strcpy(proc.cmdline, buffer);
    }
    else{
        strcpy(proc.cmdline, "");
    }

    fclose(fp);
    myproc.push_back(proc);
    return;
}

int select_condition(int uid_option, int tty_option, int uid, int index){
    if(uid_option == 0 && tty_option == 0) //now uid, no tty
        return ((uid != myproc.at(index).uid) || (strcmp(myproc.at(index).ttyname, "-") == 0));
    else if(uid_option == 0 && tty_option == 1) //now uid , all tty
        return (uid != myproc.at(index).uid);
    else if(uid_option == 1 && tty_option == 0) //all uid, no tty
        return (strcmp(myproc.at(index).ttyname, "-") == 0);
    else if(uid_option == 1 && tty_option == 1) //all uid, all tty
        return 0;
}

void sort_by_condition(int pid_option, int ppid_option, int pgid_option, int sid_option){
    if(pid_option == 1)         sort(myproc.begin(), myproc.end(), pid_compare);
    else if(ppid_option == 1)   sort(myproc.begin(), myproc.end(), ppid_compare);
    else if(pgid_option == 1)   sort(myproc.begin(), myproc.end(), pgid_compare);
    else if(sid_option == 1)    sort(myproc.begin(), myproc.end(), sid_compare);
}

void print_proc(int uid_option, int tty_option){
    int uid = getuid();

    printf("  pid   uid   gid  ppid  pgid   sid      tty St (img) cmd\n");
    for(int i = 0; i < myproc.size(); i++){
        if(select_condition(uid_option, tty_option, uid, i))
            continue;
        if(strcmp(myproc.at(i).cmdline, "") == 0){
            printf("%5d%6d%6d%6d%6d%6d%9s%3c %s\n", 
                myproc.at(i).pid, 
                myproc.at(i).uid, 
                myproc.at(i).gid, 
                myproc.at(i).ppid, 
                myproc.at(i).pgid, 
                myproc.at(i).sid, 
                myproc.at(i).ttyname, 
                myproc.at(i).state, 
                myproc.at(i).cmd);
        }
        else{
            printf("%5d%6d%6d%6d%6d%6d%9s%3c %s %s\n", 
                myproc.at(i).pid, 
                myproc.at(i).uid, 
                myproc.at(i).gid, 
                myproc.at(i).ppid, 
                myproc.at(i).pgid, 
                myproc.at(i).sid, 
                myproc.at(i).ttyname, 
                myproc.at(i).state, 
                myproc.at(i).cmd,
                myproc.at(i).cmdline);
        }
    }
}

int main(int argc, char *argv[]){
    char name[] = "/dev";
    struct dirent *file;
    DIR *dir;
    int uid_option = 0;
    int tty_option = 0;
    int pid_option = 0;
    int ppid_option = 0;
    int pgid_option = 0;
    int sid_option = 0;

    //get devID/ttyname mapping table
    get_devmap(name);
    
    //get all proc info
    dir = opendir("/proc/");
    if(dir){
        while((file = readdir(dir)) != NULL){
            if((strcmp(file->d_name, ".") == 0) || (strcmp(file->d_name, "..") == 0) || (atoi(file->d_name) == 0))
                continue;
            get_proc_info(file->d_name);
        }
        closedir(dir);
    }

    //get output option
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-a") == 0)      uid_option = 1;
        else if(strcmp(argv[i], "-x") == 0) tty_option = 1;
        else if(strcmp(argv[i], "-p") == 0) pid_option = 1;
        else if(strcmp(argv[i], "-q") == 0) ppid_option = 1;
        else if(strcmp(argv[i], "-r") == 0) pgid_option = 1;
        else if(strcmp(argv[i], "-s") == 0) sid_option = 1;
    }

    //sort proc
    sort_by_condition(pid_option, ppid_option, pgid_option, sid_option);

    //print output
    print_proc(uid_option, tty_option);

    return 0;
}
