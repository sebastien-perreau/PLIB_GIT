#include "../PLIB.h"

void ETH_StackInit(BYTE *str_mac, BYTE *str_ip, BOOL dhcpEnabled)
{    
    IP_ADDR adrIPValFormat;
    MAC_ADDR adrMACValFormat;
    
    if(!StringToMACAddress(str_mac, (BYTE*)&adrMACValFormat))
    {
        adrMACValFormat.v[0] = MY_DEFAULT_MAC_BYTE1;
        adrMACValFormat.v[1] = MY_DEFAULT_MAC_BYTE2;
        adrMACValFormat.v[2] = MY_DEFAULT_MAC_BYTE3;
        adrMACValFormat.v[3] = MY_DEFAULT_MAC_BYTE4;
        adrMACValFormat.v[4] = MY_DEFAULT_MAC_BYTE5;
        adrMACValFormat.v[5] = MY_DEFAULT_MAC_BYTE6;
    }
    
    if(!StringToIPAddress(str_ip, &adrIPValFormat))
    {
        adrIPValFormat.Val = MY_DEFAULT_IP_ADDR_BYTE1 | MY_DEFAULT_IP_ADDR_BYTE2 << 8ul | MY_DEFAULT_IP_ADDR_BYTE3 << 16ul | MY_DEFAULT_IP_ADDR_BYTE4 << 24ul;
    }
    
    memset((void*) &AppConfig, 0x00, sizeof(APP_CONFIG));
    
    AppConfig.bIsDHCPEnabled = dhcpEnabled;
    AppConfig.bInConfigMode = TRUE;
    AppConfig.MyMACAddr.v[0] = adrMACValFormat.v[0];
    AppConfig.MyMACAddr.v[1] = adrMACValFormat.v[1];
    AppConfig.MyMACAddr.v[2] = adrMACValFormat.v[2];
    AppConfig.MyMACAddr.v[3] = adrMACValFormat.v[3];
    AppConfig.MyMACAddr.v[4] = adrMACValFormat.v[4];
    AppConfig.MyMACAddr.v[5] = adrMACValFormat.v[5];
    AppConfig.MyIPAddr.Val = adrIPValFormat.v[0] | adrIPValFormat.v[1] << 8ul | adrIPValFormat.v[2] << 16ul | adrIPValFormat.v[3] << 24ul;
    AppConfig.DefaultIPAddr.Val = adrIPValFormat.v[0] | adrIPValFormat.v[1] << 8ul | adrIPValFormat.v[2] << 16ul | adrIPValFormat.v[3] << 24ul;
    AppConfig.MyMask.Val = MY_DEFAULT_MASK_BYTE1 | MY_DEFAULT_MASK_BYTE2 << 8ul | MY_DEFAULT_MASK_BYTE3 << 16ul | MY_DEFAULT_MASK_BYTE4 << 24ul;
    AppConfig.DefaultMask.Val = MY_DEFAULT_MASK_BYTE1 | MY_DEFAULT_MASK_BYTE2 << 8ul | MY_DEFAULT_MASK_BYTE3 << 16ul | MY_DEFAULT_MASK_BYTE4 << 24ul;
    AppConfig.MyGateway.Val = MY_DEFAULT_GATE_BYTE1 | MY_DEFAULT_GATE_BYTE2 << 8ul | MY_DEFAULT_GATE_BYTE3 << 16ul | MY_DEFAULT_GATE_BYTE4 << 24ul;
    AppConfig.PrimaryDNSServer.Val = MY_DEFAULT_PRIMARY_DNS_BYTE1 | MY_DEFAULT_PRIMARY_DNS_BYTE2 << 8ul | MY_DEFAULT_PRIMARY_DNS_BYTE3 << 16ul | MY_DEFAULT_PRIMARY_DNS_BYTE4 << 24ul;
    AppConfig.SecondaryDNSServer.Val = MY_DEFAULT_SECONDARY_DNS_BYTE1 | MY_DEFAULT_SECONDARY_DNS_BYTE2 << 8ul | MY_DEFAULT_SECONDARY_DNS_BYTE3 << 16ul | MY_DEFAULT_SECONDARY_DNS_BYTE4 << 24ul;

    LFSRSeedRand(GenerateRandomDWORD());
    
    if(!MACInit())
    {
        UDPInit();

        TCPInit();

        DHCPInit();
    }
}

