// get the mac address of the node
// reference: mpromonet's solution
// https://stackoverflow.com/questions/1779715/how-to-get-mac-address-of-your-machine-using-a-c-program#
# include "get_mac_address.h"

char* get_mac_address() {
    struct ifaddrs *ifaddr=NULL;
    struct ifaddrs *ifa = NULL;
    int i = 0, j = 0;
    char mac_arr[18];
    char* mac_address;

    if (getifaddrs(&ifaddr) == -1)
    {
         perror("getifaddrs");
    }
    else
    {
         for ( ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
         {
             if ( (ifa->ifa_addr) && (ifa->ifa_addr->sa_family == AF_PACKET) &&  (strcmp(ifa -> ifa_name, "eth0") == 0))
             {
                  struct sockaddr_ll *s = (struct sockaddr_ll*)ifa->ifa_addr;
                  for (i=0; i <s->sll_halen; i++)
                  {
                    sprintf(&mac_arr[j], "%02x%c", (s->sll_addr[i]), (i+1!=s->sll_halen)?':':'\n');
                    j += 3;
                  }
                
                mac_address = mac_arr;
             }
         }
         freeifaddrs(ifaddr);
    }
    return mac_address;
}
