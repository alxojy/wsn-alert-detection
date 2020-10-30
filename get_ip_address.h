#ifndef IPADDRESS_H
#define IPADDRESS_H

#include "main.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

char* get_ip_address();

#endif // IPADDRESS_H
