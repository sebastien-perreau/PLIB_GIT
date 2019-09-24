#ifndef __DEF_DMA
#define __DEF_DMA

#define DMA_MAX_TRANSFER_SIZE               65536
#define DMA_MAX_SRC_SIZE                    DMA_MAX_TRANSFER_SIZE
#define DMA_MAX_DST_SIZE                    DMA_MAX_TRANSFER_SIZE
#define DMA_MAX_CELL_SIZE                   DMA_MAX_TRANSFER_SIZE

typedef enum 
{
    DMA0                                    = 0,
    DMA1,
    DMA2,
    DMA3,
    DMA4,
    DMA5,
    DMA6,
    DMA7,
    DMA_NUMBER_OF_MODULES
} DMA_MODULE;

typedef enum
{
    DMA_CONT_PRIO_0                         = 0x00000000,       // Lowest priority
    DMA_CONT_PRIO_1                         = 0x00000001,
    DMA_CONT_PRIO_2                         = 0x00000002,
    DMA_CONT_PRIO_3                         = 0x00000003,       // Highest priority
            
    DMA_CONT_AUTO_ENABLE                    = 0x00000010,       // Channel is continuously enabled, and not automatically disabled after a block transfer is complete
    DMA_CONT_CHAIN_HIGHER                   = 0x00000020,       // Allow channel to be chained & chain to channel higher in natural priority (CH1 will be enabled by CH0 transfer complete)
    DMA_CONT_CHAIN_LOWER                    = 0x00000120,       // Allow channel to be chained & chain to channel lower in natural priority (CH1 will be enabled by CH2 transfer complete)    
            
    DMA_CONT_CHANNEL_ENABLE                 = 0x00000080,       // Make the channel ON
    DMA_CONT_CHANNEL_DISABLE                = 0x00000000,       // Make the channel OFF
            
    DMA_CONT_PATTERN_2_BYTES                = 0x00000800,       // Pattern = 2 bytes length
    DMA_CONT_PATTERN_1_BYTE                 = 0x00000000,       // Pattern = 1 byte length
} DMA_CHANNEL_CONTROL;

typedef enum
{
    DMA_EVT_FORCE_TRANSFER                  = 0x00000080,
    DMA_EVT_ABORD_TRANSFER                  = 0x00000040,
            
    DMA_EVT_ABORD_TRANSFER_ON_PATTERN_MATCH = 0x00000020,
    DMA_EVT_START_TRANSFER_ON_IRQ           = 0x00000010,
    DMA_EVT_ABORD_TRANSFER_ON_IRQ           = 0x00000008,            
    DMA_EVT_NONE                            = 0x00000000
} DMA_CHANNEL_EVENT;

typedef enum
{
    // value 32 bits = 0x00[interrupt enable bits 2 bytes]00[interrupt flag bits 2 bytes]
    DMA_INT_ADDRESS_ERROR                   = 0x00010001,
    DMA_INT_TRANSFER_ABORD                  = 0x00020002,
    DMA_INT_CELL_TRANSFER_DONE              = 0x00040004,
    DMA_INT_BLOCK_TRANSFER_DONE             = 0x00080008,
    DMA_INT_DEST_HALF_FULL                  = 0x00100010,
    DMA_INT_DEST_FULL                       = 0x00200020,
    DMA_INT_SRC_HALF_FULL                   = 0x00400040,
    DMA_INT_SRC_FULL                        = 0x00800080,
            
    DMA_INT_NONE                            = 0x00000000,
    DMA_INT_ALL                             = 0x00ff00ff
} DMA_CHANNEL_INTERRUPT;

typedef enum
{
    DMA_FLAG_ADDRESS_ERROR                  = 0x00000001,
    DMA_FLAG_TRANSFER_ABORD                 = 0x00000002,
    DMA_FLAG_CELL_TRANSFER_DONE             = 0x00000004,
    DMA_FLAG_BLOCK_TRANSFER_DONE            = 0x00000008,
    DMA_FLAG_DEST_HALF_FULL                 = 0x00000010,
    DMA_FLAG_DEST_FULL                      = 0x00000020,
    DMA_FLAG_SRC_HALF_FULL                  = 0x00000040,
    DMA_FLAG_SRC_FULL                       = 0x00000080,
            
    DMA_FLAG_ALL                            = 0x000000ff
} DMA_CHANNEL_FLAGS;

typedef struct
{
    const void      *src_start_addr; 
    void            *dst_start_addr; 
    uint16_t        src_size;
    uint16_t        dst_size;
    uint16_t        cell_size;
    uint16_t        pattern_data;
} DMA_CHANNEL_TRANSFER;

