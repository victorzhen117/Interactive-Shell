#ifndef SFISH_H
#define SFISH_H
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>


#endif

typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);
unsigned int Alarm(unsigned int seconds);
void Sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
void Sigemptyset(sigset_t *set);
void Sigfillset(sigset_t *set);
void Sigaddset(sigset_t *set, int signum);
void Sigdelset(sigset_t *set, int signum);
int Sigismember(const sigset_t *set, int signum);
extern pid_t Fork();
extern void unix_error(char *msg);
extern int Dup2(int, int);
extern int Pipe(int pipefd[2]);
