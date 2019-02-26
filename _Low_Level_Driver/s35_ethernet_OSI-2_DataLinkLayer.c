/********************************************************************
 *	Ethernet DataLink Layer
 *
 *	Author : Sébastien PERREAU
 *
 *	Revision history	:
 *		13/05/2014		- Initial release
 *      04/10/2016      - Global update for this layer
 *      22/05/2017      - Global update and add external PHYTER LAN8740 compatibility
 *********************************************************************/
#include "../PLIB.h"

/******************************************************************************
 * Prototypes
 ******************************************************************************/
static void _TxAckCallback(void* pPktBuff, int buffIx, void* fParam); // Eth tx buffer acnowledge function
static void* _MacAllocCallback(size_t nitems, size_t size, void* param);
static unsigned short __attribute__((always_inline)) _PhyReadReg(unsigned int rIx, unsigned int phyAdd);
static int _LinkReconfigure(void);

// TX buffers
static volatile sEthTxDcpt _TxDescriptors[EMAC_TX_DESCRIPTORS]; // the statically allocated TX buffers
static volatile sEthTxDcpt* _pTxCurrDcpt = NULL; // the current TX buffer
static int _TxLastDcptIx = 0; // the last TX descriptor used
static unsigned short int _TxCurrSize = 0; // the current TX buffer size

// RX buffers
static unsigned char _RxBuffers[EMAC_RX_DESCRIPTORS][EMAC_RX_BUFF_SIZE]; // rx buffers for incoming data
static unsigned char* _pRxCurrBuff = NULL; // the current RX buffer
static unsigned short int _RxCurrSize = 0; // the current RX buffer size

// general stuff
static unsigned char* _CurrWrPtr = 0; // the current write pointer
static unsigned char* _CurrRdPtr = 0; // the current read pointer

// timing and link status maintenance
static eEthLinkStat _linkPrev = 0; // last value of the link status
static int _linkPresent = 0; // if connection to the PHY properly detected
static int _linkNegotiation = 0; // if an auto-negotiation is in effect

// run time statistics
int _stackMgrRxOkPkts = 0;
int _stackMgrRxBadPkts = 0;
int _stackMgrInGetHdr = 0;
int _stackMgrRxDiscarded = 0;
int _stackMgrTxNotReady = 0;

/******************************************************************************
 * ---MACInit
 *  This function initializes the Eth controller, the MAC and the PHY. It should be called to be able to schedule
 *  any Eth transmit or receive operation.
 ******************************************************************************/

