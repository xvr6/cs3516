#include "project3.h"
#include <stdio.h>

// NOTE: my helper functions
void calculateMins(distance_table* dt, int* costs, int* mins) {
    for (int i = 0; i < MAX_NODES; i++) {
        int iMin = INFINITY;
        for (int j = 0; j < MAX_NODES; j++) {
            int temp = dt->costs[i][j];
            if (temp < iMin) iMin = temp;
        }
        mins[i] = iMin;
    }
}

void printDT(distance_table* dt) {
    for (int i = 0; i < MAX_NODES; i++) {
        for (int j = 0; j < MAX_NODES; j++) {
            printf("%d ", dt->costs[i][j]);
        }
        printf("\n");
    }
}
