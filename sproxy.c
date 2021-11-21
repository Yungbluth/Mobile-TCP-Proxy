//sproxy.c
//CSC 425
//By: Matthew Yungbluth and Cameron Richard Kazmierski
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/select.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netinet/in.h>

typedef struct LLMessages {
	char* msgCnt;
	struct LLMessages* next;
}LLMessages;

void beef(int port);

int main(int argc, char const *argv[])
{
    int port = atoi(argv[1]);
    while (1) {
		beef(port);
		sleep(1);
    }
}

int CAccept(int sDiscript, struct sockaddr_in *addr) {
    int sock = 0;
    int addLen = sizeof(*addr);
    if ((sock = accept(sDiscript, (struct sockaddr *) addr, (socklen_t *) & addLen)) < 0) {
		exit(EXIT_FAILURE);
    }
    return sock;
}

int sConnect(struct sockaddr_in *sAddr, int sock) {
    if (connect(sock, (struct sockaddr *) sAddr, sizeof(*sAddr)) < 0) {
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

int initSock(struct sockaddr_in *sAddr, char const *ip, int port)
{
    int sock = 0;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		
    }
    if (ip != NULL) {
		memset(sAddr, '0', sizeof(*sAddr));
		sAddr->sin_family = AF_INET;
		sAddr->sin_port = htons(port);
		sAddr->sin_addr.s_addr = inet_addr(ip);
    }
    else {
		sAddr->sin_family = AF_INET;
		sAddr->sin_addr.s_addr = INADDR_ANY;
		sAddr->sin_port = htons(port);
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

void beef(int port)
{

    int sCproxy = 0;
	int SDae = 0;
	int i = 0;
	int reciever = 0;
	int SID = -1;
    fd_set discript;
    struct timeval time;
    char cBuffer[9999];
	char rBuffer[9999];

    struct sockaddr_in dAddr;
    int STDae = initSock(&dAddr, "127.0.0.1", 23);

    sConnect(&dAddr, STDae);

    int IDSequence = -1;
    int AID = -1;
	//init q
    LLMessages* q = (LLMessages*)malloc(sizeof(LLMessages));
	q->next = NULL;
    while (1) {
		struct sockaddr_in adr;
		int SDF = initSock(&adr, NULL, port);

		int options = 1;
		if (setsockopt(SDF, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &options, sizeof(options))) {
			exit(EXIT_FAILURE);
		}

		BnL(SDF, &adr, 5);

		sCproxy = CAccept(SDF, &adr);
		SDae = STDae;
		int l1 = 0;
		int l2 = 0;

		struct timeval prev, cur, heartbeatTime;
		gettimeofday(&prev, NULL);
		int heartbeatSend = 0;
		int heartbeatRec = 0;
		int heartbeatPrev = -1;

		while (1) {
			FD_ZERO(&discript);
			FD_SET(sCproxy, &discript);
			FD_SET(SDae, &discript);
			if (sCproxy > SDae) {
				i = sCproxy + 1;
			} else {
				i = SDae + 1;
			}
              
			time.tv_sec = 1;
			time.tv_usec = 5;
			reciever = select(i, &discript, NULL, NULL, &time);

			gettimeofday(&cur, NULL);
			double difference = (cur.tv_sec - prev.tv_sec) + ((cur.tv_usec - prev.tv_usec) / 1000000.0);
			if (difference >= 1) {
				char heartbeatMessage[9999] = {0};
				char *heartbeatPay = " ";
				int heartbeatMsgLen = createMessage(heartbeatMessage, 2, heartbeatSend, SID, sizeof(heartbeatPay), 2020, heartbeatPay);
				send(sCproxy, heartbeatMessage, heartbeatMsgLen, 0);
				gettimeofday(&prev, NULL);
				heartbeatSend++;
			}
			if (heartbeatPrev >= 0) {
				gettimeofday(&cur, NULL);
				double hearthbeatDifference =+ ((cur.tv_usec - heartbeatTime.tv_usec) / 1000000.0) + (cur.tv_sec - heartbeatTime.tv_sec);
				if (hearthbeatDifference >= 3) {
					close(sCproxy);
					listen(SDF, 5);
					sCproxy = CAccept(SDF, &adr);
				}
			}
			if (reciever == -1) {
				perror("select");
			} else if (reciever == 0) {
				//do nothing
				;
			} else {
				if (FD_ISSET(sCproxy, &discript)) {
					l1 = recv(sCproxy, cBuffer, sizeof(cBuffer), 0);
					if (l1 > 0) {
						char *buffPacket = cBuffer;
						int lenPacket = l1;
						int retry = 0;
						do {
							int type = -1;
							int AID_C = -1;
							int IDSequenceC = -1;
							int CsessionID = -1;
							char cPayload[9999] = {0};
							int lenPayC = getInts(buffPacket, &type, &AID_C, &CsessionID, &IDSequenceC, cPayload);
							if (SID != CsessionID) {
								SID = CsessionID;
							}
							if (type == 2) {
								gettimeofday(&heartbeatTime, NULL);
								heartbeatRec++;
								heartbeatPrev++;
							} else if (type == 1) {
								if(IDSequence >= AID_C){
									//do nothing
									;
								} else {
									//remove
									LLMessages *cur = q->next;
									if (cur != NULL) {
										q->next = q->next->next;
									}
								}
								AID++;
								send(SDae, cPayload, lenPayC, 0);
							} else {
								//do nothing
								;
							}
							int payHDR = lenPayC + 20;
							if (payHDR < lenPacket) {
								retry = 1;
								lenPacket = lenPacket - payHDR;
								buffPacket += payHDR;
							} else {
								retry = 0;
							}
						} while (retry);
						memset(cBuffer, 0, sizeof(cBuffer));
					} else {
					close(sCproxy);
					close(SDae);
					close(SDF);
					return;
					}
				}
				if (FD_ISSET(SDae, &discript)) {
					l2 = recv(SDae, rBuffer, sizeof(rBuffer), 0);
					if (l2 > 0) {
						IDSequence++;
						LLMessages *curQ = NULL;
						char curMSG[9999] = {0};
						int curMSGLen = createMessage(curMSG, 1, AID, SID, l2, IDSequence, rBuffer);
						LLMessages *curHead = (LLMessages*)malloc(sizeof(LLMessages));
						curHead->msgCnt = (char*)malloc(sizeof(char) * curMSGLen);
						strncpy(curHead->msgCnt, curMSG, curMSGLen);
						curHead->next = NULL;
						curQ = curHead;
						LLMessages *cur = q;
						while (cur->next != NULL) {
							cur = cur->next;
						}
						cur->next = curQ;
						send(sCproxy, curMSG, curMSGLen, 0);\
						memset(rBuffer, 0, sizeof(rBuffer));
					} else {
						usleep(50000);
					}
				}
			}
		}
		close(sCproxy);
    }
    close(SDae);
    close(STDae);
    return;
}
