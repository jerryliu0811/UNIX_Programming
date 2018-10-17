#define SIGHUP      1
#define SIGINT      2
#define SIGQUIT     3
#define SIGILL      4
#define SIGTRAP     5
#define SIGABRT     6
#define SIGBUS      7
#define SIGFPE      8
#define SIGKILL     9
#define SIGUSR1     10
#define SIGSEGV     11
#define SIGUSR2     12
#define SIGPIPE     13
#define SIGALRM     14
#define SIGTERM     15
#define SIGSTKFLT   16
#define SIGCHLD     17
#define SIGCONT     18
#define SIGSTOP     19
#define SIGTSTP     20
#define SIGTTIN     21
#define SIGTTOU     22
#define SIGURG      23
#define SIGXCPU     24
#define SIGXFSZ     25
#define SIGVTALRM   26
#define SIGPROF     27
#define SIGWINCH    28
#define SIGIO       29
#define SIGWR       30

#define SIG_BLOCK   0
#define SIG_UNBLOCK 1
#define SIG_SETMASK 2

#define SA_RESTORER 0x04000000

#define NULL 0
/* bits in x64
short               2 bits
int                 4 bits
long                8 bits
long long           8 bits
unsigned long int   8 bits
*/

typedef int                ssize_t;     //4 bits
typedef long                time_t;     //8 bits
typedef unsigned long       size_t;     //8 bits
typedef unsigned long int sigset_t;     //8 bits

struct timespec {
    time_t  tv_sec;
    long    tv_nsec;
};

struct sigaction {
    void    (*sa_handler)(int);
    int     sa_flags;
    void    (*sa_restorer)(void);
};

typedef struct jmp_buf_s {
    long long reg[8];
    sigset_t mask;
} jmp_buf[1];

long sys_write(unsigned int fd, char *buf, size_t count);
long sys_alarm(unsigned int seconds);
long sys_pause(void);
long sys_nanosleep(struct timespec *rqtp, struct timespec *rmtp);
long sys_rt_sigaction(int how, struct sigaction *nact, struct sigaction *oact, size_t size);
long sys_sigprocmask(int how, sigset_t *set, sigset_t *oset);

__attribute__ ((noreturn))
void sys_exit(int error_code);

ssize_t write(int fd, void *buf, size_t count){
    return sys_write(fd, buf, count);
}

unsigned int alarm(unsigned int seconds){
    return sys_alarm(seconds);
}

int pause(void){
    return sys_pause();
}

long sleep(long s){
    struct timespec req = { s, 0 };
    return sys_nanosleep(&req, NULL);
}

int setjmp(jmp_buf env);

void longjmp(jmp_buf env, int val);

void __myrt(void);

int sigaction(int how, struct sigaction *nact, struct sigaction *oact){
    nact->sa_flags |= SA_RESTORER;
    nact->sa_restorer = __myrt;
    return sys_rt_sigaction(how, nact, oact, sizeof(sigset_t));
}

int sigprocmask(int how, sigset_t *set, sigset_t *oset){
    return sys_sigprocmask(how, set, oset);
}

void exit(int error_code){
    sys_exit(error_code);
}
