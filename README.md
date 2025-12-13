# Socket-Programming-Assignment-Data-Transmission-with-Error-Detection-Methods
My name is MURUTS TSEGU GEBRETSADIK AND THIS IS an assignment(project) i did for data communications and systems course
# Data Communication Project

This project demonstrates **error detection in data communication** using **TCP socket programming in C**.

The system uses a **Client–Server–Client architecture**:
- **Client 1** sends data with control information
- **Server** intentionally corrupts the data
- **Client 2** receives the data and detects errors

Implemented error detection methods:
- Parity Bit  
- 2D Parity  
- CRC-8  
- Hamming Code  
- Internet Checksum  

The server applies random transmission errors such as bit flips, character insertion/deletion, substitution, swapping, and burst errors.

All packets follow this format:

### Files
- `client1.c` – Sender
- `server.c` – Error injector
- `client2.c` – Receiver and verifier

### How to Run
Compile:
```bash
gcc server.c -o server
gcc client1.c -o client1
gcc client2.c -o client2
./server
./client2
./client1

