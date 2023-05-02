#ifdef _WIN32
#define _WIN32_WINNT _WIN32_WINNT_WIN7
#include <winsock2.h> //for all socket programming
#include <ws2tcpip.h> //for getaddrinfo, inet_pton, inet_ntop
#include <stdio.h> //for fprintf, perror
#include <unistd.h> //for close
#include <stdlib.h> //for exit
#include <string.h> //for memset
#include <stdbool.h>//for booling around
#include <stdint.h>//for int magix
#include <time.h>//for rand init
void OSInit(void)
{
	WSADATA wsaData;
	int WSAError = WSAStartup(MAKEWORD(2, 0), &wsaData);
	if (WSAError != 0)
	{
		fprintf(stderr, "WSAStartup errno = %d\n", WSAError);
		exit(-1);
	}
}
void OSCleanup(void)
{
	WSACleanup();
}
#define perror(string) fprintf( stderr, string ": WSA errno = %d\n", WSAGetLastError() )
#else
#include <sys/socket.h> //for sockaddr, socket, socket
#include <sys/types.h> //for size_t
#include <netdb.h> //for getaddrinfo
#include <netinet/in.h> //for sockaddr_in
#include <arpa/inet.h> //for htons, htonl, inet_pton, inet_ntop
#include <errno.h> //for errno
#include <stdio.h> //for fprintf, perror
#include <unistd.h> //for close
#include <stdlib.h> //for exit
#include <string.h> //for memset
#endif

struct Node
{
	uint32_t data;
	struct Node* next;
};



void cleanup(int internet_socket);

int main(int argc, char* argv[])
{
	//////////////////
	//Initialization//
	//////////////////

	OSInit();
	printf("passed init\n");
	//Step 1.1
	struct addrinfo internet_address_setup;
	struct addrinfo* internet_address = NULL;
	memset(&internet_address_setup, 0, sizeof internet_address_setup);
	internet_address_setup.ai_family = AF_INET;
	internet_address_setup.ai_socktype = SOCK_DGRAM;
	getaddrinfo("::1", "24042", &internet_address_setup, &internet_address);


	int internet_socket;
	internet_socket = socket(internet_address->ai_family, internet_address->ai_socktype, internet_address->ai_protocol);



	//Step 2.1
	sendto(internet_socket, "GO", 32, 0, internet_address->ai_addr, internet_address->ai_addrlen);
	printf("Go SSent!\n");
	struct Node* head = NULL;
	struct Node* current = NULL;
	char buffer[4];
	int counter = 1;
	int number_of_bytes_received = 0;
	socklen_t internet_address_length = internet_address->ai_addrlen;

	while (1) {
		struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));

		number_of_bytes_received = 0;
		UINT32 intBuffer = 0, hostInt = 0;



		number_of_bytes_received = recvfrom(internet_socket, buffer, (sizeof buffer) - 1, 0, internet_address->ai_addr, &internet_address_length);
		if (number_of_bytes_received == -1)
		{
			//perror("recvfrom");
			break;
		}


		intBuffer = *((UINT32*)(buffer));
		hostInt = ntohl(intBuffer);
		newNode->data = intBuffer;
		newNode->next = NULL;
		if (head == NULL) {
			head = newNode;
			current = newNode;
		}
		else {
			current->next = newNode;
			current = newNode;
		}
		printf("%d] Received Network int : %10d //Converted Host int: %d  //Received Char:%d\n", counter, *buffer, hostInt, intBuffer);
		//free(buffer);
		counter++;
	}
	printf("end receive list\n");

	/////////////
	//Execution//
	/////////////



}



void cleanup(int internet_socket) {};