/*********************************************************************
 *	Ethernet Transport Layer
 *	Author : ALEXIS TRAN
 *
 *	Revision history	:
 *		21/05/2014		- Initial release
 *********************************************************************/

#include "../PLIB.h"

UDP_SOCKET_INFO		UDPSocketInfo[MAX_UDP_SOCKETS];
UDP_SOCKET          activeUDPSocket;
WORD                UDPTxCount;	// Number of bytes written to this UDP segment
WORD                UDPRxCount;	// Number of bytes read from this UDP segment
static UDP_SOCKET	LastPutSocket = INVALID_UDP_SOCKET;	// Indicates the last socket to which data was written
static WORD         wPutOffset;		// Offset from beginning of payload where data is to be written.
static WORD         wGetOffset;		// Offset from beginning of payload from where data is to be read.
static UDP_SOCKET   SocketWithRxData = INVALID_UDP_SOCKET;
/******************************************************************************
 * ---UDPSetTxBuffer
 * This function allows the write location within the TX buffer to be
 * specified.  Future calls to UDPPut, UDPPutArray, UDPPutString, etc will
 * write data from the indicated location.

 ******************************************************************************/
void UDPSetTxBuffer(WORD wOffset)
{
	IPSetTxBuffer(wOffset+sizeof(UDP_HEADER));
	wPutOffset = wOffset;
}

/******************************************************************************
 * ---UDPSetRxBuffer
 * This function allows the read location within the RX buffer to be
 * specified.  Future calls to UDPGet and UDPGetArray will read data from
 * the indicated location forward.
 ******************************************************************************/
void UDPSetRxBuffer(WORD wOffset)
{
	IPSetRxBuffer(wOffset+sizeof(UDP_HEADER));
	wGetOffset = wOffset;
}

/******************************************************************************
 * ---FindMatchingSocket
 * This function attempts to match an incoming UDP segment to a currently
 * active socket for processing.
 ******************************************************************************/
static UDP_SOCKET FindMatchingSocket(UDP_HEADER *h, NODE_INFO *remoteNode)
{
	UDP_SOCKET s;
	UDP_SOCKET partialMatch;
	UDP_SOCKET_INFO *p;

	if(remoteNode->IPAddr.Val == AppConfig.MyIPAddr.Val)
    {
		return INVALID_UDP_SOCKET;
    }

	partialMatch = INVALID_UDP_SOCKET;

	p = UDPSocketInfo;
    for(s = 0; s < MAX_UDP_SOCKETS; s++)
	{
		// This packet is said to be matching with current socket:
		// 1. If its destination port matches with our local port and
		// 2. Packet source IP address matches with previously saved socket remote IP address and
		// 3. Packet source port number matches with previously saved socket remote port number
		if(p->localPort == h->DestinationPort)
		{
			if(p->remotePort == h->SourcePort)
			{
				if(p->remoteNode.IPAddr.Val == remoteNode->IPAddr.Val)
				{
					return s;
				}
			}

			partialMatch = s;
		}
		p++;
	}

	if(partialMatch != INVALID_UDP_SOCKET)
	{
		p = &UDPSocketInfo[partialMatch];

		memcpy((void*)&p->remoteNode, (const void*)remoteNode, sizeof(p->remoteNode) );

		p->remotePort = h->SourcePort;
	}

	return partialMatch;
}

/******************************************************************************
 * ---UDPInit
 *  Initializes the UDP module.  This function initializes all the UDP
 * sockets to the closed state.
 ******************************************************************************/
void UDPInit(void)
{
    UDP_SOCKET s;

    for ( s = 0; s < MAX_UDP_SOCKETS; s++ )
    {
		UDPClose(s);
    }
	Flags.bWasDiscarded = 1;
}

/******************************************************************************
 * ---UDPIsGetReady
 * This function determines if bytes can be read from the specified UDP
 * socket.  It also prepares the UDP module for reading by setting the
 * indicated socket as the currently active connection.
 ******************************************************************************/
WORD UDPIsGetReady(UDP_SOCKET s)
{
    activeUDPSocket = s;
	if(SocketWithRxData != s)
		return 0;

    // If this is the very first time we are accessing this packet,
    // move the read point to the begining of the packet.
    if(Flags.bFirstRead)
    {
        Flags.bFirstRead = 0;
        UDPSetRxBuffer(0);
    }

    return UDPRxCount - wGetOffset;
}

/******************************************************************************
 * ---UDPIsPutReady
 *  This function determines if bytes can be written to the specified UDP
 *  socket.  It also prepares the UDP module for writing by setting the
 *  indicated socket as the currently active connection.
 ******************************************************************************/
WORD UDPIsPutReady(UDP_SOCKET s)
{
	if(!MACIsTxReady())
		return 0;

	if(LastPutSocket != s)
	{
		LastPutSocket = s;
		UDPTxCount = 0;
		UDPSetTxBuffer(0);
	}

	activeUDPSocket = s;

	return MAC_TX_BUFFER_SIZE - sizeof(IP_HEADER) - sizeof(UDP_HEADER) - UDPTxCount;
}

/******************************************************************************
 * ---UDPGet
 * This function reads a single byte from the currently active UDP socket,
 * while decrementing the remaining buffer length.  UDPIsGetReady should be
 * used before calling this function to specify the currently active socket.
 ******************************************************************************/
BOOL UDPGet(BYTE *v)
{
	// Make sure that there is data to return
    if((wGetOffset >= UDPRxCount) || (SocketWithRxData != activeUDPSocket))
        return FALSE;

    *v = MACGet();
    wGetOffset++;

    return TRUE;
}

/******************************************************************************
 * ---UDPGetArray
 * This function reads an array of bytes from the currently active UDP socket,
 * while decrementing the remaining bytes available. UDPIsGetReady should be
 * used before calling this function to specify the currently active socket.
 ******************************************************************************/
WORD UDPGetArray(BYTE *cData, WORD wDataLen)
{
	WORD wBytesAvailable;

	// Make sure that there is data to return
    if((wGetOffset >= UDPRxCount) || (SocketWithRxData != activeUDPSocket))
		return 0;

	// Make sure we don't try to read more data than exists
	wBytesAvailable = UDPRxCount - wGetOffset;
	if(wBytesAvailable < wDataLen)
		wDataLen = wBytesAvailable;

	wDataLen = MACGetArray(cData, wDataLen);
    wGetOffset += wDataLen;

    return wDataLen;
}

/******************************************************************************
 * ---UDPDiscard
 * This function discards any remaining received data in the currently
 * active UDP socket.
 ******************************************************************************/
void UDPDiscard(void)
{
	if(!Flags.bWasDiscarded)
	{
		MACDiscardRx();
		UDPRxCount = 0;
		SocketWithRxData = INVALID_UDP_SOCKET;
		Flags.bWasDiscarded = 1;
	}
}

/******************************************************************************
 * ---UDPPutArray
 * This function writes an array of bytes to the currently active UDP socket,
 * while incrementing the buffer length.  UDPIsPutReady should be used
 * before calling this function to specify the currently active socket.
 ******************************************************************************/
WORD UDPPutArray(BYTE *cData, WORD wDataLen)
{
	WORD wTemp;

	wTemp = (MAC_TX_BUFFER_SIZE - sizeof(IP_HEADER) - sizeof(UDP_HEADER)) - wPutOffset;
	if(wTemp < wDataLen)
    {
		wDataLen = wTemp;
    }

	wPutOffset += wDataLen;
	if(wPutOffset > UDPTxCount)
    {
		UDPTxCount = wPutOffset;
    }

    // Load application data bytes
    MACPutArray(cData, wDataLen);

    return wDataLen;
}

/******************************************************************************
 * ---UDPPut
 * This function writes a single byte to the currently active UDP socket,
 * while incrementing the buffer length.  UDPIsPutReady should be used
 * before calling this function to specify the currently active socket.
 ******************************************************************************/
BOOL UDPPut(BYTE v)
{
	// See if we are out of transmit space.
	if(wPutOffset >= (MAC_TX_BUFFER_SIZE - sizeof(IP_HEADER) - sizeof(UDP_HEADER)))
	{
		return FALSE;
	}

    // Load application data byte
    MACPut(v);
	wPutOffset++;
	if(wPutOffset > UDPTxCount)
    {
		UDPTxCount = wPutOffset;
    }

    return TRUE;
}

/******************************************************************************
 * ---UDPOpenEx
 *  Provides a unified method for opening UDP sockets. This function can open both client and
 *  server   sockets. For client sockets, it can accept a host name string to query in DNS, an IP
 *  address as a string, an IP address in binary form, or a previously resolved NODE_INFO
 *  structure containing the remote IP address and associated MAC address. When a host name
 *  or IP address only is provided, UDP module will internally perform the necessary DNSResolve
 *  and/or ARP resolution steps before reporting that the UDP socket is connected (via a call to
 *  UDPISOpen returning TRUE). Server sockets ignore this destination parameter and listen
 *  only on the indicated port.	Sockets are statically allocated on boot, but can be claimed with
 *  this function and freed using UDPClose.
 ******************************************************************************/
UDP_SOCKET UDPOpenEx(DWORD remoteHost, BYTE remoteHostType, UDP_PORT localPort, UDP_PORT remotePort)
{
	UDP_SOCKET s;
	UDP_SOCKET_INFO *p;

	// Local temp port numbers.
	static WORD NextPort __attribute__((persistent));

	p = UDPSocketInfo;
	for ( s = 0; s < MAX_UDP_SOCKETS; s++ )
	{
		if(p->localPort == INVALID_UDP_PORT)
		{
		 	p->localPort = localPort;
			if(localPort == 0x0000u)
		   	{
                if(NextPort > LOCAL_UDP_PORT_END_NUMBER || NextPort < LOCAL_UDP_PORT_START_NUMBER)
                {
                   NextPort = LOCAL_UDP_PORT_START_NUMBER;
                }
                p->localPort    = NextPort++;
		   	}
			if((remoteHostType == UDP_OPEN_SERVER) || (remoteHost == 0))
			{
                //Set remote node as 0xFF ( broadcast address)
                // else Set broadcast address
                memset((void*)&p->remoteNode, 0xFF, sizeof(p->remoteNode));
                p->smState = UDP_OPENED;
			}
			else
			{
                switch(remoteHostType)
                {
                    case UDP_OPEN_IP_ADDRESS:
                        // remoteHost is a literal IP address.	This doesn't need DNS and can skip directly to the Gateway ARPing step.
                        //Next UDP state UDP_GATEWAY_SEND_ARP;
                        p->remoteNode.IPAddr.Val = remoteHost;
                        p->retryCount = 0;
                        p->retryInterval = (TICK_1S/4)/256;
                        p->smState = UDP_GATEWAY_SEND_ARP;
                        break;
                    case UDP_OPEN_NODE_INFO:
                        //skip DNS and ARP resolution steps if connecting to a remote node which we've already
                        memcpy((void*)(BYTE*)&p->remoteNode,(void*)(BYTE*)(PTR_BASE)remoteHost,sizeof(p->remoteNode));
                        p->smState = UDP_OPENED;
                        // CALL UDPFlushto transmit incluind peding data.
                        break;
                    default:
                        break;
                }
			}
			p->remotePort   = remotePort;

			// Mark this socket as active.
			// Once an active socket is set, subsequent operation can be
			// done without explicitely supply socket identifier.
			activeUDPSocket = s;
			return s;
		}
		p++;
	}

	return (UDP_SOCKET)INVALID_UDP_SOCKET;
}

/******************************************************************************
 * ---UDPProcess
 * This function handles an incoming UDP segment to determine if it is
 * acceptable and should be handed to one of the stack applications for
 * processing.
 ******************************************************************************/
BOOL UDPProcess(NODE_INFO *remoteNode, IP_ADDR localIP, WORD len)
{
    UDP_HEADER		h;
    UDP_SOCKET		s;
    PSEUDO_HEADER	pseudoHeader;
    DWORD_VAL		checksums;

	UDPRxCount = 0;

    // Retrieve UDP header.
    MACGetArray((BYTE*)&h, sizeof(h));

    h.SourcePort        = swap_word(h.SourcePort);
    h.DestinationPort   = swap_word(h.DestinationPort);
    h.Length            = swap_word(h.Length) - sizeof(UDP_HEADER);

	// See if we need to validate the checksum field (0x0000 is disabled)
	if(h.Checksum)
	{
	    // Calculate IP pseudoheader checksum.
	    pseudoHeader.SourceAddress		= remoteNode->IPAddr;
	    pseudoHeader.DestAddress.Val	= localIP.Val;
	    pseudoHeader.Zero				= 0x0;
	    pseudoHeader.Protocol			= IP_PROTOCOLE_UDP;
	    pseudoHeader.Length				= len;

	    SwapPseudoHeader(pseudoHeader);

	    checksums.w[0] = ~CalcIPChecksum((BYTE*)&pseudoHeader, sizeof(pseudoHeader));

	    // Now calculate UDP packet checksum in NIC RAM -- should match pseudoHeader
	    IPSetRxBuffer(0);
	    checksums.w[1] = CalcIPBufferChecksum(len);

	    if(checksums.w[0] != checksums.w[1])
	    {
	        MACDiscardRx();
	        return FALSE;
	    }
	}

    s = FindMatchingSocket(&h, remoteNode);
    if(s == INVALID_UDP_SOCKET)
    {
        // If there is no matching socket, There is no one to handle
        // this data.  Discard it.
        MACDiscardRx();
		return FALSE;
    }
    else
    {
		SocketWithRxData = s;
        UDPRxCount = h.Length;
        Flags.bFirstRead = 1;
		Flags.bWasDiscarded = 0;
    }
    
    return TRUE;
}


