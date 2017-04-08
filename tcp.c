/*
 * tcp.c
 *
 *  Created on: 2017年3月18日
 *      Author: Home
 */


#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#if 1

///////////////////////////////////////////
struct ifaddrs * ifAddrStruct=NULL;
void * tmpAddrPtr=NULL;
char addressBuffer[INET_ADDRSTRLEN];
void get_local_ip(void)
{
#if 1
	if (getifaddrs(&ifAddrStruct) == -1) {
	        perror("getifaddrs");
	        exit(EXIT_FAILURE);
	    }

    while (ifAddrStruct!=NULL)
	{
        if (ifAddrStruct->ifa_addr->sa_family==AF_INET)
		{   // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr = &((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            printf("%s IPV4 Address %s\n", ifAddrStruct->ifa_name, addressBuffer);
        }
		/* else if (ifAddrStruct->ifa_addr->sa_family==AF_INET6)
		{   // check it is IP6
            // is a valid IP6 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
            printf("%s IPV6 Address %s\n", ifAddrStruct->ifa_name, addressBuffer);
        }  */
        ifAddrStruct = ifAddrStruct->ifa_next;
    }
    return ;
#endif
}


#endif
