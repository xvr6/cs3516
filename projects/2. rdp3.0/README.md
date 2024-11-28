# Project 2: rdp 3.0

> Xavier Bonavita
> 12/3/2024

As a note, my code is only within the student2.c file. All the other .c/.h files in this folder are provided by the professor.

## How to run this program

1) Compile the program

- Use the provided makefile using the `make` command
- If manual compilation is necessary: `gcc -Wall -o p2 -g project2.c student2.c`

2) Execution of project
`./p2`

- To run the p2 binary compiled in step 1, you will need to specify several arguments. I have listed the name of each argument as they appear, followed by recommended values.
  - Enter the number of messages to simulate: `100`
  - Packet loss probability [enter number between 0.0 and 1.0]: `.1`
  - Packet corruption probability [0.0 for no corruption]: `.2`
  - Packet out-of-order probability [0.0 for no out-of-order]: `.2`
  - Average time between messages from sender's layer5 [ > 0.0]: `1000`
    - Any lower than 1000 appears to be unstable!
  - Enter Level of tracing desired: `0`
  - Do you want actions randomized: (1 = yes, 0 = no)? `1`
  - Do you want Bidirectional: (1 = yes, 0 = no)? `0`