/******************************************************************************
 * ---UDPTask
 * This function performs any required periodic UDP tasks.  Each socket's state machine is
 * checked, and any elapsed timeout periods are handled.
 ******************************************************************************/
void UDPTask(void)
{
	UDP_SOCKET ss;

	for ( ss = 0; ss < MAX_UDP_SOCKETS; ss++ )
	{
		if((UDPSocketInfo[ss].smState == UDP_OPENED) || (UDPSocketInfo[ss].smState == UDP_CLOSED))
        {
			continue;
        }
        
		switch(UDPSocketInfo[ss].smState)
		{
            case UDP_GATEWAY_SEND_ARP:
                // Obtain the MAC address associated with the server's IP address
                //(either direct MAC address on same subnet, or the MAC address of the Gateway machine)
                UDPSocketInfo[ss].eventTime = (QWORD)(mGetTick() >> 8);
                ARPResolve(&UDPSocketInfo[ss].remoteNode.IPAddr);
                UDPSocketInfo[ss].smState = UDP_GATEWAY_GET_ARP;
                break;
            case UDP_GATEWAY_GET_ARP:
                if(!ARPIsResolved(&UDPSocketInfo[ss].remoteNode.IPAddr, &UDPSocketInfo[ss].remoteNode.MACAddr))
                {
                    // Time out if too much time is spent in this state
                    // Note that this will continuously send out ARP
                    // requests for an infinite time if the Gateway
                    // never responds
                    if((QWORD)(mGetTick() >> 8) - UDPSocketInfo[ss].eventTime > (QWORD)UDPSocketInfo[ss].retryInterval) 
                    {
                        // Exponentially increase timeout until we reach 6 attempts then stay constant
                        if(UDPSocketInfo[ss].retryCount < 6u)
                        {
                                UDPSocketInfo[ss].retryCount++;
                                UDPSocketInfo[ss].retryInterval <<= 1;
                        }
                        // Retransmit ARP request
                        UDPSocketInfo[ss].smState = UDP_GATEWAY_SEND_ARP;
                    }
                }
                else
                {
                    UDPSocketInfo[ss].smState = UDP_OPENED;
                }
                break;
        }
	}
}

/******************************************************************************
 * ---UDPFlush
 * This function builds a UDP packet with the pending TX data and marks it
 * for transmission over the network interface.  Since UDP is a frame-based
 * protocol, this function must be called before returning to the main
 * stack loop whenever any data is written.
 ******************************************************************************/
void UDPFlush(void)
{
    UDP_HEADER      h;
    UDP_SOCKET_INFO *p;
    WORD wUDPLength;

    p = &UDPSocketInfo[activeUDPSocket];

    wUDPLength = UDPTxCount + sizeof(UDP_HEADER);

	// Generate the correct UDP header
    h.SourcePort        = swap_word(p->localPort);
    h.DestinationPort   = swap_word(p->remotePort);
    h.Length            = swap_word(wUDPLength);
    h.Checksum 			= 0x0000;

	// Position the hardware write pointer where we will need to
	// begin writing the IP header
	MACSetWritePtr(BASE_TX_ADDR + sizeof(ETHER_HEADER));

	// Write IP header to packet
	IPPutHeader(&p->remoteNode, IP_PROTOCOLE_UDP, wUDPLength);

    // Write UDP header to packet
    MACPutArray((BYTE*)&h, sizeof(h));

	// Transmit the packet
    MACFlush();

	// Reset packet size counter for the next TX operation
    UDPTxCount = 0;
	LastPutSocket = INVALID_UDP_SOCKET;
}

/******************************************************************************
 * ---UDPClose
 *  Closes a UDP socket and frees the handle.  Call this function to release
 * a socket and return it to the pool for use by future communications.
 ******************************************************************************/
void UDPClose(UDP_SOCKET s)
{
	if(s >= MAX_UDP_SOCKETS)
    {
		return;
    }

	UDPSocketInfo[s].localPort = INVALID_UDP_PORT;
	UDPSocketInfo[s].remoteNode.IPAddr.Val = 0x00000000;
	UDPSocketInfo[s].smState = UDP_CLOSED;
}

/******************************************************************************
 * ---UDPPutString
 * This function writes a null-terminated string to the currently active UDP socket,
 * while incrementing the buffer length.  UDPIsPutReady should
 * be used before calling this function to specify the currently active socket.
 ******************************************************************************/
BYTE* UDPPutString(BYTE *strData)
{
	return strData + UDPPutArray(strData, strlen((char*)strData));
}
        
TCB                         MyTCB;
TCB_STUB                    TCBStubs[MAX_TCP_SOCKETS];
static TCP_SOCKET           hCurrentTCP = INVALID_SOCKET;
static TCP_SYN_QUEUE        SYNQueue[TCP_SYN_QUEUE_MAX_ENTRIES];	// Array of saved incoming SYN requests that need to be serviced later
static BYTE                 TCPBufferInPIC[TCP_PIC_RAM_SIZE];
static WORD                 NextPort __attribute__((persistent));	// Tracking variable for next local client port number

/******************************************************************************
 * ---TCPInit
 * Initializes the TCP module.  This function sets up the TCP buffers
 * in memory and initializes each socket to the CLOSED state.  If
 * insufficient memory was allocated for the TCP sockets, the function
 * will hang here to be captured by the debugger.
 ******************************************************************************/
void TCPInit(void)
{
	BYTE vSocketsAllocated;
	PTR_BASE ptrBaseAddress;
	PTR_BASE ptrCurrentPICAddress = TCP_PIC_RAM_BASE_ADDRESS;

    if(NextPort == 0u)
    {
        NextPort = (((DWORD)GenerateRandomDWORD()) & 0x07FFu) + LOCAL_PORT_START_NUMBER;
    }

	// Mark all SYN Queue entries as invalid by zeroing the memory
	memset((void*)SYNQueue, 0x00, sizeof(SYNQueue));

	// Allocate all socket FIFO addresses
	for(vSocketsAllocated = 0; vSocketsAllocated < MAX_TCP_SOCKETS; vSocketsAllocated++)
	{
		// Generate all needed sockets of each type (TCP_PURPOSE_*)
		hCurrentTCP = vSocketsAllocated;

        ptrBaseAddress = ptrCurrentPICAddress;
        ptrCurrentPICAddress += sizeof(TCB) + TCP_SOCKET_TX_BUFFER_SIZE+1 + TCP_SOCKET_RX_BUFFER_SIZE+1;
		// Do a sanity check to ensure that we aren't going to use memory that hasn't been allocated to us.
		// If your code locks up right here, it means you've incorrectly allocated your TCP socket buffers.
		while(ptrCurrentPICAddress > TCP_PIC_RAM_BASE_ADDRESS + TCP_PIC_RAM_SIZE);
			
		TCBStubs[hCurrentTCP].bufferTxStart	= ptrBaseAddress + sizeof(TCB);
		TCBStubs[hCurrentTCP].bufferRxStart	= TCBStubs[hCurrentTCP].bufferTxStart + TCP_SOCKET_TX_BUFFER_SIZE + 1;
		TCBStubs[hCurrentTCP].bufferEnd	= TCBStubs[hCurrentTCP].bufferRxStart + TCP_SOCKET_RX_BUFFER_SIZE;
		TCBStubs[hCurrentTCP].smState	= TCP_CLOSED;
		TCBStubs[hCurrentTCP].Flags.bServer	= FALSE;

		SyncTCB();
		CloseSocket();
	}
}

/******************************************************************************
 * ---TCPTick
 * This function performs any required periodic TCP tasks.  Each
 * socket's state machine is checked, and any elapsed timeout periods are handled.
 ******************************************************************************/
