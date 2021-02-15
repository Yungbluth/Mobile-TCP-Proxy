#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 

//handles text communication with the server
void comm(int curSocket) { 
    char buff[1024];
    uint32_t n;
    while (1) {
        bzero(buff, sizeof(buff));
        n = 4;
		//get user input until \n
        while ((buff[n++] = getchar()) != '\n')
			//check for EOF (ctrl+d)
            if (feof(stdin)) {
				close(curSocket);
				exit(1);
			};
		buff[--n] = 0;
		n-=4;
		//encode size of string into the first 4 bytes of buff
		char *tempSize = (char*)&n;
		buff[0] = tempSize[0];
		buff[1] = tempSize[1];
		buff[2] = tempSize[2];
		buff[3] = tempSize[3];
		//write the message
        write(curSocket, buff, n+4);
    } 
} 
  
int main(int argc, char *argv[]) {
	char *IP = argv[1];
	int port = atoi(argv[2]);
    int curSocket;
    struct sockaddr_in serverAddress;
  
    //create the socket
    curSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (curSocket == -1) {
        printf("socket fail\n");
        exit(0);
    }
    bzero(&serverAddress, sizeof(serverAddress));
  
	//add ip and port
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(IP);
    serverAddress.sin_port = htons(port);
  
    //connect the client and server
    if (connect(curSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) != 0) {
        printf("connection fail\n");
        exit(0);
    }
  
    //communicate with the server
    comm(curSocket);
  
    close(curSocket);
}