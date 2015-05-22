/*
 * main.c
 *
 *  Created on: May 22, 2015
 *      Author: Jason Kwon
 */


#include <stdio.h>
#include "testpacket.h"
#include "packet.h"

unsigned int processedPkts = 0;
const unsigned int totalPkts = TOTAL_PACKETS;

void pktFunction(char *pkt, unsigned int size)
{
	struct packet_base structPktBase;
	struct packet_identity structPktIdentity;
	struct packet_idnack structPktIdnack;

	char *varStart;
	unsigned int varSize;

	processedPkts++;
	switch(getPacketType(pkt, size))
	{
	case PACKET_RTN_BASE:
		unpackBasePacket(pkt, size, &structPktBase, &varStart, &varSize);
		break;
	case PACKET_RTN_IDENTITY:
		unpackIdentityPacket(pkt, size, &structPktIdentity);
		break;
	case PACKET_RTN_IDNACK:
		unpackIdnackPacket(pkt, size, &structPktIdnack);
		break;
	default:
		break;
	}
}

int main(int argc, char* argv[])
{
	unpackUART((char*)uartReceived1, sizeof(uartReceived1), pktFunction);
	unpackUART((char*)uartReceived2, sizeof(uartReceived2), pktFunction);
	freePktbuf();
	printf("processed: %d\n", processedPkts);
	if(processedPkts == totalPkts)
	{
		printf("Okay.\n");
		return 0;
	}
	else
	{
		printf("should be %d.\nSomethings wrong.", totalPkts);
		return -1;
	}
}
