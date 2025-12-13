#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>

#define PORT_CLIENT1 8080
#define PORT_CLIENT2 8081
#define BACKLOG 5              // Maximum pending connections
#define BUFFER_SIZE 8192

/* ===================== RANDOM UTIL ===================== */
int randomInRange(int min, int max) {
    return min + rand() % (max - min + 1);
}

/* ===================== CORRUPTION METHODS ===================== */

// Flip a single random bit
void bitFlipOnce(unsigned char *data, int len) {
    if (len <= 0) return;
    int byteIndex = randomInRange(0, len - 1);
    int bit = randomInRange(0, 7);
    data[byteIndex] ^= (1 << bit);
}

// Flip multiple random bits
void multipleBitFlips(unsigned char *data, int len) {
    if (len <= 0) return;
    int flips = randomInRange(2, (len * 8 > 16 ? 8 : len * 8));
    for (int i = 0; i < flips; i++) {
        int byteIndex = randomInRange(0, len - 1);
        int bit = randomInRange(0, 7);
        data[byteIndex] ^= (1 << bit);
    }
}

// Replace a character with another random printable character
void characterSubstitution(unsigned char *data, int len) {
    if (len <= 0) return;
    int index = randomInRange(0, len - 1);
    unsigned char original = data[index];
    unsigned char replacement;

    do {
        replacement = (unsigned char)randomInRange(32, 126);
    } while (replacement == original);

    data[index] = replacement;
}

// Delete a character
void characterDeletion(unsigned char *data, int *len) {
    if (*len <= 1) return;
    int index = randomInRange(0, (*len) - 1);
    memmove(&data[index], &data[index + 1], (*len) - index - 1);
    (*len)--;
}

// Insert a random character
void characterInsertion(unsigned char *data, int *len) {
    if (*len + 1 >= BUFFER_SIZE - 10) return;
    int index = randomInRange(0, *len);
    unsigned char c = (unsigned char)randomInRange(32, 126);
    memmove(&data[index + 1], &data[index], (*len) - index);
    data[index] = c;
    (*len)++;
}

// Swap two adjacent characters
void characterSwapping(unsigned char *data, int len) {
    if (len <= 1) return;
    int index = randomInRange(0, len - 2);
    unsigned char temp = data[index];
    data[index] = data[index + 1];
    data[index + 1] = temp;
}

// Burst error (multiple bit flips in sequence)
void burstError(unsigned char *data, int *len) {
    if (*len <= 0) return;
    int burstLength = randomInRange(3, (*len < 8 ? *len : 8));
    int start = randomInRange(0, (*len) - burstLength);

    for (int i = 0; i < burstLength; i++) {
        int bit = randomInRange(0, 7);
        data[start + i] ^= (1 << bit);
    }
}

/* ===================== APPLY RANDOM CORRUPTION ===================== */
void applyRandomCorruption(unsigned char *data, int *len) {
    int method = randomInRange(1, 7);

    switch (method) {
        case 1: bitFlipOnce(data, *len);        printf("[Corruption] Single Bit Flip\n"); break;
        case 2: characterSubstitution(data, *len); printf("[Corruption] Character Substitution\n"); break;
        case 3: characterDeletion(data, len);  printf("[Corruption] Character Deletion\n"); break;
        case 4: characterInsertion(data, len); printf("[Corruption] Character Insertion\n"); break;
        case 5: characterSwapping(data, *len); printf("[Corruption] Character Swapping\n"); break;
        case 6: multipleBitFlips(data, *len);  printf("[Corruption] Multiple Bit Flips\n"); break;
        case 7: burstError(data, len);          printf("[Corruption] Burst Error\n"); break;
        default: bitFlipOnce(data, *len); break;
    }
}

/* ===================== PACKET PARSING ===================== */
// Find first and second '|' positions
int findPacketSeparators(const char *buf, int len, int *p1, int *p2) {
    int first = -1, second = -1;

    for (int i = 0; i < len; i++) {
        if (buf[i] == '|') {
            if (first == -1) first = i;
            else {
                second = i;
                break;
            }
        }
    }

    if (first == -1 || second == -1) return 0;
    *p1 = first;
    *p2 = second;
    return 1;
}

