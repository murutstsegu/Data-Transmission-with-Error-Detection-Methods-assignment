# Socket Programming Assignment – Data Transmission with Error Detection

This project demonstrates data transmission using socket programming and
multiple error detection techniques.

## Architecture
Client 1 (Sender) → Server (Agent & Data Corruptor) → Client 2 (Receiver)

## Client 1 – Data Sender
- Takes text input from the user
- Generates control information using:
  - Parity Bit (Even / Odd)
  - 2D Parity
  - CRC-16
  - Hamming Code
  - Internet Checksum
- Sends packets in the format:
  DATA|METHOD|CONTROL_INFORMATION

## Server – Agent + Data Corruptor
- Receives packets from Client 1
- Injects random transmission errors such as:
  - Bit flip
  - Character substitution
  - Character deletion
  - Character insertion
  - Character swapping
  - Multiple bit flips
  - Burst errors
- Forwards corrupted packets to Client 2 without breaking the packet structure

## Client 2 – Receiver + Error Controller
- Receives packet from server
- Separates data, method, and control information
- Recomputes control information based on method
- Compares received and computed control values
- Reports whether the data is correct or corrupted

## Requirements
- Python 3.x
- Windows / Linux / macOS

## How to Run (Order Matters)

Open three terminals:

Terminal 1:
python server.py

Terminal 2:
python client1.py

Terminal 3:
python client2.py
