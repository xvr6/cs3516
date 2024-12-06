#include <stdio.h>
#include <stdlib.h>

#include "project3.h"

extern int TraceLevel;

distance_table dt0;
NeighborCosts* neighbor0;

#define INDEX 0

/* students to write the following two routines, and maybe some opthers */


// initializes node 0.
void rtinit0() {
    if (TraceLevel == 2) printf("rtinit%d: initializing node %d...\n\n", INDEX, INDEX);

    distance_table* dt = &dt0;

    // initalize everything to infinity
    for (int i = 0; i < MAX_NODES; i++) {
        for (int j = 0; j < MAX_NODES; j++) {
            dt->costs[i][j] = INFINITY;
            // if(TraceLevel == 2) printf("rtinit%d: dt[%d,%d] = %d\n", INDEX, i, j, dt->costs[i][j]);
        }
    }

    // get neighbor costs
    NeighborCosts* n = neighbor0;
    n = getNeighborCosts(INDEX);

    int costs[MAX_NODES];
    for (int i = 0; i < n->NodesInNetwork; i++) {
        dt->costs[i][i] = n->NodeCosts[i];
        costs[i] = n->NodeCosts[i];
    };

    if (TraceLevel == 2) printDT(dt);


    // create each packet to send
    for (int p = 0; p < MAX_NODES; p++) {
        if (p == INDEX) continue;  // ensures we aren't sending packet to self.

        RoutePacket* pkt = malloc(sizeof(RoutePacket));

        pkt->sourceid = INDEX;
        pkt->destid = p;

        int minimums[MAX_NODES];
        calculateMins(dt, costs, minimums);
        for (int i = 0; i < MAX_NODES; i++) {
            pkt->mincost[i] = minimums[i];
        };

        toLayer2(*pkt);
    }
}

// update values stored in node 0
void rtupdate0(RoutePacket* rcvdpkt) {
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
void printdt0(int MyNodeNumber, NeighborCosts* neighbor,
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
}  // End of printdt0