void TCPTick(void)
{
	TCP_SOCKET hTCP;
	BOOL bRetransmit;
	BOOL bCloseSocket;
	BYTE vFlags;
	WORD w;

	// Periodically all "not closed" sockets must perform timed operations
	for(hTCP = 0; hTCP < MAX_TCP_SOCKETS; hTCP++)
	{
		hCurrentTCP = hTCP;

		vFlags = 0x00;
		bRetransmit = FALSE;
		bCloseSocket = FALSE;

		// Transmit ASAP data if the medium is available
		if(TCBStubs[hCurrentTCP].Flags.bTXASAP || TCBStubs[hCurrentTCP].Flags.bTXASAPWithoutTimerReset)
		{
			if(MACIsTxReady())
			{
				vFlags = ACK;
				bRetransmit = TCBStubs[hCurrentTCP].Flags.bTXASAPWithoutTimerReset;
			}
		}

		// Perform any needed window updates and data transmissions
		if(TCBStubs[hCurrentTCP].Flags.bTimer2Enabled)
		{
			// See if the timeout has occured, and we need to send a new window update and pending data
			if((SHORT)(TCBStubs[hCurrentTCP].eventTime2 - (QWORD)(mGetTick() >> 8)) <= (SHORT)0)
            {
				vFlags = ACK;
            }
		}

		// Process Delayed ACKnowledgement timer
		if(TCBStubs[hCurrentTCP].Flags.bDelayedACKTimerEnabled)
		{
			// See if the timeout has occured and delayed ACK needs to be sent
			if((SHORT)(TCBStubs[hCurrentTCP].OverlappedTimers.delayedACKTime - (QWORD)(mGetTick() >> 8)) <= (SHORT)0)
            {
				vFlags = ACK;
            }
		}

		// Process TCP_CLOSE_WAIT timer
		if(TCBStubs[hCurrentTCP].smState == TCP_CLOSE_WAIT)
		{
			// Automatically close the socket on our end if the application
			// fails to call TCPDisconnect() is a reasonable amount of time.
			if((SHORT)(TCBStubs[hCurrentTCP].OverlappedTimers.closeWaitTime - (QWORD)(mGetTick() >> 8)) <= (SHORT)0)
			{
				vFlags = FIN | ACK;
				TCBStubs[hCurrentTCP].smState = TCP_LAST_ACK;
			}
		}

		// Process listening server sockets that might have a SYN waiting in the SYNQueue[]
        if(TCBStubs[hCurrentTCP].smState == TCP_LISTEN)
        {
            for(w = 0; w < TCP_SYN_QUEUE_MAX_ENTRIES; w++)
            {
                // Abort search if there are no more valid records
                if(SYNQueue[w].wDestPort == 0u)
                    break;

                if(SYNQueue[w].wDestPort == TCBStubs[hCurrentTCP].remoteHash.Val)
                {
                    // Set up our socket and generate a reponse SYN+ACK packet
                    SyncTCB();

                    memcpy((void*)&MyTCB.remote.niRemoteMACIP, (void*)&SYNQueue[w].niSourceAddress, sizeof(NODE_INFO));
                    MyTCB.remotePort.Val = SYNQueue[w].wSourcePort;
                    MyTCB.RemoteSEQ = SYNQueue[w].dwSourceSEQ + 1;
                    TCBStubs[hCurrentTCP].remoteHash.Val = (MyTCB.remote.niRemoteMACIP.IPAddr.w[1] + MyTCB.remote.niRemoteMACIP.IPAddr.w[0] + MyTCB.remotePort.Val) ^ MyTCB.localPort.Val;
                    vFlags = SYN | ACK;
                    TCBStubs[hCurrentTCP].smState = TCP_SYN_RECEIVED;

                    // Delete this SYN from the SYNQueue and compact the SYNQueue[] array
                    TCPRAMCopy((PTR_BASE)&SYNQueue[w], TCP_PIC_RAM, (PTR_BASE)&SYNQueue[w+1], TCP_PIC_RAM, (TCP_SYN_QUEUE_MAX_ENTRIES-1u-w)*sizeof(TCP_SYN_QUEUE));
                    SYNQueue[TCP_SYN_QUEUE_MAX_ENTRIES-1].wDestPort = 0u;

                    break;
                }
            }
        }
		if(vFlags)
			SendTCP(vFlags, bRetransmit ? 0 : SENDTCP_RESET_TIMERS);

		// The TCP_CLOSED, TCP_LISTEN, and sometimes the TCP_ESTABLISHED
		// state don't need any timeout events, so see if the timer is enabled
		if(!TCBStubs[hCurrentTCP].Flags.bTimerEnabled)
		{
			#if defined(TCP_KEEP_ALIVE_TIMEOUT)
				// Only the established state has any use for keep-alives
				if(TCBStubs[hCurrentTCP].smState == TCP_ESTABLISHED)
				{
					// If timeout has not occured, do not do anything.
					if((LONG)(mGetTick() - TCBStubs[hCurrentTCP].eventTime) < (LONG)0)
						continue;

					// If timeout has occured and the connection appears to be dead (no
					// responses from remote node at all), close the connection so the
					// application doesn't sit around indefinitely with a useless socket
					// that it thinks is still open
					if(TCBStubs[hCurrentTCP].Flags.vUnackedKeepalives == TCP_MAX_UNACKED_KEEP_ALIVES)
					{
						vFlags = TCBStubs[hCurrentTCP].Flags.bServer;

						// Force an immediate FIN and RST transmission
						// Double calling TCPDisconnect() will also place us
						// back in the listening state immediately if a server socket.
						TCPDisconnect(hTCP);
                                                TCPDisconnect(hTCP);

						// Prevent client mode sockets from getting reused by other applications.
						// The application must call TCPDisconnect() with the handle to free this
						// socket (and the handle associated with it)
						if(!vFlags)
							TCBStubs[hCurrentTCP].smState = TCP_CLOSED_BUT_RESERVED;

						continue;
					}

					// Otherwise, if a timeout occured, simply send a keep-alive packet
					SyncTCB();
					SendTCP(ACK, SENDTCP_KEEP_ALIVE);
					TCBStubs[hCurrentTCP].eventTime = mGetTick() + TCP_KEEP_ALIVE_TIMEOUT;
				}
			#endif
			continue;
		}

		// If timeout has not occured, do not do anything.
		if((LONG)(mGetTick() - TCBStubs[hCurrentTCP].eventTime) < (LONG)0)
			continue;

		// Load up extended TCB information
		SyncTCB();

		// A timeout has occured.  Respond to this timeout condition
		// depending on what state this socket is in.
		switch(TCBStubs[hCurrentTCP].smState)
		{
                    case TCP_GATEWAY_SEND_ARP:
                        // Obtain the MAC address associated with the server's IP address (either direct MAC address on same subnet, or the MAC address of the Gateway machine)
                        TCBStubs[hCurrentTCP].eventTime2 = (QWORD)(mGetTick() >> 8);
                        ARPResolve(&MyTCB.remote.niRemoteMACIP.IPAddr);
                        TCBStubs[hCurrentTCP].smState = TCP_GATEWAY_GET_ARP;
                        break;
                    case TCP_GATEWAY_GET_ARP:
                        // Wait for the MAC address to finish being obtained
                        if(!ARPIsResolved(&MyTCB.remote.niRemoteMACIP.IPAddr, &MyTCB.remote.niRemoteMACIP.MACAddr))
                        {
                            // Time out if too much time is spent in this state
                            // Note that this will continuously send out ARP
                            // requests for an infinite time if the Gateway
                            // never responds
                            if((QWORD)(mGetTick() >> 8) - TCBStubs[hCurrentTCP].eventTime2 > (QWORD)MyTCB.retryInterval)
                            {
                                // Exponentially increase timeout until we reach 6 attempts then stay constant
                                if(MyTCB.retryCount < 6u)
                                {
                                    MyTCB.retryCount++;
                                    MyTCB.retryInterval <<= 1;
                                }
                                // Retransmit ARP request
                                TCBStubs[hCurrentTCP].smState = TCP_GATEWAY_SEND_ARP;
                            }
                            break;
                        }
                        // Send out SYN connection request to remote node
                        // This automatically disables the Timer from
                        // continuously firing for this socket
                        vFlags = SYN;
                        bRetransmit = FALSE;
                        TCBStubs[hCurrentTCP].smState = TCP_SYN_SENT;
                        break;
                    case TCP_SYN_SENT:
                        // Keep sending SYN until we hear from remote node.
                        // This may be for infinite time, in that case
                        // caller must detect it and do something.
                        vFlags = SYN;
                        bRetransmit = TRUE;

                        // Exponentially increase timeout until we reach TCP_MAX_RETRIES attempts then stay constant
                        if(MyTCB.retryCount >= (TCP_MAX_RETRIES - 1))
                        {
                            MyTCB.retryCount = TCP_MAX_RETRIES - 1;
                            MyTCB.retryInterval = TCP_START_TIMEOUT_VAL<<(TCP_MAX_RETRIES-1);
                        }
                        break;
                    case TCP_SYN_RECEIVED:
                        // We must receive ACK before timeout expires.
                        // If not, resend SYN+ACK.
                        // Abort, if maximum attempts counts are reached.
                        if(MyTCB.retryCount < TCP_MAX_SYN_RETRIES)
                        {
                            vFlags = SYN | ACK;
                            bRetransmit = TRUE;
                        }
                        else
                        {
                            if(TCBStubs[hCurrentTCP].Flags.bServer)
                            {
                                    vFlags = RST | ACK;
                                    bCloseSocket = TRUE;
                            }
                            else
                            {
                                    vFlags = SYN;
                            }
                        }
                        break;
                    case TCP_ESTABLISHED:
                    case TCP_CLOSE_WAIT:
                        // Retransmit any unacknowledged data
                        if(MyTCB.retryCount < TCP_MAX_RETRIES)
                        {
                            vFlags = ACK;
                            bRetransmit = TRUE;
                        }
                        else
                        {
                            // No response back for too long, close connection
                            // This could happen, for instance, if the communication
                            // medium was lost
                            TCBStubs[hCurrentTCP].smState = TCP_FIN_WAIT_1;
                            vFlags = FIN | ACK;
                        }
                        break;
                    case TCP_FIN_WAIT_1:
                        if(MyTCB.retryCount < TCP_MAX_RETRIES)
                        {
                            // Send another FIN
                            vFlags = FIN | ACK;
                            bRetransmit = TRUE;
                        }
                        else
                        {
                            // Close on our own, we can't seem to communicate
                            // with the remote node anymore
                            vFlags = RST | ACK;
                            bCloseSocket = TRUE;
                        }
                        break;
                    case TCP_FIN_WAIT_2:
                        // Close on our own, we can't seem to communicate
                        // with the remote node anymore
                        vFlags = RST | ACK;
                        bCloseSocket = TRUE;
                        break;
                    case TCP_CLOSING:
                        if(MyTCB.retryCount < TCP_MAX_RETRIES)
                        {
                            // Send another ACK+FIN (the FIN is retransmitted
                            // automatically since it hasn't been acknowledged by
                            // the remote node yet)
                            vFlags = ACK;
                            bRetransmit = TRUE;
                        }
                        else
                        {
                            // Close on our own, we can't seem to communicate
                            // with the remote node anymore
                            vFlags = RST | ACK;
                            bCloseSocket = TRUE;
                        }
                        break;
                    case TCP_LAST_ACK:
                        // Send some more FINs or close anyway
                        if(MyTCB.retryCount < TCP_MAX_RETRIES)
                        {
                            vFlags = FIN | ACK;
                            bRetransmit = TRUE;
                        }
                        else
                        {
                            vFlags = RST | ACK;
                            bCloseSocket = TRUE;
                        }
                        break;
                    default:
                        break;
		}

		if(vFlags)
		{
                    // Transmit all unacknowledged data over again
                    if(bRetransmit)
                    {
                        // Set the appropriate retry time
                        MyTCB.retryCount++;
                        MyTCB.retryInterval <<= 1;

                        // Calculate how many bytes we have to roll back and retransmit
                        w = MyTCB.txUnackedTail - TCBStubs[hCurrentTCP].txTail;
                        if(MyTCB.txUnackedTail < TCBStubs[hCurrentTCP].txTail)
                                w += TCBStubs[hCurrentTCP].bufferRxStart - TCBStubs[hCurrentTCP].bufferTxStart;

                        // Perform roll back of local SEQuence counter, remote window
                        // adjustment, and cause all unacknowledged data to be
                        // retransmitted by moving the unacked tail pointer.
                        MyTCB.MySEQ -= w;
                        MyTCB.remoteWindow += w;
                        MyTCB.txUnackedTail = TCBStubs[hCurrentTCP].txTail;
                        SendTCP(vFlags, 0);
                    }
                    else
                    {
                        SendTCP(vFlags, SENDTCP_RESET_TIMERS);
                    }
		}

		if(bCloseSocket)
                {
			CloseSocket();
                }
	}

    // Process SYN Queue entry timeouts
    for(w = 0; w < TCP_SYN_QUEUE_MAX_ENTRIES; w++)
    {
        // Abort search if there are no more valid records
        if(SYNQueue[w].wDestPort == 0u)
            break;

        // See if this SYN has timed out
        if((QWORD)(mGetTick() >> 8) - SYNQueue[w].wTimestamp > (WORD)(TCP_SYN_QUEUE_TIMEOUT >> 8))
        {
            // Delete this SYN from the SYNQueue and compact the SYNQueue[] array
            TCPRAMCopy((PTR_BASE)&SYNQueue[w], TCP_PIC_RAM, (PTR_BASE)&SYNQueue[w+1], TCP_PIC_RAM, (TCP_SYN_QUEUE_MAX_ENTRIES-1u-w)*sizeof(TCP_SYN_QUEUE));
            SYNQueue[TCP_SYN_QUEUE_MAX_ENTRIES-1].wDestPort = 0u;

            // Since we deleted an entry, we need to roll back one
            // index so next loop will process the correct record
            w--;
        }
    }
}

/******************************************************************************
 * ---TCPProcess
 * This function handles incoming TCP segments.  When a segment arrives, it
 * is compared to open sockets using a hash of the remote port and IP.
 * On a match, the data is passed to HandleTCPSeg for further processing.
 ******************************************************************************/
BOOL TCPProcess(NODE_INFO* remote, IP_ADDR localIP, WORD len)
{
	TCP_HEADER      TCPHeader;
	PSEUDO_HEADER   pseudoHeader;
	WORD_VAL        checksum1;
	WORD_VAL        checksum2;
	BYTE            optionsSize;

	// Calculate IP pseudoheader checksum.
	pseudoHeader.SourceAddress      = remote->IPAddr;
	pseudoHeader.DestAddress        = localIP;
	pseudoHeader.Zero               = 0x0;
	pseudoHeader.Protocol           = IP_PROTOCOLE_TCP;
	pseudoHeader.Length          	= len;

	SwapPseudoHeader(pseudoHeader);

	checksum1.Val = ~CalcIPChecksum((BYTE*)&pseudoHeader,
		sizeof(pseudoHeader));

	// Now calculate TCP packet checksum in NIC RAM - should match
	// pesudo header checksum
	checksum2.Val = CalcIPBufferChecksum(len);

	// Compare checksums.
	if(checksum1.Val != checksum2.Val)
	{
		MACDiscardRx();
		return TRUE;
	}

	// Retrieve TCP header.
	IPSetRxBuffer(0);
	MACGetArray((BYTE*)&TCPHeader, sizeof(TCPHeader));
	SwapTCPHeader(&TCPHeader);


	// Skip over options to retrieve data bytes
	optionsSize = (BYTE)((TCPHeader.DataOffset.Val << 2)-
		sizeof(TCPHeader));
	len = len - optionsSize - sizeof(TCPHeader);

	// Find matching socket.
	if(FindMatchingSocket_TCP(&TCPHeader, remote))
	{
		HandleTCPSeg(&TCPHeader, len);
	}
//	else
//	{
//		// NOTE: RFC 793 specifies that if the socket is closed and a segment
//		// arrives, we should send back a RST if the RST bit in the incoming
//		// packet is not set.  Instead, we will just silently ignore such a
//		// packet since this is what firewalls do on purpose to enhance
//		// security.
//		//if(!TCPHeader.Flags.bits.flagRST)
//		//	SendTCP(RST, SENDTCP_RESET_TIMERS);
//	}

	// Finished with this packet, discard it and free the Ethernet RAM for new packets
	MACDiscardRx();

	return TRUE;
}


/******************************************************************************
 * ---FindMatchingSocket
 * This function searches through the sockets and attempts to match one with
 * a given TCP header and NODE_INFO structure.  If a socket is found, its
 * index is saved in hCurrentTCP and the associated TCBStubs[hCurrentTCP] and MyTCB are
 * loaded. Otherwise, INVALID_SOCKET is placed in hCurrentTCP.
 ******************************************************************************/
static BOOL FindMatchingSocket_TCP(TCP_HEADER* h, NODE_INFO* remote)
{
	TCP_SOCKET hTCP;
	TCP_SOCKET partialMatch;
	WORD hash;

	// Prevent connections on invalid port 0
	if(h->DestPort == 0u)
		return FALSE;

	partialMatch = INVALID_SOCKET;
	hash = (remote->IPAddr.w[1]+remote->IPAddr.w[0] + h->SourcePort) ^ h->DestPort;

	// Loop through all sockets looking for a socket that is expecting this
	// packet or can handle it.
	for(hTCP = 0; hTCP < MAX_TCP_SOCKETS; hTCP++ )
	{
		hCurrentTCP = hTCP;

		if(TCBStubs[hCurrentTCP].smState == TCP_CLOSED)
		{
			continue;
		}
		else if(TCBStubs[hCurrentTCP].smState == TCP_LISTEN)
		{// For listening ports, check if this is the correct port
			if(TCBStubs[hCurrentTCP].remoteHash.Val == h->DestPort)
				partialMatch = hTCP;

			continue;
		}
		else if(TCBStubs[hCurrentTCP].remoteHash.Val != hash)
		{// Ignore if the hash doesn't match
			continue;
		}

		SyncTCB();
		if(	h->DestPort == MyTCB.localPort.Val &&
			h->SourcePort == MyTCB.remotePort.Val &&
			remote->IPAddr.Val == MyTCB.remote.niRemoteMACIP.IPAddr.Val)
		{
			return TRUE;
		}
	}


	// If there is a partial match, then a listening socket is currently
	// available.  Set up the extended TCB with the info needed
	// to establish a connection and return this socket to the
	// caller.
	if(partialMatch != INVALID_SOCKET)
	{
        hCurrentTCP = partialMatch;
		SyncTCB();


		// Make sure the above check didn't fail (this is unfortunately
		// redundant for non-SSL sockets).  Otherwise, fall out to below
		// and add to the SYN queue.
		if(partialMatch != INVALID_SOCKET)
		{
			TCBStubs[hCurrentTCP].remoteHash.Val = hash;

			memcpy((void*)&MyTCB.remote, (void*)remote, sizeof(NODE_INFO));
			MyTCB.remotePort.Val = h->SourcePort;
			MyTCB.localPort.Val = h->DestPort;
			MyTCB.txUnackedTail	= TCBStubs[hCurrentTCP].bufferTxStart;

			// All done, and we have a match
			return TRUE;
		}
	}

	// No available sockets are listening on this port.  (Or, for
	// SSL requests, perhaps no SSL sessions were available.  However,
	// there may be a server socket which is currently busy but
	// could handle this packet, so we should check.
	{
		WORD wQueueInsertPos;

		// See if this is a SYN packet
		if(!h->Flags.bits.flagSYN)
			return FALSE;

		// See if there is space in our SYN queue
		if(SYNQueue[TCP_SYN_QUEUE_MAX_ENTRIES-1].wDestPort)
			return FALSE;

		// See if we have this SYN already in our SYN queue.
		// If not already in the queue, find out where we
		// should insert this SYN to the queue
		for(wQueueInsertPos = 0; wQueueInsertPos < TCP_SYN_QUEUE_MAX_ENTRIES; wQueueInsertPos++)
		{
			// Exit loop if we found a free record
			if(SYNQueue[wQueueInsertPos].wDestPort == 0u)
				break;

			// Check if this SYN packet is already in the SYN queue
			if(SYNQueue[wQueueInsertPos].wDestPort != h->DestPort)
				continue;
			if(SYNQueue[wQueueInsertPos].wSourcePort != h->SourcePort)
				continue;
			if(SYNQueue[wQueueInsertPos].niSourceAddress.IPAddr.Val != remote->IPAddr.Val)
				continue;

			// SYN matches SYN queue entry.  Update timestamp and do nothing.
			SYNQueue[wQueueInsertPos].wTimestamp = (QWORD)(mGetTick() >> 8);
			return FALSE;
		}

		// Check to see if we have any server sockets which
		// are currently connected, but could handle this SYN
		// request at a later time if the client disconnects.
		for(hTCP = 0; hTCP < MAX_TCP_SOCKETS; hTCP++)
		{
            hCurrentTCP = hTCP;
			if(!TCBStubs[hCurrentTCP].Flags.bServer)
				continue;

			SyncTCB();
            
			if(MyTCB.localPort.Val != h->DestPort)
				continue;

			// Generate the SYN queue entry
			memcpy((void*)&SYNQueue[wQueueInsertPos].niSourceAddress, (void*)remote, sizeof(NODE_INFO));
			SYNQueue[wQueueInsertPos].wSourcePort = h->SourcePort;
			SYNQueue[wQueueInsertPos].dwSourceSEQ = h->SeqNumber;
			SYNQueue[wQueueInsertPos].wDestPort = h->DestPort;
			SYNQueue[wQueueInsertPos].wTimestamp = (QWORD)(mGetTick() >> 8);

			return FALSE;
		}
	}

	return FALSE;

}

