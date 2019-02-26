#ifndef ETHERNET_DATALINKLAYER_H
#define	ETHERNET_DATALINKLAYER_H
#endif

#define IP_ADDR                         DWORD_VAL

#define	PHY_ADDRESS                     PHY_ADRESS_LAN8740

#define	ETH_CFG_AUTO                    1		// use auto negotiation
#define	ETH_CFG_10                      1		// use/advertise 10 Mbps capability
#define	ETH_CFG_100                     1       // use/advertise 100 Mbps capability
#define	ETH_CFG_HDUPLEX                 1		// use/advertise half duplex capability
#define	ETH_CFG_FDUPLEX                 1		// use/advertise full duplex capability
#define	ETH_CFG_AUTO_MDIX               1  		// use/advertise auto MDIX capability
#define	ETH_CFG_SWAP_MDIX               1		// use swapped MDIX. else normal MDIX

// =======================================================================
//   NETWORK CONFIGURATION
// =======================================================================
//Physical 
#define PHY_ADRESS_LAN8740              0x0
#define PHY_ADRESS_DP83848              0x1

//MAC
#define BASE_TX_ADDR                    (MACGetTxBaseAddr())
#define MAC_TX_BUFFER_SIZE              (1500)
#define MAC_IP                          (0x00u)
#define MAC_ARP                         (0x06u)
#define MAC_UNKNOWN                     (0xFFu)

#define EMAC_TX_DESCRIPTORS             2		// number of the TX descriptors to be created
#define EMAC_RX_DESCRIPTORS             8		// number of the RX descriptors and RX buffers to be created
#define	EMAC_RX_BUFF_SIZE               1536	// size of a RX buffer. should be multiple of 16
// this is the size of all receive buffers processed by the ETHC
// The size should be enough to accomodate any network received packet
// If the packets are larger, they will have to take multiple RX buffers
// The current implementation does not handle this situation right now and the packet is discarded.

#define RXSIZE                         (EMAC_RX_BUFF_SIZE)
#define RAMSIZE                        (2*RXSIZE)


// Basic Mode Control Register (@: 0x00)
#define	_BMCON_COLTEST_MASK             0x0080
#define	_BMCON_DUPLEX_MASK              0x0100
#define	_BMCON_AN_RESTART_MASK          0x0200
#define	_BMCON_ISOLATE_MASK             0x0400
#define	_BMCON_PDWN_MASK                0x0800
#define	_BMCON_AN_ENABLE_MASK           0x1000
#define	_BMCON_SPEED_MASK               0x2000
#define	_BMCON_LOOPBACK_MASK            0x4000
#define	_BMCON_RESET_MASK               0x8000

// Basic Mode Status Register (@: 0x01)
#define	_BMSTAT_EXTEND_ABLE_MASK        0x0001
#define	_BMSTAT_JABBER_DET_MASK         0x0002
#define	_BMSTAT_LINK_STAT_MASK          0x0004
#define	_BMSTAT_AN_ABLE_MASK            0x0008
#define	_BMSTAT_REM_FAULT_MASK          0x0010
#define	_BMSTAT_AN_COMPLETE_MASK        0x0020
#define _BMSTAT_EXT_STAT_MASK           0x0100
#define _BMSTAT_BASE100T2_HDX_MASK      0x0200
#define _BMSTAT_BASE100T2_FDX_MASK      0x0400
#define	_BMSTAT_BASE10T_HDX_MASK        0x0800
#define	_BMSTAT_BASE10T_FDX_MASK        0x1000
#define	_BMSTAT_BASE100TX_HDX_MASK      0x2000
#define	_BMSTAT_BASE100TX_FDX_MASK      0x4000
#define	_BMSTAT_BASE100T4_MASK          0x8000

// Auto Negotiation Advertisement Register (@: 0x04)
#define	_ANAD_PROT_SEL_MASK             0x001f
#define	_ANAD_BASE10T_MASK              0x0020
#define	_ANAD_BASE10T_FDX_MASK          0x0040
#define	_ANAD_BASE100TX_MASK            0x0080
#define	_ANAD_BASE100TX_FDX_MASK        0x0100
#define	_ANAD_BASE100T4_MASK            0x0200
#define	_ANAD_PAUSE_MASK                0x0400
#define	_ANAD_ASM_DIR_MASK              0x0800
#define	_ANAD_REM_FAULT_MASK            0x2000
#define	_ANAD_NEXT_PAGE_MASK            0x8000

