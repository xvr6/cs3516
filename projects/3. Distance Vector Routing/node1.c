#include <stdio.h>
#include <stdlib.h>

#include "project3.h"

extern int TraceLevel;
extern float clocktime;

distance_table dt1;
NeighborCosts* neighbor1;

#define INDEX 1

/* students to write the following two routines, and maybe some opthers */

// initializes node
void rtinit1() {
    if (TraceLevel == 2) printf("rtinit%d: initializing node %d...\n\n", INDEX, INDEX);

    distance_table* dt = &dt1;

    // initalize everything to infinity
    for (int i = 0; i < MAX_NODES; i++) {
        for (int j = 0; j < MAX_NODES; j++) {
            dt->costs[i][j] = INFINITY;
        }
    }

    // get neighbor costs
    neighbor1 = getNeighborCosts(INDEX);

    for (int i = 0; i < neighbor1->NodesInNetwork; i++) {
        dt->costs[i][i] = neighbor1->NodeCosts[i];
    };

    if (TraceLevel == 2) {
        printf("Printing dt%d\n", INDEX);
        printDT(dt);
    }

    printdt1(INDEX, neighbor1, dt);

    // create each packet to send
    for (int p = 0; p < MAX_NODES; p++) {
        if (p == INDEX || neighbor1->NodeCosts[p] == INFINITY) continue;  // ensures we aren't sending packet to self.

        RoutePacket* pkt = malloc(sizeof(RoutePacket));

        pkt->sourceid = INDEX;
        pkt->destid = p;

        int minimums[MAX_NODES];
        calculateMins(dt, minimums);
        for (int i = 0; i < MAX_NODES; i++) {
            pkt->mincost[i] = minimums[i];
        };

        toLayer2(*pkt);
    }
}

// update values stored in node
void rtupdate1(RoutePacket* rcvdpkt) {
    // extrapolate for simpler calls
    int from = rcvdpkt->sourceid;
    int* rvccosts = rcvdpkt->mincost;
    int hasUpdated = NO;
    distance_table* dt = &dt1;

    // compare values
    for (int i = 0; i < MAX_NODES; i++) {
        int curCost = dt->costs[from][from] + rvccosts[i];
        int prevCost = dt->costs[i][from];
        if (curCost < prevCost) {
            dt->costs[i][from] = curCost;
            hasUpdated = YES;
            if (TraceLevel == 2) printf("rtupdate%d: dt[%d][%d] updated to %d\n", INDEX, i, from, curCost);
        }
    }

    if (TraceLevel == 2) printf("rtupdate%d: called at t=%.3f by %d. Has updated: %d\n", INDEX, clocktime, from, hasUpdated);

    if (!hasUpdated) return;
    printCurrentDistanceVector1();

    NeighborCosts* n = getNeighborCosts(INDEX);

    //  create each packet to send
    for (int p = 0; p < MAX_NODES; p++) {
        if (p == INDEX || n->NodeCosts[p] == INFINITY) continue;  // ensures we aren't sending packet to self.

        RoutePacket* pkt = malloc(sizeof(RoutePacket));

        pkt->sourceid = INDEX;
        pkt->destid = p;

        int minimums[MAX_NODES];
        calculateMins(dt, minimums);
        for (int i = 0; i < MAX_NODES; i++) {
            pkt->mincost[i] = minimums[i];
        };

        toLayer2(*pkt);
    }

    if (TraceLevel == 2) printdt1(INDEX, neighbor1, dt);
}

// print current distance vector
void printCurrentDistanceVector1() {
    int minimums[MAX_NODES];
    calculateMins(&dt1, minimums);
    printf("At time t=%f, node %d current distance vector: %d %d %d %d\n", clocktime, INDEX, minimums[0], minimums[1], minimums[2], minimums[3]);
}

/////////////////////////////////////////////////////////////////////
//  printdt
//  This routine is being supplied to you.  It is the same code in
//  each node and is tailored based on the input arguments.
//  Required arguments:
//  MyNodeNumber:  This routine assumes that you know your node
//                 number and supply it when making this call.
//  NeighborCosts *neighbor:  A pointer to the structure
//                 that's supplied via a call to getNeighborCosts().
//                 It tells this print routine the configuration
//                 of nodes surrounding the node we're working on.
//  distance_table *dtptr: This is the running record of the
//                 current costs as seen by this node.  It is
//                 constantly updated as the node gets new
//                 messages from other nodes.
/////////////////////////////////////////////////////////////////////
void printdt1(int MyNodeNumber, NeighborCosts* neighbor,
              distance_table* dtptr) {
    int i, j;
    int TotalNodes = neighbor->NodesInNetwork;  // Total nodes in network
    int NumberOfNeighbors = 0;                  // How many neighbors
    int Neighbors[MAX_NODES];                   // Who are the neighbors

    // Determine our neighbors
    for (i = 0; i < TotalNodes; i++) {
        if ((neighbor->NodeCosts[i] != INFINITY) && i != MyNodeNumber) {
            Neighbors[NumberOfNeighbors] = i;
            NumberOfNeighbors++;
        }
    }
    // Print the header
    printf("                via     \n");
    printf("   D%d |", MyNodeNumber);
    for (i = 0; i < NumberOfNeighbors; i++)
        printf("     %d", Neighbors[i]);
    printf("\n");
    printf("  ----|-------------------------------\n");

    // For each node, print the cost by travelling thru each of our neighbors
    for (i = 0; i < TotalNodes; i++) {
        if (i != MyNodeNumber) {
            printf("dest %d|", i);
            for (j = 0; j < NumberOfNeighbors; j++) {
                printf("  %4d", dtptr->costs[i][Neighbors[j]]);
            }
            printf("\n");
        }
    }
    printf("\n");
}  // End of printdt1
