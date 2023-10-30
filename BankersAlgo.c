#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>

int numResources,
    numProcesses;
int *availableResources;
int **resourcesAllocated;
int **maxResourcesRequired;
int **resourcesNeeded;
int *safeSequence;
int numProcessesRun = 0;

pthread_mutex_t resourceLock;
pthread_cond_t condition;

// Function to calculate a safe sequence or return false
bool calculateSafeSequence();

// Function to represent the process behavior
void* processCode(void* );

int main(int argc, char** argv) {
    srand(time(NULL));

    printf("Enter the number of processes: ");
    scanf("%d", &numProcesses);

    printf("Enter the number of resource: ");
    scanf("%d", &numResources);

    availableResources = (int *)malloc(numResources * sizeof(*availableResources));
    printf("Enter the AVAILABLE resources, space-separated(e.g., 5 3 2 1 ...): ");
     for(int i=0; i<numResources; i++)
        scanf("%d", &availableResources[i]);

    resourcesAllocated = (int **)malloc(numProcesses * sizeof(*resourcesAllocated));
    for(int i=0; i<numProcesses; i++)
        resourcesAllocated[i] = (int *)malloc(numResources * sizeof(**resourcesAllocated));

    maxResourcesRequired = (int **)malloc(numProcesses * sizeof(*maxResourcesRequired));
    for(int i=0; i<numProcesses; i++)
        maxResourcesRequired[i] = (int *)malloc(numResources * sizeof(**maxResourcesRequired));

    // Resource allocation
    printf("\n");
    for(int i=0; i<numProcesses; i++) {
        printf("Enter the resource ALLOCATION for Process%d, space-separated(e.g., 5 3 2 1 ...): ", i+1);
        for(int j=0; j<numResources; j++)
            scanf("%d", &resourcesAllocated[i][j]);
    }
    printf("\n");

    // Maximum resource requirements
    for(int i=0; i<numProcesses; i++) {
        printf("Enter the MAXIMUM resource requirements for Process%d, space-separated(e.g., 5 3 2 1 ...): ", i+1);
        for(int j=0; j<numResources; j++)
            scanf("%d", &maxResourcesRequired[i][j]);
    }
    printf("\n");

    // Calculate the resources needed matrix
    resourcesNeeded = (int **)malloc(numProcesses * sizeof(*resourcesNeeded));
    for(int i=0; i<numProcesses; i++)
        resourcesNeeded[i] = (int *)malloc(numResources * sizeof(**resourcesNeeded));

    for(int i=0; i<numProcesses; i++)
        for(int j=0; j<numResources; j++)
            resourcesNeeded[i][j] = maxResourcesRequired[i][j] - resourcesAllocated[i][j];
     // Calculate the safe sequence
    safeSequence = (int *)malloc(numProcesses * sizeof(*safeSequence));
    for(int i=0; i<numProcesses; i++)
        safeSequence[i] = -1;

    if (!calculateSafeSequence()) {
        printf("The system is in an unsafe state. No safe sequence exists.\n\n");
        exit(-1);
    }

    printf("Safe Sequence Found: ");
    for (int i = 0; i < numProcesses; i++) {
        printf("%d  ", safeSequence[i] + 1);
    }

    printf("\nExecuting Processes...\n\n");
    sleep(1);

    // Run threads
    pthread_t processes[numProcesses];
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    int processNumber[numProcesses];
    for (int i = 0; i < numProcesses; i++)
        processNumber[i] = i;

    for (int i = 0; i < numProcesses; i++)
        pthread_create(&processes[i], &attr, processCode, (void *)(&processNumber[i]));

    for (int i = 0; i < numProcesses; i++)
        pthread_join(processes[i], NULL);

    printf("All Processes Finished\n");
 // Free resources
    free(availableResources);
    for (int i = 0; i < numProcesses; i++) {
        free(resourcesAllocated[i]);
        free(maxResourcesRequired[i]);
        free(resourcesNeeded[i]);
    }
    free(resourcesAllocated);
    free(maxResourcesRequired);
    free(resourcesNeeded);
    free(safeSequence);
}

bool calculateSafeSequence() {
    // Calculate the safe sequence
    int tempResources[numResources];
    for (int i = 0; i < numResources; i++) tempResources[i] = availableResources[i];

    bool finished[numProcesses];
    for (int i = 0; i < numProcesses; i++) finished[i] = false;
    int numFinished = 0;
    while (numFinished < numProcesses) {
        bool safe = false;

        for (int i = 0; i < numProcesses; i++) {
            if (!finished[i]) {
                bool possible = true;

                for (int j = 0; j < numResources; j++)
                    if (resourcesNeeded[i][j] > tempResources[j]) {
                        possible = false;
                        break;
                    }

                if (possible) {
                    for (int j = 0; j < numResources; j++)
                        tempResources[j] += resourcesAllocated[i][j];
                    safeSequence[numFinished] = i;
                    finished[i] = true;
                    ++numFinished;
                    safe = true;
                }
            }
        }

        if (!safe) {
            for (int k = 0; k < numProcesses; k++) safeSequence[k] = -1;
            return false; // No safe sequence found
        }
    }
    return true; // Safe sequence found
}

// Process behavior
void* processCode(void *arg) {
    int p = *((int *) arg);

    // Lock resources
    pthread_mutex_lock(&resourceLock);

    // Condition check
    while (p != safeSequence[numProcessesRun])
        pthread_cond_wait(&condition, &resourceLock);

    // Process execution
    printf("Executing Process%d\n", p + 1);
    printf("Resource Allocation: ");
    for (int i = 0; i < numResources; i++)
        printf("%d  ", resourcesAllocated[p][i]);

    printf("\nResources Needed: ");
    for (int i = 0; i < numResources; i++)
       printf("%d  ", resourcesNeeded[p][i]);

    printf("\nAvailable Resources: ");
    for (int i = 0; i < numResources; i++)
        printf("%d  ", availableResources[i]);

    printf("\n");
    sleep(1);

    printf("Resource Allocated!\n");
    sleep(1);
    printf("Process Code Running...\n");
    sleep(rand() % 3 + 2); // Simulated process code
    printf("Process Code Completed...\n");
    sleep(1);
    printf("Releasing Resources...\n");
    sleep(1);
    printf("Resources Released!\n");

    for (int i = 0; i < numResources; i++)
        availableResources[i] += resourcesAllocated[p][i];

    printf("Now Available Resources: ");
    for (int i = 0; i < numResources; i++)
        printf("%d  ", availableResources[i]);
    printf("\n\n");

    sleep(1);

    // Condition broadcast
    numProcessesRun++;
    pthread_cond_broadcast(&condition);
    pthread_mutex_unlock(&resourceLock);
    pthread_exit(NULL);
}