#define	_ANAD_NEGOTIATION_MASK          (_ANAD_BASE10T_MASK|_ANAD_BASE10T_FDX_MASK|_ANAD_BASE100TX_MASK|_ANAD_BASE100TX_FDX_MASK| _ANAD_BASE100T4_MASK)
#define	_ANAD_NEGOTIATION_POS           5		
#define	_BMSTAT_NEGOTIATION_POS         11
#define	MAC_COMM_CPBL_MASK              (_BMSTAT_BASE10T_HDX_MASK|_BMSTAT_BASE10T_FDX_MASK|_BMSTAT_BASE100TX_HDX_MASK|_BMSTAT_BASE100TX_FDX_MASK)

// =======================================================================
//   ---_PhyInitIo
//   Helper to properly set the Eth i/o pins to digital pins.
// =======================================================================
#if defined(__32MX795F512L__) 

typedef enum 
{
    ETH_PHY_CFG_RMII = 0x01, // check that configuration fuses is RMII
    ETH_PHY_CFG_MII = 0x00, // check that configuration fuses is MII
    ETH_PHY_CFG_ALTERNATE = 0x02, // check that configuration fuses is ALT
    ETH_PHY_CFG_DEFAULT = 0x00, // check that configuration fuses is DEFAULT
    ETH_PHY_CFG_AUTO = 0x10 // use the fuses configuration to detect if you are RMII/MII and ALT/DEFAULT configuration
} eEthPhyCfgFlags;

typedef enum 
{
    // basic registers, accross all registers: 0-1
    PHY_REG_BMCON = 0,
    PHY_REG_BMSTAT = 1,
    // extended registers: 2-15
    PHY_REG_PHYID1 = 2,
    PHY_REG_PHYID2 = 3,
    PHY_REG_ANAD = 4,
    PHY_REG_ANLPAD = 5,
    PHY_REG_ANLPADNP = 5,
    PHY_REG_ANEXP = 6,
    PHY_REG_ANNPTR = 7,
    PHY_REG_ANLPRNP = 8,
    // vendor registers
    PHY_LAN8740_SMR = 18           // Special Mode Register
} ePHY_BASIC_REG;

typedef struct __attribute__((__packed__)) 
{
    BYTE v[6];
}
MAC_ADDR;

typedef struct __attribute__((aligned(2), packed)) 
{
    MAC_ADDR DestMACAddr;
    MAC_ADDR SourceMACAddr;
    WORD_VAL Type;
}
ETHER_HEADER;

typedef struct __attribute__((__packed__)) 
{
    IP_ADDR IPAddr;
    MAC_ADDR MACAddr;
}
NODE_INFO;

typedef struct 
{
    int txBusy; // busy flag
    unsigned int dataBuff[(MAC_TX_BUFFER_SIZE + sizeof (ETHER_HEADER) + sizeof (int) - 1) / sizeof (int) ]; // actual data buffer
} sEthTxDcpt; // TX buffer descriptor

typedef union 
{
    struct 
    {
        unsigned : 7;
        unsigned COLTEST : 1;
        unsigned DUPLEX : 1;
        unsigned AN_RESTART : 1;
        unsigned ISOLATE : 1;
        unsigned PDWN : 1;
        unsigned AN_ENABLE : 1;
        unsigned SPEED : 1;
        unsigned LOOPBACK : 1;
        unsigned RESET : 1;
    };

    struct 
    {
        unsigned short w : 16;
    };
} __BMCONbits_t; // reg 0: PHY_REG_BMCON

typedef union 
{
    struct 
    {
        unsigned EXTEND_ABLE : 1;
        unsigned JABBER_DET : 1;
        unsigned LINK_STAT : 1;
        unsigned AN_ABLE : 1;
        unsigned REM_FAULT : 1;
        unsigned AN_COMPLETE : 1;
        unsigned PREAMBLE_SUPPRESS : 1;
        unsigned : 4;
        unsigned BASE10T_HDX : 1;
        unsigned BASE10T_FDX : 1;
        unsigned BASE100TX_HDX : 1;
        unsigned BASE100TX_FDX : 1;
        unsigned BASE100T4 : 1;
    };

    struct 
    {
        unsigned short w : 16;
    };
} __BMSTATbits_t; // reg 1: PHY_REG_BMSTAT