BYTE MACInit(void) 
{
    int initFail = 0;
    int ix;
    eEthRes ethRes, phyInitRes;
    unsigned short* pS = (unsigned short*) AppConfig.MyMACAddr.v;
    BYTE useFactMACAddr[6]  = {0x00, 0x04, 0xa3, 0x00, 0x00, 0x00}; // to check if factory programmed MAC address needed
    BYTE unsetMACAddr[6]    = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // not set MAC address
    
    eEthOpenFlags linkFlags;
    eEthOpenFlags oFlags = (ETH_CFG_AUTO ? ETH_OPEN_AUTO : 0) | (ETH_CFG_10 ? ETH_OPEN_10 : 0) | (ETH_CFG_100 ? ETH_OPEN_100 : 0) | (ETH_CFG_HDUPLEX ? ETH_OPEN_HDUPLEX : 0) | (ETH_CFG_FDUPLEX ? ETH_OPEN_FDUPLEX : 0) | (ETH_CFG_AUTO_MDIX ? ETH_OPEN_MDIX_AUTO : (ETH_CFG_SWAP_MDIX ? ETH_OPEN_MDIX_SWAP : ETH_OPEN_MDIX_NORM));
    eEthMacPauseType pauseType = (oFlags & ETH_OPEN_FDUPLEX) ? ETH_MAC_PAUSE_CPBL_MASK : ETH_MAC_PAUSE_TYPE_NONE;

    _pTxCurrDcpt = _TxDescriptors;
    for (ix = 0; ix < (sizeof (_TxDescriptors) / sizeof (*_TxDescriptors)); ix++) 
    {
        _TxDescriptors[ix].txBusy = 0;
    }

    EthInit(); // Enable module, clear flags...
    phyInitRes = EthPhyInit(oFlags, ETH_PHY_CFG_RMII | ETH_PHY_CFG_ALTERNATE, &linkFlags);

    // let the auto-negotiation (if any) take place
    // continue the initialization
    EthRxFiltersClr(ETH_FILT_ALL_FILTERS);
    EthRxFiltersSet(ETH_FILT_CRC_ERR_REJECT | ETH_FILT_RUNT_REJECT | ETH_FILT_ME_UCAST_ACCEPT | ETH_FILT_MCAST_ACCEPT | ETH_FILT_BCAST_ACCEPT);

    // set the MAC address
    if (!memcmp(AppConfig.MyMACAddr.v, useFactMACAddr, sizeof (useFactMACAddr)) || !memcmp(AppConfig.MyMACAddr.v, unsetMACAddr, sizeof (unsetMACAddr))) 
    { 
        // use the factory programmed address existent in the MAC
        *pS++ = EMACxSA2;
        *pS++ = EMACxSA1;
        *pS = EMACxSA0;
    } 
    else 
    { 
        // use the supplied address
        EMACxSA2 = *pS++;
        EMACxSA1 = *pS++;
        EMACxSA0 = *pS;
    }


    if (EthDescriptorsPoolAdd(EMAC_TX_DESCRIPTORS, ETH_DCPT_TYPE_TX, _MacAllocCallback, 0) != EMAC_TX_DESCRIPTORS) 
    {
        initFail++;
    }

    if (EthDescriptorsPoolAdd(EMAC_RX_DESCRIPTORS, ETH_DCPT_TYPE_RX, _MacAllocCallback, 0) != EMAC_RX_DESCRIPTORS) 
    {
        initFail++;
    }

    EthRxSetBufferSize(EMAC_RX_BUFF_SIZE);

    // set the RX buffers as permanent receive buffers
    for (ix = 0, ethRes = ETH_RES_OK; ix < EMAC_RX_DESCRIPTORS && ethRes == ETH_RES_OK; ix++) 
    {
        void* pRxBuff = _RxBuffers[ix];
        ethRes = EthRxBuffersAppend(&pRxBuff, 1, ETH_BUFF_FLAG_RX_STICKY);
    }

    if (ethRes != ETH_RES_OK) 
    {
        initFail++;
    }

    if (phyInitRes == ETH_RES_OK) 
    { // PHY was detected
        _linkPresent = 1;
        if (oFlags & ETH_OPEN_AUTO) 
        { 
            // we'll just wait for the negotiation to be done
            _linkNegotiation = 1; // performing the negotiation
            _linkPrev = _LinkReconfigure() ? ETH_LINK_ST_UP : ETH_LINK_ST_DOWN; // if negotiation not done yet we need to try it next time
        } 
        else 
        { 
            // no need of negotiation results; just update the MAC
            EthMACOpen(linkFlags, pauseType);
            _linkPrev = EthPhyGetLinkStatus();
        }
    } 
    else 
    {
        initFail++;
    }
    
    return initFail;
}

/******************************************************************************
 * ---MACIsLinked
 * This function checks the link status
 ******************************************************************************/
BOOL MACIsLinked(void) 
{
    return (_linkPrev & ETH_LINK_ST_UP) != 0;
}

/******************************************************************************
 * ---MACIsTxReady
 * TRUE: If data can be inserted in the current TX buffer
 * FALSE: there is no free TX buffer
 ******************************************************************************/
BOOL MACIsTxReady(void) 
{
    int ix;

    EthTxAcknowledgeBuffer(0, _TxAckCallback, 0); // acknowledge everything

    if (_pTxCurrDcpt == 0) 
    {
        for (ix = _TxLastDcptIx + 1; ix<sizeof (_TxDescriptors) / sizeof (*_TxDescriptors); ix++) 
        {
            if (_TxDescriptors[ix].txBusy == 0) 
            { // found a non busy descriptor
                _pTxCurrDcpt = _TxDescriptors + ix;
                _TxLastDcptIx = ix;
                break;
            }
        }
        if (_pTxCurrDcpt == 0) 
        {
            for (ix = 0; ix < _TxLastDcptIx; ix++) 
            {
                if (_TxDescriptors[ix].txBusy == 0) 
                { // found a non busy descriptor
                    _pTxCurrDcpt = _TxDescriptors + ix;
                    _TxLastDcptIx = ix;
                    break;
                }
            }
        }
    }


    if (_pTxCurrDcpt == 0) 
    {
        _stackMgrTxNotReady++;
    }

    return _pTxCurrDcpt != 0;
}

/******************************************************************************
 * ---MACGet
 * MACGet returns the byte pointed to by the current read pointer location and
 * increments the read pointer.
 ******************************************************************************/
BYTE MACGet(void) 
{
    return *_CurrRdPtr++;
}

/******************************************************************************
 * ---MACPut
 * Writes a byte to the current write location and increments the write pointer. 
 ******************************************************************************/
void MACPut(BYTE val) 
{
    *_CurrWrPtr++ = val;
}

/******************************************************************************
 * ---MACGetArray
 * Copies data in the supplied buffer.
 ******************************************************************************/
WORD MACGetArray(BYTE *address, WORD len) 
{
    if (address) 
    {
        memcpy(address, _CurrRdPtr, len);
    }

    _CurrRdPtr += len;
    return len;
}

