#ifndef __DEF_CAN
#define	__DEF_CAN

#ifdef _CAN1
    #define CAN1_BASE_ADDRESS			_CAN1_BASE_ADDRESS
#endif

#ifdef _CAN2
    #define CAN2_BASE_ADDRESS 			_CAN2_BASE_ADDRESS
#endif

#ifdef _CAN1
    #ifdef _CAN2
          #define CAN_NUM_OF_MODULES 	2
    #else
          #define CAN_NUM_OF_MODULES 	1
    #endif
#endif

// x * y * z
// x: number of channel use
// y: number of message buffer use (with all channel use - define by x)
// z: if full message then 16 else if data-only then 8
// For more details, cf PIC32 MX CAN Peripheral Libraries
#define CAN_SIZE_MESSAGE_FIFO_AREA ((32 + (31 * 5)) * 16)

// Prop + Seg1 >= Seg2
// Seg2 > SJW
// 8 <= total_qt <= 25
// prescale = (perif_clk/(baudrate*total_qt*2))-1
// total_qt = SYNC_SEG + PROP_SEG + PHASE_SEG1 + PHASE_SEG2 with SYNC_SEG = 1
// For more details, cf PIC32 MX CAN Peripheral Libraries
#if (PERIPHERAL_FREQ == 80000000)
#define PHASE_SEG2                  CAN_BIT_3TQ
#define PHASE_SEG1                  CAN_BIT_3TQ
#define PROP_SEG                    CAN_BIT_3TQ
#define SJW                         CAN_BIT_2TQ
#elif (PERIPHERAL_FREQ == 48000000)
#define PHASE_SEG2                  CAN_BIT_4TQ
#define PHASE_SEG1                  CAN_BIT_4TQ
#define PROP_SEG                    CAN_BIT_3TQ
#define SJW                         CAN_BIT_2TQ
#else
#warning "Frequency not define for CAN speed configuration"
#endif

#define CAN_ID_STANDARD             FALSE
#define CAN_ID_EXTENDED             TRUE

#define CAN_ENABLE_FILTER           0x80
#define CAN_DISABLE_FILTER          0x00

#define CAN_DEFAULT_MASK0           0x1FFFFFFF
#define CAN_DEFAULT_MASK1           0x1FFFFFFC
#define CAN_DEFAULT_MASK2           0x1FFFF800
#define CAN_DEFAULT_MASK3           0x00000000

#define CAN_NUM_OF_FILTERS 				32
#define CAN_NUM_OF_FILTER_MASKS 		4
#define CAN_NUM_OF_FILTER_CONTROL_REGS 	8
#define CAN_NUM_OF_CHANNELS				32

/*******************************************************************
  This data structure encapsulates the CAN Filter Control registers.
  *******************************************************************/
typedef struct _CAN_FILTER_CONTROL_REGS
{
	union
	{
		volatile UINT32 CxFLTCON;
		volatile BYTE CxFLTCONbyte[4];
	};
	volatile UINT32 CxFLTCONCLR  ;
	volatile UINT32 CxFLTCONSET  ;
	volatile UINT32 CxFLTCONINV  ;
	

}CAN_FILTER_CONTROL_REGS;

/*******************************************************************
  This enumerates the total number of CAN filter control registers
  contained in each CAN module.
  *******************************************************************/

typedef enum _CAN_FILTER_CONTROL
{
	CAN_FILTER_CONTROL0,
	CAN_FILTER_CONTROL1,
	CAN_FILTER_CONTROL2,
	CAN_FILTER_CONTROL3,
	CAN_FILTER_CONTROL4,
	CAN_FILTER_CONTROL5,
	CAN_FILTER_CONTROL6,
	CAN_FILTER_CONTROL7
}CAN_FILTER_CONTROL;

typedef struct _CAN_FLTCON_BYTES
{
	CAN_FILTER_CONTROL fltcon;
	UINT byteIndex;
	UINT fltEnMask;
}CAN_FLTCON_BYTES;

/*******************************************************************
  This data structure encapsulates the fifo control registers.
  *******************************************************************/
typedef struct {
    unsigned RXNEMPTYIF:1;
    unsigned RXHALFIF:1;
    unsigned RXFULLIF:1;
    unsigned RXOVFLIF:1;
    unsigned :4;
    unsigned TXEMPTYIF:1;
    unsigned TXHALFIF:1;
    unsigned TXNFULLIF:1;
    unsigned :5;
    unsigned RXNEMPTYIE:1;
    unsigned RXHALFIE:1;
    unsigned RXFULLIE:1;
    unsigned RXOVFLIE:1;
    unsigned :4;
    unsigned TXEMPTYIE:1;
    unsigned TXHALFIE:1;
    unsigned TXNFULLIE:1;
} CxFIFOINT_t;

