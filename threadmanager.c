#include "./include/threadmanager.h"

#include "./include/simplelogger.h"

void *RunHolder(void *pArgs) {
    THREAD_HOLDER *holder = (THREAD_HOLDER *) pArgs;
    holder->isRunning = 1;
    LogDebug("HOLDER[%d] - Started", holder->iId);
    pthread_mutex_lock(&holder->mutex);
    while (holder->isRunning) {
        LogDebug("HOLDER[%d] - Waiting for work", holder->iId);
        pthread_cond_wait(&holder->cond, &holder->mutex);
        // Do not execute function if Holder has received shutdown signal (isRunning = 0)
        if (holder->isRunning) {
            LogDebug("HOLDER[%d] - Working", holder->iId);
            (holder->FunctionPointer)(holder->FunctionArgs);
            LogDebug("HOLDER[%d] - Done working", holder->iId);
        } else {
            LogDebug("HOLDER[%d] - Stopping", holder->iId);
        }
        holder->isActive = 0;
    }
    pthread_mutex_unlock(&holder->mutex);
    return NULL;
}

int CreateManager(int64_t *pManagerHandler) {
    int i;
    int iStatus = 0;

    LogInfo("Creating THREAD_MANAGER with a Thread Pool of %d threads", THREAD_POOL_SIZE);

    THREAD_MANAGER *pManager = malloc(sizeof(THREAD_MANAGER));
    if (pManager == NULL) {
        LogError("Failed to allocate memory for THREAD_MANAGER");
        iStatus = 1;
    } else {
        // Create and start threads
        for (i = 0; i < THREAD_POOL_SIZE; i++) {
            pManager->pThreadPool[i] = malloc(sizeof(THREAD_HOLDER));
            if (pManager->pThreadPool[i] == NULL) {
                LogError("Failed to allocate memory for THREAD_HOLDER[%d]", i);
                i--;
                while (i >= 0) {
                    // Free already allocated pThreadPool
                    free(pManager->pThreadPool[i]);
                    i--;
                }
                // Free manager
                free(pManager);
                iStatus = 1;
                break;
            } else {
                pManager->pThreadPool[i]->iId = i;
                pManager->pThreadPool[i]->isRunning = 0;
                pManager->pThreadPool[i]->isActive = 0;
                pthread_mutex_init(&pManager->pThreadPool[i]->mutex, NULL);
                pthread_cond_init(&pManager->pThreadPool[i]->cond, NULL);
                pManager->pThreadPool[i]->FunctionPointer = NULL;
                pManager->pThreadPool[i]->FunctionArgs = NULL;
                pthread_create(&pManager->pThreadPool[i]->thread, NULL, RunHolder, (void *) pManager->pThreadPool[i]);
            }
        }
        *pManagerHandler = (int64_t) pManager;
    }
    // Sleep for 0.5 seconds to let threads start
    usleep(500000);
    if (iStatus != 0) {
        LogError("Failed to create THREAD_MANAGER");
    } else {
        LogInfo("Successfully created THREAD_MANAGER");
    }
    return iStatus;
}

int DestroyManager(int64_t pManagerHandler) {
    THREAD_MANAGER *pManager = (THREAD_MANAGER *) pManagerHandler;
    int i;
    for (i = 0; i < THREAD_POOL_SIZE; i++) {
        LogDebug("Stopping HOLDER[%d]", i);
        // Wait for holder to be available
        pthread_mutex_lock(&pManager->pThreadPool[i]->mutex);
        // Prepare holder for shutdown
        pManager->pThreadPool[i]->isRunning = 0;
        pManager->pThreadPool[i]->isActive = 1;
        pManager->pThreadPool[i]->FunctionPointer = NULL;
        pManager->pThreadPool[i]->FunctionArgs = NULL;
        pthread_cond_signal(&pManager->pThreadPool[i]->cond);
        pthread_mutex_unlock(&pManager->pThreadPool[i]->mutex);
        // Wait for thread in holder to finish
        pthread_join(pManager->pThreadPool[i]->thread, NULL);
        // Deallocate memory for holder
        free(pManager->pThreadPool[i]);
        pManager->pThreadPool[i] = NULL;
    }
    // Deallocate memory for pManager
    free(pManager);
    pManager = NULL;
    return 0;
}

int ExecuteFunction(int64_t pManagerHandler, void *pFunction, void *pArgs) {
    THREAD_MANAGER *pManager = (THREAD_MANAGER *) pManagerHandler;
    int bExecuted = 0;
    // Find a free THREAD_HOLDER to execute the function
    int i = 0;
    while (i < THREAD_POOL_SIZE) {
        if (pManager->pThreadPool[i]->isActive != 0) {
            i++;
        } else {
            LogDebug("HOLDER[%d] assigned to task", i);
            pthread_mutex_lock(&pManager->pThreadPool[i]->mutex);
            pManager->pThreadPool[i]->isActive = 1;
            // Set pThreadPool function pointer and args to provided pointers
            pManager->pThreadPool[i]->FunctionPointer = pFunction;
            pManager->pThreadPool[i]->FunctionArgs = pArgs;
            // Signal holder to execute function
            pthread_cond_signal(&pManager->pThreadPool[i]->cond);
            pthread_mutex_unlock(&pManager->pThreadPool[i]->mutex);
            bExecuted = 1;
            break;
        }
    }
    if (bExecuted == 0) {
        LogWarn("Could not find a free thread to execute task");
        return 1;
    } else {
        return 0;
    }
}

