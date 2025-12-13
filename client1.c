#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

char colParity[9]; // 8 bits + null character

// Display available methods
void showMenu() {
    printf("Methods\n");
    printf("1. Parity Bit\n");
    printf("2. 2D Parity Calculation\n");
    printf("3. CRC 8-bit\n");
    printf("4. Hamming Encoding\n");
    printf("5. Internet Checksum\n");
    printf("-1. Exit\n");
}

// Calculate parity bit (even parity)
int calculateParity(char *text) {
    int len = strlen(text);
    int onesCount = 0;

    for (int i = 0; i < len; i++) {
        unsigned char c = text[i];
        printf("%c -> ", (c == ' ' ? '_' : c));

        for (int bit = 7; bit >= 0; bit--) {
            int b = (c >> bit) & 1;
            printf("%d", b);
            onesCount += b;
        }

        int parity = onesCount % 2;
        printf(" | Parity = %d\n", parity);
    }
    return onesCount % 2;
}

// Calculate 2D parity
void calculate2DParity(char *text, char *rowParity, char *colParity) {
    text[strcspn(text, "\n")] = '\0';
    int len = strlen(text);
    int colOnes[8] = {0};

    for (int i = 0; i < len; i++) {
        unsigned char c = text[i];
        int rowOnes = 0;
        printf("%c -> ", c);

        for (int bit = 7; bit >= 0; bit--) {
            int b = (c >> bit) & 1;
            printf("%d", b);
            rowOnes += b;
            colOnes[7 - bit] += b;
        }

        rowParity[i] = (rowOnes % 2) + '0';
        printf(" | Row Parity = %c\n", rowParity[i]);
    }

    rowParity[len] = '\0';

    for (int j = 0; j < 8; j++)
        colParity[j] = (colOnes[j] % 2) + '0';

    colParity[8] = '\0';
    printf("\nColumn Parities: %s\n", colParity);
}

// CRC-8 polynomial: x^8 + x^2 + x + 1 (0x07)
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

// Encode 8-bit character into 12-bit Hamming code
void hammingEncode(unsigned char data, unsigned char *hamming) {
    unsigned char d[8];
    for (int i = 0; i < 8; i++)
        d[i] = (data >> (7 - i)) & 1;

    unsigned char h[12];
    h[2] = d[0]; h[4] = d[1]; h[5] = d[2]; h[6] = d[3];
    h[8] = d[4]; h[9] = d[5]; h[10] = d[6]; h[11] = d[7];

    h[0] = h[2] ^ h[4] ^ h[6] ^ h[8] ^ h[10];
    h[1] = h[2] ^ h[5] ^ h[6] ^ h[9] ^ h[10];
    h[3] = h[4] ^ h[5] ^ h[6] ^ h[11];
    h[7] = h[8] ^ h[9] ^ h[10] ^ h[11];

    for (int i = 0; i < 12; i++)
        hamming[i] = h[i] + '0';

    hamming[12] = '\0';
}

// Internet checksum calculation
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
    printf("Client 1 is running...\n");

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Failed to create socket!\n");
        return 1;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        printf("Failed to connect to server!\n");
        return 1;
    }

    printf("Connected to server successfully!\n");

    while (1) {
        printf("\nChoose a method:\n");
        showMenu();

        int choice;
        scanf("%d", &choice);
        getchar();

        if (choice == -1) {
            printf("Exiting...\n");
            close(sock);
            break;
        }

        char text[100];
        printf("Enter text:\n");
        fgets(text, sizeof(text), stdin);
        text[strcspn(text, "\n")] = '\0';

        char packet[8192];

        switch (choice) {
            case 1:
                sprintf(packet, "%s|PARITY|%d", text, calculateParity(text));
                write(sock, packet, strlen(packet));
                break;

            case 2: {
                char rowParity[strlen(text) + 1];
                calculate2DParity(text, rowParity, colParity);
                sprintf(packet, "%s|2DPARITY|%s|%s", text, rowParity, colParity);
                write(sock, packet, strlen(packet));
                break;
            }

            case 3: {
                unsigned char crc = crc8((unsigned char*)text, strlen(text));
                sprintf(packet, "%s|CRC8|%02X", text, crc);
                write(sock, packet, strlen(packet));
                break;
            }

            case 4: {
                char encoded[4096] = "";
                char hamming[13];

                for (int i = 0; i < strlen(text); i++) {
                    hammingEncode(text[i], hamming);
                    strcat(encoded, hamming);
                }

                sprintf(packet, "%s|HAMMING|%s", text, encoded);
                write(sock, packet, strlen(packet));
                break;
            }

            case 5: {
                uint16_t checksum = internetChecksum((unsigned char*)text, strlen(text));
                sprintf(packet, "%s|IPCHECKSUM|%04X", text, checksum);
                write(sock, packet, strlen(packet));
                break;
            }

            default:
                printf("Invalid choice, please try again!\n");
        }
    }
    return 0;
}

