#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include<stdbool.h>

/* OpenSSL headers */
#include <openssl/sha.h>

#define MAX 49
#define SA struct sockaddr

typedef struct
{
	uint8_t hashvalue[32];
	uint64_t start;
	uint64_t end;
	uint8_t p;
} packet;

typedef struct
{
	uint64_t number;
	uint8_t resultHash[32];
} resultStruct;

struct Node
{
	resultStruct *data;
	struct Node *next;	
};

struct Node *startNode = NULL;

void* pushResult (struct Node **refNode, resultStruct *newData, size_t dataSize) {
	struct Node* newNode = (struct Node*) malloc(sizeof(struct Node));

	newNode->data = malloc(dataSize);
	newNode->next = (*refNode);

	newNode->data->number = newData->number;
	memcpy(newNode->data->resultHash, newData->resultHash, sizeof(newData->resultHash));

	return newNode;
}

bool compareHashes(unsigned char *guess, unsigned char *target) {
	for (int i = 0; i < 32; i++)
	{
		if (guess[i] != target[i])
		{
			return false;
		}
	}
	return true;
}

// Function designed for chat between client and server.
void func(int sockfd)
{
	char buff[MAX];
	int n;
	bzero(buff, MAX);
	packet *Packet1;

	// read the message from client and copy it in buffer
	read(sockfd, buff, sizeof(buff));

	Packet1 = (packet *)buff;

	// print buffer which contains the client contents
	int i;
	for (i = 0; i < 32; i++)
		printf("%02x", Packet1->hashvalue[i]);
	printf("\n");
	uint64_t x;
	uint64_t result;
	uint64_t start = be64toh(Packet1->start);
	uint64_t end = be64toh(Packet1->end);
	bool resultLock = false;
	result = -1;
	struct Node *node = startNode;
	resultStruct resultData;
	
	while(node != NULL) {
		if(compareHashes(node->data->resultHash, Packet1->hashvalue)){
			result = node->data->number;
			resultLock = true;
			break;
		}
		node = node->next;
	}

	if (!resultLock)
	{
		for (x = start; x < end; x++)
		{
			unsigned char *guess = SHA256((unsigned char *)&x, 8, 0);

			if (compareHashes(guess, Packet1->hashvalue))
			{
				resultData.number = x;
				//Arrays cannot be assigned in structs, so we use memcpy
				memcpy(resultData.resultHash, Packet1->hashvalue, sizeof(Packet1->hashvalue));
				startNode = pushResult(&startNode, &resultData, sizeof(resultStruct));
				result = x;
				break;
			}
		}
	}

	result = htobe64(result);
	write(sockfd, &result, sizeof(result));
}

// Driver function
int main(int argc, char *argv[])
{
	char *a = argv[1];
	int port = atoi(a);
	int sockfd, connfd, len;
	struct sockaddr_in servaddr, cli;

	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		printf("socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);

	// Binding newly created socket to given IP and verification
	if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0)
	{
		printf("socket bind failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully binded..\n");

	for (;;)
	{
		// Now server is ready to listen and verification
		if ((listen(sockfd, 5)) != 0)
		{
			printf("Listen failed...\n");
			exit(0);
		}
		else
			printf("Server listening..\n");
		len = sizeof(cli);

		// Accept the data packet from client and verification
		connfd = accept(sockfd, (SA *)&cli, &len);
		if (connfd < 0)
		{
			printf("server accept failed...\n");
			exit(0);
		}
		else
			printf("server accept the client...\n");

		// Function for chatting between client and server
		func(connfd);
	}

	// After chatting close the socket
	close(sockfd);
}
