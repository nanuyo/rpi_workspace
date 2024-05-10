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
    char command[100];

    // Set the interface to AP mode
    sprintf(command, "iw dev %s interface add wlan0_ap type __ap", WLAN_INTERFACE);
    system(command);

    // Bring up the interface
    sprintf(command, "ip link set dev wlan0_ap up");
    system(command);

    // Configure the SSID
    sprintf(command, "iw dev wlan0_ap ibss join %s %d 2412", AP_SSID, AP_CHANNEL);
    system(command);

    // Set passphrase
    sprintf(command, "iw dev wlan0_ap station set 00:11:22:33:44:55 tdls_peer 0 set_key 3 %s %s", AP_PASSPHRASE, AP_PASSPHRASE);
    system(command);

    printf("SoftAP configured successfully\n");

    return 0;
}
