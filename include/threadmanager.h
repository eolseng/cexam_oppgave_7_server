/**
 * ThreadManager™ v.1 by Student X
 * ---------------------------------------------------------------
 * ___ _  _ ____ ____ ____ ___  _  _ ____ _  _ ____ ____ ____ ____ ™
 *  |  |__| |__/ |___ |__| |  \ |\/| |__| |\ | |__| | __ |___ |__/
 *  |  |  | |  \ |___ |  | |__/ |  | |  | | \| |  | |__] |___ |  \
 * ---------------------------------------------------------------
 *
 * ThreadManager™ is a lightweight thread manager written by Student X.
 * It starts the given amount of threads, defined by 'THREAD_POOL_SIZE',
 * and lets users execute any functions in one of the available threads.
 */


#ifndef THREAD_MANAGER_THREADMANAGER_H
#define THREAD_MANAGER_THREADMANAGER_H

#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define THREAD_POOL_SIZE 5

typedef struct {
    int iId;
    int isRunning;
    int isActive;

    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond;

    void (*FunctionPointer)(void *);

    void *FunctionArgs;
} THREAD_HOLDER;

typedef struct {
    THREAD_HOLDER *pThreadPool[THREAD_POOL_SIZE];
} THREAD_MANAGER;

/**
 * Creates a Thread Manager™ and starts the given defined amount of worker threads
 *
 * Returns 0 on success and 1 on failure
 */
int CreateManager(int64_t *pManagerHandler);

/**
 * Stops all running threads, joins them and deallocates all memory allocated by the manager
 *
 * Returns 0 always
 */
int DestroyManager(int64_t pManagerHandler);

/**
 * Execute the provided function in one of the available threads.
 *
 * Returns 0 on success (function being executed in thread) and 1 on failure (no thread available)
 */
int ExecuteFunction(int64_t pManagerHandler, void *pFunction, void *pArgs);

#endif //THREAD_MANAGER_THREADMANAGER_H