void ETH_StackTask(void)
{
    NODE_INFO remoteNode;
    IP_HEADER IPHeader;
    BYTE frameType;

    if (AppConfig.bIsDHCPEnabled)
    {
        static BOOL bLastLinkState = FALSE;
        BOOL bCurrentLinkState;

        bCurrentLinkState = MACIsLinked();
        if (bCurrentLinkState != bLastLinkState)
        {
            bLastLinkState = bCurrentLinkState;
            if (!bCurrentLinkState)
            {
                AppConfig.MyIPAddr.Val = AppConfig.DefaultIPAddr.Val;
                AppConfig.MyMask.Val = AppConfig.DefaultMask.Val;
                AppConfig.bInConfigMode = TRUE;
                DHCPInit();
            }
        }

        DHCPTask();

        if (DHCPIsBound())
        {
            AppConfig.bInConfigMode = FALSE;
        }
    }

    TCPTick();

    UDPTask();

    // Process as many incomming packets as we can
    while (1)
    {
        UDPDiscard();

        if (!MACGetHeader(&remoteNode.MACAddr, &frameType))
        {
            break;
        }
        
        // Dispatch the packet to the appropriate handler
        switch (frameType) 
        {
            case MAC_ARP:
                ARPProcess();
                break;
            case MAC_IP:
                if (IPGetHeader(&remoteNode.IPAddr, &IPHeader))
                {
                    if(IPHeader.Protocol == IP_PROTOCOLE_ICMP)
                    {
                        ICMPProcess(&remoteNode, IPHeader.DestAddress, (WORD)(swap_word(IPHeader.TotalLength) - ((IPHeader.VersionIHL & 0x0f) << 2)));
                    }
                    else if(IPHeader.Protocol == IP_PROTOCOLE_TCP) 
                    {
                        TCPProcess(&remoteNode, IPHeader.DestAddress, (WORD)(swap_word(IPHeader.TotalLength) - ((IPHeader.VersionIHL & 0x0f) << 2)));
                    }
                    else if(IPHeader.Protocol == IP_PROTOCOLE_UDP) 
                    {
                        // Stop processing packets if we came upon a UDP frame with application data in it
                        if(UDPProcess(&remoteNode, IPHeader.DestAddress, (WORD)(swap_word(IPHeader.TotalLength) - ((IPHeader.VersionIHL & 0x0f) << 2))))
                        {
                            return;
                        }
                    }
                }
                break;
            case MAC_UNKNOWN:
                break;
        }
    }
}


BOOL DHCPClientInitializedOnce = FALSE;
static DHCP_CLIENT_VARS DHCPClient;
static BYTE _DHCPReceive(void);
static void _DHCPSend(BYTE messageType, BOOL bRenewing);

/*****************************************************************************
  Function:
        void DHCPInit(BYTE vInterface)

  Summary:
        Resets the DHCP client module for the specified interface.

  Description:
        Resets the DHCP client module, giving up any current lease, knowledge of
        DHCP servers, etc. for the specified interface.

  Precondition:
        None

  Parameters:
        vInterface - Interface number to initialize DHCP client state variables
                for.   If you only have one interface, specify 0x00.

  Returns:
        None

  Remarks:
        This function may be called multiple times throughout the life of the
        application, if desired.
 ***************************************************************************/
void DHCPInit(void)
{
    // Upon the first call after POR, we must reset all handles to invalid so
    // that we don't inadvertently close someone else's handle.
    if (!DHCPClientInitializedOnce)
    {
        DHCPClientInitializedOnce = TRUE;
        DHCPClient.hDHCPSocket = INVALID_UDP_SOCKET;
    }

    if (DHCPClient.hDHCPSocket != INVALID_UDP_SOCKET)
    {
        UDPClose(DHCPClient.hDHCPSocket);
        DHCPClient.hDHCPSocket = INVALID_UDP_SOCKET;
    }

    // Reset state machine and flags to default values
    DHCPClient.smState = SM_DHCP_GET_SOCKET;

    //	DHCPClient.flags = 0;
    DHCPClient.bIsBound = 0;
    DHCPClient.bEvent = 0;
    DHCPClient.bOfferReceived = 0;
    DHCPClient.bDHCPServerDetected = 0;
    DHCPClient.bUseUnicastMode = 0;

    DHCPClient.bUseUnicastMode = TRUE; // This flag toggles before use, so this statement actually means to start out using broadcast mode.
    DHCPClient.bEvent = TRUE;
}

