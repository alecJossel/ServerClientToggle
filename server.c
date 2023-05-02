#ifdef _WIN32
#define _WIN32_WINNT _WIN32_WINNT_WIN7
#include <winsock2.h> //for all socket programming
#include <ws2tcpip.h> //for getaddrinfo, inet_pton, inet_ntop
#include <stdio.h> //for fprintf, perror
#include <unistd.h> //for close
#include <stdlib.h> //for exit
#include <string.h> //for memset
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
int OSInit(void) {}
int OSCleanup(void) {}
#endif

struct Node
{
	uint32_t data;
	struct Node* next;
};

int initialization();
void execution(int internet_socket);
void cleanup(int internet_socket);
struct Node* createIntList(struct Node* head, int count);
struct Node* findMaxNode(struct Node* head);
uint32_t getRandomUint32();// Function to generate a random 32-bit unsigned integer
struct Node* findMaxNode(struct Node* head);
uint8_t getRandomUint8(int floor, int ceiling);

int main(int argc, char* argv[])
{
	//////////////////
	//Initialization//
	//////////////////

	OSInit();
	srand(time(NULL));// Seed the random number generator
	int internet_socket = initialization();

	/////////////
	//Execution//
	/////////////

	execution(internet_socket);


	////////////
	//Clean up//
	////////////

	cleanup(internet_socket);

	OSCleanup();

	return 0;

}

int initialization()
{
	//Step 1.1
	struct addrinfo internet_address_setup;
	struct addrinfo* internet_address_result;
	memset(&internet_address_setup, 0, sizeof internet_address_setup);
	internet_address_setup.ai_family = AF_UNSPEC;
	internet_address_setup.ai_socktype = SOCK_DGRAM;
	internet_address_setup.ai_flags = AI_PASSIVE;
	int getaddrinfo_return = getaddrinfo(NULL, "24042", &internet_address_setup, &internet_address_result);
	if (getaddrinfo_return != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddrinfo_return));
		exit(1);
	}

	int internet_socket = -1;
	struct addrinfo* internet_address_result_iterator = internet_address_result;
	//struct in_addr client_ip_addr;
	//struct sockaddr_in client_addr;
	while (internet_address_result_iterator != NULL)
	{
		//Step 1.2
		internet_socket = socket(internet_address_result_iterator->ai_family, internet_address_result_iterator->ai_socktype, internet_address_result_iterator->ai_protocol);
		if (internet_socket == -1)
		{
			perror("socket");
		}
		else
		{
			//Step 1.3
			int bind_return = bind(internet_socket, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen);
			if (bind_return == -1)
			{
				close(internet_socket);
				perror("bind");
			}
			else
			{
				break;
			}
		}
		internet_address_result_iterator = internet_address_result_iterator->ai_next;
	}

	freeaddrinfo(internet_address_result);

	if (internet_socket == -1)
	{
		fprintf(stderr, "socket: no valid socket address found\n");
		exit(2);
	}

	return internet_socket;
}

void execution(int internet_socket)
{
	//Step 2.1
	//set up timeout receive of 3 seconds
	struct timeval tv = { 0, 100 };
	//timeout.tv_sec = 10;
	//timeout.tv_usec = 0;

	int number_of_bytes_received = 0;
	int number_of_bytes_send = 0;
	int maxVal;
	char buffer[1000];
	// Create an empty linked list
	struct Node* head = NULL;
	struct Node* current = NULL;

	struct sockaddr_storage client_internet_address;
	socklen_t client_internet_address_length = sizeof client_internet_address;
	while (strcmp((const char*)buffer, "GO") != 0) {
		number_of_bytes_received = recvfrom(internet_socket, buffer, (sizeof buffer) - 1, 0, (struct sockaddr*)&client_internet_address, &client_internet_address_length);
		//setsockopt(internet_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)(struct timeval*)&tv, sizeof(struct timeval));

		if (number_of_bytes_received == -1)
		{
			perror("recvfrom");
		}
		else {
			buffer[number_of_bytes_received] = '\0';
			printf("Received : %s\n", buffer);
		}
	}


	//STARTING ROUTINE
	while (strcmp((const char*)buffer, "OK") != 0) {
		char sendBuffer[4] = "0";
		int intCount = getRandomUint8(5, 45);
		head = createIntList(head, intCount);
		printf("Integer list generated, ready to transmetit\n");
		printf("Sending Linked List of %d Random 32-bit Unsigned Integers:\n", intCount);
		current = head;
		while (current != NULL) {
			UINT32 netInt = htonl( current->data);
			char byte1 = (netInt >> 24) & 0xFF;
			char byte2 = (netInt >> 16) & 0xFF;
			char byte3 = (netInt >> 8) & 0xFF;
			char byte4 = netInt & 0xFF;
			
			sendBuffer[0] = byte1;
			sendBuffer[1] = byte2;
			sendBuffer[2] = byte3;
			sendBuffer[3] = byte4;
			number_of_bytes_send = 0;
			number_of_bytes_send = sendto(internet_socket, sendBuffer, sizeof(sendBuffer), 0, (struct sockaddr*)&client_internet_address, client_internet_address_length);
			if (number_of_bytes_send == -1)
			{
				perror("sendto");
			}
			else {
				printf("Sent int:%d\nSent buffer:%d\n", current->data, netInt);
				current = current->next;
			}
		}

		struct Node* maxNode = findMaxNode(head);
		if (maxNode != NULL) {
			printf("Node with the highest value: %d\n", maxNode->data);
			maxVal = maxNode->data;
		}
		else {
			printf("Linked list is empty\n");
		}
		// Free the memory allocated for the linked list
		current = head;
		while (current != NULL) {
			struct Node* next = current->next;
			free(current);
			current = next;
		}

		number_of_bytes_received = 0;
		number_of_bytes_received = recvfrom(internet_socket, buffer, (sizeof buffer) - 1, 0, (struct sockaddr*)&client_internet_address, &client_internet_address_length);
		if (number_of_bytes_received == -1)
		{
			perror("recvfrom");
		}
		else {
			buffer[number_of_bytes_received] = '\0';
			printf("Received : %s\n", buffer);
			continue;
		}
	}
}

void cleanup(int internet_socket)
{
	//Step 3.1
	close(internet_socket);
}

uint32_t getRandomUint32() {
	return (uint32_t)rand();
}

struct Node* createIntList(struct Node* head, int count) {

	
	struct Node* current = NULL;


	// Generate and append 6 random integers to the linked list
	for (int i = 0; i < count; i++) {
		// Create a new node
		struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));

		// Generate a random integer and store it in the data field
		newNode->data = getRandomUint32();
		newNode->next = NULL;

		// If it's the first node, set it as the head
		if (head == NULL) {
			head = newNode;
			head->data = count;
			current = newNode;
		}
		else {
			// Append the new node to the end of the list
			current->next = newNode;
			current = newNode;
		}
	}
	return head;


}


struct Node* findMaxNode(struct Node* head) {
	if (head == NULL) {
		// Empty list
		return NULL;
	}

	struct Node* maxNode = head; // Assume first node as maximum
	int maxValue = head->data; // Assume first value as maximum

	struct Node* current = head->next; // Start from the second node

	// Iterate through the linked list
	while (current != NULL) {
		if (current->data > maxValue) {
			// Update maximum value and node if a higher value is found
			maxValue = current->data;
			maxNode = current;
		}
		current = current->next;
	}

	return maxNode;
}

uint8_t getRandomUint8(int floor, int ceiling) {

	int num = (rand() % (ceiling - floor + 1)) + floor;
	return num;
}