typedef struct 
{
	volatile UINT32 DMACON;
    volatile UINT32 DMACONCLR;
    volatile UINT32 DMACONSET;
    volatile UINT32 DMACONINV;

    volatile UINT32 DMASTAT;
    volatile UINT32 DMASTATCLR;
    volatile UINT32 DMASTATSET;
    volatile UINT32 DMASTATINV;

    volatile UINT32 DMAADDR;
    volatile UINT32 DMAADDRCLR;
    volatile UINT32 DMAADDRSET;
    volatile UINT32 DMAADDRINV;
    
    volatile UINT32 DCRCCON;
    volatile UINT32 DCRCCONCLR;
    volatile UINT32 DCRCCONSET;
    volatile UINT32 DCRCCONINV;

    volatile UINT32 DCRCDATA;
    volatile UINT32 DCRCDATACLR;
    volatile UINT32 DCRCDATASET;
    volatile UINT32 DCRCDATAINV;

    volatile UINT32 DCRCXOR;
    volatile UINT32 DCRCXORCLR;
    volatile UINT32 DCRCXORSET;
    volatile UINT32 DCRCXORINV;
} DMA_REGISTERS;

typedef struct
{
    volatile UINT32 DCHCON;
    volatile UINT32 DCHCONCLR;
    volatile UINT32 DCHCONSET;
    volatile UINT32 DCHCONINV;
    
    volatile UINT32 DCHECON;
    volatile UINT32 DCHECONCLR;
    volatile UINT32 DCHECONSET;
    volatile UINT32 DCHECONINV;
    
    volatile UINT32 DCHINT;
    volatile UINT32 DCHINTCLR;
    volatile UINT32 DCHINTSET;
    volatile UINT32 DCHINTINT;
    
    volatile UINT32 DCHSSA;
    volatile UINT32 DCHSSACLR;
    volatile UINT32 DCHSSASET;
    volatile UINT32 DCHSSAINV;
    
    volatile UINT32 DCHDSA;
    volatile UINT32 DCHDSACLR;
    volatile UINT32 DCHDSASET;
    volatile UINT32 DCHDSAINV;
    
    volatile UINT32 DCHSSIZ;
    volatile UINT32 DCHSSIZCLR;
    volatile UINT32 DCHSSIZSET;
    volatile UINT32 DCHSSIZINV;
    
    volatile UINT32 DCHDSIZ;
    volatile UINT32 DCHDSIZCLR;
    volatile UINT32 DCHDSIZSET;
    volatile UINT32 DCHDSIZINV;
    
    volatile UINT32 DCHSPTR;
    volatile UINT32 DCHSPTRCLR;
    volatile UINT32 DCHSPTRSET;
    volatile UINT32 DCHSPTRINV;
    
    volatile UINT32 DCHDPTR;
    volatile UINT32 DCHDPTRCLR;
    volatile UINT32 DCHDPTRSET;
    volatile UINT32 DCHDPTRINV;
    
    volatile UINT32 DCHCSIZ;
    volatile UINT32 DCHCSIZCLR;
    volatile UINT32 DCHCSIZSET;
    volatile UINT32 DCHCSIZINV;
    
    volatile UINT32 DCHCPTR;
    volatile UINT32 DCHCPTRCLR;
    volatile UINT32 DCHCPTRSET;
    volatile UINT32 DCHCPTRINV;
    
    volatile UINT32 DCHDAT;
    volatile UINT32 DCHDATCLR;
    volatile UINT32 DCHDATSET;
    volatile UINT32 DCHDATINV;
} DMA_CHANNEL_REGISTERS;

typedef void (*dma_event_handler_t)(uint8_t id, DMA_CHANNEL_FLAGS flags);

extern __inline__ unsigned int __attribute__((always_inline)) _VirtToPhys2(const void* p)
{
	return (int)p<0?((int)p&0x1fffffffL):(unsigned int)((unsigned char*)p+0x40000000L);
}

void dma_init(  DMA_MODULE id, 
                dma_event_handler_t evt_handler, 
                DMA_CHANNEL_CONTROL dma_channel_control,
                DMA_CHANNEL_INTERRUPT dma_channel_interrupt,
                DMA_CHANNEL_EVENT dma_channel_event,
                uint8_t irq_num_tx_start,
                uint8_t irq_num_tx_abord);
DMA_MODULE dma_get_free_channel();
void dma_channel_enable(DMA_MODULE id, bool enable);
bool dma_channel_is_enable(DMA_MODULE id);
void dma_set_transfer(DMA_MODULE id, DMA_CHANNEL_TRANSFER * channel_transfer, bool enable_channel, bool force_transfer);
void dma_force_transfer(DMA_MODULE id);
void dma_abord_transfer(DMA_MODULE id);
DMA_CHANNEL_FLAGS dma_get_flags(DMA_MODULE id);
void dma_clear_flags(DMA_MODULE id, DMA_CHANNEL_FLAGS flags);

const uint8_t dma_get_irq(DMA_MODULE id);
void dma_interrupt_handler(DMA_MODULE id);
	
#endif