/*****************************************************************************
  Function:
        void DHCPDisable(BYTE vInterface)

  Summary:
        Disables the DHCP Client for the specified interface.

  Description:
        Disables the DHCP client for the specified interface by sending the state
        machine to "SM_DHCP_DISABLED".  If the interface was previously configured
        by DHCP, the configuration will continue to be used but the module will no
        longer preform any renewals.

  Precondition:
        None

  Parameters:
        vInterface - Interface number to disable the DHCP client on.   If you only
                have one interface, specify 0x00.

  Returns:
        None

  Remarks:
        Since the interface continues using its old configuration, it is possible
        that the lease may expire and the DHCP server provide the IP to another
        client.  The application should replace the current IP address and other
        configuration with static information following a call to this function.
 ***************************************************************************/
void DHCPDisable(void)
{
    if (DHCPClient.hDHCPSocket != INVALID_UDP_SOCKET)
    {
        UDPClose(DHCPClient.hDHCPSocket);
        DHCPClient.hDHCPSocket = INVALID_UDP_SOCKET;
    }

    DHCPClient.smState = SM_DHCP_DISABLED;
}

/*****************************************************************************
  Function:
        void DHCPEnable(BYTE vInterface)

  Summary:
        Enables the DHCP client for the specified interface.

  Description:
        Enables the DHCP client for the specified interface, if it is disabled.
        If it is already enabled, nothing is done.

  Precondition:
        None

  Parameters:
        vInterface - Interface number to enable the DHCP client on.   If you only
                have one interface, specify 0x00.

  Returns:
        None
 ***************************************************************************/
void DHCPEnable(void)
{
    if (DHCPClient.smState == SM_DHCP_DISABLED)
    {
        DHCPClient.smState = SM_DHCP_GET_SOCKET;
        DHCPClient.bIsBound = FALSE;
    }
}

/*****************************************************************************
  Function:
        BOOL DHCPIsEnabled(BYTE vInterface)

  Summary:
        Determins if the DHCP client is enabled on the specified interface.

  Description:
        Determins if the DHCP client is enabled on the specified interface.

  Precondition:
        None

  Parameters:
        vInterface - Interface number to query.   If you only have one interface,
                specify 0x00.

  Returns:
        None
 ***************************************************************************/
BOOL DHCPIsEnabled(void)
{
    return DHCPClient.smState != SM_DHCP_DISABLED;
}

/*****************************************************************************
  Function:
        BOOL DHCPIsBound(BYTE vInterface)

  Summary:
        Determins if the DHCP client has an IP address lease on the specified
        interface.

  Description:
        Determins if the DHCP client has an IP address lease on the specified
        interface.

  Precondition:
        None

  Parameters:
        vInterface - Interface number to query.   If you only have one interface,
                specify 0x00.

  Returns:
        TRUE - DHCP client has obtained an IP address lease (and likely other
                parameters) and these values are currently being used.
        FALSE - No IP address is currently leased
 ***************************************************************************/
BOOL DHCPIsBound(void)
{
    return DHCPClient.bIsBound;
}

/*****************************************************************************
  Function:
        BOOL DHCPStateChanged(BYTE vInterface)

  Summary:
        Determins if the DHCP client on the specified interface has changed states
        or refreshed its IP address lease.

  Description:
        Determins if the DHCP client on the specified interface has changed states
        or refreshed its IP address lease.  This function can be used to determine
        when to update an LCD or other display whenever the DHCP assigned IP
        address has potentially changed.
	
  Precondition:
        None

  Parameters:
        vInterface - Interface number to query.   If you only have one interface,
                specify 0x00.

  Returns:
        TRUE - The IP address lease have been reliquished (due to reinitilization),
                obtained (first event), or renewed since the last call to
                DHCPStateChanged().
        FALSE - The DHCP client has not detected any changes since the last call to
                DHCPStateChanged().
 ***************************************************************************/
BOOL DHCPStateChanged(void)
{
    if (DHCPClient.bEvent)
    {
        DHCPClient.bEvent = 0;
        return TRUE;
    }
    return FALSE;
}

