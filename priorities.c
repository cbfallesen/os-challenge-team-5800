#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/* OpenSSL headers */
#include <openssl/sha.h>

#define MAX 49
#define PORT 8080
#define SA struct sockaddr

typedef struct
{
	uint8_t hashvalue[32];
	uint64_t start;
	uint64_t end;
	uint8_t p;
} packet;

struct Node
{
	packet *data;
	struct Node *next;	
};

struct Node *startNode = NULL;

void* pushResult (struct Node **refNode, packet *newData, size_t dataSize) {
	printf("push");
	struct Node* newNode = (struct Node*) malloc(sizeof(struct Node));

	newNode->data = malloc(dataSize);
	newNode->next = (*refNode);

	newNode->data->start = newData->start;
	newNode->data->end = newData->end;
	newNode->data->p = newData->p;
	memcpy(newNode->data->hashvalue, newData->hashvalue, sizeof(newData->hashvalue));


	//(*refNode) = newNode;

	return newNode;
}

void print(struct Node *head) {
    struct Node *current_node = head;
   	while ( current_node != NULL) {
        printf("%li ", current_node->data->p);
        current_node = current_node->next;
    }
}

// Function designed for chat between client and server.
void func(int sockfd)
{
	char buff[MAX];
	int n;
	packet *Packet1;

	bzero(buff, MAX);

	// read the message from client and copy it in buffer
	read(sockfd, buff, sizeof(buff));
	Packet1 = (packet *)buff;

	// print buffer which contains the client contents
	printf("\n\n");
	int i;
	for (i = 0; i < 32; i++)
		printf("%02x", Packet1->hashvalue[i]);

	printf("\nFrom start: %li end: %li priority: %d\n", be64toh(Packet1->start), be64toh(Packet1->end), Packet1->p);
	pushResult(&startNode, &Packet1, sizeof(Packet1));


	// and send that buffer to client
	// }
}

// Driver function
int main()
{
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
	servaddr.sin_port = htons(PORT);

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