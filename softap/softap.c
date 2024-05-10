#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>

#define WLAN_INTERFACE "wlan0"
#define AP_SSID "MySoftAP"
#define AP_PASSPHRASE "password"
#define AP_CHANNEL 6

int main() {
    int sockfd;
    struct ifreq ifr;
    struct sockaddr_in addr;
    
    // Open a socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    // Set up the interface name
    strncpy(ifr.ifr_name, WLAN_INTERFACE, IFNAMSIZ);

    // Enable the wireless interface
    ifr.ifr_flags |= IFF_UP;
    if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0) {
        perror("ioctl");
        close(sockfd);
        exit(1);
    }

    // Set up the SoftAP configuration
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(0);

    // Set the AP mode
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, WLAN_INTERFACE, IFNAMSIZ);
    ifr.ifr_addr.sa_data[0] = 0x02; // IEEE80211_IF_TYPE_AP
    if (ioctl(sockfd, SIOCSIFMODE, &ifr) < 0) {
        perror("ioctl");
        close(sockfd);
        exit(1);
    }

    // Set the SSID of the AP
    strncpy(ifr.ifr_name, WLAN_INTERFACE, IFNAMSIZ);
    strncpy(ifr.ifr_newname, AP_SSID, IFNAMSIZ);
    if (ioctl(sockfd, SIOCSIWESSID, &ifr) < 0) {
        perror("ioctl");
        close(sockfd);
        exit(1);
    }

    // Set the channel of the AP
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, WLAN_INTERFACE, IFNAMSIZ);
    ifr.ifr_data = (char *)&AP_CHANNEL;
    if (ioctl(sockfd, SIOCSIWFREQ, &ifr) < 0) {
        perror("ioctl");
        close(sockfd);
        exit(1);
    }

    // Set the passphrase of the AP
    ifr.ifr_data = (char *)AP_PASSPHRASE;
    ifr.ifr_data_length = strlen(AP_PASSPHRASE);
    if (ioctl(sockfd, SIOCSIWENCODE, &ifr) < 0) {
        perror("ioctl");
        close(sockfd);
        exit(1);
    }

    printf("SoftAP configured successfully\n");

    // Close the socket
    close(sockfd);

    return 0;
}