void MACPutArray(BYTE *buff, WORD len) 
{
    memcpy(_CurrWrPtr, buff, len);
    _CurrWrPtr += len;
}

/******************************************************************************
 * ---MACGetHeader
 * Input:           *remote: Location to store the Source MAC address of the
 *                           received frame.
 *                  *type: Location of a BYTE to store the constant
 *                         MAC_UNKNOWN, ETHER_IP, or ETHER_ARP, representing
 *                         the contents of the Ethernet type field.
 *
 * Output:          TRUE: If a packet was waiting in the RX buffer.  The
 *                        remote, and type values are updated.
 *                  FALSE: If a packet was not pending.  remote and type are
 *                         not changed.
 ******************************************************************************/
BOOL MACGetHeader(MAC_ADDR *remote, BYTE* type) 
{
    void* pNewPkt;
    const sEthRxPktStat* pRxPktStat;
    eEthRes res;

    _stackMgrInGetHdr++;

    // verify the link status
    // if auto negotiation is enabled we may have to reconfigure the MAC

    while (_linkPresent) 
    {
        eEthLinkStat linkCurr;

        linkCurr = EthPhyGetLinkStatus(); // read current PHY status

        if (_linkNegotiation) 
        { // the auto-negotiation turned on
            if ((linkCurr & ETH_LINK_ST_UP) && !(_linkPrev & ETH_LINK_ST_UP)) 
            { // we're up after being done. do renegotiate!
                linkCurr = _LinkReconfigure() ? ETH_LINK_ST_UP : ETH_LINK_ST_DOWN; // if negotiation not done yet we need to try it next time
            }
            // else link went/still down; nothing to do yet
        }
        _linkPrev = linkCurr;

        break;
    }


    MACDiscardRx(); // discard/acknowledge the old RX buffer, if any

    res = EthRxGetBuffer(&pNewPkt, &pRxPktStat);

    if (res == ETH_RES_OK) 
    { // available packet; minimum check

        if (pRxPktStat->rxOk && !pRxPktStat->runtPkt && !pRxPktStat->crcError) 
        { // valid packet;
            WORD_VAL newType;
            _RxCurrSize = pRxPktStat->rxBytes;
            _pRxCurrBuff = pNewPkt;
            _CurrRdPtr = _pRxCurrBuff + sizeof (ETHER_HEADER); // skip the packet header
            // set the packet type
            memcpy(remote, &((ETHER_HEADER*) pNewPkt)->SourceMACAddr, sizeof (*remote));
            *type = MAC_UNKNOWN;
            newType = ((ETHER_HEADER*) pNewPkt)->Type;
            if (newType.v[0] == 0x08 && (newType.v[1] == MAC_IP || newType.v[1] == MAC_ARP)) 
            {
                *type = newType.v[1];
            }

            _stackMgrRxOkPkts++;
        }
    }

    if (_pRxCurrBuff == 0 && pNewPkt) 
    { // failed packet, discard
        EthRxAcknowledgeBuffer(pNewPkt, 0, 0);
        _stackMgrRxBadPkts++;
    }


    return _pRxCurrBuff != 0;
}

/******************************************************************************
 * ---MACPutHeader
 * Sets the write pointer at the beginning of the current TX buffer
 * and sets the ETH header and the frame length. Updates the write pointer
 ******************************************************************************/
void MACPutHeader(MAC_ADDR *remote, BYTE type, WORD dataLen) 
{
    _TxCurrSize = dataLen + sizeof(ETHER_HEADER);
    _CurrWrPtr = (unsigned char*) _pTxCurrDcpt->dataBuff;

    memcpy(_CurrWrPtr, remote, sizeof(MAC_ADDR));
    _CurrWrPtr += sizeof(MAC_ADDR);
    memcpy(_CurrWrPtr, &AppConfig.MyMACAddr, sizeof(MAC_ADDR));
    _CurrWrPtr += sizeof(MAC_ADDR);

    *_CurrWrPtr++ = 0x08;
    *_CurrWrPtr++ = type;
}

/******************************************************************************
 * ---MACCalcRxChecksum
 * This function performs a checksum calculation in the current receive buffer.
 ******************************************************************************/
WORD MACCalcRxChecksum(WORD offset, WORD len) 
{
    return CalcIPChecksum(_pRxCurrBuff + sizeof(ETHER_HEADER) + offset, len);
}

/******************************************************************************
 * ---MACDiscardRx
 * Marks the last received packet (obtained using
 * MACGetHeader())as being processed and frees the buffer
 * memory associated with it.
 * It acknowledges the ETHC.
 ******************************************************************************/
void MACDiscardRx(void)
{
    if (_pRxCurrBuff) 
    { // an already existing packet
        EthRxAcknowledgeBuffer(_pRxCurrBuff, 0, 0);
        _pRxCurrBuff = 0;
        _RxCurrSize = 0;

        _stackMgrRxDiscarded++;
    }
}

