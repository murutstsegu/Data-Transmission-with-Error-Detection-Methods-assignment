# Socket Programming Assignment – Data Transmission with Error Detection

This project demonstrates data transmission using socket programming in C
with error detection techniques such as Parity Bit and 2D Parity.

The system uses a Client–Server–Client architecture:
- Client1 sends data
- Server processes and forwards data
- Client2 receives and checks for errors

## Files
- server.c   : Handles communication between Client1 and Client2
- client1.c  : Sender (Parity generation)
- client2.c  : Receiver (Error detection)

## Requirements
- Linux / macOS / WSL
- GCC compiler

## Compile
Run the following commands in the project directory:

gcc server.c -o server  
gcc client1.c -o client1  
gcc client2.c -o client2  

## Run (Order Matters)
Open three terminals:

Terminal 1:
./server

Terminal 2:
./client2

Terminal 3:
./client1
