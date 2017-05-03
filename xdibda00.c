#define _POSIX_C_SOURCE 199506L
#define _XOPEN_SOURCE 500
#define _XOPEN_SOURCE_EXTENDED 1

#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

const int NUMBER_OF_ALLOWED_ARGUMENTS = 3;

int counterTicket;
int counterProcess;

struct {
    int threads;
    int critical;
} arguments;

struct {
    pthread_t *pthreads;
    pthread_cond_t *cond;
    pthread_mutex_t tickets;
    pthread_mutex_t mutex;
} thread;

bool controlArguments(int number, char* arguments[]);
bool controlIfDigitsOnly(char* argument);

void await(int aenter);
void advance();
void waiting();

void threadsCreate();
void threadsJoin();
void* threadsRoutine(void* id);

int getticket(void);
int generateNumber();

int main(int argc, char* argv[]) {
    if (!controlArguments(argc, argv)) {
        fprintf(stderr, "Bad arguments\n");
        return EXIT_FAILURE;
    }

    counterTicket = 0;
    counterProcess = 0;

    thread.pthreads = (pthread_t*) malloc(sizeof(pthread_t) * arguments.threads);
    thread.cond = (pthread_cond_t *) malloc(sizeof(pthread_cond_t) * arguments.critical);

    pthread_mutex_init(&thread.tickets, NULL);
    pthread_mutex_init(&thread.mutex, NULL);

    for (int i = 0; i < arguments.critical; ++i) {
        pthread_cond_init(&thread.cond[i], NULL);
    }

    threadsCreate();
    threadsJoin();

    return EXIT_SUCCESS;
}

void* threadsRoutine(void* id) {
    int ticket, idn = (intptr_t) id;

    while ((ticket = getticket()) < arguments.critical) {
        if (ticket == -1) {
            break;
        }

        waiting();

        await(ticket);
        printf("%d\t(%d)\n", ticket, idn);
        fflush(stdout);
        advance();

        waiting();
    }

    return EXIT_SUCCESS;
}

int getticket(void) {
    int ticket;
    pthread_mutex_lock(&thread.tickets);
    if (counterProcess >= arguments.critical) {
        pthread_mutex_unlock(&thread.tickets);
        return -1;
    }

    ticket = counterTicket++;
    pthread_mutex_unlock(&thread.tickets);
    return ticket;
}

void await(int aenter) {
    pthread_mutex_lock(&thread.mutex);
    while (aenter != counterProcess)
        pthread_cond_wait(&thread.cond[aenter], &thread.mutex);
}

void advance() {
    counterProcess++;

    if (counterProcess >= arguments.critical) {
        pthread_mutex_unlock(&thread.mutex);
        return;
    }

    pthread_cond_signal(&thread.cond[counterProcess]);
    pthread_mutex_unlock(&thread.mutex);
}

int generateNumber() {
    unsigned int seed = (unsigned int) time(NULL);
    return (rand_r(&seed) % 500000000 + 1);
}

void waiting() {
    struct timespec timespec1 = {
            0, generateNumber()
    };

    struct timespec *timespec2 = &timespec1;

    nanosleep(timespec2, NULL);
}

void threadsCreate() {
    for (int i = 0; i < arguments.threads; i++) {
        pthread_create(&thread.pthreads[i], NULL, threadsRoutine, (void*) (intptr_t) (i + 1));
    }
}

void threadsJoin() {
    for (int i = 0; i < arguments.threads; i++) {
        pthread_join(thread.pthreads[i], NULL);
    }
}

bool controlIfDigitsOnly(char* argument) {
    for (int i = 0; i < (int) strlen(argument) ; i++) {
        if (!isdigit(argument[i]))
            return false;
    }

    return true;
}

bool controlArguments(int number, char* params[]) {
    if (number != NUMBER_OF_ALLOWED_ARGUMENTS) {
        return false;
    }

    for (int i = 1; i < NUMBER_OF_ALLOWED_ARGUMENTS; i++) {
        if(!controlIfDigitsOnly(params[i])) {
            return false;
        }
    }

    arguments.threads = atoi(params[1]);
    arguments.critical = atoi(params[2]);

    return true;
}
