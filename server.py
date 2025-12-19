# server.py
# Server – Agent + Data Corruptor

import socket
import random
import string

HOST = "127.0.0.1"
PORT1 = 5000   # Client 1
PORT2 = 6000   # Client 2

# ---------------- Error Injection Methods ----------------

def bit_flip(data):
    binary = ''.join(format(ord(c), '08b') for c in data)
    i = random.randint(0, len(binary) - 1)
    flipped = '1' if binary[i] == '0' else '0'
    binary = binary[:i] + flipped + binary[i+1:]

    chars = [chr(int(binary[i:i+8], 2)) for i in range(0, len(binary), 8)]
    return ''.join(chars)

def char_substitution(data):
    i = random.randint(0, len(data) - 1)
    new_char = random.choice(string.ascii_uppercase)
    return data[:i] + new_char + data[i+1:]

def char_deletion(data):
    i = random.randint(0, len(data) - 1)
    return data[:i] + data[i+1:]

def char_insertion(data):
    i = random.randint(0, len(data))
    new_char = random.choice(string.ascii_letters)
    return data[:i] + new_char + data[i:]

def char_swap(data):
    if len(data) < 2:
        return data
    i = random.randint(0, len(data) - 2)
    lst = list(data)
    lst[i], lst[i+1] = lst[i+1], lst[i]
    return ''.join(lst)

def multiple_bit_flips(data, flips=3):
    for _ in range(flips):
        data = bit_flip(data)
    return data

def burst_error(data):
    if len(data) < 3:
        return data
    start = random.randint(0, len(data) - 3)
    end = min(start + random.randint(3, 8), len(data))
    corrupted = ''.join(random.choice(string.ascii_letters) for _ in range(end - start))
    return data[:start] + corrupted + data[end:]

ERROR_METHODS = [
    bit_flip,
    char_substitution,
    char_deletion,
    char_insertion,
    char_swap,
    multiple_bit_flips,
    burst_error
]

# ---------------- Socket Setup ----------------

server1 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

server1.bind((HOST, PORT1))
server2.bind((HOST, PORT2))

server1.listen(1)
server2.listen(1)

print("Server listening for Client 1 and Client 2...")

conn1, _ = server1.accept()
print("Client 1 connected")

conn2, _ = server2.accept()
print("Client 2 connected")

# ---------------- Receive Packet ----------------

packet = conn1.recv(8192).decode()
print("\nReceived packet from Client 1:")
print(packet)

# Packet format: DATA|METHOD|CONTROL
try:
    data, method, control = packet.split("|", 2)
except ValueError:
    print("Invalid packet format")
    conn1.close()
    conn2.close()
    exit()

# ---------------- Error Injection ----------------

error_func = random.choice(ERROR_METHODS)
corrupted_data = error_func(data)

print("\nOriginal DATA:", data)
print("Corrupted DATA:", corrupted_data)
print("Error Method Used:", error_func.__name__)

# ---------------- Forward Packet ----------------

new_packet = f"{corrupted_data}|{method}|{control}"

conn2.send(new_packet.encode())
print("\nForwarded corrupted packet to Client 2:")
print(new_packet)

# ---------------- Cleanup ----------------

conn1.close()
conn2.close()
server1.close()
server2.close()
