//cproxy.c
//CSC 425
//By: Matthew Yungbluth and Cameron Richard Kazmierski
#include <time.h>
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/types.h>


typedef struct LLMessages {
	char* msgCnt;
	struct LLMessages* next;
}LLMessages;

void beef(char const *ip, int sPort, int tPort);

int main(int argc, char const *argv[])
{
	//server ip
    char const *ip = argv[2];
	//server port
    int sPort = atoi(argv[3]);
	//telnet port
    int tPort = atoi(argv[1]);
    while (1) {
		//beef forever
		beef(ip, sPort, tPort);
		sleep(1);
    }
}

int cAccept(int sDiscript, struct sockaddr_in *addr) {
    int sock = 0;
    int addLen = sizeof(*addr);
    if ((sock = accept(sDiscript, (struct sockaddr *) addr, (socklen_t *) & addLen)) < 0) {
		exit(EXIT_FAILURE);
    }
    return sock;
}

int sConnect(struct sockaddr_in *sAdd, int sock) {
    if (connect(sock, (struct sockaddr *) sAdd, sizeof(*sAdd)) < 0) {
		return 0;
    }
    return 1;
}

void BnL(int sDiscript, struct sockaddr_in *addr, int bLog) {
    if (bind(sDiscript, (struct sockaddr *) addr, sizeof(*addr)) < 0) {
		exit(EXIT_FAILURE);
    }
    if (listen(sDiscript, bLog) < 0) {
		exit(EXIT_FAILURE);
    }
}

int sockInit(struct sockaddr_in *sAdd, char const *ip, int port)
{
    int sock = 0;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		
    }
    if (ip != NULL) {
		memset(sAdd, '0', sizeof(*sAdd));
		sAdd->sin_family = AF_INET;
		sAdd->sin_port = htons(port);
		sAdd->sin_addr.s_addr = inet_addr(ip);
    }
    else {
		sAdd->sin_family = AF_INET;
		sAdd->sin_addr.s_addr = INADDR_ANY;
		sAdd->sin_port = htons(port);
    }
    return sock;
}

int intFromChar(char *str, int i)
{
    int x = str[i] << 24 | (str[i + 1] & 0xFF) << 16 | (str[i + 2] & 0xFF) << 8 | (str[i + 3] & 0xFF);
    return x;
}

void intToChar(int y, char *mess, int i)
{
    long x = (long) y;
    mess[i] = (x >> 24) & 0xFF;
    mess[i + 1] = (x >> 16) & 0xFF;
    mess[i + 2] = (x >> 8) & 0xFF;
    mess[i + 3] = x & 0xFF;
}

int createMessage(char *mess, int t, int AID, int SID, int pLen, int sequenceID, char *pay)
{
    intToChar(t, mess, 0);
    intToChar(AID, mess, 4);
    intToChar(SID, mess, 8);
    intToChar(pLen, mess, 12);
    intToChar(sequenceID, mess, 16);
    memcpy(&mess[20], pay, pLen);
    return 20 + pLen;
}

int getInts(char *mess, int *t, int *AID, int *SID, int *sequenceID, char *pay)
{
    *t = intFromChar(mess, 0);
    *AID = intFromChar(mess, 4);
    *SID = intFromChar(mess, 8);
    int paylen = intFromChar(mess, 12);
    *sequenceID = intFromChar(mess, 16);
    memcpy(pay, &mess[20], paylen);
    return paylen;
}