static void SyncTCB(void)
{
	static TCP_SOCKET hLastTCB = INVALID_SOCKET;

	if(hLastTCB == hCurrentTCP)
		return;

	if(hLastTCB != INVALID_SOCKET)
	{
		// Save the current TCB
		TCPRAMCopy(TCBStubs[hLastTCB].bufferTxStart - sizeof(MyTCB), TCP_PIC_RAM, (PTR_BASE)&MyTCB, TCP_PIC_RAM, sizeof(MyTCB));
	}

	// Load up the new TCB
	hLastTCB = hCurrentTCP;
	TCPRAMCopy((PTR_BASE)&MyTCB, TCP_PIC_RAM, TCBStubs[hCurrentTCP].bufferTxStart - sizeof(MyTCB), TCP_PIC_RAM, sizeof(MyTCB));

}

/******************************************************************************
 * ---TCPRAMCopy
 * This function copies data between memory mediums (PIC RAM, SPI, RAM, and Ethernet buffer RAM).
 ******************************************************************************/
static void TCPRAMCopy(PTR_BASE ptrDest, BYTE vDestType, PTR_BASE ptrSource, BYTE vSourceType, WORD wLength)
{

	switch(vSourceType)
	{
		case TCP_PIC_RAM:
			switch(vDestType)
			{
				case TCP_PIC_RAM:
					memcpy((void*)(BYTE*)ptrDest, (void*)(BYTE*)ptrSource, wLength);
					break;
                case TCP_ETH_RAM:
					if(ptrDest!=(PTR_BASE)-1)
                    {
						MACSetWritePtr(ptrDest);
                    }
					MACPutArray((BYTE*)ptrSource, wLength);
					break;
			}
			break;
		case TCP_ETH_RAM:
			switch(vDestType)
			{
				case TCP_PIC_RAM:
					if(ptrSource!=(PTR_BASE)-1)
                    {
						MACSetReadPtr(ptrSource);
                    }
					MACGetArray((BYTE*)ptrDest, wLength);
					break;
				case TCP_ETH_RAM:
					MACMemCopyAsync(ptrDest, ptrSource, wLength);
					break;
			}
			break;
	}
}

/******************************************************************************
 * ---SwapTCPHeader
 * This function swaps the endian-ness of a given TCP header for comparison.
 ******************************************************************************/
static void SwapTCPHeader(TCP_HEADER* header)
{
	header->SourcePort      = swap_word(header->SourcePort);
	header->DestPort        = swap_word(header->DestPort);
	header->SeqNumber       = swap_dword(header->SeqNumber);
	header->AckNumber       = swap_dword(header->AckNumber);
	header->Window          = swap_word(header->Window);
	header->Checksum        = swap_word(header->Checksum);
	header->UrgentPointer   = swap_word(header->UrgentPointer);
}

/*****************************************************************************
 * ---HandleTCPSeg
 * This function handles incoming TCP segments.  When a segment arrives, it
 * is compared to open sockets using a hash of the remote port and IP.
 * On a match, the data is passed to HandleTCPSeg for further processing.
 ******************************************************************************/
