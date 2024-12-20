#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "project2.h"

/* ***************************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for unidirectional or bidirectional
   data transfer protocols from A to B and B to A.
   Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets may be delivered out of order.

   Compile as gcc -g project2.c student2.c -o p2
**********************************************************************/

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
/*
 * The routines you will write are detailed below. As noted above,
 * such procedures in real-life would be part of the operating system,
 * and would be called by other procedures in the operating system.
 * All these routines are in layer 4.
 */


#define TIME_INCR 1500

#define DEBUG 0 // Define statement for debug prints

#if DEBUG
    #define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
    #define DEBUG_PRINT(...) 
#endif

typedef struct msg msg;
typedef struct pkt pkt;

struct pktq {
    pkt packet;
    struct pktq* next;
};

typedef struct pktq pktq;

pktq* pktqHead;        // packet queue
pktq* pktqTail;        // Last packet in queue
int pktqSize;

// Sender (A-side) variables
int A_seqnum;          // Current sequence number for A
int A_acknum;          // Currently accepted ACK
int A_wait;            // Flag for unacknowledged packet

// Receiver (B-side) variables
int B_expected; // Sequence number expected by B

// Function to calculate the checksum of a packet
int calculateChecksum(pkt* p) {
    char* data = p->payload;
    int acknum = p->acknum;
    int seqnum = p->seqnum;

    int i, checksum = 0;
    for (i = 0; i < MESSAGE_LENGTH; i++) {
        checksum += (int)(data[i]) * i;
    }
    checksum += acknum * 21;
    checksum += seqnum * 22;
    return checksum;
}

// Function to resend packets from the queue
void resendPkt(){
    stopTimer(AEntity);

    pktq* curPkt = pktqHead;
    int max = pktqSize;

    for (int i = 0; i < max; i++){
        if(!(curPkt->packet.seqnum < B_expected)){
            DEBUG_PRINT("Resending packet with seqnum: %d\n", curPkt->packet.seqnum); // Debug print
            tolayer3(AEntity, curPkt->packet);
        }
        curPkt = curPkt->next;
    }
    
    startTimer(AEntity, TIME_INCR); 
}

// Function to update the packet queue by removing acknowledged packets
void updatePktq(){
    while((pktqHead != NULL) && (pktqHead->packet.seqnum < A_acknum)){
        DEBUG_PRINT("Removing packet with seqnum: %d from queue\n", pktqHead->packet.seqnum); // Debug print
        pktq* temp = pktqHead;
        pktqHead = pktqHead->next;
        --pktqSize;
        free(temp);
    }
    if (pktqHead == NULL) {
        pktqTail = NULL; // Ensure tail is also NULL when queue is empty
    }
    // Update A_acknum and A_wait after cleaning the queue
    if (pktqHead != NULL) {
        A_acknum = pktqHead->packet.seqnum;
    } else {
        A_acknum = A_seqnum;
        A_wait = FALSE;
    }
}

/*
 * A_output(message), where message is a structure of type msg, containing
 * data to be sent to the B-side. This routine will be called whenever the
 * upper layer at the sending side (A) has a message to send. It is the job
 * of your protocol to insure that the data in such a message is delivered
 * in-order, and correctly, to the receiving side upper layer.
 */
// Function to handle output from A-side
void A_output(struct msg message) {
    // Create packet
    pkt pktToSend;
    memcpy(pktToSend.payload, message.data, sizeof(message.data));
    pktToSend.seqnum = A_seqnum++; 
    pktToSend.checksum = calculateChecksum(&pktToSend);

    // Create entry and add to queue
    pktq* pktqEntry = (pktq*) malloc(sizeof(pktq));
    pktqEntry->packet = pktToSend;
    pktqEntry->next = NULL; // Initialize next pointer to NULL

    if (pktqHead == NULL){ // If queue hasn't been initialized
        pktqHead = pktqEntry; 
    } else { // Ensure new entry is pointed to
        pktqTail->next = pktqEntry;
    }

    ++pktqSize;
    pktqTail = pktqEntry; // Add new entry to tail
    tolayer3(AEntity, pktToSend); // Transmit to B
    startTimer(AEntity, TIME_INCR);

    A_wait = TRUE;
}

