#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <time.h>

struct ntp_packet
{
    unsigned char leap_version_mode;
    unsigned char stratum;
    unsigned char poll;
    unsigned char precision;
    unsigned int root_delay;
    unsigned int root_dispersion;
    char refid[4];
    unsigned long long ref_timestamp;
    unsigned long long original_timestamp;
    unsigned long long receive_timestamp;
    unsigned long long transmit_timestamp;
};

#define ntp_format ">BBBBII4sIIIIIIII"
#define leap 0
#define version 4
#define mode 3

size_t convertBigToLittleEndian(size_t bigEndianValue) {
    return ((bigEndianValue & 0xFF000000) >> 24) |
           ((bigEndianValue & 0x00FF0000) >> 8)  |
           ((bigEndianValue & 0x0000FF00) << 8)  |
           ((bigEndianValue & 0x000000FF) << 24);
}

int main(int argc, char **argv) {

    WSADATA wsa_data;
    if (WSAStartup(0x0202, &wsa_data)) {
        printf("Unable to initialize Winsock\n");
        return -1;
    }

    time_t now = time(NULL);

    struct ntp_packet packet = {
        leap << 6 | version << 3 | mode,
        0,
        0,
        0,
        0,
        0,
        { 0, 0, 0, 0},
        0,
        0,
        now,
        0
    };

    const char *ntp_server_ip = "216.239.35.12";
    int ntp_server_port = 123;

    SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == INVALID_SOCKET) {
        fprintf(stderr, "Failed to create socket\n");
        WSACleanup();
        return 1;
    }

    struct sockaddr_in ntp_server;
    ntp_server.sin_family = AF_INET;
    ntp_server.sin_port = htons(ntp_server_port);

    if (inet_pton(AF_INET, ntp_server_ip, &(ntp_server.sin_addr)) != 1) {
        fprintf(stderr, "Invalid IP address\n");
        closesocket(s);
        WSACleanup();
        return 1;
    }
    sendto(s, (const char *)&packet, sizeof(packet), 0, (struct sockaddr *)&ntp_server, sizeof(ntp_server));

    char response[256];
    struct sockaddr_in sender;
    int sender_len = sizeof(sender);
    int bytes_received = recvfrom(s, response, sizeof(response), 0, (struct sockaddr *)&sender, &sender_len);
    if (bytes_received == SOCKET_ERROR) {
        fprintf(stderr, "Failed to receive NTP response\n");
        closesocket(s);
        WSACleanup();
        return 1;
    }

    char sender_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(sender.sin_addr), sender_ip, sizeof(sender_ip));
    printf("Sender Address: %s\n", sender_ip);

    printf("Response: ");
    for (int i = 0; i < bytes_received; i++) {
        printf("%02X ", response[i]);
    }
    printf("\n");

    printf("Response Length: %d bytes\n", bytes_received);

    struct ntp_packet *ntp_fields = (struct ntp_packet *)response;

    printf("%u %u %u\n", ntp_fields->leap_version_mode >> 6, (ntp_fields->leap_version_mode >> 3) & 0b111, ntp_fields->leap_version_mode & 0b111);
    printf("Stratum: %d\n",ntp_fields->stratum);
    printf("Poll: %d\n",ntp_fields->poll);
    printf("Precision: %d\n",ntp_fields->precision);
    printf("Root delay: %d\n",ntp_fields->root_delay);
    printf("Root dispersion: %zu\n", convertBigToLittleEndian(ntp_fields->root_dispersion));
    printf("Refid: %.4s\n", ntp_fields->refid);
    printf("Ref TimeStamp: %llu\n", convertBigToLittleEndian(ntp_fields->ref_timestamp));
    printf("Ref Orignal TimeStamp: %llu\n", convertBigToLittleEndian(ntp_fields->original_timestamp));
    printf("Receive TimeStamp: %llu\n", convertBigToLittleEndian(ntp_fields->receive_timestamp));
    printf("Transmit TimeStamp: %llu\n", convertBigToLittleEndian(ntp_fields->transmit_timestamp));

    closesocket(s);
    WSACleanup();
    return 0;
}