static void HandleTCPSeg(TCP_HEADER* h, WORD len)
{
	DWORD dwTemp;
	PTR_BASE wTemp;
	LONG lMissingBytes;
	WORD wMissingBytes;
	WORD wFreeSpace;
	BYTE localHeaderFlags;
	DWORD localAckNumber;
	DWORD localSeqNumber;
	WORD wSegmentLength;
	BOOL bSegmentAcceptable;
	WORD wNewWindow;


	// Cache a few variables in local RAM.
	// PIC18s take a fair amount of code and execution time to
	// dereference pointers frequently.
	localHeaderFlags = h->Flags.byte;
	localAckNumber = h->AckNumber;
	localSeqNumber = h->SeqNumber;

	// We received a packet, reset the keep alive timer and count
	#if defined(TCP_KEEP_ALIVE_TIMEOUT)
		TCBStubs[hCurrentTCP].Flags.vUnackedKeepalives = 0;
		if(!TCBStubs[hCurrentTCP].Flags.bTimerEnabled)
			TCBStubs[hCurrentTCP].eventTime = (mGetTick() >> 8) + TCP_KEEP_ALIVE_TIMEOUT;
	#endif

	// Handle TCP_LISTEN and TCP_SYN_SENT states
	// Both of these states will return, so code following this
	// state machine need not check explicitly for these two
	// states.
	switch(TCBStubs[hCurrentTCP].smState)
	{
		case TCP_LISTEN:
			// First: check RST flag
			if(localHeaderFlags & RST)
			{
				CloseSocket();	// Unbind remote IP address/port info
				return;
			}

			// Second: check ACK flag, which would be invalid
			if(localHeaderFlags & ACK)
			{
				// Use a believable sequence number and reset the remote node
				MyTCB.MySEQ = localAckNumber;
				SendTCP(RST, 0);
				CloseSocket();	// Unbind remote IP address/port info
				return;
			}

			// Third: check for SYN flag, which is what we're looking for
			if(localHeaderFlags & SYN)
			{
				// We now have a sequence number for the remote node
				MyTCB.RemoteSEQ = localSeqNumber + 1;

				// Get MSS option
				MyTCB.wRemoteMSS = GetMaxSegSizeOption();

				// Set Initial Send Sequence (ISS) number
				// Nothing to do on this step... ISS already set in CloseSocket()

				// Respond with SYN + ACK
				SendTCP(SYN | ACK, SENDTCP_RESET_TIMERS);
				TCBStubs[hCurrentTCP].smState = TCP_SYN_RECEIVED;
			}
			else
			{
				CloseSocket();	// Unbind remote IP address/port info
			}

			// Fourth: check for other text and control
			// Nothing to do since we don't support this
			return;

		case TCP_SYN_SENT:
			// Second: check the RST bit
			// This is out of order because this stack has no API for
			// notifying the application that the connection seems to
			// be failing.  Instead, the application must time out and
			// the stack will just keep trying in the mean time.
			if(localHeaderFlags & RST)
				return;

			// First: check ACK bit
			if(localHeaderFlags & ACK)
			{
				if(localAckNumber != MyTCB.MySEQ)
				{
					// Send a RST packet with SEQ = SEG.ACK, but retain our SEQ
					// number for arivial of any other SYN+ACK packets
					localSeqNumber = MyTCB.MySEQ;	// Save our original SEQ number
					MyTCB.MySEQ = localAckNumber;	// Set SEQ = SEG.ACK
					SendTCP(RST, SENDTCP_RESET_TIMERS);		// Send the RST
					MyTCB.MySEQ = localSeqNumber;	// Restore original SEQ number
					return;
				}
			}

			// Third: check the security and precedence
			// No such feature in this stack.  We want to accept all connections.

			// Fourth: check the SYN bit
			if(localHeaderFlags & SYN)
			{
				// We now have an initial sequence number and window size
				MyTCB.RemoteSEQ = localSeqNumber + 1;
				MyTCB.remoteWindow = h->Window;

				// Get MSS option
				MyTCB.wRemoteMSS = GetMaxSegSizeOption();

				if(localHeaderFlags & ACK)
				{
					SendTCP(ACK, SENDTCP_RESET_TIMERS);
					TCBStubs[hCurrentTCP].smState = TCP_ESTABLISHED;
					// Set up keep-alive timer
					#if defined(TCP_KEEP_ALIVE_TIMEOUT)
						TCBStubs[hCurrentTCP].eventTime = (mGetTick() >> 8) + TCP_KEEP_ALIVE_TIMEOUT;
					#endif
					TCBStubs[hCurrentTCP].Flags.bTimerEnabled = 0;
				}
				else
				{
					SendTCP(SYN | ACK, SENDTCP_RESET_TIMERS);
					TCBStubs[hCurrentTCP].smState = TCP_SYN_RECEIVED;
				}
			}

			// Fifth: drop the segment if neither SYN or RST is set
			return;

		default:
			break;
	}

	//
	// First: check the sequence number
	//
	wSegmentLength = len;
	if(localHeaderFlags & FIN)
		wSegmentLength++;
	if(localHeaderFlags & SYN)
		wSegmentLength++;

	// Calculate the RX FIFO space
	if(TCBStubs[hCurrentTCP].rxHead >= TCBStubs[hCurrentTCP].rxTail)
		wFreeSpace = (TCBStubs[hCurrentTCP].bufferEnd - TCBStubs[hCurrentTCP].bufferRxStart) - (TCBStubs[hCurrentTCP].rxHead - TCBStubs[hCurrentTCP].rxTail);
	else
		wFreeSpace = TCBStubs[hCurrentTCP].rxTail - TCBStubs[hCurrentTCP].rxHead - 1;

	// Calculate the number of bytes ahead of our head pointer this segment skips
	lMissingBytes = localSeqNumber - MyTCB.RemoteSEQ;
	wMissingBytes = (WORD)lMissingBytes;

	// Run TCP acceptability tests to verify that this packet has a valid sequence number
	bSegmentAcceptable = FALSE;
	if(wSegmentLength)
	{
		// Check to see if we have free space, and if so, if any of the data falls within the freespace
		if(wFreeSpace)
		{
			// RCV.NXT =< SEG.SEQ < RCV.NXT+RCV.WND
			if((lMissingBytes >= (LONG)0) && (wFreeSpace > (DWORD)lMissingBytes))
				bSegmentAcceptable = TRUE;
			else
			{
				// RCV.NXT =< SEG.SEQ+SEG.LEN-1 < RCV.NXT+RCV.WND
				if((lMissingBytes + (LONG)wSegmentLength > (LONG)0) && (lMissingBytes <= (LONG)(SHORT)(wFreeSpace - wSegmentLength)))
					bSegmentAcceptable = TRUE;
			}

			if((lMissingBytes < (LONG)wFreeSpace) && ((SHORT)wMissingBytes + (SHORT)wSegmentLength > (SHORT)0))
				bSegmentAcceptable = TRUE;
		}
		// Segments with data are not acceptable if we have no free space
	}
	else
	{
		// Zero length packets are acceptable if they fall within our free space window
		// SEG.SEQ = RCV.NXT
		if(lMissingBytes == 0)
		{
			bSegmentAcceptable = TRUE;
		}
		else
		{
			// RCV.NXT =< SEG.SEQ < RCV.NXT+RCV.WND
			if((lMissingBytes >= (LONG)0) && (wFreeSpace > (DWORD)lMissingBytes))
				bSegmentAcceptable = TRUE;
		}
	}

	if(!bSegmentAcceptable)
	{
		// Unacceptable segment, drop it and respond appropriately
		if(!(localHeaderFlags & RST))
			SendTCP(ACK, SENDTCP_RESET_TIMERS);
		return;
	}


	//
	// Second: check the RST bit
	//
	//
	// Fourth: check the SYN bit
	//
	// Note, that since the third step is not implemented, we can
	// combine this second and fourth step into a single operation.
	if(localHeaderFlags & (RST | SYN))
	{
		CloseSocket();
		return;
	}

	//
	// Third: check the security and precedence
	//
	// Feature not supported.  Let's process this segment.

	//
	// Fifth: check the ACK bit
	//
	if(!(localHeaderFlags & ACK))
		return;

	switch(TCBStubs[hCurrentTCP].smState)
	{
		case TCP_SYN_RECEIVED:
			if(localAckNumber != MyTCB.MySEQ)
			{
				// Send a RST packet with SEQ = SEG.ACK, but retain our SEQ
				// number for arivial of any other correct packets
				localSeqNumber = MyTCB.MySEQ;	// Save our original SEQ number
				MyTCB.MySEQ = localAckNumber;	// Set SEQ = SEG.ACK
				SendTCP(RST, SENDTCP_RESET_TIMERS);		// Send the RST
				MyTCB.MySEQ = localSeqNumber;	// Restore original SEQ number
				return;
			}
			TCBStubs[hCurrentTCP].smState = TCP_ESTABLISHED;
			// No break

		case TCP_ESTABLISHED:
		case TCP_FIN_WAIT_1:
		case TCP_FIN_WAIT_2:
		case TCP_CLOSE_WAIT:
		case TCP_CLOSING:
			// Calculate what the highest possible SEQ number in our TX FIFO is
			wTemp = TCBStubs[hCurrentTCP].txHead - MyTCB.txUnackedTail;
			if((SHORT)wTemp < (SHORT)0)
				wTemp += TCBStubs[hCurrentTCP].bufferRxStart - TCBStubs[hCurrentTCP].bufferTxStart;
			dwTemp = MyTCB.MySEQ + (DWORD)wTemp;

			// Drop the packet if it ACKs something we haven't sent
            dwTemp = (LONG)localAckNumber - (LONG)dwTemp;
            if((LONG)dwTemp > 0)
            {   // acknowledged more than we've sent??
                if(!MyTCB.flags.bFINSent || dwTemp != 1)
                {
                    SendTCP(ACK, 0);
                    return;
                }
                else
                {
                    localAckNumber--;   // since we don't count the FIN anyway
                }
            }

			// Throw away all ACKnowledged TX data:
			// Calculate what the last acknowledged sequence number was (ignoring any FINs we sent)
			dwTemp = MyTCB.MySEQ - (LONG)(SHORT)(MyTCB.txUnackedTail - TCBStubs[hCurrentTCP].txTail);
			if(MyTCB.txUnackedTail < TCBStubs[hCurrentTCP].txTail)
				dwTemp -= TCBStubs[hCurrentTCP].bufferRxStart - TCBStubs[hCurrentTCP].bufferTxStart;

			// Calcluate how many bytes were ACKed with this packet
			dwTemp = localAckNumber - dwTemp;
			if(((LONG)(dwTemp) > (LONG)0) && (dwTemp <= TCBStubs[hCurrentTCP].bufferRxStart - TCBStubs[hCurrentTCP].bufferTxStart))
			{
				MyTCB.flags.bRXNoneACKed1 = 0;
				MyTCB.flags.bRXNoneACKed2 = 0;
				TCBStubs[hCurrentTCP].Flags.bHalfFullFlush = FALSE;

				// Bytes ACKed, free up the TX FIFO space
				wTemp = TCBStubs[hCurrentTCP].txTail;
				TCBStubs[hCurrentTCP].txTail += dwTemp;
				if(MyTCB.txUnackedTail >= wTemp)
				{
					if(MyTCB.txUnackedTail < TCBStubs[hCurrentTCP].txTail)
					{
						MyTCB.MySEQ += TCBStubs[hCurrentTCP].txTail - MyTCB.txUnackedTail;
						MyTCB.txUnackedTail = TCBStubs[hCurrentTCP].txTail;
					}
				}
				else
				{
					wTemp = MyTCB.txUnackedTail + (TCBStubs[hCurrentTCP].bufferRxStart - TCBStubs[hCurrentTCP].bufferTxStart);
					if(wTemp < TCBStubs[hCurrentTCP].txTail)
					{
						MyTCB.MySEQ += TCBStubs[hCurrentTCP].txTail - wTemp;
						MyTCB.txUnackedTail = TCBStubs[hCurrentTCP].txTail;
					}
				}
				if(TCBStubs[hCurrentTCP].txTail >= TCBStubs[hCurrentTCP].bufferRxStart)
					TCBStubs[hCurrentTCP].txTail -= TCBStubs[hCurrentTCP].bufferRxStart - TCBStubs[hCurrentTCP].bufferTxStart;
				if(MyTCB.txUnackedTail >= TCBStubs[hCurrentTCP].bufferRxStart)
					MyTCB.txUnackedTail -= TCBStubs[hCurrentTCP].bufferRxStart - TCBStubs[hCurrentTCP].bufferTxStart;
			}
			else
			{
				// See if we have outstanding TX data that is waiting for an ACK
				if(TCBStubs[hCurrentTCP].txTail != MyTCB.txUnackedTail)
				{
					if(MyTCB.flags.bRXNoneACKed1)
					{
						if(MyTCB.flags.bRXNoneACKed2)
						{
							// Set up to perform a fast retransmission
							// Roll back unacknowledged TX tail pointer to cause retransmit to occur
							MyTCB.MySEQ -= (LONG)(SHORT)(MyTCB.txUnackedTail - TCBStubs[hCurrentTCP].txTail);
							if(MyTCB.txUnackedTail < TCBStubs[hCurrentTCP].txTail)
								MyTCB.MySEQ -= (LONG)(SHORT)(TCBStubs[hCurrentTCP].bufferRxStart - TCBStubs[hCurrentTCP].bufferTxStart);
							MyTCB.txUnackedTail = TCBStubs[hCurrentTCP].txTail;
							TCBStubs[hCurrentTCP].Flags.bTXASAPWithoutTimerReset = 1;
						}
						MyTCB.flags.bRXNoneACKed2 = 1;
					}
					MyTCB.flags.bRXNoneACKed1 = 1;
				}
			}

			// No need to keep our retransmit timer going if we have nothing that needs ACKing anymore
			if(TCBStubs[hCurrentTCP].txTail == TCBStubs[hCurrentTCP].txHead)
			{
				// Make sure there isn't a "FIN byte in our TX FIFO"
				if(TCBStubs[hCurrentTCP].Flags.bTXFIN == 0u)
				{
					// Convert retransmission timer to keep-alive timer
					#if defined(TCP_KEEP_ALIVE_TIMEOUT)
						TCBStubs[hCurrentTCP].eventTime = mGetTick() + TCP_KEEP_ALIVE_TIMEOUT;
					#endif
					TCBStubs[hCurrentTCP].Flags.bTimerEnabled = 0;
				}
				else
				{
					// "Throw away" FIN byte from our TX FIFO if it has been ACKed
					if((MyTCB.MySEQ == localAckNumber) && MyTCB.flags.bFINSent)
					{
						TCBStubs[hCurrentTCP].Flags.bTimerEnabled = 0;
						TCBStubs[hCurrentTCP].Flags.bTXFIN = 0;
					}
				}
			}

			// The window size advirtised in this packet is adjusted to account
			// for any bytes that we have transmitted but haven't been ACKed yet
			// by this segment.
			wNewWindow = h->Window - ((WORD)(MyTCB.MySEQ - localAckNumber));

			// Update the local stored copy of the RemoteWindow.
			// If previously we had a zero window, and now we don't, then
			// immediately send whatever was pending.
			if((MyTCB.remoteWindow == 0u) && wNewWindow)
				TCBStubs[hCurrentTCP].Flags.bTXASAP = 1;
			MyTCB.remoteWindow = wNewWindow;

			// A couple of states must do all of the TCP_ESTABLISHED stuff, but also a little more
			if(TCBStubs[hCurrentTCP].smState == TCP_FIN_WAIT_1)
			{
				// Check to see if our FIN has been ACKnowledged
				if((MyTCB.MySEQ == localAckNumber) && MyTCB.flags.bFINSent)
				{
					// Reset our timer for forced closure if the remote node
					// doesn't send us a FIN in a timely manner.
					TCBStubs[hCurrentTCP].eventTime = mGetTick() + TCP_FIN_WAIT_2_TIMEOUT;
					TCBStubs[hCurrentTCP].Flags.bTimerEnabled = 1;
					TCBStubs[hCurrentTCP].smState = TCP_FIN_WAIT_2;
				}
			}
			else if(TCBStubs[hCurrentTCP].smState == TCP_FIN_WAIT_2)
			{
				// RFC noncompliance:
				// The remote node should not keep sending us data
				// indefinitely after we send a FIN to it.
				// However, some bad stacks may still keep sending
				// us data indefinitely after ACKing our FIN.  To
				// prevent this from locking up our socket, let's
				// send a RST right now and close forcefully on
				// our side.
				if(!(localHeaderFlags & FIN))
				{
					MyTCB.MySEQ = localAckNumber;	// Set SEQ = SEG.ACK
					SendTCP(RST | ACK, 0);
					CloseSocket();
					return;
				}
			}
			else if(TCBStubs[hCurrentTCP].smState == TCP_CLOSING)
			{
				// Check to see if our FIN has been ACKnowledged
				if(MyTCB.MySEQ == localAckNumber)
				{
					// RFC not recommended: We should be going to
					// the TCP_TIME_WAIT state right here and
					// starting a 2MSL timer, but since we have so
					// few precious sockets, we can't afford to
					// leave a socket waiting around doing nothing
					// for a long time.  If the remote node does
					// not recieve this ACK, it'll have to figure
					// out on it's own that the connection is now
					// closed.
					CloseSocket();
				}

				return;
			}

			break;

		case TCP_LAST_ACK:
			// Check to see if our FIN has been ACKnowledged
			if(MyTCB.MySEQ + 1 == localAckNumber)
				CloseSocket();
			return;

//		case TCP_TIME_WAIT:
//			// Nothing is supposed to arrive here.  If it does, reset the quiet timer.
//			SendTCP(ACK, SENDTCP_RESET_TIMERS);
//			return;

		default:
			break;
	}

	//
	// Sixth: Check the URG bit
	//
	// Urgent packets are not supported in this stack, so we
	// will throw them away instead
	if(localHeaderFlags & URG)
		return;

	//
	// Seventh: Process the segment text
	//
	// Throw data away if in a state that doesn't accept data
	if(TCBStubs[hCurrentTCP].smState == TCP_CLOSE_WAIT)
		return;
	if(TCBStubs[hCurrentTCP].smState == TCP_CLOSING)
		return;
	if(TCBStubs[hCurrentTCP].smState == TCP_LAST_ACK)
		return;
//	if(TCBStubs[hCurrentTCP].smState == TCP_TIME_WAIT)
//		return;

	// Copy any valid segment data into our RX FIFO, if any
	if(len)
	{
		// See if there are bytes we must skip
		if((SHORT)wMissingBytes <= 0)
		{
			// Position packet read pointer to start of useful data area.
			IPSetRxBuffer((h->DataOffset.Val << 2) - wMissingBytes);
			len += wMissingBytes;

			// Truncate packets that would overflow our TCP RX FIFO
			// and request a retransmit by sending a duplicate ACK
			if(len > wFreeSpace)
				len = wFreeSpace;

			MyTCB.RemoteSEQ += (DWORD)len;

			// Copy the application data from the packet into the socket RX FIFO
			// See if we need a two part copy (spans bufferEnd->bufferRxStart)
			if(TCBStubs[hCurrentTCP].rxHead + len > TCBStubs[hCurrentTCP].bufferEnd)
			{
				wTemp = TCBStubs[hCurrentTCP].bufferEnd - TCBStubs[hCurrentTCP].rxHead + 1;
				TCPRAMCopy(TCBStubs[hCurrentTCP].rxHead, TCP_PIC_RAM, (PTR_BASE)-1, TCP_ETH_RAM, wTemp);
				TCPRAMCopy(TCBStubs[hCurrentTCP].bufferRxStart, TCP_PIC_RAM, (PTR_BASE)-1, TCP_ETH_RAM, len - wTemp);
				TCBStubs[hCurrentTCP].rxHead = TCBStubs[hCurrentTCP].bufferRxStart + (len - wTemp);
			}
			else
			{
				TCPRAMCopy(TCBStubs[hCurrentTCP].rxHead, TCP_PIC_RAM, (PTR_BASE)-1, TCP_ETH_RAM, len);
				TCBStubs[hCurrentTCP].rxHead += len;
			}

			// See if we have a hole and other data waiting already in the RX FIFO
			if(MyTCB.sHoleSize != -1)
			{
				MyTCB.sHoleSize -= len;
				wTemp = MyTCB.wFutureDataSize + MyTCB.sHoleSize;

				// See if we just closed up a hole, and if so, advance head pointer
				if((SHORT)wTemp < (SHORT)0)
				{
					MyTCB.sHoleSize = -1;
				}
				else if(MyTCB.sHoleSize <= 0)
				{
					MyTCB.RemoteSEQ += wTemp;
					TCBStubs[hCurrentTCP].rxHead += wTemp;
					if(TCBStubs[hCurrentTCP].rxHead > TCBStubs[hCurrentTCP].bufferEnd)
						TCBStubs[hCurrentTCP].rxHead -= TCBStubs[hCurrentTCP].bufferEnd - TCBStubs[hCurrentTCP].bufferRxStart + 1;
					MyTCB.sHoleSize = -1;
				}
			}
		} // This packet is out of order or we lost a packet, see if we can generate a hole to accomodate it
		else if((SHORT)wMissingBytes > 0)
		{
			// Truncate packets that would overflow our TCP RX FIFO
			if(len + wMissingBytes > wFreeSpace)
				len = wFreeSpace - wMissingBytes;

			// Position packet read pointer to start of useful data area.
			IPSetRxBuffer(h->DataOffset.Val << 2);

			// See if we need a two part copy (spans bufferEnd->bufferRxStart)
			if(TCBStubs[hCurrentTCP].rxHead + wMissingBytes + len > TCBStubs[hCurrentTCP].bufferEnd)
			{
				// Calculate number of data bytes to copy before wraparound
				wTemp = TCBStubs[hCurrentTCP].bufferEnd - TCBStubs[hCurrentTCP].rxHead + 1 - wMissingBytes;
				if((SHORT)wTemp >= 0)
				{
					TCPRAMCopy(TCBStubs[hCurrentTCP].rxHead + wMissingBytes, TCP_PIC_RAM, (PTR_BASE)-1, TCP_ETH_RAM, wTemp);
					TCPRAMCopy(TCBStubs[hCurrentTCP].bufferRxStart, TCP_PIC_RAM, (PTR_BASE)-1, TCP_ETH_RAM, len - wTemp);
				}
				else
				{
					TCPRAMCopy(TCBStubs[hCurrentTCP].rxHead + wMissingBytes - (TCBStubs[hCurrentTCP].bufferEnd - TCBStubs[hCurrentTCP].bufferRxStart + 1), TCP_PIC_RAM, (PTR_BASE)-1, TCP_ETH_RAM, len);
				}
			}
			else
			{
				TCPRAMCopy(TCBStubs[hCurrentTCP].rxHead + wMissingBytes, TCP_PIC_RAM, (PTR_BASE)-1, TCP_ETH_RAM, len);
			}

			// Record the hole is here
			if(MyTCB.sHoleSize == -1)
			{
				MyTCB.sHoleSize = wMissingBytes;
				MyTCB.wFutureDataSize = len;
			}
			else
			{
				// We already have a hole, see if we can shrink the hole
				// or extend the future data size
				if(wMissingBytes < (WORD)MyTCB.sHoleSize)
				{
					if((wMissingBytes + len > (WORD)MyTCB.sHoleSize + MyTCB.wFutureDataSize) || (wMissingBytes + len < (WORD)MyTCB.sHoleSize))
						MyTCB.wFutureDataSize = len;
					else
						MyTCB.wFutureDataSize = (WORD)MyTCB.sHoleSize + MyTCB.wFutureDataSize - wMissingBytes;
					MyTCB.sHoleSize = wMissingBytes;
				}
				else if(wMissingBytes + len > (WORD)MyTCB.sHoleSize + MyTCB.wFutureDataSize)
				{
					// Make sure that there isn't a second hole between
					// our future data and this TCP segment's future data
					if(wMissingBytes <= (WORD)MyTCB.sHoleSize + MyTCB.wFutureDataSize)
						MyTCB.wFutureDataSize += wMissingBytes + len - (WORD)MyTCB.sHoleSize - MyTCB.wFutureDataSize;
				}

			}
		}
	}

	// Send back an ACK of the data (+SYN | FIN) we just received,
	// if any.  To minimize bandwidth waste, we are implementing
	// the delayed acknowledgement algorithm here, only sending
	// back an immediate ACK if this is the second segment received.
	// Otherwise, a 200ms timer will cause the ACK to be transmitted.
	if(wSegmentLength)
	{
		// For non-established sockets, let's delete all data in
		// the RX buffer immediately after receiving it.  This is
		// not really how TCP was intended to operate since a
		// socket cannot receive any response after it sends a FIN,
		// but our TCP application API doesn't readily accomodate
		// receiving data after calling TCPDisconnect(), which
		// invalidates the application TCP handle.  By deleting all
		// data, we'll ensure that the RX window is nonzero and
		// the remote node will be able to send us a FIN response,
		// which needs an RX window of at least 1.
		if(TCBStubs[hCurrentTCP].smState != TCP_ESTABLISHED)
			TCBStubs[hCurrentTCP].rxTail = TCBStubs[hCurrentTCP].rxHead;

		if(TCBStubs[hCurrentTCP].Flags.bOneSegmentReceived)
		{
			SendTCP(ACK, SENDTCP_RESET_TIMERS);
			SyncTCB();
			// bOneSegmentReceived is cleared in SendTCP(), so no need here
		}
		else
		{
			TCBStubs[hCurrentTCP].Flags.bOneSegmentReceived = TRUE;

			// Do not send an ACK immediately back.  Instead, we will
			// perform delayed acknowledgements.  To do this, we will
			// just start a timer
			if(!TCBStubs[hCurrentTCP].Flags.bDelayedACKTimerEnabled)
			{
				TCBStubs[hCurrentTCP].Flags.bDelayedACKTimerEnabled = 1;
				TCBStubs[hCurrentTCP].OverlappedTimers.delayedACKTime = (QWORD)(mGetTick() >> 8) + (QWORD)((TCP_DELAYED_ACK_TIMEOUT) >> 8);
			}
		}
	}

	//
	// Eighth: check the FIN bit
	//
	if(localHeaderFlags & FIN)
	{
		// Note: Since we don't have a good means of storing "FIN bytes"
		// in our TCP RX FIFO, we must ensure that FINs are processed
		// in-order.
		if(MyTCB.RemoteSEQ + 1 == localSeqNumber + (DWORD)wSegmentLength)
		{
			// FINs are treated as one byte of data for ACK sequencing
			MyTCB.RemoteSEQ++;

			switch(TCBStubs[hCurrentTCP].smState)
			{
				case TCP_SYN_RECEIVED:
					// RFC in exact: Our API has no need for the user
					// to explicitly close a socket that never really
					// got opened fully in the first place, so just
					// transmit a FIN automatically and jump to
					// TCP_LAST_ACK
					TCBStubs[hCurrentTCP].smState = TCP_LAST_ACK;
					SendTCP(FIN | ACK, SENDTCP_RESET_TIMERS);
					return;

				case TCP_ESTABLISHED:
					// Go to TCP_CLOSE_WAIT state
					TCBStubs[hCurrentTCP].smState = TCP_CLOSE_WAIT;

					// For legacy applications that don't call
					// TCPDisconnect() as needed and expect the TCP/IP
					// Stack to automatically close sockets when the
					// remote node sends a FIN, let's start a timer so
					// that we will eventually close the socket automatically
					TCBStubs[hCurrentTCP].OverlappedTimers.closeWaitTime = (QWORD)(mGetTick() >> 8) + (WORD)((TCP_CLOSE_WAIT_TIMEOUT) >> 8);
					break;

				case TCP_FIN_WAIT_1:
					if(MyTCB.MySEQ == localAckNumber)
					{
						// RFC not recommended: We should be going to
						// the TCP_TIME_WAIT state right here and
						// starting a 2MSL timer, but since we have so
						// few precious sockets, we can't afford to
						// leave a socket waiting around doing nothing
						// for a long time.  If the remote node does
						// not recieve this ACK, it'll have to figure
						// out on it's own that the connection is now
						// closed.
						SendTCP(ACK, 0);
						CloseSocket();
						return;
					}
					else
					{
						TCBStubs[hCurrentTCP].smState = TCP_CLOSING;
					}
					break;

				case TCP_FIN_WAIT_2:
					// RFC not recommended: We should be going to
					// the TCP_TIME_WAIT state right here and
					// starting a 2MSL timer, but since we have so
					// few precious sockets, we can't afford to
					// leave a socket waiting around doing nothing
					// for a long time.  If the remote node does
					// not recieve this ACK, it'll have to figure
					// out on it's own that the connection is now
					// closed.
					SendTCP(ACK, 0);
					CloseSocket();
					return;

				default:
					break;
			}

			// Acknowledge receipt of FIN
			SendTCP(ACK, SENDTCP_RESET_TIMERS);
		}
	}
}

