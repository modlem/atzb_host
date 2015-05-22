/*
 * packet.c
 *
 *  Created on: May 22, 2015
 *      Author: Jason Kwon
 */

#include "packet.h"
#include <stdlib.h>
#include <string.h>

enum UART_STATE uartState = UART_IDLE;
int incomingTransmission = 0;
char *pktBuf = NULL;
unsigned int pktBufSize, pktBufPnt;
unsigned char checksum = 0;
enum UART_RTN_CODE uartRtn = UART_RTN_CONTINUE;

enum UART_RTN_CODE _parseUARTChar(const char chr, char *buf, unsigned int *bufPnt)
{
	enum UART_RTN_CODE rtn = UART_RTN_CONTINUE;
	switch(chr)
	{
	case _UART_PKT_DELIMITER:
		switch(uartState)
		{
		case UART_IDLE:
			uartState = UART_DELIMITER;
			break;
		case UART_DELIMITER:
			if(incomingTransmission)
			{
				buf[*bufPnt] = chr;
				(*bufPnt)++;
				uartState = UART_STARTED;
				checksum += (chr + chr);
			}
			break;
		case UART_STARTED:
			uartState = UART_DELIMITER;
			break;
		}
		break;
	case _UART_PKT_START:
		switch(uartState)
		{
		case UART_IDLE:
			break;
		case UART_DELIMITER:
			uartState = UART_STARTED;
			bufPnt = 0;
			incomingTransmission = 1;
			checksum = _UART_PKT_DELIMITER + chr;
			break;
		case UART_STARTED:
			buf[*bufPnt] = chr;
			(*bufPnt)++;
			checksum += chr;
			break;
		}
		break;
	case _UART_PKT_END:
		switch(uartState)
		{
		case UART_IDLE:
			break;
		case UART_DELIMITER:
			uartState = UART_IDLE;
			if(incomingTransmission)
			{
				rtn = UART_RTN_PACKET_DONE;
				checksum += (_UART_PKT_DELIMITER + chr);
			}
			incomingTransmission = 0;
			break;
		case UART_STARTED:
			buf[*bufPnt] = chr;
			(*bufPnt)++;
			checksum += chr;
			break;
		}
		break;
	default:
		if(incomingTransmission)
		{
			buf[*bufPnt] = chr;
			(*bufPnt)++;
			checksum += chr;
		}
		break;
	}
	return rtn;
}

unsigned int unpackUART(const char *uartReceived, unsigned int size, void (*pktCBFunction)(char *, unsigned int))
{
	unsigned int i;

	if(pktBuf == NULL)
	{
		pktBufSize = 2 * size;
		pktBufPnt = 0;
		pktBuf = (char *)calloc(pktBufSize, sizeof(char));
	}

	for(i = 0; i < size; i++)
	{
		if(uartRtn == UART_RTN_PACKET_DONE)
		{
			if(checksum == (unsigned char)uartReceived[i])
			{
				char *tmpbuf = (char*)malloc(pktBufPnt);
				memcpy(tmpbuf, pktBuf, pktBufPnt);
				pktCBFunction(tmpbuf, pktBufPnt);
				free(tmpbuf);
			}
			pktBufPnt = 0;
			uartRtn = UART_RTN_CONTINUE;
			checksum = 0x0;
			continue;
		}
		else
		{
			uartRtn = _parseUARTChar(uartReceived[i], pktBuf, &pktBufPnt);
		}
		if((pktBufPnt + 10) > pktBufSize)
		{
			void *newBuf = realloc(pktBuf, (pktBufPnt + 10) * 2);
			if(newBuf != NULL)
			{
				pktBuf = (char *)newBuf;
				pktBufSize = (pktBufPnt + 10) * 2;
			}
		}
	}
	return 0;
}

void freePktbuf()
{
	free(pktBuf);
	pktBufSize = 0;
	pktBufPnt = 0;
	pktBuf = NULL;
}