/*****************************************************************************
  Function:
        BOOL DHCPIsServerDetected(BYTE vInterface)

  Summary:
        Determins if the DHCP client on the specified interface has seen a DHCP
        server.

  Description:
        Determins if the DHCP client on the specified interface has seen a DHCP
        server.
	
  Precondition:
        None

  Parameters:
        vInterface - Interface number to query.   If you only have one interface,
                specify 0x00.

  Returns:
        TRUE - At least one DHCP server is attached to the specified network
                interface.
        FALSE - No DHCP servers are currently detected on the specified network
                interface.
 ***************************************************************************/
BOOL DHCPIsServerDetected(void)
{
    return DHCPClient.bDHCPServerDetected;
}

/*****************************************************************************
  Function:
        void DHCPTask(void)

  Summary:
        Performs periodic DHCP tasks for all interfaces.

  Description:
        This function performs any periodic tasks requied by the DHCP module,
        such as sending and receiving messages involved with obtaining and
        maintaining a lease.

  Precondition:
        None

  Parameters:
        None

  Returns:
        None
 ***************************************************************************/
void DHCPTask(void)
{
    switch (DHCPClient.smState)
    {
        case SM_DHCP_DISABLED:
            // When the module is disabled, do absolutely nothing
            break;

        case SM_DHCP_GET_SOCKET:
            // Open a socket to send and receive broadcast messages on
            DHCPClient.hDHCPSocket = UDPOpenEx(0, UDP_OPEN_SERVER, DHCP_CLIENT_PORT, DHCP_SERVER_PORT);
            if (DHCPClient.hDHCPSocket == INVALID_UDP_SOCKET)
                break;

            DHCPClient.smState = SM_DHCP_SEND_DISCOVERY;
            // No break

        case SM_DHCP_SEND_DISCOVERY:
            // Assume default IP Lease time of 60 seconds.
            // This should be minimum possible to make sure that if the
            // server did not specify lease time, we try again after this
            // minimum time.
            DHCPClient.dwLeaseTime = 60;

            DHCPClient.validValues.IPAddress = 0;
            DHCPClient.validValues.Gateway = 0;
            DHCPClient.validValues.Mask = 0;

            DHCPClient.bIsBound = FALSE;
            DHCPClient.bOfferReceived = FALSE;

            // No point in wasting time transmitting a discovery if we are
            // unlinked.  No one will see it.
            if (!MACIsLinked())
                break;

            // Ensure transmitter is ready to accept data
            if (UDPIsPutReady(DHCPClient.hDHCPSocket) < 300u)
                break;

            // Toggle the BOOTP Broadcast flag to ensure compatibility with
            // bad DHCP servers that don't know how to handle broadcast
            // responses.  This results in the next discovery attempt to be
            // made using the opposite mode.
            DHCPClient.bUseUnicastMode ^= 1;

            // Ensure that we transmit to the broadcast IP and MAC addresses
            // The UDP Socket remembers who it was last talking to
            memset((void*) &UDPSocketInfo[DHCPClient.hDHCPSocket].remoteNode, 0xFF, sizeof (UDPSocketInfo[0].remoteNode));

            // Send the DHCP Discover broadcast
            _DHCPSend(DHCP_DISCOVER_MESSAGE, FALSE);

            // Start a timer and begin looking for a response
            DHCPClient.dwTimer = mGetTick();
            DHCPClient.smState = SM_DHCP_GET_OFFER;
            break;

        case SM_DHCP_GET_OFFER:
            // Check to see if a packet has arrived
            if (UDPIsGetReady(DHCPClient.hDHCPSocket) < 250u) {
                // Go back and transmit a new discovery if we didn't get an offer after 2 seconds
                if(mTickCompare(DHCPClient.dwTimer) >= DHCP_TIMEOUT)
                {
                    DHCPClient.smState = SM_DHCP_SEND_DISCOVERY;
                }
                break;
            }

            // Let the DHCP server module know that there is a DHCP server
            // on this network
            DHCPClient.bDHCPServerDetected = TRUE;

            // Check to see if we received an offer
            if (_DHCPReceive() != DHCP_OFFER_MESSAGE)
                break;

            DHCPClient.smState = SM_DHCP_SEND_REQUEST;
            // No break

        case SM_DHCP_SEND_REQUEST:
            if (UDPIsPutReady(DHCPClient.hDHCPSocket) < 258u)
                break;

            // Ensure that we transmit to the broadcast IP and MAC addresses
            // The UDP Socket remembers who it was last talking to, so
            // we must set this back to the broadcast address since the
            // current socket values are the unicast addresses of the DHCP
            // server.
            memset((void*) &UDPSocketInfo[DHCPClient.hDHCPSocket].remoteNode, 0xFF, sizeof (UDPSocketInfo[0].remoteNode));

            // Send the DHCP request message
            _DHCPSend(DHCP_REQUEST_MESSAGE, FALSE);

            // Start a timer and begin looking for a response
            DHCPClient.dwTimer = mGetTick();
            DHCPClient.smState = SM_DHCP_GET_REQUEST_ACK;
            break;

        case SM_DHCP_GET_REQUEST_ACK:
            // Check to see if a packet has arrived
            if (UDPIsGetReady(DHCPClient.hDHCPSocket) < 250u)
            {
                // Go back and transmit a new discovery if we didn't get an ACK after 2 seconds
                if(mTickCompare(DHCPClient.dwTimer) >= DHCP_TIMEOUT)
                {
                    DHCPClient.smState = SM_DHCP_SEND_DISCOVERY;
                }
                break;
            }

            // Check to see if we received an offer
            switch (_DHCPReceive())
            {
                case DHCP_ACK_MESSAGE:
                    UDPClose(DHCPClient.hDHCPSocket);
                    DHCPClient.hDHCPSocket = INVALID_UDP_SOCKET;
                    DHCPClient.dwTimer = mGetTick();
                    DHCPClient.smState = SM_DHCP_BOUND;
                    DHCPClient.bEvent = 1;
                    DHCPClient.bIsBound = TRUE;

                    if (DHCPClient.validValues.IPAddress)
                    {
                        AppConfig.MyIPAddr = DHCPClient.tempIPAddress;
                    }
                    if (DHCPClient.validValues.Mask)
                        AppConfig.MyMask = DHCPClient.tempMask;
                    if (DHCPClient.validValues.Gateway)
                        AppConfig.MyGateway = DHCPClient.tempGateway;

                    break;

                case DHCP_NAK_MESSAGE:
                    DHCPClient.smState = SM_DHCP_SEND_DISCOVERY;
                    break;
            }
            break;

        case SM_DHCP_BOUND:
            if(mTickCompare(DHCPClient.dwTimer) < TICK_1S)
            {
                break;
            }

            // Check to see if our lease is still valid, if so, decrement lease
            // time
            if (DHCPClient.dwLeaseTime >= 2ul)
            {
                DHCPClient.dwTimer += TICK_1S;
                DHCPClient.dwLeaseTime--;
                break;
            }

            // Open a socket to send and receive DHCP messages on
            DHCPClient.hDHCPSocket = UDPOpenEx(0, UDP_OPEN_SERVER, DHCP_CLIENT_PORT, DHCP_SERVER_PORT);
            if (DHCPClient.hDHCPSocket == INVALID_UDP_SOCKET)
                break;

            DHCPClient.smState = SM_DHCP_SEND_RENEW;
            // No break

        case SM_DHCP_SEND_RENEW:
        case SM_DHCP_SEND_RENEW2:
        case SM_DHCP_SEND_RENEW3:
            if (UDPIsPutReady(DHCPClient.hDHCPSocket) < 258u)
                break;

            // Send the DHCP request message
            _DHCPSend(DHCP_REQUEST_MESSAGE, TRUE);
            DHCPClient.bOfferReceived = FALSE;

            // Start a timer and begin looking for a response
            DHCPClient.dwTimer = mGetTick();
            DHCPClient.smState++;
            break;

        case SM_DHCP_GET_RENEW_ACK:
        case SM_DHCP_GET_RENEW_ACK2:
        case SM_DHCP_GET_RENEW_ACK3:
            // Check to see if a packet has arrived
            if (UDPIsGetReady(DHCPClient.hDHCPSocket) < 250u)
            {
                // Go back and transmit a new discovery if we didn't get an ACK after 2 seconds
                if(mTickCompare(DHCPClient.dwTimer) >= DHCP_TIMEOUT)
                {
                    if (++DHCPClient.smState > SM_DHCP_GET_RENEW_ACK3)
                    {
                        DHCPClient.smState = SM_DHCP_SEND_DISCOVERY;
                    }
                }
                break;
            }

            // Check to see if we received an offer
            switch (_DHCPReceive()) {
                case DHCP_ACK_MESSAGE:
                    UDPClose(DHCPClient.hDHCPSocket);
                    DHCPClient.hDHCPSocket = INVALID_UDP_SOCKET;
                    DHCPClient.dwTimer = mGetTick();
                    DHCPClient.smState = SM_DHCP_BOUND;
                    DHCPClient.bEvent = 1;
                    break;

                case DHCP_NAK_MESSAGE:
                    DHCPClient.smState = SM_DHCP_SEND_DISCOVERY;
                    break;
            }
            break;
    }
}

