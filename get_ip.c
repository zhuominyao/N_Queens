#include <stdio.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

int main()
{
	struct ifaddrs * ifAddrStruct = NULL;
	void * tmpAddrPtr = NULL;

	getifaddrs(&ifAddrStruct);

	while(ifAddrStruct != NULL)
	{
		if(ifAddrStruct->ifa_addr->sa_family == AF_INET)
		{
			tmpAddrPtr = &((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
			char addressBuffer[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
			printf("%s IP Address %s\n", ifAddrStruct->ifa_name, addressBuffer); 
		}
		ifAddrStruct=ifAddrStruct->ifa_next;
	}
}
