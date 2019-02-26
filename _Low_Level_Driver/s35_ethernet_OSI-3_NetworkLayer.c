#include "../PLIB.h"

static NODE_INFO    Cache;
static BYTE         IPHeaderLen;

static ICMP_FLAGS   ICMPFlags = {0};
static ICMP_STATE   ICMPState = 0;
static NODE_INFO    ICMPRemote = {0};
static QWORD        ICMPTimer;
static WORD         ICMPSequenceNumber;

/*********************************************************************
 * ---ARPProcess
 * Retrieves an ARP packet from the MAC buffer and determines if it is a
 * response to our request (in which case the ARP is resolved) or if it
 * is a request requiring our response (in which case we transmit one.)
 *
 * TRUE - All processing of this ARP packet is complete.  Do not call
 * again until a new ARP packet is waiting in the RX buffer.
 * FALSE - This function must be called again.  More time is needed to
 * send an ARP response.
 ********************************************************************/
void ARPProcess(void) 
{
    ARP_PACKET packet;

    // Obtain the incoming ARP packet (either an ARP request from a host OR a response from a host to our ARP request)
    MACGetArray((BYTE*) &packet, sizeof(ARP_PACKET));
    MACDiscardRx();
    // Validate the ARP packet
    if((packet.HardwareType == swap_word(HW_ETHERNET)) && (packet.MACAddrLen == sizeof(MAC_ADDR)) && (packet.ProtocolLen == sizeof(IP_ADDR)))
    {
        if (packet.Operation == swap_word(ARP_OPERATION_RESP))  // Handle incoming ARP responses (a host is sending a response to our ARP request)
        {
            Cache.MACAddr = packet.SenderMACAddr;
            Cache.IPAddr = packet.SenderIPAddr;
        }
        else if((packet.Operation == swap_word(ARP_OPERATION_REQ)) && (packet.TargetIPAddr.Val == AppConfig.MyIPAddr.Val))   // Handle incoming ARP requests for our MAC address (a host is sending an ARP request)
        {
            packet.HardwareType = swap_word(HW_ETHERNET);
            packet.Protocol = swap_word(ARP_IP);
            packet.MACAddrLen = sizeof(MAC_ADDR);
            packet.ProtocolLen = sizeof(IP_ADDR);
            packet.Operation = swap_word(ARP_OPERATION_RESP);
            packet.TargetMACAddr = packet.SenderMACAddr;
            packet.TargetIPAddr = packet.SenderIPAddr;
            packet.SenderMACAddr = AppConfig.MyMACAddr;
            packet.SenderIPAddr = AppConfig.MyIPAddr;

            while(!MACIsTxReady());
            MACSetWritePtr(BASE_TX_ADDR);
            MACPutHeader(&packet.TargetMACAddr, MAC_ARP, sizeof(ARP_PACKET));
            MACPutArray((BYTE*) &packet, sizeof(ARP_PACKET));
            MACFlush();
        }
    }
}

/*********************************************************************
 * ---ARPResolve
 * This function transmits and ARP request to determine the hardware
 * address of a given IP address.
 ********************************************************************/
void ARPResolve(IP_ADDR* IPAddr) 
{
    ARP_PACKET packet;

    packet.HardwareType = swap_word(HW_ETHERNET);
    packet.Protocol = swap_word(ARP_IP);
    packet.MACAddrLen = sizeof(MAC_ADDR);
    packet.ProtocolLen = sizeof(IP_ADDR);
    packet.Operation = swap_word(ARP_OPERATION_REQ);
    memset((void*) &packet.TargetMACAddr, 0xff, sizeof(MAC_ADDR));
    packet.TargetIPAddr = ((AppConfig.MyIPAddr.Val ^ IPAddr->Val) & AppConfig.MyMask.Val) ? AppConfig.MyGateway : *IPAddr;
    packet.SenderMACAddr = AppConfig.MyMACAddr;
    packet.SenderIPAddr = AppConfig.MyIPAddr;

    while(!MACIsTxReady());
    MACSetWritePtr(BASE_TX_ADDR);
    MACPutHeader(&packet.TargetMACAddr, MAC_ARP, sizeof(ARP_PACKET));
    MACPutArray((BYTE*) &packet, sizeof(ARP_PACKET));
    MACFlush();
}

/*********************************************************************
 * ---ARPIsResolved
 * This function checks if an ARP request has been resolved yet, and if
 * so, stores the resolved MAC address in the pointer provided.
 ********************************************************************/
BOOL ARPIsResolved(IP_ADDR* IPAddr, MAC_ADDR* MACAddr) 
{
    if((Cache.IPAddr.Val == IPAddr->Val) || ((Cache.IPAddr.Val == AppConfig.MyGateway.Val) && ((AppConfig.MyIPAddr.Val ^ IPAddr->Val) & AppConfig.MyMask.Val))) 
    {
        *MACAddr = Cache.MACAddr;
//        memset((void*) &Cache, 0xff, sizeof(NODE_INFO));
        return TRUE;
    }
    return FALSE;
}

