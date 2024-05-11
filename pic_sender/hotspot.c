#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COMMAND_LENGTH 256

int main() {
    FILE *fp;
    char command[MAX_COMMAND_LENGTH];
    char interface[50]; // Assuming interface name is at most 50 characters long

    // Check if the interface is up
    fp = popen("iw dev | grep Interface | awk '{print $2}'", "r");
    if (fp == NULL) {
        printf("Failed to run command\n");
        exit(1);
    }
    fgets(interface, sizeof(interface)-1, fp);
    pclose(fp);

    if (interface[0] == '\0') {
        printf("No wireless interface found\n");
        exit(1);
    }

    // Strip newline character from interface name
    strtok(interface, "\n");

    // Check if the hotspot interface already exists
    snprintf(command, sizeof(command), "iw dev %s interface list | grep wlan0", interface);
    fp = popen(command, "r");
    if (fp != NULL && fgets(command, sizeof(command), fp) != NULL) {
        printf("Hotspot interface already exists\n");
        pclose(fp);
        exit(1);
    }
    pclose(fp);

    // Set up the hotspot
    snprintf(command, sizeof(command), "iw dev %s interface add wlan0 type __ap", interface);
    system(command);

    snprintf(command, sizeof(command), "ip link set dev wlan0 up");
    system(command);

    snprintf(command, sizeof(command), "ip addr add 192.168.1.1/24 dev wlan0");
    system(command);

    snprintf(command, sizeof(command), "dnsmasq --interface=wlan0 --dhcp-range=192.168.1.2,192.168.1.10,255.255.255.0,12h");
    system(command);

    // Replace <internet-interface> with the actual name of your internet-facing interface
    snprintf(command, sizeof(command), "iptables -t nat -A POSTROUTING -o <internet-interface> -j MASQUERADE");
    system(command);

    printf("Hotspot created successfully\n");

    return 0;
}