/******************************************************************************
 * ---TCPOpen
 * Provides a unified method for opening TCP sockets. This function can
 * open both client and server sockets. For client sockets, it can accept
 * a host name string to query in DNS, an IP address as a string, an IP
 * address in binary form, or a previously resolved NODE_INFO structure
 * containing the remote IP address and associated MAC address. When a
 * host name or IP address only is provided, the TCP module will
 * internally perform the necessary DNS and/or ARP resolution steps before
 * reporting that the TCP socket is connected (via a call to
 * TCPISConnected returning TRUE). Server sockets ignore this destination
 * parameter and listen only on the indicated port.

 * Sockets are statically allocated on boot, but can be claimed with this
 * \function and freed using TCPDisconnect or TCPClose (for client
 * sockets). Server sockets can be freed using TCPClose only (calls to
 * TCPDisconnect will return server sockets to the listening state,
 * allowing reuse).
 ******************************************************************************/
TCP_SOCKET TCPOpen(DWORD dwRemoteHost, BYTE vRemoteHostType, WORD wPort)
{
	TCP_SOCKET hTCP;

	// Find an available socket that matches the specified socket type
	for(hTCP = 0; hTCP < MAX_TCP_SOCKETS; hTCP++)
	{
        hCurrentTCP = hTCP;
		// Sockets that are in use will be in a non-closed state
		if(TCBStubs[hCurrentTCP].smState != TCP_CLOSED)
        {
			continue;
        }
		SyncTCB();
		// Start out assuming worst case Maximum Segment Size (changes when MSS
		// option is received from remote node)
		MyTCB.wRemoteMSS = 536;
		if(vRemoteHostType == TCP_OPEN_SERVER)
		{
			MyTCB.localPort.Val = wPort;
			TCBStubs[hCurrentTCP].Flags.bServer = TRUE;
			TCBStubs[hCurrentTCP].smState = TCP_LISTEN;
			TCBStubs[hCurrentTCP].remoteHash.Val = wPort;
		}
		else
		{
            // Each new socket that is opened by this node, gets the
            // next sequential local port number.
            if(NextPort < LOCAL_PORT_START_NUMBER || NextPort > LOCAL_PORT_END_NUMBER)
            {
                NextPort = LOCAL_PORT_START_NUMBER;
            }
            // Set the non-zero TCB fields
            MyTCB.localPort.Val = NextPort++;
            MyTCB.remotePort.Val = wPort;
            // Flag to start the DNS, ARP, SYN processes
            TCBStubs[hCurrentTCP].eventTime = mGetTick();
            TCBStubs[hCurrentTCP].Flags.bTimerEnabled = 1;
            TCBStubs[hCurrentTCP].remoteHash.Val = (((DWORD_VAL*)&dwRemoteHost)->w[1]+((DWORD_VAL*)&dwRemoteHost)->w[0] + wPort) ^ MyTCB.localPort.Val;
            MyTCB.remote.niRemoteMACIP.IPAddr.Val = dwRemoteHost;
            MyTCB.retryCount = 2;
            MyTCB.retryInterval = (TICK_1S/4)/256;
            TCBStubs[hCurrentTCP].smState = TCP_GATEWAY_SEND_ARP;
		}
        TCBStubs[hCurrentTCP].mLocalPort.Val = NextPort;
        TCBStubs[hCurrentTCP].mRemotePort.Val = wPort;
        memcpy((void*)&TCBStubs[hCurrentTCP].mRemoteNode, (void*)&MyTCB.remote, sizeof(NODE_INFO));
		return hTCP;
	}
	return INVALID_SOCKET;
}

/******************************************************************************
 * ---SendTCP
 * This function assembles and transmits a TCP segment, including any
 * pending data.  It also supports retransmissions, keep-alives, and other packet types.
 ******************************************************************************/
