#ifndef ETHERNET_TRANSPORTLAYER_H
#define	ETHERNET_TRANSPORTLAYER_H


/*********************************************************************
 * <UDP>
 ********************************************************************/
typedef BYTE UDP_SOCKET;
typedef WORD UDP_PORT;

// First port number for randomized local port number selection
#define LOCAL_UDP_PORT_START_NUMBER (4096u)

// Last port number for randomized local port number selection
#define LOCAL_UDP_PORT_END_NUMBER   (8192u)

#define IPSetTxBuffer(b) MACSetWritePtr(b + BASE_TX_ADDR + sizeof(ETHER_HEADER) + sizeof(IP_HEADER))
#define SwapPseudoHeader(h)  (h.Length = swap_word(h.Length))

#define MAX_UDP_SOCKETS             (8u)
#define INVALID_UDP_SOCKET          (0xffu)		// Indicates a UDP socket that is not valid
#define INVALID_UDP_PORT            (0ul)		// Indicates a UDP port that is not valid

#define UDP_OPEN_SERVER		0u
#define UDP_OPEN_IP_ADDRESS	3u
#define UDP_OPEN_NODE_INFO	4u

#define UDPPutROMArray(a,b)	UDPPutArray((BYTE*)a,b)
#define UDPPutROMString(a)	UDPPutString((BYTE*)a)

static struct {
    unsigned char bFirstRead : 1; // No data has been read from this segment yet
    unsigned char bWasDiscarded : 1; // The data in this segment has been discarded
} Flags;

typedef enum {
    UDP_DNS_IS_RESOLVED, // Special state for UDP client mode sockets
    UDP_DNS_RESOLVE, // Special state for UDP client mode sockets
    UDP_GATEWAY_SEND_ARP, // Special state for UDP client mode sockets
    UDP_GATEWAY_GET_ARP, // Special state for UDP client mode sockets
    UDP_CLOSED, // Socket is idle and unallocated
    UDP_OPENED
} UDP_STATE;

typedef struct {
    UDP_PORT SourcePort; // Source UDP port
    UDP_PORT DestinationPort; // Destination UDP port
    WORD Length; // Length of data
    WORD Checksum; // UDP checksum of the data
} UDP_HEADER;

typedef struct
{
    NODE_INFO	remoteNode;		// 10 bytes for MAC and IP address
    DWORD		remoteHost;		// RAM or ROM pointer to a hostname string (ex: "www.microchip.com")
    UDP_PORT    remotePort;		// Remote node's UDP port number
    UDP_PORT    localPort;		// Local UDP port number, or INVALID_UDP_PORT when free
    UDP_STATE smState;			// State of this socket
    QWORD retryInterval;
	BYTE retryCount;
	struct {
		unsigned char bRemoteHostIsROM : 1;	// Remote host is stored in ROM
	}flags;
	QWORD eventTime;
} UDP_SOCKET_INFO;

/*********************************************************************
 * <TCP>
 ********************************************************************/
typedef BYTE TCP_SOCKET;
#define LOCAL_PORT_START_NUMBER                 (1024u)

// TCP Timeout and retransmit numbers
#define TCP_START_TIMEOUT_VAL                   ((QWORD)TICK_1S)	// Timeout to retransmit unacked data
#define TCP_DELAYED_ACK_TIMEOUT                 ((QWORD)TICK_100MS)	// Timeout for delayed-acknowledgement algorithm
#define TCP_FIN_WAIT_2_TIMEOUT                  ((QWORD)TICK_5S)	// Timeout for FIN WAIT 2 state
#define TCP_KEEP_ALIVE_TIMEOUT                  ((QWORD)TICK_10S)	// Timeout for keep-alive messages when no traffic is sent
#define TCP_CLOSE_WAIT_TIMEOUT                  ((QWORD)TICK_200MS)	// Timeout for the CLOSE_WAIT state
#define TCP_MAX_RETRIES                         (5u)					// Maximum number of retransmission attempts
#define TCP_MAX_UNACKED_KEEP_ALIVES             (6u)					// Maximum number of keep-alive messages that can be sent without receiving a response before automatically closing the connection
#define TCP_MAX_SYN_RETRIES                     (2u)	// Smaller than all other retries to reduce SYN flood DoS duration