void MACFlush(void) 
{
    if (_pTxCurrDcpt && _TxCurrSize) // there is a buffer to transmit
    { 
        _pTxCurrDcpt->txBusy = 1;
        EthTxSendBuffer((void*) _pTxCurrDcpt->dataBuff, _TxCurrSize);
        _pTxCurrDcpt = 0;
        _TxCurrSize = 0;
    }
}

/******************************************************************************
 * ---MACMemCopyAsync
 * Copies data from one address to another within the Ethernet memory.
 * Overlapped memory regions are allowed only if the destination start address
 * is at a lower memory address than the source address.
 ******************************************************************************/
void MACMemCopyAsync(PTR_BASE destAddr, PTR_BASE sourceAddr, WORD len) 
{
    if (len) 
    {
        unsigned char *pDst, *pSrc;

        pDst = (destAddr == -1) ? _CurrWrPtr : (unsigned char*) destAddr;
        pSrc = (sourceAddr == -1) ? _CurrRdPtr : (unsigned char*) sourceAddr;

        memcpy(pDst, pSrc, len);
    }
}

/******************************************************************************
 * ---MACSetReadPtrInRx
 * The current read pointer is updated.  All calls to
 * MACGet() and MACGetArray() will use these new values
 ******************************************************************************/
void MACSetReadPtrInRx(WORD offset) 
{
    _CurrRdPtr = _pRxCurrBuff + sizeof(ETHER_HEADER) + offset;
}

PTR_BASE MACGetTxBaseAddr(void) 
{
    return _pTxCurrDcpt ? (PTR_BASE) _pTxCurrDcpt->dataBuff : 0;
}

/******************************************************************************
 * ---MACSetWritePtr
 *  This function sets the new write pointer.
 ******************************************************************************/
PTR_BASE MACSetWritePtr(PTR_BASE address) 
{
    unsigned char* oldPtr;
    oldPtr = _CurrWrPtr;
    _CurrWrPtr = (unsigned char*) address;
    return (PTR_BASE) oldPtr;
}

/******************************************************************************
 * ---MACSetReadPtr
 *   This function sets the new read pointer value.
 ******************************************************************************/
PTR_BASE MACSetReadPtr(PTR_BASE address) 
{
    unsigned char* oldPtr;
    oldPtr = _CurrRdPtr;
    _CurrRdPtr = (unsigned char*) address;
    return (PTR_BASE) oldPtr;
}

/******************************************************************************
 * ---MACGetFreeRxSize
 *   An estimate of how much RX buffer space is free at the present time.
 ******************************************************************************/
WORD MACGetFreeRxSize(void) 
{
    int avlblRxBuffs = sizeof (_RxBuffers) / sizeof (*_RxBuffers) - EthDescriptorsGetRxUnack(); // avlbl=allBuffs-unAck
    return avlblRxBuffs * (sizeof (_RxBuffers[0]) / sizeof (*_RxBuffers[0])); // avlbl* sizeof(buffer)
}

/*********************************************************************
 * ---CalcIPBufferChecksum
 *  This function performs a checksum calculation of the buffer
 *  pointed by the current value of the read pointer.
 ********************************************************************/
WORD CalcIPBufferChecksum(WORD len) 
{
    return CalcIPChecksum(_CurrRdPtr, len);
}

/******************************************************************************
 * ---_EthPhyInit
 * This function initializes the PHY communication.
 ******************************************************************************/
