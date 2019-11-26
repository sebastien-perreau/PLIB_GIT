#include "../PLIB.h"

/****************************************************************************************************
  Function:
           	 BYTE Discovery(BOOL loop, QWORD waitingPeriod);
  Description:
		   	 Discovery opens a UDP socket and transmits a broadcast packet to port
		   	 \30303. If a computer is on the same subnet and a utility is looking
		  	 for packets on the UDP port, it will receive the broadcast. For this
		  	 application, it is used to announce the change of this board's IP
		  	 address. 
             Frame details:
             '#''#''SizeName''SizeDesc''NumberOfOpenSockets''name''desc''DhcpActive''Mac''Ip''Mask''[SOCKETS 0..X]''END''UCLad''USCTemperature'
  Remarks:
    		 A UDP socket must be available before this function is called. It is
    		 freed at the end of the function. MAX_UDP_SOCKETS may need to be
    		 increased if other modules use UDP sockets.
  ****************************************************************************************************/
BYTE Discovery(BOOL loop, QWORD waitingPeriod, acquisitions_params_t acquisitions)
{
    static enum
    {
        DISCOVERY_HOME = 0,
        DISCOVERY_LISTEN,
        DISCOVERY_REPLY,
        DISCOVERY_END
    } discoverySM = DISCOVERY_HOME;
    static BYTE i, j;
    static BYTE udpBuffer[10];
    static UDP_SOCKET socket;
    static QWORD tickAnnounce;
    static QWORD tickTimeout;
    
    switch(discoverySM)
    {
        case DISCOVERY_HOME:
            // Vérification du lien MAC
            if (MACIsLinked())
            {
                if(loop)
                {
                    if (mTickCompare(tickAnnounce) > waitingPeriod)
                    {
                        tickAnnounce = mGetTick();
                        // Création d'un socket UDP SERVER 
                        if((socket = UDPOpenEx(0, UDP_OPEN_SERVER, 30303, 30303)) != INVALID_UDP_SOCKET)
                        {
                            discoverySM = DISCOVERY_REPLY;
                            tickTimeout = mGetTick();
                        }
                        else
                        {
                            discoverySM = DISCOVERY_END;
                        }
                    }
                }
                else
                {
                    if((socket = UDPOpenEx(0, UDP_OPEN_SERVER, 30303, 30303)) != INVALID_UDP_SOCKET)
                    {
                        discoverySM = DISCOVERY_LISTEN;
                        tickTimeout = mGetTick();
                    }
                    else
                    {
                        discoverySM = DISCOVERY_END;
                    }
                }
            }
            break;
        case DISCOVERY_LISTEN:
            // Si données reçues
            if(UDPIsGetReady(socket))
            {
                UDPGetArray(udpBuffer, sizeof(udpBuffer));
                UDPDiscard();
                if (!strcmp((char *) udpBuffer, "DISCOVERY"))
                {
                    // Lorsque le serveur UDP reçoit une donnée d'un client, alors il récupère et stock son IP & port (via UDPSocketInfo).
                    // Comme Discovery est un serveur UDP alors on écoute tout ce qu'il y a sur le PORT 30303 et on envoi des données 
                    // en 'broadcast' donc à tout le monde (d'ou le fait de réinitialiser - comme UDPOpenEx() les données Info du socket UDP avant le REPLY).
                    memset((void*) &UDPSocketInfo[socket].remoteNode, 0xFF, sizeof(NODE_INFO));
                    UDPSocketInfo[socket].remotePort = 30303;
                    discoverySM = DISCOVERY_REPLY;
                    tickTimeout = mGetTick();
                }
                memset(udpBuffer, 0, sizeof(udpBuffer));
            }
            else if(mTickCompare(tickTimeout) >= TICK_1S)
            {
                discoverySM = DISCOVERY_END;
            }
            break;
        case DISCOVERY_REPLY:
            // Attente d'être certain qu'on peut écrire dans le socket
            if(UDPIsPutReady(socket))
            {
#if (DISCOVERY_VERSION >= 1)
                /* Start of frame */
                UDPPut('#');
                UDPPut('#');
                /* Version of discovery */
                UDPPut(DISCOVERY_VERSION);
#if (DISCOVERY_VERSION >= 2)
                /* Name & Description*/
                UDPPut(sizeof(MY_DEFAULT_NAME)-1);
                UDPPut(sizeof(MY_DEFAULT_DESCT)-1);
#endif
#if (DISCOVERY_VERSION >= 3)
                for(i = 0, j = 0 ; j < MAX_UDP_SOCKETS ; j++)
                {
                    if(UDPSocketInfo[j].smState != UDP_CLOSED)
                    {
                        i++;
                    }
                }
                for(j = 0 ; j < MAX_TCP_SOCKETS ; j++)
                {
                    if(TCBStubs[j].smState != TCP_CLOSED) 
                    {
                        i++;
                    }
                }
                UDPPut(i);
#endif
                UDPPutString((BYTE*) MY_DEFAULT_NAME);
                UDPPutString((BYTE*) MY_DEFAULT_DESCT);
                /* DHCP Enabled */
                UDPPut((BYTE) (AppConfig.bIsDHCPEnabled & 0x01));
                /* MAC Address */
                for (i = 0; i < 6; i++)
                {
                    UDPPut(AppConfig.MyMACAddr.v[i]);
                }
                /* IP Address */
                for (i = 0; i < 4; i++)
                {
                    UDPPut(AppConfig.MyIPAddr.v[i]);
                }
                /* IP Mask */
                for (i = 0; i < 4; i++)
                {
                    UDPPut(AppConfig.MyMask.v[i]);
                }
#if (DISCOVERY_VERSION >= 3)
                /* Sockets LIST with ports */
                for(j = 0 ; j < MAX_UDP_SOCKETS ; j++)
                {
                    if(UDPSocketInfo[j].smState != UDP_CLOSED)
                    {
                        UDPPutString((BYTE*)"#UDP");
                        UDPPut(UDPSocketInfo[j].localPort >> 8);
                        UDPPut(UDPSocketInfo[j].localPort >> 0);
                        UDPPut(UDPSocketInfo[j].remotePort >> 8);
                        UDPPut(UDPSocketInfo[j].remotePort >> 0);
                        for (i = 0; i < 4; i++)
                        {
                            UDPPut(UDPSocketInfo[j].remoteNode.IPAddr.v[i]);
                        }
                        for (i = 0; i < 6; i++)
                        {
                            UDPPut(UDPSocketInfo[j].remoteNode.MACAddr.v[i]);
                        }
                    }
                }
                for(j = 0 ; j < MAX_TCP_SOCKETS ; j++)
                {
                    if(TCBStubs[j].smState != TCP_CLOSED) 
                    {
                        UDPPutString((BYTE*)"#TCP");
                        UDPPut(TCBStubs[j].mLocalPort.Val >> 8);
                        UDPPut(TCBStubs[j].mLocalPort.Val >> 0);
                        UDPPut(TCBStubs[j].mRemotePort.Val >> 8);
                        UDPPut(TCBStubs[j].mRemotePort.Val >> 0);
                        for (i = 0; i < 4; i++)
                        {
                            UDPPut(TCBStubs[j].mRemoteNode.IPAddr.v[i]);
                        }
                        for (i = 0; i < 6; i++)
                        {
                            UDPPut(TCBStubs[j].mRemoteNode.MACAddr.v[i]);
                        }
                    }
                }
                UDPPutString((BYTE*)"END");
                /* UC Load */
                UDPPut((acquisitions.speed >> 24)&0xFF);
                UDPPut((acquisitions.speed >> 16)&0xFF);
                UDPPut((acquisitions.speed >> 8)&0xFF);
                UDPPut((acquisitions.speed >> 0)&0xFF);               
                /* UC Temperature */
                UDPPut(fu_get_integer_value(acquisitions.ntc.temperature));
                UDPPut(fu_get_decimal_value(acquisitions.ntc.temperature, 2));
#if (DISCOVERY_VERSION >= 4)
                /* UC Voltage */
                UDPPut(fu_get_integer_value(acquisitions.voltage.average));
                UDPPut(fu_get_decimal_value(acquisitions.voltage.average, 2));
                /* UC Current */
                UDPPut(fu_get_integer_value(acquisitions.current.average));
                UDPPut(fu_get_decimal_value(acquisitions.current.average, 2));
#endif
#endif
#endif
                /* Send packet */
                UDPFlush();
                discoverySM = DISCOVERY_END;
            }
            else if(mTickCompare(tickTimeout) >= TICK_1S)
            {
                discoverySM = DISCOVERY_HOME;
            }
            break;
        case DISCOVERY_END:
            // Fermeture du socket. Il peut maintenant être utilisé par d'autres modules.
            UDPClose(socket);
            discoverySM = DISCOVERY_HOME;
            break;
    }
    
    return discoverySM;
}

