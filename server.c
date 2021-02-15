#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
  
//handles text communication with the client
void comm(int curSocket) {
	//only reading 1020 because it doesn't include the header of 4 bytes, bringing the total to 1024 max bytes read
    char buff[1020];
    int n;
	uint32_t size;
    while (1) {
        bzero(buff, 1020);
        //if client disconnects, exit
		if (read(curSocket, (char*)&size, 4) <= 0) {
			close(curSocket);
			exit(1);
		}
		//read and print out size and message info
		printf("Size: %d\n", size);
        read(curSocket, buff, size);
        printf("Message: %s\n", buff);
    }
}
  
int main(int argc, char *argv[]) {
	int port = atoi(argv[1]);
    int curSocket, curAccept, len;
    struct sockaddr_in serverAddress, client;
  
    //create the socket
    curSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (curSocket == -1) {
        printf("socket 1 fail\n");
        exit(0);
    }
    bzero(&serverAddress, sizeof(serverAddress));
  
    //add ip and port
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(port);
  
    //bind the socket
    if ((bind(curSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress))) != 0) {
        printf("socket 2 fail\n");
        exit(0);
    }
  
    //listen
    if ((listen(curSocket, 5)) != 0) {
        printf("listen fail\n");
        exit(0);
    }
    len = sizeof(client);
  
    //allow data packets
    curAccept = accept(curSocket, (struct sockaddr*)&client, &len);
    if (curAccept < 0) {
        printf("acccept fail\n");
        exit(0);
    }
  
    //communicate with the client
    comm(curAccept);
  
    close(curSocket);
} 