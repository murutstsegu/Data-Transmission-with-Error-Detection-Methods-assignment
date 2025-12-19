# client1.py
# Client 1 – Data Sender (FULL VERSION)

import socket
import binascii
import struct

HOST = "127.0.0.1"
PORT = 5000

# ------------------ Utility Functions ------------------

def text_to_binary(text):
    return ''.join(format(ord(c), '08b') for c in text)

# 1. Parity Bit (Even / Odd)
def parity_bit(binary, mode="even"):
    ones = binary.count("1")
    if mode == "even":
        parity = ones % 2
    else:  # odd
        parity = (ones + 1) % 2
    return binary + str(parity)

# 2. 2D Parity
def parity_2d(binary, row_size=8):
    rows = [binary[i:i+row_size] for i in range(0, len(binary), row_size)]
    matrix = []

    for row in rows:
        row_parity = row.count("1") % 2
        matrix.append(row + str(row_parity))

    col_parity = ""
    for col in range(len(matrix[0])):
        col_parity += str(sum(int(r[col]) for r in matrix) % 2)

    matrix.append(col_parity)
    return matrix

# 3. CRC-16 (CCITT)
def crc16(data):
    crc = 0xFFFF
    for b in data.encode():
        crc ^= b << 8
        for _ in range(8):
            crc = (crc << 1) ^ 0x1021 if crc & 0x8000 else crc << 1
            crc &= 0xFFFF
    return format(crc, "04X")

# 4. Hamming Code (7,4)
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

# 5. Internet Checksum
def internet_checksum(data):
    if len(data) % 2 != 0:
        data += '\0'

    checksum = 0
    for i in range(0, len(data), 2):
        word = (ord(data[i]) << 8) + ord(data[i+1])
        checksum += word
        checksum = (checksum & 0xFFFF) + (checksum >> 16)

    return format(~checksum & 0xFFFF, "04X")

# ------------------ Main Program ------------------

print("Client 1 – Data Sender")
print("1. Parity Bit (Even)")
print("2. Parity Bit (Odd)")
print("3. 2D Parity")
print("4. CRC-16")
print("5. Hamming Code")
print("6. Internet Checksum")

choice = input("Choose method: ")
text = input("Enter text data: ")

binary = text_to_binary(text)

method = ""
control = ""

if choice == "1":
    method = "PARITY_EVEN"
    control = parity_bit(binary, "even")

elif choice == "2":
    method = "PARITY_ODD"
    control = parity_bit(binary, "odd")

elif choice == "3":
    method = "2D_PARITY"
    control = '\n'.join(parity_2d(binary))

elif choice == "4":
    method = "CRC16"
    control = crc16(text)

elif choice == "5":
    method = "HAMMING"
    control = hamming_encode(binary)

elif choice == "6":
    method = "CHECKSUM"
    control = internet_checksum(text)

else:
    print("Invalid choice")
    exit()

packet = f"{text}|{method}|{control}"

print("\nPacket sent:")
print(packet)

# ------------------ Send to Server ------------------

client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect((HOST, PORT))
client.send(packet.encode())
client.close()