typedef struct {
    unsigned TXPRI:2;
    unsigned RTREN:1;
    unsigned TXREQ:1;
    unsigned TXERR:1;
    unsigned TXLARB:1;
    unsigned TXABAT:1;
    unsigned TXEN:1;
    unsigned :4;
    unsigned DONLY:1;
    unsigned UINC:1;
    unsigned FRESET:1;
    unsigned :1;
    unsigned FSIZE:5;
} CxFIFOCON_t;

typedef struct {
    unsigned DNCNT:5;
    unsigned :6;
    unsigned CANBUSY:1;
    unsigned :1;
    unsigned SIDL:1;
    unsigned :1;
    unsigned bON:1;
    unsigned :4;
    unsigned CANCAP:1;
    unsigned OPMOD:3;
    unsigned REQOP:3;
    unsigned ABAT:1;
} CxCON_t;

typedef struct {
    unsigned BRP:6;
    unsigned SJW:2;
    unsigned PRSEG:3;
    unsigned SEG1PH:3;
    unsigned SAM:1;
    unsigned SEG2PHTS:1;
    unsigned SEG2PH:3;
    unsigned :3;
    unsigned WAKFIL:1;
} CxCFG_t;

typedef struct {
    unsigned CANTSPRE:16;
    unsigned CANTS:16;
}CxTMR_t;

typedef struct {
    unsigned ICODE:7;
    unsigned :1;
    unsigned FILHIT:5;
} CxVEC_t;

typedef struct {
    unsigned EID:18;
    unsigned :1;
    unsigned EXID:1;
    unsigned :1;
    unsigned SID:11;
} CxRXF_t;

typedef struct {
    unsigned EID:18;
    unsigned :1;
    unsigned MIDE:1;
    unsigned :1;
    unsigned SID:11;
} CxRXM_t;

typedef struct _CAN_FIFO_REGS
{
	union{
		volatile UINT32 CxFIFOCON;
		volatile CxFIFOCON_t CxFIFOCONbits;
	};
	volatile UINT32 CxFIFOCONCLR ;
	volatile UINT32 CxFIFOCONSET ;
	volatile UINT32 CxFIFOCONINV ;
	union
	{
		volatile UINT32 CxFIFOINT    ;
		volatile CxFIFOINT_t CxFIFOINTbits;
	};
	volatile UINT32 CxFIFOINTCLR ;
	volatile UINT32 CxFIFOINTSET ;
	volatile UINT32 CxFIFOINTINV ;
	volatile UINT32 CxFIFOUA     ;
	volatile UINT32 CxFIFOUACLR  ;
	volatile UINT32 CxFIFOUASET  ;
	volatile UINT32 CxFIFOUAINV  ;
	volatile UINT32 CxFIFOCI     ;
	volatile UINT32 CxFIFOCICLR  ;
	volatile UINT32 CxFIFOCISET  ;
	volatile UINT32 CxFIFOCIINV  ;
}CAN_FIFO_REGS;

/*******************************************************************
  This data structure encapsulates the filter specification registers.
  *******************************************************************/

typedef struct _CAN_FILTER_REGS
{
	union
	{
		volatile UINT32 CxRXF;
		volatile CxRXF_t CxRXFbits;
	};
	volatile UINT32 CxRXFCLR     ;
	volatile UINT32 CxRXFSET     ;
	volatile UINT32 CxRXFINV     ;
	

}CAN_FILTER_REGS;

/*******************************************************************
  This data structure encapsulates the filter mask specification
  registers.
  *******************************************************************/
typedef struct _CAN_FILTER_MASK_REGS
{
	union
	{
		volatile UINT32 CxRXM;
		volatile CxRXM_t CxRXMbits;
	};
	volatile UINT32 CxRXMCLR     ;
	volatile UINT32 CxRXMSET     ;
	volatile UINT32 CxRXMINV     ;

}CAN_FILTER_MASK_REGS;

/*******************************************************************
  This data structure encapsulates all the CAN module registers.
  *******************************************************************/