#define TCP_AUTO_TRANSMIT_TIMEOUT_VAL           (TICK_40MS)	// Timeout before automatically transmitting unflushed data
#define TCP_WINDOW_UPDATE_TIMEOUT_VAL           (TICK_200MS)	// Timeout before automatically transmitting a window update due to a TCPGet() or TCPGetArray() function call

#define TCP_SYN_QUEUE_MAX_ENTRIES               (3u) 			// Number of TCP RX SYN packets to save if they cannot be serviced immediately
#define TCP_SYN_QUEUE_TIMEOUT                   ((QWORD)TICK_3S)	// Timeout for when SYN queue entries are deleted if unserviceable

#define INVALID_SOCKET                          (0xFE)	// The socket is invalid or could not be opened
#define UNKNOWN_SOCKET                          (0xFF)	// The socket is not known

#define MAX_TCP_SOCKETS                         (3)
#define TCP_SOCKET_TX_BUFFER_SIZE               (200)   // For each TCP Socket
#define TCP_SOCKET_RX_BUFFER_SIZE               (500)  // For each TCP Socket
#define TCP_PIC_RAM_SIZE                        (MAX_TCP_SOCKETS*(TCP_SOCKET_TX_BUFFER_SIZE + TCP_SOCKET_RX_BUFFER_SIZE + 150))

#define TCP_ETH_RAM                             0u
#define TCP_PIC_RAM                             1u
#define TCP_PIC_RAM_BASE_ADDRESS                ((PTR_BASE)&TCPBufferInPIC[0])


#define FIN     (0x01)		// FIN Flag as defined in RFC
#define SYN     (0x02)		// SYN Flag as defined in RFC
#define RST     (0x04)		// Reset Flag as defined in RFC
#define PSH     (0x08)		// Push Flag as defined in RFC
#define ACK     (0x10)		// Acknowledge Flag as defined in RFC
#define URG     (0x20)		// Urgent Flag as defined in RFC

// Indicates if this packet is a retransmission (no reset) or a new packet (reset required)
#define SENDTCP_RESET_TIMERS            0x01
// Instead of transmitting normal data, a garbage octet is transmitted according to RFC 1122 section 4.2.3.6
#define SENDTCP_KEEP_ALIVE              0x02
#define TCP_OPEN_SERVER                 0u
#define TCP_OPEN_IP_ADDRESS             3u
#define LOCAL_PORT_END_NUMBER           (5000u)

#define TCPPutROMString(a,b)            TCPPutString(a,(BYTE*)b)
#define TCP_MAX_SEG_SIZE_TX             (1460u)
#define TCP_MAX_SEG_SIZE_RX             (536u)

#define TCP_OPTIONS_END_OF_LIST         (0x00u)		// End of List TCP Option Flag
#define TCP_OPTIONS_NO_OP               (0x01u)		// No Op TCP Option
#define TCP_OPTIONS_MAX_SEG_SIZE        (0x02u)		// Maximum segment size TCP flag

typedef enum {
    TCP_GET_DNS_MODULE, // Special state for TCP client mode sockets
    TCP_DNS_RESOLVE, // Special state for TCP client mode sockets
    TCP_GATEWAY_SEND_ARP, // Special state for TCP client mode sockets
    TCP_GATEWAY_GET_ARP, // Special state for TCP client mode sockets

    TCP_LISTEN, // Socket is listening for connections
    TCP_SYN_SENT, // A SYN has been sent, awaiting an SYN+ACK
    TCP_SYN_RECEIVED, // A SYN has been received, awaiting an ACK
    TCP_ESTABLISHED, // Socket is connected and connection is established
    TCP_FIN_WAIT_1, // FIN WAIT state 1
    TCP_FIN_WAIT_2, // FIN WAIT state 2
    TCP_CLOSING, // Socket is closing
    //	TCP_TIME_WAIT, state is not implemented
    TCP_CLOSE_WAIT, // Waiting to close the socket
    TCP_LAST_ACK, // The final ACK has been sent
    TCP_CLOSED, // Socket is idle and unallocated

    TCP_CLOSED_BUT_RESERVED // Special state for TCP client mode sockets.  Socket is idle, but still allocated pending application closure of the handle.
} TCP_STATE;