/*
 * Just like A_output, but residing on the B side.  USED only when the
 * implementation is bi-directional.
 */
void B_output(struct msg message) {  // not needed
}

/*
 * A_input(packet), where packet is a structure of type pkt. This routine
 * will be called whenever a packet sent from the B-side (i.e., as a result
 * of a tolayer3() being done by a B-side procedure) arrives at the A-side.
 * packet is the (possibly corrupted) packet sent from the B-side.
 */
// Function to handle input at A-side (ACKs from B-side)
void A_input(struct pkt packet) {
    // Check for corruption
    int recSum = calculateChecksum(&packet);
    if (recSum != packet.checksum){
        DEBUG_PRINT("A_input: corrupted ACK received\n");
        resendPkt();
        return;
    }

    // Check for unwanted ACK
    if (!A_wait || packet.acknum != A_acknum) { 
        DEBUG_PRINT("A_input: unexpected ACK received\n");
        resendPkt();
        return;
    }

    stopTimer(AEntity);
    A_acknum = packet.acknum;
    DEBUG_PRINT("A_input: ACK received for seqnum: %d\n", A_acknum); // Debug print
    updatePktq();

    if(pktqSize == 0) A_wait = FALSE; // Corrected condition
}

/*
 * A_timerinterrupt()  This routine will be called when A's timer expires
 * (thus generating a timer interrupt). You'll probably want to use this
 * routine to control the retransmission of packets. See starttimer()
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
// Function to handle timer interrupt at A-side
void A_timerinterrupt() {
    if(!A_wait) return; // Not waiting for ACK, ignore
    DEBUG_PRINT("A_timerinterrupt: waiting for correct ACK\n");
    resendPkt();
}

/* The following routine will be called once (only) before any other    */
/* entity A routines are called. You can use it to do any initialization */
// Initialization function for A-side
void A_init() {
    A_seqnum = 0;
    A_acknum = 0;
    pktqHead = NULL;
    pktqTail = NULL;
    A_wait = FALSE;
    pktqSize = 0;
}

/*
 * Note that with simplex transfer from A-to-B, there is no routine  B_output()
 */

/*
 * B_input(packet),where packet is a structure of type pkt. This routine
 * will be called whenever a packet sent from the A-side (i.e., as a result
 * of a tolayer3() being done by a A-side procedure) arrives at the B-side.
 * packet is the (possibly corrupted) packet sent from the A-side.
 */
// Function to handle input at B-side (packets from A-side)
void B_input(struct pkt packet) {
    // Check for corruption
    int receivedSum = calculateChecksum(&packet);
    if (packet.checksum != receivedSum) {
        DEBUG_PRINT("B_input: pkt %d corrupted (rec: %d exp: %d)\n", packet.seqnum, receivedSum, packet.checksum);
        return;
    }

    // Check for out of order/loss
    if (packet.seqnum != B_expected) {
        DEBUG_PRINT("B_input: pkt %d received, expected pkt %d\n", packet.seqnum, B_expected);
        return;
    }

    // Received correct, uncorrupted packet
    DEBUG_PRINT("B_input: Correct packet %d received\n", packet.seqnum); // Debug print
    ++B_expected;

    // Send ACK
    pkt ACK;
    ACK.acknum = packet.seqnum;
    ACK.checksum = calculateChecksum(&ACK);
    tolayer3(BEntity, ACK);

    // 'Transmit' data
    msg msgConfirm;
    memcpy(msgConfirm.data, packet.payload, sizeof(packet.payload));
    tolayer5(BEntity, msgConfirm);

}

/*
 * B_timerinterrupt()  This routine will be called when B's timer expires
 * (thus generating a timer interrupt). You'll probably want to use this
 * routine to control the retransmission of packets. See starttimer()
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void B_timerinterrupt() { // Not bidirectional, not needed
}

/*
 * The following routine will be called once (only) before any other
 * entity B routines are called. You can use it to do any initialization
 */
// Initialization function for B-side
void B_init() {
    B_expected = 0;
}
