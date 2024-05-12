#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/genetlink.h> // Include genetlink.h for struct genlmsghdr
#include <linux/nl80211.h>   // Include nl80211.h for NL80211_ATTR_IFNAME
#include <linux/wireless.h>

#define MAX_PAYLOAD 4096
#define NL80211_GENL_NAME "nl80211"
#define NL80211_CMD_NEW_INTERFACE 8
#define NLA_DATA(na) ((void *)((char*)(na) + NLA_HDRLEN))


static int nl_socket;

// Function to send a message to the nl80211 kernel module
void send_nl_msg(struct nlmsghdr *nlh) {
    struct sockaddr_nl dest_addr;

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0; // For Linux kernel, pid 0 means the message goes to the kernel
    dest_addr.nl_groups = 0; // Unicast message

    struct iovec iov = {nlh, nlh->nlmsg_len};
    struct msghdr msg = {&dest_addr, sizeof(dest_addr), &iov, 1, NULL, 0, 0};

    sendmsg(nl_socket, &msg, 0);
}

// Function to create a WiFi hotspot using hostapd
void create_wifi_hotspot(const char *interface_name) {
    char command[256];
    sprintf(command, "hostapd -B /etc/hostapd/hostapd.conf -i %s", interface_name);
    system(command);
}

int main() {
    struct nlmsghdr *nlh;
    struct genlmsghdr *gnlh;
    struct nlattr *na;
    int ret;

    nl_socket = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (nl_socket < 0) {
        perror("Error creating netlink socket");
        return -1;
    }

    nlh = malloc(NLMSG_SPACE(MAX_PAYLOAD));
    if (!nlh) {
        perror("Error allocating memory for netlink message");
        close(nl_socket);
        return -1;
    }
    
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;

    gnlh = (struct genlmsghdr *)NLMSG_DATA(nlh);
    gnlh->cmd = NL80211_CMD_NEW_INTERFACE;
    gnlh->version = 1;

    na = (struct nlattr *)(gnlh + 1);
    na->nla_type = NL80211_ATTR_IFNAME;
    na->nla_len = strlen("wlan0") + 1 + sizeof(struct nlattr);
    strcpy((char *)NLA_DATA(na), "wlan0");

    nlh->nlmsg_len = NLMSG_ALIGN(nlh->nlmsg_len) + NLMSG_ALIGN(na->nla_len);

    send_nl_msg(nlh);

    // Assuming 'wlan0' is the name of the WiFi interface you want to use for the hotspot
    create_wifi_hotspot("wlan0");

    free(nlh);
    close(nl_socket);
    return 0;
}