/*********************************************************************
 * ---IPGetHeader
 * TRUE, if valid packet was received
 * FALSE otherwise
 ********************************************************************/
BOOL IPGetHeader(IP_ADDR *remote, IP_HEADER *IPHeader) 
{
    WORD_VAL CalcChecksum;

    MACGetArray((BYTE*) IPHeader, sizeof(IP_HEADER));
    
    IPHeaderLen = (IPHeader->VersionIHL & 0x0f) << 2;
    CalcChecksum.Val = MACCalcRxChecksum(0, IPHeaderLen); // Cheksum validation (0 == OK)
    MACSetReadPtrInRx(IPHeaderLen);     // Seek to the end of the IP header

    if((!CalcChecksum.Val) && ((IPHeader->VersionIHL & 0xf0) == IP_IPv4) || !(IPHeader->FragmentInfo & 0xff1f))
    {
        remote->Val = IPHeader->SourceAddress.Val;
    
        return TRUE;
    }
    
    return FALSE;
}

/*********************************************************************
 * ---IPPutHeader
 *   *remote   - Destination node address
 *   protocol  - Current packet protocol
 *   len       - Current packet data length
 ********************************************************************/
void IPPutHeader(NODE_INFO *remote, BYTE protocol, WORD dataLength) 
{
    IP_HEADER header;
    static WORD identifier = 0x0000;

    identifier++;
    header.VersionIHL = IP_IPv4 | IP_IHL_STANDARD;
    header.TypeOfService = IP_DSCP | IP_ECN;
    header.TotalLength = swap_word(sizeof(IP_HEADER) + dataLength);
    header.Identification = swap_word(identifier);
    header.FragmentInfo = 0x0000;
    header.TimeToLive = IP_TTL;
    header.Protocol = protocol;
    header.HeaderChecksum = 0x0000;
    header.SourceAddress.Val = AppConfig.MyIPAddr.Val;
    header.DestAddress.Val = remote->IPAddr.Val;
    header.HeaderChecksum = CalcIPChecksum((BYTE*) &header, sizeof(IP_HEADER));

    MACPutHeader(&remote->MACAddr, MAC_IP, (sizeof(IP_HEADER) + dataLength));
    MACPutArray((BYTE*) &header, sizeof(IP_HEADER));
}

/*********************************************************************
 * ---IPSetRxBuffer
 *  Next Read/Write access to receive buffer is set to Offset
 ********************************************************************/
void IPSetRxBuffer(WORD Offset)
{
    MACSetReadPtrInRx(Offset + IPHeaderLen);
}

/*********************************************************************
 * ---ICMPBeginUsage
 *  TRUE: You have successfully gained ownership of the ICMP client module and can now use the
 *  ICMPSendPing() and ICMPGetReply() functions.
 *  FALSE: Some other application is using the ICMP
 *  client module.  Calling ICMPSendPing() will corrupt the other application's ping result.
 ********************************************************************/
BOOL ICMPBeginUsage(void)
{
    if(ICMPFlags.bICMPInUse)
    {
        return FALSE;
    }
    ICMPFlags.bICMPInUse = TRUE;
    return TRUE;
}

/*********************************************************************
 * ---ICMPSendPing
 * Begins the process of transmitting an ICMP echo
 * request.  This normally involves an ARP
 * resolution procedure first.
 ********************************************************************/
void ICMPSendPing(DWORD dwRemoteIP) 
{
    ICMPTimer = mGetTick();
    ICMPFlags.bReplyValid = 0;
    ICMPRemote.IPAddr.Val = dwRemoteIP;
    ICMPState = SM_ARP_SEND_QUERY;
}

/*********************************************************************
 * ---ICMPProcess
 * Generates an echo reply, if requested
 * Validates and sets ICMPFlags.bReplyValid if a
 * correct ping response to one of ours is received.
 ********************************************************************/