int main() {
    srand(time(NULL));

    int listenClient1 = -1, listenClient2 = -1;
    int client1Sock = -1, client2Sock = -1;

    /* ===================== SETUP CLIENT 1 LISTENER ===================== */
    listenClient1 = socket(AF_INET, SOCK_STREAM, 0);
    if (listenClient1 < 0) { perror("socket client1"); exit(1); }

    int opt = 1;
    setsockopt(listenClient1, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addrClient1 = {0};
    addrClient1.sin_family = AF_INET;
    addrClient1.sin_addr.s_addr = INADDR_ANY;
    addrClient1.sin_port = htons(PORT_CLIENT1);

    if (bind(listenClient1, (struct sockaddr*)&addrClient1, sizeof(addrClient1)) < 0) {
        perror("bind client1"); exit(1);
    }
    listen(listenClient1, BACKLOG);
    printf("Listening for Client1 (Sender) on port %d...\n", PORT_CLIENT1);

    /* ===================== SETUP CLIENT 2 LISTENER ===================== */
    listenClient2 = socket(AF_INET, SOCK_STREAM, 0);
    if (listenClient2 < 0) { perror("socket client2"); exit(1); }

    setsockopt(listenClient2, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addrClient2 = {0};
    addrClient2.sin_family = AF_INET;
    addrClient2.sin_addr.s_addr = INADDR_ANY;
    addrClient2.sin_port = htons(PORT_CLIENT2);

    if (bind(listenClient2, (struct sockaddr*)&addrClient2, sizeof(addrClient2)) < 0) {
        perror("bind client2"); exit(1);
    }
    listen(listenClient2, BACKLOG);
    printf("Listening for Client2 (Receiver) on port %d...\n", PORT_CLIENT2);

    printf("Waiting for both clients to connect...\n");

    /* ===================== ACCEPT BOTH CLIENTS ===================== */
    fd_set master, readfds;
    FD_ZERO(&master);
    FD_SET(listenClient1, &master);
    FD_SET(listenClient2, &master);
    int fdmax = (listenClient1 > listenClient2 ? listenClient1 : listenClient2);

    while (client1Sock < 0 || client2Sock < 0) {
        readfds = master;
        select(fdmax + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(listenClient1, &readfds) && client1Sock < 0)
            client1Sock = accept(listenClient1, NULL, NULL);

        if (FD_ISSET(listenClient2, &readfds) && client2Sock < 0)
            client2Sock = accept(listenClient2, NULL, NULL);
    }

    printf("Both clients connected. Forwarding loop started.\n");

    /* ===================== MAIN LOOP ===================== */
    while (1) {
        char buffer[BUFFER_SIZE];
        ssize_t received = recv(client1Sock, buffer, sizeof(buffer) - 1, 0);
        if (received <= 0) break;

        buffer[received] = '\0';
        printf("\nPacket received from Client1: %s\n", buffer);

        int p1, p2;
        if (!findPacketSeparators(buffer, received, &p1, &p2)) {
            send(client2Sock, buffer, received, 0);
            continue;
        }

        int dataLen = p1;
        unsigned char data[BUFFER_SIZE];
        memcpy(data, buffer, dataLen);
        data[dataLen] = '\0';

        char suffix[BUFFER_SIZE];
        memcpy(suffix, &buffer[p1], received - p1);
        suffix[received - p1] = '\0';

        int newLen = dataLen;
        unsigned char corrupted[BUFFER_SIZE];
        memcpy(corrupted, data, dataLen);
        corrupted[dataLen] = '\0';

        applyRandomCorruption(corrupted, &newLen);

        char output[BUFFER_SIZE];
        memcpy(output, corrupted, newLen);
        memcpy(output + newLen, suffix, strlen(suffix));
        int outLen = newLen + strlen(suffix);
        output[outLen] = '\0';

        printf("Sending corrupted packet to Client2: %s\n", output);
        send(client2Sock, output, outLen, 0);
    }

    close(client1Sock);
    close(client2Sock);
    close(listenClient1);
    close(listenClient2);
    return 0;
}

