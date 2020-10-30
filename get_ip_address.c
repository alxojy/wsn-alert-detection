// this code is used to get the id address of the process
// reference: Vishnu Monn - code on slack #general channel
#include "get_ip_address.h"

char* get_ip_address() {
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];
    char* ip_address;

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        exit(1);
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;

        s=getnameinfo(ifa->ifa_addr,sizeof(struct sockaddr_in),host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
        
        if (strcmp(ifa -> ifa_name, "eth0") == 0) {
            ip_address = host;
        }

    }
    freeifaddrs(ifaddr);
    return ip_address;
}