void beef(char const *ip, int sPort, int tPort)
{
	int sSock = 0;
    int sTelnet = 0;
	int reciever = 0;
	int i = 0;
    fd_set discript;
    struct timeval time;
    char cBuffer[9999];
	char rBuffer[9999];

    struct sockaddr_in tAddr;
    int tSock = sockInit(&tAddr, "127.0.0.1", tPort);

    //link with telnet
    BnL(tSock, &tAddr, 5);

    sTelnet = cAccept(tSock, &tAddr);
    int SID = rand();
    int sequenceID = -1;
	//init the LL
    LLMessages *q = (LLMessages*)malloc(sizeof(LLMessages));
	q->next = NULL;
    int AID = -1;

    int l1 = 0;
	int l2 = 0;
	//continue forever
    while (1){
		struct sockaddr_in sAddr;
		int sock = sockInit(&sAddr, ip, sPort);

		int options = 1;
		if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
				&options, sizeof(options))) {
			perror("setsockopt");
			exit(EXIT_FAILURE);
		}


		sConnect(&sAddr, sock);

		sSock = sock;

		//heartbeat
		struct timeval prev, cur, heartbeatTime;
		gettimeofday(&prev, NULL);
		int heartbeatSend = 0;
		int heartbeatRec = 0;
		int heartbeatPrev = -1;
		while (1) {
			FD_ZERO(&discript);
			FD_SET(sTelnet, &discript);
			FD_SET(sSock, &discript);
			if (sTelnet > sSock){
				i = sTelnet + 1;
			} else {
				i = sSock + 1;
			}
			time.tv_usec = 500000;
			time.tv_sec = 1;
			reciever = select(i, &discript, NULL, NULL, &time);

			// Check if time to send heartbeat    
			gettimeofday(&cur, NULL);
			double difference = (cur.tv_sec - prev.tv_sec) + ((cur.tv_usec - prev.tv_usec) / 1000000.0);
			if (difference >= 1) {
				//send heartbeat;
				char *heartbeatPay = " ";
				char heartbeatMsg[9999] = {0};
				int heartbeatMsgLen = createMessage(heartbeatMsg, 2, heartbeatSend, SID, sizeof(heartbeatPay), 2020, heartbeatPay);
				send(sSock, heartbeatMsg, heartbeatMsgLen, 0);
				gettimeofday(&prev, NULL);
				heartbeatSend++;
			}
			// Detect if timeout by heartbeat
			if (heartbeatPrev >= 0) {
				gettimeofday(&cur, NULL);
				double heartbeatDifference = (cur.tv_sec - heartbeatTime.tv_sec) + ((cur.tv_usec - heartbeatTime.tv_usec) / 1000000.0);
				if (heartbeatDifference >= 3) {
					close(sSock);

					struct sockaddr_in tryNewAddr;
					sSock = sockInit(&tryNewAddr, ip, sPort);

					int try = 1;
					do {
						try = sConnect(&tryNewAddr, sSock);
					} while (try == 0);
					gettimeofday(&heartbeatTime, NULL);
				}
			}
			// Read recv()'s
			if (reciever == -1) {
				perror("select");
			} else if (reciever == 0) {
				//do nothing
			;
			} else {
				if (FD_ISSET(sTelnet, &discript)) {
					l1 = recv(sTelnet, cBuffer, sizeof(cBuffer), 0);
					if (l1 > 0) {
						sequenceID++;
						LLMessages *curQ = NULL;
						char tMsg[9999] = {0};
						int tMsgLen = createMessage(tMsg, 1, AID, SID, l1, sequenceID, cBuffer);
						//create new message
						curQ = (LLMessages*)malloc(sizeof(LLMessages));
						curQ->msgCnt = (char*)malloc(sizeof(char) * tMsgLen);
						strncpy(curQ->msgCnt, tMsg, tMsgLen);
						curQ->next = NULL;
						//add to LL
						LLMessages *cur = q;
						while (cur->next != NULL) {
							cur = cur->next;
						}
						cur->next = curQ;
						send(sSock, tMsg, tMsgLen, 0);
						memset(cBuffer, 0, sizeof(cBuffer));
					} else {
						//close it all
						close(sTelnet);
						close(sSock);
						close(tSock);
					return;
					}
				}
				if (FD_ISSET(sSock, &discript)) {
					l2 = recv(sSock, rBuffer, sizeof(rBuffer), 0);
					if (l2 > 0) {
						char *pBuff = rBuffer;
						int pLen = l2;
						int retry = 0;
						do {
							int SIDS = -1;
							int sequenceIDS = -1;
							int AIDS = -1;
							char SPay[9999] = {0};
							int type = -1;
							int SLenPay = getInts(pBuff, &type, &AIDS, &SIDS, &sequenceIDS, SPay);

							if (type == 2) {
								gettimeofday(&heartbeatTime, NULL);
								heartbeatRec++;
								heartbeatPrev++;
							}
							else if (type == 1) {
								if(sequenceID >= AIDS){
									//do nothing
									;
								}
								else{
									//remove from LL
									LLMessages *cur = q->next;
									if (cur != NULL) {
										q->next = q->next->next;
									}
								}
								AID++;
								send(sTelnet, SPay, SLenPay, 0);
							} else {
								printf("error wrong message type: %d\n", type);
							}

							int HDRLenPay = SLenPay + 20;
							if (HDRLenPay < pLen) {
								retry = 1;   
								pLen = pLen - HDRLenPay;
								pBuff += HDRLenPay;
							} else {
								retry = 0;
							}
						} while (retry);
						memset(rBuffer, 0, sizeof(rBuffer));
					} else {
						close(sSock);
						sSock = sockInit(&sAddr, ip, sPort);
						sConnect(&sAddr, sock);
					}
				}
			}
		}
		close(sock);
		close(sSock);
    }
    close(sTelnet);
    close(tSock);
    return;
}