/*****************************************************************************
  Function:
        BYTE SendPingRequest(BYTE *str_ip, QWORD *time)

  Summary:
        Demonstrates use of the ICMP (Ping) client (send a Ping command to a host).
 ***************************************************************************/
BYTE SendPingRequest(BYTE *str_ip, QWORD *time)
{
    static enum
    {
        SM_HOME = 0,
        SM_SET_ICMP_REQUEST,
        SM_GET_ICMP_RESPONSE,
        SM_END
    } smPingIndex = SM_HOME;
    static QWORD tickNoFlood;
    static IP_ADDR adrIPValFormat;
    static BYTE numberOfSendingRequest = 0;
    signed long ret;
    
    switch(smPingIndex)
    {
        case SM_HOME:
            if(mTickCompare(tickNoFlood) >= TICK_1S)
            {
                numberOfSendingRequest = 0;
                smPingIndex = SM_SET_ICMP_REQUEST;
            }
            break;
        case SM_SET_ICMP_REQUEST:
            if(mTickCompare(tickNoFlood) >= TICK_200MS)
            {
                if(ICMPBeginUsage())
                {
                    if(StringToIPAddress(str_ip, &adrIPValFormat))
                    {
                        ICMPSendPing(adrIPValFormat.Val);
                        numberOfSendingRequest++;
                        smPingIndex = SM_GET_ICMP_RESPONSE;
                    }
                }
            }
            break;
        case SM_GET_ICMP_RESPONSE:
            ret = ICMPGetReply();   // -2: In Progress... / -1: Request timeout >4sec / x: Echo receive, time in mS store in ret var.
            if(ret != -2)
            {
                time[numberOfSendingRequest-1] = ret;
                if(numberOfSendingRequest >= 4)
                {
                    smPingIndex = SM_HOME;
                }
                else
                {
                    smPingIndex = SM_SET_ICMP_REQUEST;
                }
                tickNoFlood = mGetTick();
            }
            ICMPEndUsage(); // Finished with the ICMP module, release it so other apps can begin using it
            break;
    }
    return smPingIndex;
}
