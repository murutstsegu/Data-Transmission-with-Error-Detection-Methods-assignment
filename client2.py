# client2.py
# Client 2 – Receiver + Error Controller

import socket

HOST = "127.0.0.1"
PORT = 6000

# ---------------- Utility Functions ----------------

def text_to_binary(text):
    return ''.join(format(ord(c), '08b') for c in text)

# Parity (Even / Odd)
def parity_bit(binary, mode="even"):
    ones = binary.count("1")
    if mode == "even":
        parity = ones % 2
    else:
        parity = (ones + 1) % 2
    return binary + str(parity)

# 2D Parity
def parity_2d(binary, row_size=8):
    rows = [binary[i:i+row_size] for i in range(0, len(binary), row_size)]
    matrix = []

    for row in rows:
        matrix.append(row + str(row.count("1") % 2))

    col_parity = ""
    for col in range(len(matrix[0])):
        col_parity += str(sum(int(r[col]) for r in matrix) % 2)

    matrix.append(col_parity)
    return '\n'.join(matrix)

# CRC-16 (CCITT)
def crc16(data):
    crc = 0xFFFF
    for b in data.encode():
        crc ^= b << 8
        for _ in range(8):
            crc = (crc << 1) ^ 0x1021 if crc & 0x8000 else crc << 1
            crc &= 0xFFFF
    return format(crc, "04X")

# Hamming (7,4)
def hamming_encode(binary):
    encoded = ""
    for i in range(0, len(binary), 4):
        d = binary[i:i+4].ljust(4, '0')
        d1, d2, d3, d4 = map(int, d)

        p1 = d1 ^ d2 ^ d4
        p2 = d1 ^ d3 ^ d4
        p3 = d2 ^ d3 ^ d4

        encoded += f"{p1}{p2}{d1}{p3}{d2}{d3}{d4}"
    return encoded

# Internet Checksum
def internet_checksum(data):
    if len(data) % 2 != 0:
        data += '\0'

    checksum = 0
    for i in range(0, len(data), 2):
        word = (ord(data[i]) << 8) + ord(data[i+1])
        checksum += word
        checksum = (checksum & 0xFFFF) + (checksum >> 16)

    return format(~checksum & 0xFFFF, "04X")

# ---------------- Receive Packet ----------------

client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect((HOST, PORT))

packet = client.recv(8192).decode()
client.close()

print("\nReceived packet:")
print(packet)

# ---------------- Partition Packet ----------------

try:
    data, method, incoming_control = packet.split("|", 2)
except ValueError:
    print("Invalid packet format")
    exit()

binary = text_to_binary(data)
computed_control = ""

# ---------------- Re-compute Control ----------------

if method == "PARITY_EVEN":
    computed_control = parity_bit(binary, "even")

elif method == "PARITY_ODD":
    computed_control = parity_bit(binary, "odd")

elif method == "2D_PARITY":
    computed_control = parity_2d(binary)

elif method == "CRC16":
    computed_control = crc16(data)

elif method == "HAMMING":
    computed_control = hamming_encode(binary)

elif method == "CHECKSUM":
    computed_control = internet_checksum(data)

else:
    print("Unknown method")
    exit()

# ---------------- Compare & Output ----------------

status = "DATA CORRECT" if computed_control == incoming_control else "DATA CORRUPTED"

print("\n---------------- RESULT ----------------")
print(f"Received Data        : {data}")
print(f"Method               : {method}")
print(f"Sent Check Bits      : {incoming_control}")
print(f"Computed Check Bits  : {computed_control}")
print(f"Status               : {status}")
print("---------------------------------------")