eEthRes EthPhyInit(eEthOpenFlags oFlags, eEthPhyCfgFlags cFlags, eEthOpenFlags* pResFlags) 
{
    unsigned short ctrlReg;
    eEthPhyCfgFlags hwFlags = EthPhyGetHwConfigFlags();
    unsigned short phyCpbl, openReqs, matchCpbl;
    eEthRes res;
    
    mInitIOAsOutput(__PD11);    // ALT MDC
    mInitIOAsOutput(__PA15);    // ALT TXEN
    mInitIOAsOutput(__PD14);    // ALT TXD0
    mInitIOAsOutput(__PD15);    // ALT TXD1
    
    mInitIOAsInput(__PD8);      // ALT MDIO
    mInitIOAsInput(__PG9);      // ALT RXCLK
    mInitIOAsInput(__PG8);      // ALT RXDV
    mInitIOAsInput(__PE8);      // ALT RXD0
    mInitIOAsInput(__PE9);      // ALT RXD1
    mInitIOAsInput(__PG15);     // ALT RXERR
    
    EthMIIMConfig(PERIPHERAL_FREQ, (UINT) 2500000);
    
    if(EthPhyDetectAndReset() > 0) 
    { 
        return ETH_RES_DTCT_ERR;    // failed to detect and/or reset the PHY
    }

    if ((cFlags ^ hwFlags) != 0) 
    { 
        return ETH_RES_CFG_ERR;     // hw-sw configuration mismatch MII/RMII, ALT/DEF config
    }

    oFlags |= (cFlags & ETH_PHY_CFG_RMII) ? ETH_OPEN_RMII : ETH_OPEN_MII;

    // provide some defaults
    if (!(oFlags & (ETH_OPEN_FDUPLEX | ETH_OPEN_HDUPLEX))) 
    {
        oFlags |= ETH_OPEN_HDUPLEX;
    }
    if (!(oFlags & (ETH_OPEN_100 | ETH_OPEN_10))) 
    {
        oFlags |= ETH_OPEN_10;
    }

    if (oFlags & ETH_OPEN_AUTO) 
    {
        openReqs = _BMSTAT_AN_ABLE_MASK;

        if (oFlags & ETH_OPEN_100) 
        {
            if (oFlags & ETH_OPEN_FDUPLEX) 
            {
                openReqs |= _BMSTAT_BASE100TX_FDX_MASK;
            }
            if (oFlags & ETH_OPEN_HDUPLEX) 
            {
                openReqs |= _BMSTAT_BASE100TX_HDX_MASK;
            }
        }

        if (oFlags & ETH_OPEN_10) 
        {
            if (oFlags & ETH_OPEN_FDUPLEX) 
            {
                openReqs |= _BMSTAT_BASE10T_FDX_MASK;
            }
            if (oFlags & ETH_OPEN_HDUPLEX) 
            {
                openReqs |= _BMSTAT_BASE10T_HDX_MASK;
            }
        }
    } 
    else 
    { // no auto negotiation
        if (oFlags & ETH_OPEN_100) 
        {
            openReqs = (oFlags & ETH_OPEN_FDUPLEX) ? _BMSTAT_BASE100TX_FDX_MASK : _BMSTAT_BASE100TX_HDX_MASK;
        } 
        else 
        {
            openReqs = (oFlags & ETH_OPEN_FDUPLEX) ? _BMSTAT_BASE10T_FDX_MASK : _BMSTAT_BASE10T_HDX_MASK;
        }
    }

    // try to match the oFlags with the PHY capabilities
    phyCpbl = _PhyReadReg(PHY_REG_BMSTAT, PHY_ADDRESS);
    matchCpbl = (openReqs & (MAC_COMM_CPBL_MASK | _BMSTAT_AN_ABLE_MASK)) & phyCpbl; // common features
    if (!(matchCpbl & MAC_COMM_CPBL_MASK)) 
    {
        return ETH_RES_CPBL_ERR;
    }

    // we're ok, we can configure the PHY
    res = EthPhyConfigureMII(cFlags);
    if (res != ETH_RES_OK) 
    {
        return res;
    }

    if (matchCpbl & _BMSTAT_AN_ABLE_MASK) 
    { // ok, we can perform auto negotiation
        unsigned short anadReg;

        anadReg = (((matchCpbl >> _BMSTAT_NEGOTIATION_POS) << _ANAD_NEGOTIATION_POS) & _ANAD_NEGOTIATION_MASK) | 0x01;
        if (ETH_MAC_PAUSE_CPBL_MASK & ETH_MAC_PAUSE_TYPE_PAUSE) 
        {
            anadReg |= _ANAD_PAUSE_MASK;
        }
        if (ETH_MAC_PAUSE_CPBL_MASK & ETH_MAC_PAUSE_TYPE_ASM_DIR) 
        {
            anadReg |= _ANAD_ASM_DIR_MASK;
        }

        EthMIIMWriteStart(PHY_REG_ANAD, PHY_ADDRESS, anadReg); // advertise our capabilities

        EthPhyRestartNegotiation(); // restart negotiation and we'll have to wait
    } 
    else 
    { // ok, just don't use negotiation

        ctrlReg = 0;
        if (matchCpbl & (_BMSTAT_BASE100TX_HDX_MASK | _BMSTAT_BASE100TX_FDX_MASK)) // set 100Mbps request/capability
        {
            ctrlReg |= _BMCON_SPEED_MASK;
        }

        if (matchCpbl & (_BMSTAT_BASE10T_FDX_MASK | _BMSTAT_BASE100TX_FDX_MASK)) 
        {
            ctrlReg |= _BMCON_DUPLEX_MASK;
        }

        if (oFlags & ETH_OPEN_PHY_LOOPBACK) 
        {
            ctrlReg |= _BMCON_LOOPBACK_MASK;
        }

        EthMIIMWriteStart(PHY_REG_BMCON, PHY_ADDRESS, ctrlReg); // update the configuration
    }


    // now update the open flags
    // the upper layer needs to know the PHY set-up to further set-up the MAC.

    // clear the capabilities
    oFlags &= ~(ETH_OPEN_AUTO | ETH_OPEN_FDUPLEX | ETH_OPEN_HDUPLEX | ETH_OPEN_100 | ETH_OPEN_10);

    if (matchCpbl & _BMSTAT_AN_ABLE_MASK) {
        oFlags |= ETH_OPEN_AUTO;
    }
    if (matchCpbl & (_BMSTAT_BASE100TX_HDX_MASK | _BMSTAT_BASE100TX_FDX_MASK)) // set 100Mbps request/capability
    {
        oFlags |= ETH_OPEN_100;
    }
    if (matchCpbl & (_BMSTAT_BASE10T_HDX_MASK | _BMSTAT_BASE10T_FDX_MASK)) // set 10Mbps request/capability
    {
        oFlags |= ETH_OPEN_10;
    }
    if (matchCpbl & (_BMSTAT_BASE10T_FDX_MASK | _BMSTAT_BASE100TX_FDX_MASK)) 
    {
        oFlags |= ETH_OPEN_FDUPLEX;
    }
    if (matchCpbl & (_BMSTAT_BASE10T_HDX_MASK | _BMSTAT_BASE100TX_HDX_MASK)) 
    {
        oFlags |= ETH_OPEN_HDUPLEX;
    }

    *pResFlags = oFlags; // upper layer needs to know the PHY set-up to further set-up the MAC.

    return ETH_RES_OK;

}