unsigned int packUART(const char *packet, unsigned int sizePacket, char *uartBuf, unsigned int sizeUartBuf)
{
	if(sizeUartBuf < sizePacket * 2 || sizeUartBuf < 4)
	{
		return 0;
	}
	else
	{
		unsigned char pktChecksum = 0;
		unsigned int i, j;
		uartBuf[0] = _UART_PKT_DELIMITER;
		uartBuf[1] = _UART_PKT_START;
		pktChecksum = _UART_PKT_DELIMITER + _UART_PKT_START;
		for(i = 2, j = 0; j < sizePacket; i++, j++)
		{
			uartBuf[i] = packet[j];
			pktChecksum += (unsigned char)packet[j];
			if(packet[j] == _UART_PKT_DELIMITER)
			{
				i++;
				uartBuf[i] = _UART_PKT_DELIMITER;
				pktChecksum += _UART_PKT_DELIMITER;
			}
		}
		uartBuf[i++] = _UART_PKT_DELIMITER;
		uartBuf[i++] = _UART_PKT_END;
		pktChecksum += (_UART_PKT_DELIMITER + _UART_PKT_END);
		uartBuf[i++] = (char)pktChecksum;
		return i;
	}
}

enum PACKET_RTN_CODE getPacketType(const char *packet, unsigned int sizePacket)
{
	if(sizePacket > 0)
	{
		switch(packet[0])
		{
		case PACKET_RTN_BASE:
			if(sizePacket < PACKET_SIZE_BASE)
			{
				break;
			}
			return PACKET_RTN_BASE;
		case PACKET_RTN_IDENTITY:
			if(sizePacket < PACKET_SIZE_IDENTITY)
			{
				break;
			}
			return PACKET_RTN_IDENTITY;
		case PACKET_RTN_IDNACK:
			if(sizePacket < PACKET_SIZE_IDNACK)
			{
				break;
			}
			return PACKET_RTN_IDNACK;
		default:
			break;
		}
	}
	return PACKET_RTN_UNKNOWN;
}

unsigned short _c2us(const char *c)
{
	unsigned short *rtn = (unsigned short*)c;
	return *rtn;
}

int _c2i(const char *c)
{
	int *rtn = (int *)c;
	return *rtn;
}

unsigned int _c2ui(const char *c)
{
	unsigned int *rtn = (unsigned int*)c;
	return *rtn;
}

unsigned long _c2ul(const char *c)
{
	unsigned long *rtn = (unsigned long*)c;
	return *rtn;
}

int unpackBasePacket(char *packet, unsigned int sizePacket, struct packet_base *outstruct, char **varStart, unsigned int *varSize)
{
	if(sizePacket < PACKET_SIZE_BASE)
	{
		return 0;
	}
	else
	{
		outstruct->msgType = (unsigned char)packet[0];
		outstruct->nodeType = (unsigned char)packet[1];
		outstruct->fullAddress = _c2ul(&packet[2]);
		outstruct->shortAddress = _c2us(&packet[10]);
		outstruct->softwareVersion = _c2ui(&packet[12]);
		outstruct->channelMask = _c2ui(&packet[16]);
		outstruct->panID = _c2us(&packet[20]);
		outstruct->workingChannel = (unsigned char)packet[22];
		outstruct->parentAddress = _c2us(&packet[23]);
		outstruct->lqi = (unsigned char)packet[25];
		outstruct->rssi = packet[26];
		*varStart = &packet[27];
		*varSize = sizePacket - PACKET_SIZE_BASE;
		return 1;
	}
}

int unpackIdentityPacket(const char *packet, unsigned int sizePacket, struct packet_identity *outstruct)
{
	if(sizePacket < PACKET_SIZE_IDENTITY)
	{
		return 0;
	}
	else
	{
		outstruct->msgType = (unsigned char)packet[0];
		outstruct->fullAddress = _c2ul(&packet[1]);
		outstruct->duration = _c2us(&packet[9]);
		outstruct->period = _c2us(&packet[11]);
		return 1;
	}
}

int unpackIdnackPacket(const char *packet, unsigned int sizePacket, struct packet_idnack *outstruct)
{
	if(sizePacket < PACKET_SIZE_IDNACK)
	{
		return 0;
	}
	else
	{
		outstruct->msgType = (unsigned char)packet[0];
		outstruct->identityStatus = (unsigned char)packet[1];
		outstruct->fullAddress = _c2ul(&packet[2]);
		return 1;
	}
}