void ICMPProcess(NODE_INFO *remote, IP_ADDR localIP, WORD len) 
{
    DWORD_VAL dwVal;
    
    // Process this ICMP packet if it the destination IP address matches our address or one of the broadcast IP addressees
    if ((localIP.Val == AppConfig.MyIPAddr.Val) || (localIP.Val == 0xFFFFFFFF) || (localIP.Val == ((AppConfig.MyIPAddr.Val & AppConfig.MyMask.Val) | ~AppConfig.MyMask.Val)))
    {
        MACGetArray((BYTE*) &dwVal, sizeof(dwVal));   // Obtain the ICMP header Type, Code, and Checksum fields

        if(dwVal.w[0] == 0x0008u)  // See if this is an ICMP echo (ping) request of a host
        {    
            if (MACCalcRxChecksum(0 + sizeof(IP_HEADER), len))
            {
                return;
            }

            // Calculate new Type, Code, and Checksum values
            dwVal.v[0] = 0x00; // Type: 0 (ICMP echo/ping reply)
            dwVal.v[2] += 8; // Subtract 0x0800 from the checksum
            if (dwVal.v[2] < 8u) 
            {
                dwVal.v[3]++;
                if (dwVal.v[3] == 0u)
                {
                    dwVal.v[2]++;
                }
            }

            // Wait for TX hardware to become available (finish transmitting
            // any previous packet)
            while (!MACIsTxReady());

            // Position the write pointer for the next IPPutHeader operation
            // NOTE: do not put this before the MACIsTxReady() call for WF compatbility
            MACSetWritePtr(BASE_TX_ADDR + sizeof(ETHER_HEADER));
            // Create IP header in TX memory
            IPPutHeader(remote, IP_PROTOCOLE_ICMP, len);
            // Copy ICMP response into the TX memory
            MACPutArray((BYTE*) &dwVal, sizeof(dwVal));
            MACMemCopyAsync(-1, -1, len - 4);
            MACFlush();
        }
        else if(dwVal.w[0] == 0x0000u) // See if this an ICMP Echo reply to our request
        {
            MACGetArray((BYTE*) &dwVal, sizeof(DWORD_VAL));   // Get the sequence number and identifier fields

            if((dwVal.w[0] != ICMP_IDENTIFIER) || (dwVal.w[1] != ICMPSequenceNumber))   // See if the identifier matches the one we sent
            {
                return;
            }

            MACSetReadPtrInRx(IPHeaderLen);   // Validate the ICMP checksum field
            if(CalcIPBufferChecksum(sizeof(ICMP_PACKET))) // Two bytes of payload were sent in the echo request
            {
                return;
            }

            ICMPFlags.bReplyValid = 1;  // Flag that we received the response and stop the timer ticking
            ICMPTimer = mGetTick() - ICMPTimer;
        }
    }
}

/*********************************************************************
 * ---ICMPGetReply
 * -2: No response received yet
 * -1: Operation timed out (longer than ICMP_TIMEOUT) has elapsed.
 * >=0: Number of TICKs that elapsed between initial ICMP transmission and reception of a valid echo.
 ********************************************************************/
LONG ICMPGetReply(void)
{
    BYTE i = 0;
    ICMP_PACKET ICMPPacket = {0};
        
    switch (ICMPState)
    {
        case SM_ARP_SEND_QUERY:
            if(ARPIsResolved(&ICMPRemote.IPAddr, &ICMPRemote.MACAddr))    // See if the ARP reponse was successfully received
            {
                ICMPState = SM_ICMP_SEND_ECHO_REQUEST;
            }
            else
            {
                ARPResolve(&ICMPRemote.IPAddr);
                ICMPState = SM_ARP_GET_RESPONSE;
            }
            break;
        case SM_ARP_GET_RESPONSE:
            if(ARPIsResolved(&ICMPRemote.IPAddr, &ICMPRemote.MACAddr))    // See if the ARP reponse was successfully received
            {
                ICMPState = SM_ICMP_SEND_ECHO_REQUEST;
            }
            break;
        case SM_ICMP_SEND_ECHO_REQUEST:
            while(!MACIsTxReady());
            ICMPTimer = mGetTick();     // Record the current time.  This will be used as a basis for finding the echo response time, which exludes the ARP and DNS steps

            ICMPPacket.TypeOfMessage = ICMP_ECHO_REQUEST;  
            ICMPPacket.Code = 0x00;
            ICMPPacket.HeaderChecksum = 0x0000;
            ICMPPacket.Identifier = ICMP_IDENTIFIER;
            ICMPPacket.SequenceNumber = ++ICMPSequenceNumber;
            for(i = 0 ; i < 23 ; i++)
            {
                ICMPPacket.Data[i] = ('a' + i);
                if((i + 23) < 32)
                {
                    ICMPPacket.Data[i+23] = ('a' + i);
                }
            }
            ICMPPacket.HeaderChecksum = CalcIPChecksum((BYTE*) &ICMPPacket, sizeof(ICMP_PACKET));

            MACSetWritePtr(BASE_TX_ADDR + sizeof(ETHER_HEADER));   // Position the write pointer for the next IPPutHeader operation
            IPPutHeader(&ICMPRemote, IP_PROTOCOLE_ICMP, sizeof(ICMP_PACKET));
            MACPutArray((BYTE*) & ICMPPacket, sizeof(ICMP_PACKET));
            MACFlush();

            ICMPState = SM_ICMP_GET_ECHO_RESPONSE;
            break;
        case SM_ICMP_GET_ECHO_RESPONSE:
            if (ICMPFlags.bReplyValid)                  // See if the echo was successfully received
            {
                return (LONG) ICMPTimer;
            }
            break;
    }
    
    if (mTickCompare(ICMPTimer) > TICK_4S) // The request timed out
    {
        ICMPTimer = mGetTick();
        ICMPState = SM_IDLE;
        return -1;
    }
    
    return -2;  // Still working.  No response to report yet.
}

/** *******************************************************************
 * ---ICMPEndUsage
 *   Your ownership of the ICMP module is released.
 *   You can no longer use ICMPSendPing().
 ********************************************************************/
void ICMPEndUsage(void)
{
    ICMPFlags.bICMPInUse = FALSE;
}