/*********************************************************************
 * ---EthPhyGetNegotiationResult
 *  This function returns the result of a previously initiated negotiation.
 *  The result is based on the PHY status!.
 ********************************************************************/
eEthLinkStat EthPhyGetNegotiationResult(eEthOpenFlags* pFlags, eEthMacPauseType* pPauseType) 
{
    eEthLinkStat linkStat;
    eEthOpenFlags oFlags;
    __BMSTATbits_t phyStat;
    __ANEXPbits_t phyExp;
    __ANLPADbits_t lpAD;
    __ANADbits_t anadReg;
    eEthMacPauseType pauseType;


    oFlags = 0; // don't know the result yet
    pauseType = ETH_MAC_PAUSE_TYPE_NONE;

    phyStat.w = _PhyReadReg(PHY_REG_BMSTAT, PHY_ADDRESS);
    if (phyStat.AN_COMPLETE == 0) {
        linkStat = (ETH_LINK_ST_DOWN | ETH_LINK_ST_NEG_TMO);
    } else if (!phyStat.LINK_STAT) {
        linkStat = ETH_LINK_ST_DOWN;
    } else { // we're up and running
        int lcl_Pause, lcl_AsmDir, lp_Pause, lp_AsmDir; // pause capabilities, local and LP

        linkStat = ETH_LINK_ST_UP;

        lcl_Pause = (ETH_MAC_PAUSE_CPBL_MASK & ETH_MAC_PAUSE_TYPE_PAUSE) ? 1 : 0;
        lcl_AsmDir = (ETH_MAC_PAUSE_CPBL_MASK & ETH_MAC_PAUSE_TYPE_ASM_DIR) ? 1 : 0;
        lp_Pause = lp_AsmDir = 0; // in case negotiation fails
        lpAD.w = _ANAD_BASE10T_MASK; // lowest priority resolution

        phyExp.w = _PhyReadReg(PHY_REG_ANEXP, PHY_ADDRESS);
        if (phyExp.LP_AN_ABLE) { // ok,valid auto negotiation info

            lpAD.w = _PhyReadReg(PHY_REG_ANLPAD, PHY_ADDRESS);
            if (lpAD.REM_FAULT) {
                linkStat |= ETH_LINK_ST_REMOTE_FAULT;
            }

            if (lpAD.PAUSE) {
                linkStat |= ETH_LINK_ST_LP_PAUSE;
                lp_Pause = 1;
            }
            if (lpAD.ASM_DIR) {
                linkStat |= ETH_LINK_ST_LP_ASM_DIR;
                lp_AsmDir = 1;
            }

        } else {
            linkStat |= ETH_LINK_ST_LP_NEG_UNABLE;
            if (phyExp.PDF) {
                linkStat |= ETH_LINK_ST_PDF;
            }
        }

        // set the PHY connection params

        anadReg.w = _PhyReadReg(PHY_REG_ANAD, PHY_ADDRESS); // get our advertised capabilities
        anadReg.w &= lpAD.w; // get the matching ones
        // get the settings, according to IEEE 802.3 Annex 28B.3 Priority Resolution
        // Note: we don't support 100BaseT4 !

        if (anadReg.w & _ANAD_BASE100TX_FDX_MASK) {
            oFlags = (ETH_OPEN_100 | ETH_OPEN_FDUPLEX);
        } else if (anadReg.w & _ANAD_BASE100TX_MASK) {
            oFlags = (ETH_OPEN_100 | ETH_OPEN_HDUPLEX);
        } else if (anadReg.w & _ANAD_BASE10T_FDX_MASK) {
            oFlags = (ETH_OPEN_10 | ETH_OPEN_FDUPLEX);
        } else if (anadReg.w & _ANAD_BASE10T_MASK) {
            oFlags = (ETH_OPEN_10 | ETH_OPEN_HDUPLEX);
        } else { // this should NOT happen!
            linkStat |= ETH_LINK_ST_NEG_FATAL_ERR;
            linkStat &= ~ETH_LINK_ST_UP; // make sure we stop...!
        }


        // set the pause type for the MAC
        // according to IEEE Std 802.3-2002 Tables 28B-2, 28B-3
        if (oFlags & ETH_OPEN_FDUPLEX) { // pause type relevant for full duplex only
            if (lp_Pause & (lcl_Pause | (lcl_AsmDir & lp_AsmDir))) {
                pauseType = ETH_MAC_PAUSE_TYPE_EN_TX;
            }
            if (lcl_Pause & (lp_Pause | (lcl_AsmDir & lp_AsmDir))) {
                pauseType |= ETH_MAC_PAUSE_TYPE_EN_RX;
            }
        }
    }

    if (pFlags) {
        *pFlags = oFlags;
    }

    if (pPauseType) {
        *pPauseType = pauseType;
    }
    return linkStat;
}

