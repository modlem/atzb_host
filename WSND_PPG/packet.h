/*
 * packet.h
 *
 *  Created on: May 22, 2015
 *      Author: procella
 */

#ifndef PACKET_H_
#define PACKET_H_

#define _UART_PKT_DELIMITER		0x10
#define _UART_PKT_START			0x02
#define _UART_PKT_END			0x03

enum UART_STATE
{
	UART_IDLE,
	UART_DELIMITER,
	UART_STARTED,
};

enum UART_RTN_CODE
{
	UART_RTN_CONTINUE,
	UART_RTN_PACKET_DONE,
};

enum PACKET_RTN_CODE
{
	PACKET_RTN_UNKNOWN,
	PACKET_RTN_BASE = 0x01,			// should be at least 27 bytes (without unlimited params)
	PACKET_RTN_IDENTITY = 0x10,		// 13 bytes
	PACKET_RTN_IDNACK = 0x11,		// 10 bytes
};

#define PACKET_SIZE_BASE		27
struct packet_base
{
	unsigned char msgType;
	unsigned char nodeType;
	unsigned long fullAddress;
	unsigned short shortAddress;
	unsigned int softwareVersion;
	unsigned int channelMask;
	unsigned short panID;
	unsigned char workingChannel;
	unsigned short parentAddress;
	unsigned char lqi;
	char rssi;
};

#define PACKET_SIZE_IDENTITY	13
struct packet_identity
{
	unsigned char msgType;
	unsigned long fullAddress;
	unsigned short duration;
	unsigned short period;
};

#define PACKET_SIZE_IDNACK		10
struct packet_idnack
{
	unsigned char msgType;
	unsigned char identityStatus;
	unsigned long fullAddress;
};

unsigned int unpackUART(const char *uartReceived, unsigned int size, void (*pktCBFunction)(char *, unsigned int));
void freePktbuf();
unsigned int packUART(const char *packet, unsigned int sizePacket, char *uartBuf, unsigned int sizeUartBuf);
enum PACKET_RTN_CODE getPacketType(const char *packet, unsigned int sizePacket);
int unpackBasePacket(char *packet, unsigned int sizePacket, struct packet_base *outstruct, char **varStart, unsigned int *varSize);
int unpackIdentityPacket(const char *packet, unsigned int sizePacket, struct packet_identity *outstruct);
int unpackIdnackPacket(const char *packet, unsigned int sizePacket, struct packet_idnack *outstruct);

#endif /* PACKET_H_ */
