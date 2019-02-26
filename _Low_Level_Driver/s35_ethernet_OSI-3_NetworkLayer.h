#ifndef __NETWORK_LAYER_H
#define __NETWORK_LAYER_H

typedef struct 
{
    BYTE VersionIHL;
    BYTE TypeOfService;
    WORD TotalLength;
    WORD Identification;
    WORD FragmentInfo;
    BYTE TimeToLive;
    BYTE Protocol;
    WORD HeaderChecksum;
    IP_ADDR SourceAddress;
    IP_ADDR DestAddress;
}IP_HEADER;

typedef enum 
{
    SM_IDLE = 0,
    SM_ARP_SEND_QUERY,
    SM_ARP_GET_RESPONSE,
    SM_ICMP_SEND_ECHO_REQUEST,
    SM_ICMP_GET_ECHO_RESPONSE
}ICMP_STATE;

typedef struct
{
    unsigned char bICMPInUse : 1;       // Indicates that the ICMP Client is in use
    unsigned char bReplyValid : 1;      // Indicates that a correct Ping response to one of our pings was received
}ICMP_FLAGS;

typedef struct 
{
    BYTE TypeOfMessage;
    BYTE Code;
    WORD HeaderChecksum;
    WORD Identifier;
    WORD SequenceNumber;
    BYTE Data[32];
}ICMP_PACKET;

typedef struct __attribute__((aligned(2), packed)) 
{
    WORD HardwareType; // Link-layer protocol type (Ethernet is 1).
    WORD Protocol; // The upper-layer protocol issuing an ARP request (0x0800 for IPv4)..
    BYTE MACAddrLen; // MAC address length (6).
    BYTE ProtocolLen; // Length of addresses used in the upper-layer protocol (4).
    WORD Operation; // The operation the sender is performing (ARP_REQ or ARP_RESP).
    MAC_ADDR SenderMACAddr; // The sender's hardware (MAC) address.
    IP_ADDR SenderIPAddr; // The sender's IP address.
    MAC_ADDR TargetMACAddr; // The target node's hardware (MAC) address.
    IP_ADDR TargetIPAddr; // The target node's IP address.
}ARP_PACKET;

// ARP
#define ARP_OPERATION_REQ           (0x0001u)		// Operation code indicating an ARP Request
#define ARP_OPERATION_RESP          (0x0002u)		// Operation code indicating an ARP Response
#define ARP_IP                      (0x0800u)       // ARP IP packet type as defined by IEEE 802.3
#define HW_ETHERNET                 (0x0001u)       // ARP Hardware type as defined by IEEE 802.3

// IP 
#define IP_IPv4                     (0x40)
#define IP_IHL_STANDARD             (0x05)
#define IP_DSCP                     (0x00)
#define IP_ECN                      (0x00)
#define IP_TTL                      (128)

// ICMP
#define ICMP_ECHO_REPLY             (0x00)
#define ICMP_ECHO_REQUEST           (0x08)
#define ICMP_IDENTIFIER             (0xEFBE)

// IP
void IPPutHeader(NODE_INFO *remote, BYTE protocol, WORD len);
BOOL IPGetHeader(IP_ADDR *remote, IP_HEADER *IPHeader);
void IPSetRxBuffer(WORD Offset);

// ARP
void ARPProcess(void);
void ARPResolve(IP_ADDR* IPAddr);
BOOL ARPIsResolved(IP_ADDR* IPAddr, MAC_ADDR* MACAddr);

// ICMP
void ICMPProcess(NODE_INFO *remote, IP_ADDR localIP, WORD len);
BOOL ICMPBeginUsage(void);
void ICMPSendPing(DWORD dwRemoteIP);
void ICMPSendPingToHost(BYTE * szRemoteHost);
LONG ICMPGetReply(void);
void ICMPEndUsage(void);

#endif