typedef struct 
{
    WORD_VAL mLocalPort;
    WORD_VAL mRemotePort;
    NODE_INFO mRemoteNode; 
    
    PTR_BASE bufferTxStart; // First byte of TX buffer
    PTR_BASE bufferRxStart; // First byte of RX buffer.  TX buffer ends 1 byte prior
    PTR_BASE bufferEnd; // Last byte of RX buffer
    PTR_BASE txHead; // Head pointer for TX
    PTR_BASE txTail; // Tail pointer for TX
    PTR_BASE rxHead; // Head pointer for RX
    PTR_BASE rxTail; // Tail pointer for RX
    QWORD eventTime; // Packet retransmissions, state changes
    QWORD eventTime2; // Window updates, automatic transmission

    union {
        QWORD delayedACKTime; // Delayed Acknowledgement timer
        QWORD closeWaitTime; // TCP_CLOSE_WAIT timeout timer
    } OverlappedTimers;
    TCP_STATE smState; // State of this socket

    struct {
        unsigned char vUnackedKeepalives : 3; // Count of how many keepalives have been sent with no response
        unsigned char bServer : 1; // Socket should return to listening state when closed
        unsigned char bTimerEnabled : 1; // Timer is enabled
        unsigned char bTimer2Enabled : 1; // Second timer is enabled
        unsigned char bDelayedACKTimerEnabled : 1; // DelayedACK timer is enabled
        unsigned char bOneSegmentReceived : 1; // A segment has been received
        unsigned char bHalfFullFlush : 1; // Flush is for being half full
        unsigned char bTXASAP : 1; // Transmit as soon as possible (for Flush)
        unsigned char bTXASAPWithoutTimerReset : 1; // Transmit as soon as possible (for Flush), but do not reset retransmission timers
        unsigned char bTXFIN : 1; // FIN needs to be transmitted
        unsigned char bSocketReset : 1; // Socket has been reset (self-clearing semaphore)
        unsigned char bSSLHandshaking : 1; // Socket is in an SSL handshake
        unsigned char filler : 2; // Future expansion
    } Flags;
    WORD_VAL remoteHash; // Consists of remoteIP, remotePort, localPort for connected sockets.  It is a localPort number only for listening server sockets.
    BYTE vMemoryMedium;
} TCB_STUB;

typedef struct {
    NODE_INFO niSourceAddress; // Remote IP address and MAC address
    WORD wSourcePort; // Remote TCP port number that the response SYN needs to be sent to
    DWORD dwSourceSEQ; // Remote TCP SEQuence number that must be ACKnowledged when we send our response SYN
    WORD wDestPort; // Local TCP port which the original SYN was destined for
    QWORD wTimestamp; // Timer to expire old SYN packets that can't be serviced at all
} TCP_SYN_QUEUE;

typedef struct {
    QWORD retryInterval; // How long to wait before retrying transmission
    DWORD MySEQ; // Local sequence number
    DWORD RemoteSEQ; // Remote sequence number
    PTR_BASE txUnackedTail; // TX tail pointer for data that is not yet acked
    WORD_VAL remotePort; // Remote port number
    WORD_VAL localPort; // Local port number
    WORD remoteWindow; // Remote window size
    WORD wFutureDataSize; // How much out-of-order data has been received

    union {
        NODE_INFO niRemoteMACIP; // 10 bytes for MAC and IP address
        DWORD dwRemoteHost; // RAM or ROM pointer to a hostname string (ex: "www.microchip.com")
    } remote;
    SHORT sHoleSize; // Size of the hole, or -1 for none exists.  (0 indicates hole has just been filled)

    struct {
        unsigned char bFINSent : 1; // A FIN has been sent
        unsigned char bSYNSent : 1; // A SYN has been sent
        unsigned char bRemoteHostIsROM : 1; // Remote host is stored in ROM
        unsigned char bRXNoneACKed1 : 1; // A duplicate ACK was likely received
        unsigned char bRXNoneACKed2 : 1; // A second duplicate ACK was likely received
        unsigned char filler : 3; // future use
    } flags;
    WORD wRemoteMSS; // Maximum Segment Size option advirtised by the remote node during initial handshaking
    BYTE retryCount; // Counter for transmission retries
    BYTE vSocketPurpose;
} TCB;