/*********************************************************************
 * ---EthPhyNegotiationComplete
 * This function waits for a previously initiated PHY negotiation to complete.
 * Subsequently, EthPhyGetNegotiationResult() can be called.
 ********************************************************************/
eEthRes EthPhyNegotiationComplete(void) 
{
    __BMCONbits_t phyBMCon;
    __BMSTATbits_t phyStat;
    eEthRes res;

    phyBMCon.w = _PhyReadReg(PHY_REG_BMCON, PHY_ADDRESS);
    if (phyBMCon.AN_ENABLE) 
    {
        phyBMCon.w = _PhyReadReg(PHY_REG_BMCON, PHY_ADDRESS);
        phyStat.w = _PhyReadReg(PHY_REG_BMSTAT, PHY_ADDRESS);
    }

    if (!phyBMCon.AN_ENABLE) 
    {
        res = ETH_RES_NEGOTIATION_INACTIVE; // no negotiation is taking place!
    } 
    else if (phyBMCon.AN_RESTART) 
    {
        res = ETH_RES_NEGOTIATION_NOT_STARTED; // not started yet/tmo
    } 
    else 
    {
        res = (phyStat.AN_COMPLETE == 0) ? ETH_RES_NEGOTIATION_ACTIVE : ETH_RES_OK; // active/tmo/ok
    }

    return res;
}

/******************************************************************************
 * ---_EthPhyRestartNegotiation
 * This function restarts the PHY negotiation.
 * After this restart the link can be reconfigured.
 * The EthPhyGetNegotiationResults() can be used to see the outcoming result.
 ******************************************************************************/
eEthRes EthPhyRestartNegotiation(void) 
{
    eEthRes res;
    __BMSTATbits_t phyCpbl;

    phyCpbl.w = _PhyReadReg(PHY_REG_BMSTAT, PHY_ADDRESS);

    if (phyCpbl.AN_ABLE) { // ok, we can perform auto negotiation
        EthMIIMWriteStart(PHY_REG_BMCON, PHY_ADDRESS, _BMCON_AN_ENABLE_MASK | _BMCON_AN_RESTART_MASK); // restart negotiation and we'll have to wait
        res = ETH_RES_OK;
    } else {
        res = ETH_RES_NEGOTIATION_UNABLE; // no negotiation ability!
    }

    return res;
}

/******************************************************************************
 * ---EthPhyGetLinkStatus
 * This function reads the PHY to get current link status
 * If refresh is specified then, if the link is down a second read
 * will be performed to return the current link status.
 ******************************************************************************/
eEthLinkStat EthPhyGetLinkStatus(void) 
{
    __BMSTATbits_t phyStat;
    eEthLinkStat linkStat;

    phyStat.w = _PhyReadReg(PHY_REG_BMSTAT, PHY_ADDRESS);
    linkStat = (phyStat.LINK_STAT) ? ETH_LINK_ST_UP : ETH_LINK_ST_DOWN;
    if (phyStat.REM_FAULT) 
    {
        linkStat |= ETH_LINK_ST_REMOTE_FAULT;
    }

    return linkStat;
}

/******************************************************************************
 * ---EthPhyConfigureMII
 * This function configures the PHY only for RMII operation mode. 
 ******************************************************************************/
