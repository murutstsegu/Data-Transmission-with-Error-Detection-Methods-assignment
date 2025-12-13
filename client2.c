#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

/* ===================== PARITY ===================== */
// Calculate even parity
int calculateParity(char *text) {
    int ones = 0;
    for (int i = 0; i < strlen(text); i++) {
        unsigned char c = text[i];
        for (int bit = 7; bit >= 0; bit--) {
            ones += ((c >> bit) & 1);
        }
    }
    return ones % 2; // even parity
}

/* ===================== 2D PARITY ===================== */
void calculate2DParity(char *text, char *rowParity, char *colParity) {
    int len = strlen(text);
    int colOnes[8] = {0};

    for (int i = 0; i < len; i++) {
        unsigned char c = text[i];
        int rowOnes = 0;

        for (int bit = 7; bit >= 0; bit--) {
            int b = (c >> bit) & 1;
            rowOnes += b;
            colOnes[7 - bit] += b;
        }
        rowParity[i] = (rowOnes % 2) + '0';
    }
    rowParity[len] = '\0';

    for (int j = 0; j < 8; j++)
        colParity[j] = (colOnes[j] % 2) + '0';

    colParity[8] = '\0';
}

/* ===================== CRC-8 ===================== */
unsigned char crc8(unsigned char *data, int len) {
    unsigned char crc = 0;

    for (int i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x07;
            else
                crc <<= 1;
        }
    }
    return crc;
}

/* ===================== INTERNET CHECKSUM ===================== */
uint16_t internetChecksum(unsigned char *data, int len) {
    uint32_t sum = 0;

    for (int i = 0; i < len; i += 2) {
        uint16_t word = data[i] << 8;
        if (i + 1 < len)
            word |= data[i + 1];

        sum += word;
        if (sum > 0xFFFF)
            sum = (sum & 0xFFFF) + 1;
    }
    return ~sum;
}

int main() {
    printf("Client 2 is running (Receiver)...\n");

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Failed to create socket!\n");
        return 1;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(8081);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        printf("Failed to connect to server!\n");
        return 1;
    }

    printf("Connected to server. Waiting for packets...\n");

    char buffer[8192];

    while (1) {
        int received = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (received <= 0) {
            printf("Server closed the connection!\n");
            break;
        }

        buffer[received] = '\0';
        printf("\n--- Packet Received ---\n%s\n", buffer);

        // Split packet
        char *data = strtok(buffer, "|");
        char *method = strtok(NULL, "|");
        char *control = strtok(NULL, "|");

        if (!data || !method || !control) {
            printf("INVALID PACKET FORMAT!\n");
            continue;
        }

        printf("DATA: %s\n", data);
        printf("METHOD: %s\n", method);
        printf("CONTROL: %s\n", control);

        /* ===================== VERIFICATION ===================== */

        if (strcmp(method, "PARITY") == 0) {
            int parity = calculateParity(data);
            printf("Calculated Parity = %d\n", parity);
            printf((parity == atoi(control)) ? "NO ERROR DETECTED\n" : "ERROR DETECTED\n");
        }

        else if (strcmp(method, "2DPARITY") == 0) {
            char row[256], col[9];
            calculate2DParity(data, row, col);

            printf("Calculated Row Parity: %s\n", row);
            printf("Calculated Column Parity: %s\n", col);

            char *recvRow = control;
            char *recvCol = strtok(NULL, "|");

            if (!recvCol) {
                printf("PACKET ERROR: Invalid 2DPARITY format!\n");
                continue;
            }

            int ok = (!strcmp(row, recvRow) && !strcmp(col, recvCol));
            printf(ok ? "NO ERROR DETECTED\n" : "ERROR DETECTED\n");
        }

        else if (strcmp(method, "CRC8") == 0) {
            unsigned char crc = crc8((unsigned char*)data, strlen(data));
            printf("Calculated CRC8 = %02X\n", crc);
            printf((crc == strtol(control, NULL, 16)) ? "NO ERROR DETECTED\n" : "ERROR DETECTED\n");
        }

        else if (strcmp(method, "IPCHECKSUM") == 0) {
            uint16_t checksum = internetChecksum((unsigned char*)data, strlen(data));
            printf("Calculated IP Checksum = %04X\n", checksum);
            printf((checksum == strtol(control, NULL, 16)) ? "NO ERROR DETECTED\n" : "ERROR DETECTED\n");
        }

        else if (strcmp(method, "HAMMING") == 0) {
            printf("Starting Hamming decode...\n");

            if (strlen(data) != 7) {
                printf("ERROR: Hamming(7,4) requires 7-bit input!\n");
            } else {
                int b1 = data[0] - '0';
                int b2 = data[1] - '0';
                int b3 = data[2] - '0';
                int b4 = data[3] - '0';
                int b5 = data[4] - '0';
                int b6 = data[5] - '0';
                int b7 = data[6] - '0';

                int p1 = b1 ^ b3 ^ b5 ^ b7;
                int p2 = b2 ^ b3 ^ b6 ^ b7;
                int p3 = b4 ^ b5 ^ b6 ^ b7;

                int errorBit = (p3 << 2) | (p2 << 1) | p1;

                if (errorBit == 0) {
                    printf("NO ERROR DETECTED. Data is correct.\n");
                } else {
                    printf("ERROR DETECTED! Faulty bit position: %d\n", errorBit);

                    int index = errorBit - 1;
                    char corrected[8];
                    strcpy(corrected, data);
                    corrected[index] = (corrected[index] == '0') ? '1' : '0';

                    printf("Corrected Data: %s\n", corrected);
                }
            }
        }

        else {
            printf("UNKNOWN METHOD!\n");
        }
    }

    close(sock);
    return 0;
}