typedef struct _CAN_REGISTERS
{
	union
	{
		volatile UINT32 CxCON;
		volatile CxCON_t CxCONbits;
	};	   
	volatile UINT32 CxCONCLR; 
	volatile UINT32 CxCONSET;
	volatile UINT32 CxCONINV;
	union
	{
		volatile UINT32 CxCFG;
		volatile CxCFG_t CxCFGbits;
	};
	volatile UINT32 CxCFGCLR;
	volatile UINT32 CxCFGSET;
	volatile UINT32 CxCFGINV;
	volatile UINT32 CxINT;
	volatile UINT32 CxINTCLR;
	volatile UINT32 CxINTSET;
	volatile UINT32 CxINTINV;
	union
	{
		volatile UINT32 CxVEC;
		volatile CxVEC_t CxVECbits;
	};
	volatile UINT32 CxVECCLR;
	volatile UINT32 CxVECSET;
	volatile UINT32 CxVECINV;

	volatile UINT32 CxTREC;
	volatile UINT32 CxTRECCLR;
	volatile UINT32 CxTRECSET;
	volatile UINT32 CxTRECINV;

	volatile UINT32 CxFSTAT;
	volatile UINT32 CxFSTATCLR;
	volatile UINT32 CxFSTATSET;
	volatile UINT32 CxFSTATINV;

	volatile UINT32 CxRXOVF;
	volatile UINT32 CxRXOVFCLR;
	volatile UINT32 CxRXOVFSET;
	volatile UINT32 CxRXOVFINV;

	union
	{
		volatile UINT32 CxTMR;
		volatile CxTMR_t CxTMRbits;
	};
	volatile UINT32 CxTMRCLR;
	volatile UINT32 CxTMRSET;
	volatile UINT32 CxTMRINV;

	volatile CAN_FILTER_MASK_REGS canFilterMaskRegs[CAN_NUM_OF_FILTER_MASKS];

	volatile CAN_FILTER_CONTROL_REGS canFilterControlRegs[CAN_NUM_OF_FILTER_CONTROL_REGS];
	volatile CAN_FILTER_REGS canFilterRegs[CAN_NUM_OF_FILTERS];
	volatile UINT32 CxFIFOBA;
	volatile UINT32 CxFIFOBACLR;
	volatile UINT32 CxFIFOBASET;
	volatile UINT32 CxFIFOBAINV;
	volatile CAN_FIFO_REGS canFifoRegisters[CAN_NUM_OF_CHANNELS];	
}CAN_REGISTERS;

extern const CAN_REGISTERS * mCANModules[CAN_NUM_OF_MODULES];
extern const CAN_FLTCON_BYTES mCANFilterControlMap[CAN_NUM_OF_FILTERS]; 

// --------------------------------------------------
// **** MACRO AND STRUCTURE FOR THE CAN ROUTINE ****
typedef struct
{
    BOOL    enable;
    DWORD   id;
    BOOL    idExtended;
    BYTE    length;
    BYTE    data[8];
    QWORD   period;
    QWORD   tick;
}CAN_FRAME;

typedef struct
{
    BYTE    numberOfFrame;
    CAN_FRAME *ptrFrames;
}CAN_FRAMES;

typedef struct
{
    UINT32  mask[4];
    UINT32  id[31];
    BYTE    enableFilterAndAttachMask[31];
}CAN_FILTERS;

#define INIT_CAN_FRAME(id, idExtended, length, period)      {ON, id, idExtended, length, {0}, period, TICK_INIT}
#define INIT_CAN_FRAMES()                                   {0, NULL}
#define INIT_CAN_FILTERS()                                  {{0}, {0}, {0}}

void CANInit(CAN_MODULE module, DWORD busSpeed);
static void CANConfigFilter(CAN_MODULE module, CAN_FILTER filter, UINT32 id);
static void CANConfigMask(CAN_MODULE module, CAN_FILTER_MASK mask, UINT32 maskbits);
static void CANLinkFilterToChannelAndEnable(CAN_MODULE module, CAN_FILTER filter, CAN_FILTER_MASK mask, CAN_CHANNEL channel, BOOL enable);
void CANDeamonFilters(CAN_MODULE module, CAN_FILTERS *filters);
static BOOL CANAddMessageFifoBuffer(CAN_MODULE module, CAN_CHANNEL channel, DWORD id, BOOL idExtended, BYTE length, BYTE* data);
BOOL CANSendMessage(CAN_MODULE module, CAN_CHANNEL channel, DWORD id, BOOL idExtended, BYTE length, BYTE* data);
void CANTaskTx(CAN_MODULE module, CAN_FRAMES *frame);
void CANAddFrame(CAN_FRAMES *frames, DWORD id, BOOL idExtended, BYTE length, QWORD period);
void CANRemoveFrame(CAN_FRAMES *frames, DWORD id);
CHAR8 CANGetIndiceID(CAN_FRAMES frames, DWORD id);
void CANEnableFrame(CAN_FRAMES *frames, DWORD id, BOOL enable);
void CANSetData(CAN_FRAMES *frames, DWORD id, BYTE data[8]);
void CANSetData_mask(CAN_FRAMES *frames, DWORD id, BYTE data[8], BYTE mask);
void CANSetData1Byte(CAN_FRAMES *frames, DWORD id, BYTE indiceData, BYTE data);
void CANSetLinkForFramesReception(CAN_MODULE module, CAN_FRAMES *frames);
void CANTaskRx(CAN_MODULE module);
static void CANAddToReceivedBuffer(CAN_MODULE module, CANRxMessageBuffer frame);
void CANFreedomReceiveMemory(CAN_FRAMES *frame, QWORD timeout);
CAN_FRAME* CANGetFrame(CAN_FRAMES frame, DWORD id);

#endif
