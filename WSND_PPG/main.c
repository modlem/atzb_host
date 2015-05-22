/*
 * main.c
 *
 *  Created on: May 22, 2015
 *      Author: Jason Kwon
 */


#include <stdio.h>
#include "testpacket.h"

unsigned int unpackUART(const char *uartReceived, unsigned int size, void (*pktCBFunction)(const char *, unsigned int));
void freePktbuf();

unsigned int processedPkts = 0;
const unsigned int totalPkts = TOTAL_PACKETS;

void pktFunction(const char *pkt, unsigned int size)
{
	processedPkts++;
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