static void SendTCP(BYTE vTCPFlags, BYTE vSendFlags)
{
	WORD_VAL        wVal;
	TCP_HEADER      header;
	TCP_OPTIONS     options;
	PSEUDO_HEADER   pseudoHeader;
	WORD 		len;

	SyncTCB();

	// FINs must be handled specially
	if(vTCPFlags & FIN)
	{
		TCBStubs[hCurrentTCP].Flags.bTXFIN = 1;
		vTCPFlags &= ~FIN;
	}

	// Status will now be synched, disable automatic future
	// status transmissions
	TCBStubs[hCurrentTCP].Flags.bTimer2Enabled = 0;
	TCBStubs[hCurrentTCP].Flags.bDelayedACKTimerEnabled = 0;
	TCBStubs[hCurrentTCP].Flags.bOneSegmentReceived = 0;
	TCBStubs[hCurrentTCP].Flags.bTXASAP = 0;
	TCBStubs[hCurrentTCP].Flags.bTXASAPWithoutTimerReset = 0;
	TCBStubs[hCurrentTCP].Flags.bHalfFullFlush = 0;

	//  Make sure that we can write to the MAC transmit area
	while(!MACIsTxReady());

	// Put all socket application data in the TX space
	if(vTCPFlags & (SYN | RST))
	{
		// Don't put any data in SYN and RST messages
		len = 0;
	}
	else
	{
		// Begin copying any application data over to the TX space
		if(TCBStubs[hCurrentTCP].txHead == MyTCB.txUnackedTail)
		{
			// All caught up on data TX, no real data for this packet
			len = 0;
		}
		else if(TCBStubs[hCurrentTCP].txHead > MyTCB.txUnackedTail)
		{
			len = TCBStubs[hCurrentTCP].txHead - MyTCB.txUnackedTail;

			if(len > MyTCB.remoteWindow)
				len = MyTCB.remoteWindow;

			if(len > MyTCB.wRemoteMSS)
			{
				len = MyTCB.wRemoteMSS;
				TCBStubs[hCurrentTCP].Flags.bTXASAPWithoutTimerReset = 1;
			}

			// Copy application data into the raw TX buffer
			TCPRAMCopy(BASE_TX_ADDR+sizeof(ETHER_HEADER)+sizeof(IP_HEADER)+sizeof(TCP_HEADER), TCP_ETH_RAM, MyTCB.txUnackedTail, TCP_PIC_RAM, len);
			MyTCB.txUnackedTail += len;
		}
		else
		{
			pseudoHeader.Length = TCBStubs[hCurrentTCP].bufferRxStart - MyTCB.txUnackedTail;
			len = pseudoHeader.Length + TCBStubs[hCurrentTCP].txHead - TCBStubs[hCurrentTCP].bufferTxStart;

			if(len > MyTCB.remoteWindow)
				len = MyTCB.remoteWindow;

			if(len > MyTCB.wRemoteMSS)
			{
				len = MyTCB.wRemoteMSS;
				TCBStubs[hCurrentTCP].Flags.bTXASAPWithoutTimerReset = 1;
			}

			if(pseudoHeader.Length > len)
				pseudoHeader.Length = len;

			// Copy application data into the raw TX buffer
			TCPRAMCopy(BASE_TX_ADDR+sizeof(ETHER_HEADER)+sizeof(IP_HEADER)+sizeof(TCP_HEADER), TCP_ETH_RAM, MyTCB.txUnackedTail, TCP_PIC_RAM, pseudoHeader.Length);
			pseudoHeader.Length = len - pseudoHeader.Length;

			// Copy any left over chunks of application data over
			if(pseudoHeader.Length)
			{
				TCPRAMCopy(BASE_TX_ADDR+sizeof(ETHER_HEADER)+sizeof(IP_HEADER)+sizeof(TCP_HEADER)+(TCBStubs[hCurrentTCP].bufferRxStart-MyTCB.txUnackedTail), TCP_ETH_RAM, TCBStubs[hCurrentTCP].bufferTxStart, TCP_PIC_RAM, pseudoHeader.Length);
			}

			MyTCB.txUnackedTail += len;
			if(MyTCB.txUnackedTail >= TCBStubs[hCurrentTCP].bufferRxStart)
				MyTCB.txUnackedTail -= TCBStubs[hCurrentTCP].bufferRxStart-TCBStubs[hCurrentTCP].bufferTxStart;
		}

		// If we are to transmit a FIN, make sure we can put one in this packet
		if(TCBStubs[hCurrentTCP].Flags.bTXFIN)
		{
			if((len != MyTCB.remoteWindow) && (len != MyTCB.wRemoteMSS))
				vTCPFlags |= FIN;
		}
	}

	// Ensure that all packets with data of some kind are
	// retransmitted by TCPTick() until acknowledged
	// Pure ACK packets with no data are not ACKed back in TCP
	if(len || (vTCPFlags & (SYN | FIN)))
	{
		// Transmitting data, update remote window variable to reflect smaller
		// window.
		MyTCB.remoteWindow -= len;

		// Push (PSH) all data for enhanced responsiveness on
		// the remote end, especially with GUIs
		if(len)
			vTCPFlags |= PSH;

		if(vSendFlags & SENDTCP_RESET_TIMERS)
		{
			MyTCB.retryCount = 0;
			MyTCB.retryInterval = TCP_START_TIMEOUT_VAL;
		}

		TCBStubs[hCurrentTCP].eventTime = mGetTick() + MyTCB.retryInterval;
		TCBStubs[hCurrentTCP].Flags.bTimerEnabled = 1;
	}
	else if(vSendFlags & SENDTCP_KEEP_ALIVE)
	{
		// Increment Keep Alive TX counter to handle disconnection if not response is returned
		TCBStubs[hCurrentTCP].Flags.vUnackedKeepalives++;

		// Generate a dummy byte
		MyTCB.MySEQ -= 1;
		len = 1;
	}
	else if(TCBStubs[hCurrentTCP].Flags.bTimerEnabled)
	{
		// If we have data to transmit, but the remote RX window is zero,
		// so we aren't transmitting any right now then make sure to not
		// extend the retry counter or timer.  This will stall our TX
		// with a periodic ACK sent to the remote node.
		if(!(vSendFlags & SENDTCP_RESET_TIMERS))
		{
			// Roll back retry counters since we can't send anything,
			// but only if we incremented it in the first place
			if(MyTCB.retryCount)
			{
				MyTCB.retryCount--;
				MyTCB.retryInterval >>= 1;
			}
		}

		TCBStubs[hCurrentTCP].eventTime = mGetTick() + MyTCB.retryInterval;
	}


	header.SourcePort			= MyTCB.localPort.Val;
	header.DestPort				= MyTCB.remotePort.Val;
	header.SeqNumber			= MyTCB.MySEQ;
	header.AckNumber			= MyTCB.RemoteSEQ;
	header.Flags.bits.Reserved2	= 0;
	header.DataOffset.Reserved3	= 0;
	header.Flags.byte			= vTCPFlags;
	header.UrgentPointer        = 0;

	// Update our send sequence number and ensure retransmissions
	// of SYNs and FINs use the right sequence number
	MyTCB.MySEQ += (DWORD)len;
	if(vTCPFlags & SYN)
	{
		// SEG.ACK needs to be zero for the first SYN packet for compatibility
		// with certain paranoid TCP/IP stacks, even though the ACK flag isn't
		// set (indicating that the AckNumber field is unused).
		if(!(vTCPFlags & ACK))
			header.AckNumber = 0x00000000;

		if(MyTCB.flags.bSYNSent)
			header.SeqNumber--;
		else
		{
			MyTCB.MySEQ++;
			MyTCB.flags.bSYNSent = 1;
		}
	}
	if(vTCPFlags & FIN)
	{
        MyTCB.flags.bFINSent = 1;   // do not advance the seq no for FIN!
	}

	// Calculate the amount of free space in the RX buffer area of this socket
	if(TCBStubs[hCurrentTCP].rxHead >= TCBStubs[hCurrentTCP].rxTail)
		header.Window = (TCBStubs[hCurrentTCP].bufferEnd - TCBStubs[hCurrentTCP].bufferRxStart) - (TCBStubs[hCurrentTCP].rxHead - TCBStubs[hCurrentTCP].rxTail);
	else
		header.Window = TCBStubs[hCurrentTCP].rxTail - TCBStubs[hCurrentTCP].rxHead - 1;

	// Calculate the amount of free space in the MAC RX buffer area and adjust window if needed
	wVal.Val = MACGetFreeRxSize();
	if(wVal.Val < 64)
    {
		wVal.Val = 0;
    }
    else
    {
		wVal.Val -= 64;
    }
	// Force the remote node to throttle back if we are running low on general RX buffer space
	if(header.Window > wVal.Val)
		header.Window = wVal.Val;

	SwapTCPHeader(&header);


	len += sizeof(header);
	header.DataOffset.Val   = sizeof(header) >> 2;

	// Insert the MSS (Maximum Segment Size) TCP option if this is SYN packet
	if(vTCPFlags & SYN)
	{
		len += sizeof(options);
		options.Kind = TCP_OPTIONS_MAX_SEG_SIZE;
		options.Length = 0x04;

		// Load MSS and swap to big endian
		options.MaxSegSize.Val = (((TCP_MAX_SEG_SIZE_RX)&0x00FF)<<8) | (((TCP_MAX_SEG_SIZE_RX)&0xFF00)>>8);

		header.DataOffset.Val   += sizeof(options) >> 2;
	}

	// Calculate IP pseudoheader checksum.
	pseudoHeader.SourceAddress	= AppConfig.MyIPAddr;
	pseudoHeader.DestAddress    = MyTCB.remote.niRemoteMACIP.IPAddr;
	pseudoHeader.Zero           = 0x0;
	pseudoHeader.Protocol       = IP_PROTOCOLE_TCP;
	pseudoHeader.Length			= len;
	SwapPseudoHeader(pseudoHeader);
	header.Checksum = ~CalcIPChecksum((BYTE*)&pseudoHeader, sizeof(pseudoHeader));

	// Write IP header
	MACSetWritePtr(BASE_TX_ADDR + sizeof(ETHER_HEADER));
	IPPutHeader(&MyTCB.remote.niRemoteMACIP, IP_PROTOCOLE_TCP, len);
	MACPutArray((BYTE*)&header, sizeof(header));
	if(vTCPFlags & SYN)
		MACPutArray((BYTE*)&options, sizeof(options));

	// Update the TCP checksum
	MACSetReadPtr(BASE_TX_ADDR + sizeof(ETHER_HEADER) + sizeof(IP_HEADER));
	wVal.Val = CalcIPBufferChecksum(len);
	MACSetWritePtr(BASE_TX_ADDR + sizeof(ETHER_HEADER) + sizeof(IP_HEADER) + 16);
	MACPutArray((BYTE*)&wVal, sizeof(WORD));

	// Physically start the packet transmission over the network
	MACFlush();
}


/******************************************************************************
 * ---TCPFlush
 * This function immediately transmits all pending TX data with a PSH
 * flag.  If this function is not called, data will automatically be sent
 * when either a) the TX buffer is half full or b) the
 * TCP_AUTO_TRANSMIT_TIMEOUT_VAL (default: 40ms) has elapsed.
 ******************************************************************************/
void TCPFlush(TCP_SOCKET hTCP)
{
    if(hTCP >= MAX_TCP_SOCKETS)
    {
        return;
    }

    hCurrentTCP = hTCP;
    SyncTCB();

    // NOTE: Pending SSL data will NOT be transferred here

    if(TCBStubs[hCurrentTCP].txHead != MyTCB.txUnackedTail)
    {
        // Send the TCP segment with all unacked bytes
        SendTCP(ACK, SENDTCP_RESET_TIMERS);
    }
}

/******************************************************************************
 * ---TCPDisconnect
 * This function closes a connection to a remote node by sending a FIN (if
 * currently connected).

 * The function can be called a second time to force a socket closed by
 * sending a RST packet.  This is useful when the application knows that
 * the remote node will not send an ACK (if it has crashed or lost its link),
 * or when the application needs to reuse the socket immediately regardless
 * of whether or not the remote node would like to transmit more data before
 * closing.

 * For client mode sockets, upon return, the hTCP handle is relinquished to
 * the TCP/IP stack and must no longer be used by the application (except for
 * an immediate subsequent call to TCPDisconnect() to force a RST
 * transmission, if needed).

 * For server mode sockets, upon return, the hTCP handle is NOT relinquished
 * to the TCP/IP stack.  After closing, the socket returns to the listening
 * state allowing future connection requests to be serviced.  This leaves the
 * hTCP handle in a valid state and must be retained for future operations on
 * the socket.  If you want to close the server and relinquish the socket back
 * to the TCP/IP stack, call the TCPClose() API instead of TCPDisconnect().
 ******************************************************************************/
void TCPDisconnect(TCP_SOCKET hTCP)
{
	if(hTCP >= MAX_TCP_SOCKETS)
    {
        return;
    }

    hCurrentTCP = hTCP;

	// Delete all data in the RX FIFO
	// In this stack's API, the application TCP handle is
	// immediately invalid after calling this function, so there
	// is no longer any way to receive data from the TCP RX FIFO,
	// even though the data is still there.  Leaving the data there
	// could interfere with the remote node sending us a FIN if our
	// RX window is zero
	TCBStubs[hCurrentTCP].rxTail = TCBStubs[hCurrentTCP].rxHead;

	switch(TCBStubs[hCurrentTCP].smState)
	{
		case TCP_GET_DNS_MODULE:
		case TCP_GATEWAY_SEND_ARP:
		case TCP_GATEWAY_GET_ARP:
		case TCP_SYN_SENT:
			CloseSocket();
			break;

		case TCP_SYN_RECEIVED:
		case TCP_ESTABLISHED:
			// Send the FIN.  This is done in a loop to ensure that if we have
			// more data wating in the TX FIFO than can be sent in a single
			// packet (due to the remote Max Segment Size packet size limit),
			// we will keep generating more packets until either all data gets
			// transmitted or the remote node's receive window fills up.
			do
			{
				SendTCP(FIN | ACK, SENDTCP_RESET_TIMERS);
				if(MyTCB.remoteWindow == 0u)
					break;
			} while(TCBStubs[hCurrentTCP].txHead != MyTCB.txUnackedTail);

			TCBStubs[hCurrentTCP].smState = TCP_FIN_WAIT_1;
			break;

		case TCP_CLOSE_WAIT:
			// Send the FIN.  This is done in a loop to ensure that if we have
			// more data wating in the TX FIFO than can be sent in a single
			// packet (due to the remote Max Segment Size packet size limit),
			// we will keep generating more packets until either all data gets
			// transmitted or the remote node's receive window fills up.
			do
			{
				SendTCP(FIN | ACK, SENDTCP_RESET_TIMERS);
				if(MyTCB.remoteWindow == 0u)
					break;
			} while(TCBStubs[hCurrentTCP].txHead != MyTCB.txUnackedTail);

			TCBStubs[hCurrentTCP].smState = TCP_LAST_ACK;
			break;
		case TCP_CLOSED_BUT_RESERVED:
			TCBStubs[hCurrentTCP].smState = TCP_CLOSED;
			break;
		// These states will close themselves after some delay, however,
		// this is handled so that the user can call TCPDisconnect()
		// twice to immediately close a socket (using an RST) without
		// having to get an ACK back from the remote node.  This is
		// great for instance when the application determines that
		// the remote node has been physically disconnected and
		// already knows that no ACK will be returned.  Alternatively,
		// if the application needs to immediately reuse the socket
		// regardless of what the other node's state is in (half open).
		case TCP_FIN_WAIT_1:
		case TCP_FIN_WAIT_2:
		case TCP_LAST_ACK:
		default:
			SendTCP(RST | ACK, 0);
			CloseSocket();
			break;
	}
}