eEthRes EthPhyConfigureMII(eEthPhyCfgFlags cFlags) 
{
#if PHY_ADDRESS == PHY_ADRESS_LAN8740
    // For LAN8740: RMII mode is determined by RMIISEL pin on the device (pull up: RMII, pull down MII)
    unsigned short phyReg = _PhyReadReg(PHY_LAN8740_SMR, PHY_ADDRESS);
    if (((cFlags & ETH_PHY_CFG_RMII) == 0) || ((phyReg & 0x4000) == 0))
    {
        return ETH_RES_CFG_ERR; 
    }
    return ETH_RES_OK;
#elif PHY_ADDRESS == PHY_ADRESS_DP83848
    return ((cFlags & ETH_PHY_CFG_RMII) == 0) ? ETH_RES_CFG_ERR : ETH_RES_OK;
#endif
}

/******************************************************************************
 * ---_PhyDetect
 * This function detects the PHY.
 ******************************************************************************/
BYTE EthPhyDetectAndReset() 
{
    __BMCONbits_t bmcon;
    QWORD tickReset;
    
    // 0. Wait until the soft reset bit is self clearing.
    tickReset = mGetTick();
    do
    {
        bmcon.w = _PhyReadReg(PHY_REG_BMCON, PHY_ADDRESS);
        if (mTickCompare(tickReset) > TICK_1S)
        {
            return 1;
        }
    }
    while(bmcon.RESET);
    
    // 1. Do a soft reset by setting the soft reset bit and wait until operation done.
    EthMIIMWriteStart(PHY_REG_BMCON, PHY_ADDRESS, _BMCON_RESET_MASK);
    tickReset = mGetTick();
    do
    {
        bmcon.w = _PhyReadReg(PHY_REG_BMCON, PHY_ADDRESS);
        if (mTickCompare(tickReset) > TICK_1S)
        {
            return 2;
        }
    }
    while(bmcon.RESET);

    // 3. Write and read back two control bits in order to verify the good communication.
    EthMIIMWriteStart(PHY_REG_BMCON, PHY_ADDRESS, _BMCON_LOOPBACK_MASK | _BMCON_ISOLATE_MASK);
    bmcon.w = _PhyReadReg(PHY_REG_BMCON, PHY_ADDRESS);
    if (bmcon.LOOPBACK == 0 || bmcon.ISOLATE == 0) 
    {
        return 3;
    }
    
    // 4. Restore previous config.
    bmcon.w &= ~(_BMCON_LOOPBACK_MASK | _BMCON_ISOLATE_MASK);
    EthMIIMWriteStart(PHY_REG_BMCON, PHY_ADDRESS, bmcon.w); // clear bits and write
    bmcon.w = _PhyReadReg(PHY_REG_BMCON, PHY_ADDRESS); // read back
    if (bmcon.LOOPBACK || bmcon.ISOLATE) 
    {
        return 4;
    }
    
    return 0;
}

/******************************************************************************
 * ---_TxAckCallback
 * TX acknowledge call back function.
 * Called by the Eth MAC when TX buffers are acknoledged (as a result of a call to EthTxAcknowledgeBuffer).
 ******************************************************************************/
static void _TxAckCallback(void* pPktBuff, int buffIx, void* fParam) 
{
    volatile sEthTxDcpt* pDcpt;
    pDcpt = (sEthTxDcpt*) ((char*) pPktBuff - offsetof(sEthTxDcpt, dataBuff));
    pDcpt->txBusy = 0;
}

static void* _MacAllocCallback(size_t nitems, size_t size, void* param) 
{
    return calloc(nitems, size);
}

static unsigned short __attribute__((always_inline)) _PhyReadReg(unsigned int rIx, unsigned int phyAdd) 
{
    EthMIIMReadStart(rIx, phyAdd);
    return EthMIIMReadResult();
}

/*********************************************************************
 * ---_LinkReconfigure
 * Performs re-configuration after auto-negotiation performed.
 ********************************************************************/
static int _LinkReconfigure(void) 
{
    eEthOpenFlags linkFlags;
    eEthLinkStat linkStat;
    eEthMacPauseType pauseType;
    eEthRes phyRes;
    int success = 0;

    phyRes = EthPhyNegotiationComplete(); // see if negotiation complete
    if (phyRes == ETH_RES_OK) 
    {
        linkStat = EthPhyGetNegotiationResult(&linkFlags, &pauseType);
        if (linkStat & ETH_LINK_ST_UP) 
        { // negotiation succeeded; properly update the MAC
            linkFlags |= (EthPhyGetHwConfigFlags() & ETH_PHY_CFG_RMII) ? ETH_OPEN_RMII : ETH_OPEN_MII;
            EthMACOpen(linkFlags, pauseType);
            success = 1;
        }
    }

    return success;
}