typedef struct {
    WORD SourcePort; // Local port number
    WORD DestPort; // Remote port number
    DWORD SeqNumber; // Local sequence number
    DWORD AckNumber; // Acknowledging remote sequence number

    struct {
        unsigned char Reserved3 : 4;
        unsigned char Val : 4;
    } DataOffset; // Data offset flags nibble

    union {

        struct {
            unsigned char flagFIN : 1;
            unsigned char flagSYN : 1;
            unsigned char flagRST : 1;
            unsigned char flagPSH : 1;
            unsigned char flagACK : 1;
            unsigned char flagURG : 1;
            unsigned char Reserved2 : 2;
        } bits;
        BYTE byte;
    } Flags; // TCP Flags as defined in RFC

    WORD Window; // Local free RX buffer window
    WORD Checksum; // Data payload checksum
    WORD UrgentPointer; // Urgent pointer
} TCP_HEADER;

typedef struct 
{
    BYTE Kind; // Type of option
    BYTE Length; // Length
    WORD_VAL MaxSegSize; // Maximum segment size
} TCP_OPTIONS; // TCP Options data structure

typedef struct  
{
    IP_ADDR SourceAddress;
    IP_ADDR DestAddress;
    BYTE Zero;
    BYTE Protocol;
    WORD Length;
}PSEUDO_HEADER;

extern UDP_SOCKET activeUDPSocket;
extern UDP_SOCKET_INFO UDPSocketInfo[MAX_UDP_SOCKETS];
extern WORD UDPTxCount;
extern WORD UDPRxCount;

extern TCB MyTCB;
extern TCB_STUB TCBStubs[MAX_TCP_SOCKETS];

//UDP
void UDPSetTxBuffer(WORD wOffset);
void UDPSetRxBuffer(WORD wOffset);
static UDP_SOCKET FindMatchingSocket(UDP_HEADER *h, NODE_INFO *remoteNode);

void UDPInit(void);
WORD UDPIsGetReady(UDP_SOCKET s);
WORD UDPIsPutReady(UDP_SOCKET s);

BOOL UDPGet(BYTE *v);
WORD UDPGetArray(BYTE *cData, WORD wDataLen);
void UDPDiscard(void);
WORD UDPPutArray(BYTE *cData, WORD wDataLen);
BOOL UDPPut(BYTE v);
UDP_SOCKET UDPOpenEx(DWORD remoteHost, BYTE remoteHostType, UDP_PORT localPort, UDP_PORT remotePort);

BOOL UDPProcess(NODE_INFO *remoteNode, IP_ADDR localIP, WORD len);
void UDPTask(void);
void UDPFlush(void);
void UDPClose(UDP_SOCKET s);

BYTE* UDPPutString(BYTE *strData);

//TCP
void TCPInit(void);
void TCPTick(void);
BOOL TCPProcess(NODE_INFO* remote, IP_ADDR localIP, WORD len);
static BOOL FindMatchingSocket_TCP(TCP_HEADER* h, NODE_INFO* remote);
static void SyncTCB(void);
static void TCPRAMCopy(PTR_BASE ptrDest, BYTE vDestType, PTR_BASE ptrSource, BYTE vSourceType, WORD wLength);
static void SwapTCPHeader(TCP_HEADER* header);
static void HandleTCPSeg(TCP_HEADER* h, WORD len);

TCP_SOCKET TCPOpen(DWORD dwRemoteHost, BYTE vRemoteHostType, WORD wPort);
static void SendTCP(BYTE vTCPFlags, BYTE vSendFlags);
void TCPFlush(TCP_SOCKET hTCP);
void TCPDisconnect(TCP_SOCKET hTCP);
static void CloseSocket(void);
void TCPClose(TCP_SOCKET hTCP);

BYTE* TCPPutString(TCP_SOCKET hTCP, BYTE* data);
WORD TCPGetArray(TCP_SOCKET hTCP, BYTE* buffer, WORD len);
WORD TCPPutArray(TCP_SOCKET hTCP, BYTE* data, WORD len);
WORD TCPIsGetReady(TCP_SOCKET hTCP);
WORD TCPIsPutReady(TCP_SOCKET hTCP);
BOOL TCPIsConnected(TCP_SOCKET hTCP);
static WORD GetMaxSegSizeOption(void);

#endif