/******************************************************************************
 * ---CloseSocket
 * This function closes a TCP socket.  All socket state information is
 * reset, and any buffered bytes are discarded.  The socket is no longer
 * accessible by the application after this point.
 ******************************************************************************/
static void CloseSocket(void)
{
	SyncTCB();

	TCBStubs[hCurrentTCP].remoteHash.Val = MyTCB.localPort.Val;
	TCBStubs[hCurrentTCP].txHead = TCBStubs[hCurrentTCP].bufferTxStart;
	TCBStubs[hCurrentTCP].txTail = TCBStubs[hCurrentTCP].bufferTxStart;
	TCBStubs[hCurrentTCP].rxHead = TCBStubs[hCurrentTCP].bufferRxStart;
	TCBStubs[hCurrentTCP].rxTail = TCBStubs[hCurrentTCP].bufferRxStart;
	TCBStubs[hCurrentTCP].smState = TCBStubs[hCurrentTCP].Flags.bServer ? TCP_LISTEN : TCP_CLOSED;
	TCBStubs[hCurrentTCP].Flags.vUnackedKeepalives = 0;
	TCBStubs[hCurrentTCP].Flags.bTimerEnabled = 0;
	TCBStubs[hCurrentTCP].Flags.bTimer2Enabled = 0;
	TCBStubs[hCurrentTCP].Flags.bDelayedACKTimerEnabled = 0;
	TCBStubs[hCurrentTCP].Flags.bOneSegmentReceived = 0;
	TCBStubs[hCurrentTCP].Flags.bHalfFullFlush = 0;
	TCBStubs[hCurrentTCP].Flags.bTXASAP = 0;
	TCBStubs[hCurrentTCP].Flags.bTXASAPWithoutTimerReset = 0;
	TCBStubs[hCurrentTCP].Flags.bTXFIN = 0;
	TCBStubs[hCurrentTCP].Flags.bSocketReset = 1;

	MyTCB.flags.bFINSent = 0;
	MyTCB.flags.bSYNSent = 0;
	MyTCB.flags.bRXNoneACKed1 = 0;
	MyTCB.flags.bRXNoneACKed2 = 0;
	MyTCB.txUnackedTail = TCBStubs[hCurrentTCP].bufferTxStart;
	((DWORD_VAL*)(&MyTCB.MySEQ))->w[0] = LFSRRand();
	((DWORD_VAL*)(&MyTCB.MySEQ))->w[1] = LFSRRand();
	MyTCB.sHoleSize = -1;
	MyTCB.remoteWindow = 1;
}

/******************************************************************************
 * ---TCPClose
 * Disconnects an open socket and destroys the socket handle, including server
 * mode socket handles.  This function performs identically to the
 * TCPDisconnect() function, except that both client and server mode socket
 * handles are relinquished to the TCP/IP stack upon return.
 ******************************************************************************/
void TCPClose(TCP_SOCKET hTCP)
{
	if(hTCP >= MAX_TCP_SOCKETS)
    {
        return;
    }

    hCurrentTCP = hTCP;
	TCBStubs[hCurrentTCP].Flags.bServer = FALSE;
	TCPDisconnect(hTCP);
}

/******************************************************************************
 * ---TCPPutString
 * Writes a null-terminated string from RAM to a TCP socket.  The
 * null-terminator is not copied to the socket.
 ******************************************************************************/
BYTE* TCPPutString(TCP_SOCKET hTCP, BYTE* data)
{
    return data + TCPPutArray(hTCP, data, strlen((char*)data));
}


/******************************************************************************
 * ---TCPGetArray
 * Reads an array of data bytes from a TCP socket's receive FIFO.  The data
 * is removed from the FIFO in the process.
 ******************************************************************************/
WORD TCPGetArray(TCP_SOCKET hTCP, BYTE* buffer, WORD len)
{
	WORD wGetReadyCount;
	WORD RightLen = 0;

	// See if there is any data which can be read
	wGetReadyCount = TCPIsGetReady(hTCP);
	if(wGetReadyCount == 0u)
		return 0x0000u;

	hCurrentTCP = hTCP;

	// Make sure we don't try to read more data than is available
	if(len > wGetReadyCount)
		len = wGetReadyCount;

	// See if we need a two part get
	if(TCBStubs[hCurrentTCP].rxTail + len > TCBStubs[hCurrentTCP].bufferEnd)
	{
		RightLen = TCBStubs[hCurrentTCP].bufferEnd - TCBStubs[hCurrentTCP].rxTail + 1;
		if(buffer)
		{
			TCPRAMCopy((PTR_BASE)buffer, TCP_PIC_RAM, TCBStubs[hCurrentTCP].rxTail, TCP_PIC_RAM, RightLen);
			buffer += RightLen;
		}
		len -= RightLen;
		TCBStubs[hCurrentTCP].rxTail = TCBStubs[hCurrentTCP].bufferRxStart;
	}

	if(buffer)
		TCPRAMCopy((PTR_BASE)buffer, TCP_PIC_RAM, TCBStubs[hCurrentTCP].rxTail, TCP_PIC_RAM, len);
	TCBStubs[hCurrentTCP].rxTail += len;
	len += RightLen;

	// Send a window update if we've run low on data
	if(wGetReadyCount - len <= len)
	{
		TCBStubs[hCurrentTCP].Flags.bTXASAPWithoutTimerReset = 1;
	}
	else if(!TCBStubs[hCurrentTCP].Flags.bTimer2Enabled)
	// If not already enabled, start a timer so a window
	// update will get sent to the remote node at some point
	{
		TCBStubs[hCurrentTCP].Flags.bTimer2Enabled = TRUE;
		TCBStubs[hCurrentTCP].eventTime2 = (QWORD)(mGetTick() >> 8) + (TCP_WINDOW_UPDATE_TIMEOUT_VAL >> 8);
	}

	return len;
}

/******************************************************************************
 * ---TCPPutArray
 * Writes an array from RAM to a TCP socket.
 ******************************************************************************/
WORD TCPPutArray(TCP_SOCKET hTCP, BYTE* data, WORD len)
{
	WORD wActualLen;
	WORD wFreeTXSpace;
	WORD wRightLen = 0;

	if(hTCP >= MAX_TCP_SOCKETS)
    {
        return 0;
    }

	hCurrentTCP = hTCP;

	wFreeTXSpace = TCPIsPutReady(hTCP);
	if(wFreeTXSpace == 0u)
	{
		TCPFlush(hTCP);
		return 0;
	}

	wActualLen = wFreeTXSpace;
	if(wFreeTXSpace > len)
		wActualLen = len;

	// Send all current bytes if we are crossing half full
	// This is required to improve performance with the delayed
	// acknowledgement algorithm
	if((!TCBStubs[hCurrentTCP].Flags.bHalfFullFlush) && (wFreeTXSpace <= ((TCBStubs[hCurrentTCP].bufferRxStart-TCBStubs[hCurrentTCP].bufferTxStart)>>1)))
	{
		TCPFlush(hTCP);
		TCBStubs[hCurrentTCP].Flags.bHalfFullFlush = TRUE;
	}

	// See if we need a two part put
	if(TCBStubs[hCurrentTCP].txHead + wActualLen >= TCBStubs[hCurrentTCP].bufferRxStart)
	{
		wRightLen = TCBStubs[hCurrentTCP].bufferRxStart-TCBStubs[hCurrentTCP].txHead;
		TCPRAMCopy(TCBStubs[hCurrentTCP].txHead, TCP_PIC_RAM, (PTR_BASE)data, TCP_PIC_RAM, wRightLen);
		data += wRightLen;
		wActualLen -= wRightLen;
		TCBStubs[hCurrentTCP].txHead = TCBStubs[hCurrentTCP].bufferTxStart;
	}

	TCPRAMCopy(TCBStubs[hCurrentTCP].txHead, TCP_PIC_RAM, (PTR_BASE)data, TCP_PIC_RAM, wActualLen);
	TCBStubs[hCurrentTCP].txHead += wActualLen;

	// Send these bytes right now if we are out of TX buffer space
	if(wFreeTXSpace <= len)
	{
		TCPFlush(hTCP);
	}
	// If not already enabled, start a timer so this data will
	// eventually get sent even if the application doens't call
	// TCPFlush()
	else if(!TCBStubs[hCurrentTCP].Flags.bTimer2Enabled)
	{
		TCBStubs[hCurrentTCP].Flags.bTimer2Enabled = TRUE;
		TCBStubs[hCurrentTCP].eventTime2 = (QWORD)(mGetTick() >> 8) + (TCP_AUTO_TRANSMIT_TIMEOUT_VAL >> 8);
	}

	return wActualLen + wRightLen;
}

/******************************************************************************
 * ---GetMaxSegSizeOption
 * Parses the current TCP packet header and extracts the Maximum Segment Size option.
 ******************************************************************************/
static WORD GetMaxSegSizeOption(void)
{
	BYTE vOptionsBytes;
	BYTE vOption;
	WORD wMSS;

	// Find out how many options bytes are in this packet.
	IPSetRxBuffer(2+2+4+4);	// Seek to data offset field, skipping Source port (2), Destination port (2), Sequence number (4), and Acknowledgement number (4)
	vOptionsBytes = MACGet();
	vOptionsBytes = ((vOptionsBytes&0xF0)>>2) - sizeof(TCP_HEADER);

	// Return minimum Maximum Segment Size value of 536 bytes if none are
	// present
	if(vOptionsBytes == 0u)
		return 536;

	// Seek to beginning of options
	MACGetArray(NULL, 7);

	// Search for the Maximum Segment Size option
	while(vOptionsBytes--)
	{
		vOption = MACGet();

		if(vOption == 0u)	// End of Options list
			break;

		if(vOption == 1u)	// NOP option
			continue;

		if(vOption == 2u)	// Maximum Segment Size option
		{
			if(vOptionsBytes < 3u)
				break;

			wMSS = 0;

			// Get option length
			vOption = MACGet();
			if(vOption == 4u)
			{// Retrieve MSS and swap value to little endian
				((BYTE*)&wMSS)[1] = MACGet();
				((BYTE*)&wMSS)[0] = MACGet();
			}

			if(wMSS < 536u)
				break;
			if(wMSS > TCP_MAX_SEG_SIZE_TX)
				return TCP_MAX_SEG_SIZE_TX;
			else
				return wMSS;
		}
		else
		{ // Assume this is a multi byte option and throw it way
			if(vOptionsBytes < 2u)
				break;
			vOption = MACGet();
			if(vOptionsBytes < vOption)
				break;
			MACGetArray(NULL, vOption);
			vOptionsBytes -= vOption;
		}

	}

	// Did not find MSS option, return worst case default
	return 536;
}


/******************************************************************************
 * ---TCPIsPutReady
 * Call this function to determine how many bytes can be written to the
 * TCP TX buffer.  If this function returns zero, the application must
 * return to the main stack loop before continuing in order to transmit more data.
 ******************************************************************************/
WORD TCPIsPutReady(TCP_SOCKET hTCP)
{
	BYTE i;

	if(hTCP >= MAX_TCP_SOCKETS)
    {
        return 0;
    }

	hCurrentTCP = hTCP;

	i = TCBStubs[hCurrentTCP].smState;

	// Unconnected sockets shouldn't be transmitting anything.
	if(!( (i == (BYTE)TCP_ESTABLISHED) || (i == (BYTE)TCP_CLOSE_WAIT) ))
    {
		return 0;
    }

	if(TCBStubs[hCurrentTCP].txHead >= TCBStubs[hCurrentTCP].txTail)
    {
		return (TCBStubs[hCurrentTCP].bufferRxStart - TCBStubs[hCurrentTCP].bufferTxStart - 1) - (TCBStubs[hCurrentTCP].txHead - TCBStubs[hCurrentTCP].txTail);
	}
    else
    {
		return TCBStubs[hCurrentTCP].txTail - TCBStubs[hCurrentTCP].txHead - 1;
    }
}


/******************************************************************************
 * ---TCPIsGetReady
 * Call this function to determine how many bytes can be read from the
 * TCP RX buffer.  If this function returns zero, the application must
 * return to the main stack loop before continuing in order to wait for
 * more data to arrive.
 ******************************************************************************/
WORD TCPIsGetReady(TCP_SOCKET hTCP)
{
	if(hTCP >= MAX_TCP_SOCKETS)
    {
        return 0;
    }

	hCurrentTCP = hTCP;

	if(TCBStubs[hCurrentTCP].rxHead >= TCBStubs[hCurrentTCP].rxTail)
    {
		return TCBStubs[hCurrentTCP].rxHead - TCBStubs[hCurrentTCP].rxTail;
    }
	else
    {
		return (TCBStubs[hCurrentTCP].bufferEnd - TCBStubs[hCurrentTCP].rxTail + 1) + (TCBStubs[hCurrentTCP].rxHead - TCBStubs[hCurrentTCP].bufferRxStart);
    }
}

/******************************************************************************
 * ---TCPIsConnected
 * This function determines if a socket has an established connection to
 * a remote node.  Call this function after calling TCPOpen to determine
 * when the connection is set up and ready for use.  This function was
 * historically used to check for disconnections, but TCPWasReset is now a
 * more appropriate solution.
 ******************************************************************************/
BOOL TCPIsConnected(TCP_SOCKET hTCP)
{
    if(hTCP >= MAX_TCP_SOCKETS)
    {
        return FALSE;
    }

    hCurrentTCP = hTCP;
    return (TCBStubs[hCurrentTCP].smState == TCP_ESTABLISHED);
}