/*****************************************************************************
Function:
  void _DHCPReceive(void)

Description:
  Receives and parses a DHCP message.

Precondition:
  A DHCP message is waiting in the UDP buffer.

Parameters:
  None

Returns:
  One of the DCHP_TYPE* contants.
 ***************************************************************************/
static BYTE _DHCPReceive(void) {
    /*********************************************************************
    DHCP PACKET FORMAT AS PER RFC 1541

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |     op (1)    |   htype (1)   |   hlen (1)    |   hops (1)    |
    +---------------+---------------+---------------+---------------+
    |                            xid (4)                            |
    +-------------------------------+-------------------------------+
    |           secs (2)            |           flags (2)           |
    +-------------------------------+-------------------------------+
    |                          ciaddr  (4)                          |
    +---------------------------------------------------------------+
    |                          yiaddr  (4)                          |
    +---------------------------------------------------------------+
    |                          siaddr  (4)                          |
    +---------------------------------------------------------------+
    |                          giaddr  (4)                          |
    +---------------------------------------------------------------+
    |                                                               |
    |                          chaddr  (16)                         |
    |                                                               |
    |                                                               |
    +---------------------------------------------------------------+
    |                                                               |
    |                          sname   (64)                         |
    +---------------------------------------------------------------+
    |                                                               |
    |                          file    (128)                        |
    +---------------------------------------------------------------+
    |                                                               |
    |                          options (312)                        |
    +---------------------------------------------------------------+

     ********************************************************************/
    BYTE v;
    BYTE i, j;
    BYTE type;
    BOOL lbDone;
    DWORD tempServerID;


    // Assume unknown message until proven otherwise.
    type = DHCP_UNKNOWN_MESSAGE;

    UDPGet(&v); // op

    // Make sure this is BOOT_REPLY.
    if (v == BOOT_REPLY) {
        // Jump to chaddr field (Client Hardware Address -- our MAC address for
        // Ethernet and WiFi networks) and verify that this message is directed
        // to us before doing any other processing.
        UDPSetRxBuffer(28); // chaddr field is at offset 28 in the UDP packet payload -- see DHCP packet format above
        for (i = 0; i < 6u; i++) {
            UDPGet(&v);
            if (v != AppConfig.MyMACAddr.v[i])
                goto UDPInvalid;
        }

        // Check to see if this is the first offer.  If it is, record its
        // yiaddr value ("Your (client) IP address") so that we can REQUEST to
        // use it later.
        if (!DHCPClient.bOfferReceived) {
            UDPSetRxBuffer(16);
            UDPGetArray((BYTE*) & DHCPClient.tempIPAddress, sizeof (DHCPClient.tempIPAddress));
            DHCPClient.validValues.IPAddress = 1;
        }

        // Jump to DHCP options (ignore htype, hlen, hops, xid, secs, flags,
        // ciaddr, siaddr, giaddr, padding part of chaddr, sname, file, magic
        // cookie fields)
        UDPSetRxBuffer(240);

        lbDone = FALSE;
        do {
            // Get the Option number
            // Break out eventually in case if this is a malformed
            // DHCP message, ie: missing DHCP_END_OPTION marker
            if (!UDPGet(&v)) {
                lbDone = TRUE;
                break;
            }

            switch (v) {
                case DHCP_MESSAGE_TYPE:
                    UDPGet(&v); // Skip len
                    // Len must be 1.
                    if (v == 1u) {
                        UDPGet(&type); // Get type

                        // Throw away the packet if we know we don't need it (ie: another offer when we already have one)
                        if (DHCPClient.bOfferReceived && (type == DHCP_OFFER_MESSAGE)) {
                            goto UDPInvalid;
                        }
                    } else
                        goto UDPInvalid;
                    break;

                case DHCP_SUBNET_MASK:
                    UDPGet(&v); // Skip len
                    // Len must be 4.
                    if (v == 4u) {
                        // Check to see if this is the first offer
                        if (DHCPClient.bOfferReceived) {
                            // Discard offered IP mask, we already have an offer
                            for (i = 0; i < 4u; i++)
                                UDPGet(&v);
                        } else {
                            UDPGetArray((BYTE*) & DHCPClient.tempMask, sizeof (DHCPClient.tempMask));
                            DHCPClient.validValues.Mask = 1;
                        }
                    } else
                        goto UDPInvalid;
                    break;

                case DHCP_ROUTER:
                    UDPGet(&j);
                    // Len must be >= 4.
                    if (j >= 4u) {
                        // Check to see if this is the first offer
                        if (DHCPClient.bOfferReceived) {
                            // Discard offered Gateway address, we already have an offer
                            for (i = 0; i < 4u; i++)
                                UDPGet(&v);
                        } else {
                            UDPGetArray((BYTE*) & DHCPClient.tempGateway, sizeof (DHCPClient.tempGateway));
                            DHCPClient.validValues.Gateway = 1;
                        }
                    } else
                        goto UDPInvalid;

                    // Discard any other router addresses.
                    j -= 4;
                    while (j--)
                        UDPGet(&v);
                    break;
                    
                case DHCP_SERVER_IDENTIFIER:
                    UDPGet(&v); // Get len
                    // Len must be 4.
                    if (v == 4u) {
                        UDPGet(&(((BYTE*) & tempServerID)[3])); // Get the id
                        UDPGet(&(((BYTE*) & tempServerID)[2]));
                        UDPGet(&(((BYTE*) & tempServerID)[1]));
                        UDPGet(&(((BYTE*) & tempServerID)[0]));
                    } else
                        goto UDPInvalid;
                    break;

                case DHCP_END_OPTION:
                    lbDone = TRUE;
                    break;

                case DHCP_IP_LEASE_TIME:
                    UDPGet(&v); // Get len
                    // Len must be 4.
                    if (v == 4u) {
                        // Check to see if this is the first offer
                        if (DHCPClient.bOfferReceived) {
                            // Discard offered lease time, we already have an offer
                            for (i = 0; i < 4u; i++)
                                UDPGet(&v);
                        } else {
                            UDPGet(&(((BYTE*) (&DHCPClient.dwLeaseTime))[3]));
                            UDPGet(&(((BYTE*) (&DHCPClient.dwLeaseTime))[2]));
                            UDPGet(&(((BYTE*) (&DHCPClient.dwLeaseTime))[1]));
                            UDPGet(&(((BYTE*) (&DHCPClient.dwLeaseTime))[0]));

                            // In case if our clock is not as accurate as the remote
                            // DHCP server's clock, let's treat the lease time as only
                            // 96.875% of the value given
                            DHCPClient.dwLeaseTime -= DHCPClient.dwLeaseTime >> 5;
                        }
                    } else
                        goto UDPInvalid;
                    break;

                default:
                    // Ignore all unsupport tags.
                    UDPGet(&j); // Get option len
                    while (j--) // Ignore option values
                        UDPGet(&v);
            }
        } while (!lbDone);
    }

    // If this is an OFFER message, remember current server id.
    if (type == DHCP_OFFER_MESSAGE) {
        DHCPClient.dwServerID = tempServerID;
        DHCPClient.bOfferReceived = TRUE;
    } else {
        // For other types of messages, make sure that received
        // server id matches with our previous one.
        if (DHCPClient.dwServerID != tempServerID)
            type = DHCP_UNKNOWN_MESSAGE;
    }

    UDPDiscard(); // We are done with this packet
    return type;

UDPInvalid:
    UDPDiscard();
    return DHCP_UNKNOWN_MESSAGE;
}