typedef union 
{
    struct 
    {
        unsigned PROT_SEL : 5;
        unsigned BASE10T : 1;
        unsigned BASE10T_FDX : 1;
        unsigned BASE100TX : 1;
        unsigned BASE100TX_FDX : 1;
        unsigned BASE100T4 : 1;
        unsigned PAUSE : 1; // NOTE: the PAUSE fields coding for SMSC is reversed!
        unsigned ASM_DIR : 1; // typo in the data sheet?
        unsigned : 1;
        unsigned REM_FAULT : 1;
        unsigned : 1;
        unsigned NP_ABLE : 1;
    };

    struct 
    {
        unsigned short w : 16;
    };
} __ANADbits_t; // reg 4: PHY_REG_ANAD

typedef union 
{
    struct 
    {
        unsigned PROT_SEL : 5;
        unsigned BASE10T : 1;
        unsigned BASE10T_FDX : 1;
        unsigned BASE100TX : 1;
        unsigned BASE100TX_FDX : 1;
        unsigned BASE100T4 : 1;
        unsigned PAUSE : 1;
        unsigned ASM_DIR : 1;
        unsigned : 1;
        unsigned REM_FAULT : 1;
        unsigned NP_ABLE : 1;
        //unsigned ACK:1;
    };

    struct 
    {
        unsigned short w : 16;
    };
} __ANLPADbits_t; // reg 5: PHY_REG_ANLPAD

typedef union 
{
    struct 
    {
        unsigned LP_AN_ABLE : 1;
        unsigned PAGE_RX : 1;
        unsigned NP_ABLE : 1;
        unsigned LP_NP_ABLE : 1;
        unsigned PDF : 1;
        unsigned : 11;
    };

    struct 
    {
        unsigned short w : 16;
    };
} __ANEXPbits_t;    // reg 6: PHY_REG_ANER

BYTE MACInit(void);
BOOL MACIsLinked(void);
BOOL MACIsTxReady(void);

BYTE MACGet(void);
void MACPut(BYTE val);
WORD MACGetArray(BYTE *address, WORD len);
void MACPutArray(BYTE *buff, WORD len);
BOOL MACGetHeader(MAC_ADDR *remote, BYTE *type);
void MACPutHeader(MAC_ADDR *remote, BYTE type, WORD dataLen);

WORD MACCalcRxChecksum(WORD offset, WORD len);
void MACDiscardRx(void);

void MACFlush(void);
void MACMemCopyAsync(PTR_BASE destAddr, PTR_BASE sourceAddr, WORD len);
void MACSetReadPtrInRx(WORD offset);
PTR_BASE MACGetTxBaseAddr(void);
PTR_BASE MACSetWritePtr(PTR_BASE address);
PTR_BASE MACSetReadPtr(PTR_BASE address);
WORD MACGetFreeRxSize(void);
WORD CalcIPBufferChecksum(WORD len);

eEthRes EthPhyInit(eEthOpenFlags oFlags, eEthPhyCfgFlags cFlags, eEthOpenFlags* pResFlags);
eEthLinkStat EthPhyGetNegotiationResult(eEthOpenFlags* pFlags, eEthMacPauseType* pPauseType);
eEthRes EthPhyNegotiationComplete(void);
eEthRes EthPhyRestartNegotiation(void);
eEthLinkStat EthPhyGetLinkStatus(void);
eEthRes EthPhyConfigureMII(eEthPhyCfgFlags cFlags);
BYTE EthPhyDetectAndReset();
#define EthPhyGetHwConfigFlags()    (((DEVCFG3bits.FMIIEN != 0) ? ETH_PHY_CFG_MII : ETH_PHY_CFG_RMII) | ((DEVCFG3bits.FETHIO != 0) ? ETH_PHY_CFG_DEFAULT : ETH_PHY_CFG_ALTERNATE))

#endif	/* ETHERNET_DATALINKLAYER_H */

