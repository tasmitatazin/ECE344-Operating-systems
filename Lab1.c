#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <unistd.h>

void check(int ret, const char* message) {
if (ret != -1) {
    return;
}
int error = errno;
perror(message);
exit(error);
}

#define mp 300

typedef struct { //global struct that will keep track of all information about the process created
    pid_t pid;       
    char *cmd;      
    int situation;     
} pinfo;

static pinfo ptab[mp];
static size_t keepc = 0; //keeping count of our processes here

void opendir2()
{
    perror("opendir");
                exit(EXIT_FAILURE);
}

void ssp_init() {

    keepc = 0;
}

    void check1(DIR* direcing)
    {
        if (!direcing) 
            {
                opendir2();
            }

    }

int ssp_create(char *const *argv, int fd0, int fd1, int fd2) {
    
    pid_t pid_we = fork(); 
    check(pid_we, "fork");

    if (pid_we== 0) //entering child processs
    { 

    //connecting our file descriptors 
    check(dup2(fd0,0),"dup2");
    check(dup2(fd1,1),"dup2");
    check(dup2(fd2,2),"dup2");

        struct dirent *noww; //entering dircetory
        DIR *direc_open= opendir("/proc/self/fd");
        check1(direc_open);
        while ((noww = readdir(direc_open)) != NULL) {

            if (noww->d_type == DT_LNK) {

            int fd = atoi(noww->d_name);
            if (fd>2)
            {
            close(fd);

            }
            }
            }
            closedir(direc_open); 

           execvp(argv[0], argv);

        }

    ptab[keepc].pid = pid_we;
    ptab[keepc].cmd = strdup(argv[0]); 
    ptab[keepc].situation = -1; 
    keepc++;
    return keepc - 1; 
}

 
bool checkifalready(int id)
{
    if (ptab[id].situation != -1) {
        return true;
    }
    else 
    {
        return false;
    }
}
void failwaitpid()
{
    perror("waitpid failed");
            exit(errno);
}
int ssp_get_status(int ssp_id) {


        pid_t pid = ptab[ssp_id].pid;
        int addressofinfo;
        int answer;


        if (checkifalready(ssp_id)) {
            answer = ptab[ssp_id].situation;  
        } 
        else 
        {           
            pid_t answer2 = waitpid(pid, &addressofinfo, WNOHANG);

            switch (answer2) {
               case -1:
        if (errno == ECHILD) {
            answer = ECHILD;
        } else {
            failwaitpid();
        }
        break;

    case 0:
        answer = -1; // will happen if still running
        break;

    default: //what if process has ended?
        if (WIFEXITED(addressofinfo)) {
            answer = WEXITSTATUS(addressofinfo);  
            ptab[ssp_id].situation = answer;  
        } 
        else if (WIFSIGNALED(addressofinfo)) { //checking for signal 
            answer = 128 + WTERMSIG(addressofinfo);  
            ptab[ssp_id].situation = answer;  
        } else {
            answer = ECHILD;   //error 
        }
        break;
}

        }

   return answer;
}



void ssp_send_signal(int ssp_id, int signum) {

    kill(ptab[ssp_id].pid, signum);

}



void ssp_wait()
{

    for (size_t i = 0; i < keepc; i++) {
        int ourstat; 
        pid_t pid = ptab[i].pid; 
        if (ptab[i].situation==-1)
        {
        //we have to check wait only if status is -1 otherwise no
        pid_t answer = waitpid(pid, & ourstat, 0);
        
        switch(answer)
        {
            case -1:
            perror("error on waitpid");
            exit(errno);
            break;
            default:

        if (WIFEXITED( ourstat)) {
            ptab[i].situation = WEXITSTATUS( ourstat);
        } else if (WIFSIGNALED( ourstat)) { //check for signal 
            ptab[i].situation = WTERMSIG( ourstat) + 128; 
        } else {
            ptab[i].situation = -1; 

        }
       
    }
    }

}
}

void printheader(int bl)
{
    printf("%7s %-*s %s\n", "PID", bl, "CMD", "STATUS");
}
void printheader2(pid_t pids,int bl, int i, int answer)
{
    printf("%7d %-*s %d\n", pids, bl, ptab[i].cmd,answer);

}

  void ssp_print() {

    int bl = 3;     
    for (size_t i = 0; i < keepc; i++) { //update the biggest name size
        size_t l = strlen(ptab[i].cmd); 
        if (l >bl) {
            bl = l;       
        }
    }

printheader(bl);

    for (size_t i = 0; i < keepc; i++) {
        pid_t pid = ptab[i].pid; // Get the PID
        int addressofinfo;
        int answer;

        
        if (ptab[i].situation != -1) {
           answer = ptab[i].situation;  
                   } else {


            
            
            pid_t answer2 = waitpid(pid, &addressofinfo, WNOHANG);
            switch(answer2)

            {
                case -1:

                if (errno == ECHILD) {
                    answer= ECHILD;
                } else {
                    perror("fail for waitpid");
                    exit(errno);
                }
                break; 

                case 0:
                answer = -1;

                break;

                default:

                if (WIFEXITED(addressofinfo)) {
                answer = WEXITSTATUS(addressofinfo);  
                ptab[i].situation = answer; 
            } else if (WIFSIGNALED(addressofinfo)) {
                answer= 128 + WTERMSIG(addressofinfo);  
                ptab[i].situation = answer; 
            } else {
                answer = ECHILD; 
            }

            }
            
        }

          printheader2(pid,bl,i,answer);
    }
}

    
        
    