/*****************************************************************************
  Function:
        static void _DHCPSend(BYTE messageType, BOOL bRenewing)

  Description:
        Sends a DHCP message.

  Precondition:
        UDP is ready to write a DHCP packet.

  Parameters:
        messageType - One of the DHCP_TYPE constants
        bRenewing - Whether or not this is a renewal request

  Returns:
        None
 ***************************************************************************/
static void _DHCPSend(BYTE messageType, BOOL bRenewing) {
    BYTE i;
    IP_ADDR MyIP;


    UDPPut(BOOT_REQUEST); // op
    UDPPut(BOOT_HW_TYPE); // htype
    UDPPut(BOOT_LEN_OF_HW_TYPE); // hlen
    UDPPut(0); // hops
    UDPPut(0x12); // xid[0]
    UDPPut(0x23); // xid[1]
    UDPPut(0x34); // xid[2]
    UDPPut(0x56); // xid[3]
    UDPPut(0); // secs[0]
    UDPPut(0); // secs[1]
    UDPPut(DHCPClient.bUseUnicastMode ? 0x00 : 0x80); // flags[0] with Broadcast flag clear/set to correspond to bUseUnicastMode
    UDPPut(0); // flags[1]

    // If this is DHCP REQUEST message, use previously allocated IP address.
    if ((messageType == DHCP_REQUEST_MESSAGE) && bRenewing) {
        UDPPutArray((BYTE*) & DHCPClient.tempIPAddress, sizeof (DHCPClient.tempIPAddress));
    } else {
        UDPPut(0x00);
        UDPPut(0x00);
        UDPPut(0x00);
        UDPPut(0x00);
    }

    // Set yiaddr, siaddr, giaddr as zeros,
    for (i = 0; i < 12u; i++)
        UDPPut(0x00);

    // Load chaddr - Client hardware address.
    UDPPutArray((BYTE*) & AppConfig.MyMACAddr, sizeof (AppConfig.MyMACAddr));

    // Set chaddr[6..15], sname and file as zeros.
    for (i = 0; i < 202u; i++)
        UDPPut(0);

    // Load magic cookie as per RFC 1533.
    UDPPut(99);
    UDPPut(130);
    UDPPut(83);
    UDPPut(99);

    // Load message type.
    UDPPut(DHCP_MESSAGE_TYPE);
    UDPPut(DHCP_MESSAGE_TYPE_LEN);
    UDPPut(messageType);

    if (messageType == DHCP_DISCOVER_MESSAGE) {
        // Reset offered flag so we know to act upon the next valid offer
        DHCPClient.bOfferReceived = FALSE;
    }


    if ((messageType == DHCP_REQUEST_MESSAGE) && !bRenewing) {
        // DHCP REQUEST message must include server identifier the first time
        // to identify the server we are talking to.
        // _DHCPReceive() would populate "serverID" when it
        // receives DHCP OFFER message. We will simply use that
        // when we are replying to server.
        // If this is a renwal request, we must not include server id.
        UDPPut(DHCP_SERVER_IDENTIFIER);
        UDPPut(DHCP_SERVER_IDENTIFIER_LEN);
        UDPPut(((BYTE*) (&DHCPClient.dwServerID))[3]);
        UDPPut(((BYTE*) (&DHCPClient.dwServerID))[2]);
        UDPPut(((BYTE*) (&DHCPClient.dwServerID))[1]);
        UDPPut(((BYTE*) (&DHCPClient.dwServerID))[0]);
    }

    // Load our interested parameters
    // This is hardcoded list.  If any new parameters are desired,
    // new lines must be added here.
    UDPPut(DHCP_PARAM_REQUEST_LIST);
    UDPPut(DHCP_PARAM_REQUEST_LIST_LEN);
    UDPPut(DHCP_SUBNET_MASK);
    UDPPut(DHCP_ROUTER);
    UDPPut(DHCP_DNS);
    UDPPut(DHCP_HOST_NAME);

    // Add requested IP address to DHCP Request Message
    if (((messageType == DHCP_REQUEST_MESSAGE) && !bRenewing) ||
            ((messageType == DHCP_DISCOVER_MESSAGE) && DHCPClient.tempIPAddress.Val)) {
        UDPPut(DHCP_PARAM_REQUEST_IP_ADDRESS);
        UDPPut(DHCP_PARAM_REQUEST_IP_ADDRESS_LEN);
        UDPPutArray((BYTE*) & DHCPClient.tempIPAddress, DHCP_PARAM_REQUEST_IP_ADDRESS_LEN);
    }

    // Add any new paramter request here.

    // End of Options.
    UDPPut(DHCP_END_OPTION);

    // Add zero padding to ensure compatibility with old BOOTP relays that discard small packets (<300 UDP octets)
    while (UDPTxCount < 300u)
        UDPPut(0);

    // Make sure we advertise a 0.0.0.0 IP address so all DHCP servers will respond.  If we have a static IP outside the DHCP server's scope, it may simply ignore discover messages.
    MyIP.Val = AppConfig.MyIPAddr.Val;
    if (!bRenewing)
        AppConfig.MyIPAddr.Val = 0x00000000;
    UDPFlush();
    AppConfig.MyIPAddr.Val = MyIP.Val;

}
