#ifndef __DEF_COMMUNICATION
#define	__DEF_COMMUNICATION

// ------------------------------------------------------
// **** MACRO AND STRUCTURE FOR THE ENCODING ROUTINE ****

#define START_OF_FRAME                  0x90
#define STATE_FRAME_PENDING_MASK        0x08
#define STATE_FRAME_PENDING_POSITION    0x03
#define STATE_FULL_PERIOD_MASK          0x01
#define STATE_FULL_PERIOD_POSITION      0x00
#define STATE_HALF_PERIOD_MASK          0x10
#define STATE_HALF_PERIOD_POSITION      0x04
#define STEP_START                      0
#define STEP_SYNCH                      1
#define STEP_LENGTH                     2
#define STEP_DATA                       3
#define STEP_CHKSM                      4
#define STEP_END_FRAME                  5
#define STEP_DEFAULT                    6
#define IDLE_LOW                        0x01
#define IDLE_HIGH                       0x02
#define IDLE_ACTIVE_LOW                 0x04
#define IDLE_ACTIVE_HIGH                0x08
#define LENGTH_0                        0x00
#define LENGTH_1                        0x10
#define LENGTH_2                        0x20
#define LENGTH_3                        0x30
#define LENGTH_4                        0x40
#define LENGTH_5                        0x50
#define LENGTH_6                        0x60
#define LENGTH_7                        0x70
#define LENGTH_8                        0x80
#define LENGTH_9                        0x90
#define LENGTH_10                       0xA0
#define LENGTH_11                       0xB0
#define LENGTH_12                       0xC0
#define LENGTH_13                       0xD0
#define LENGTH_14                       0xE0
#define LENGTH_15                       0xF0

typedef struct
{
    // -----------------
    WORD    actualBitrate;
    WORD    bitrate;
    BYTE    idleState;
    QWORD    period;
    // -----------------
    BYTE    length;
    BYTE    buffer[16];
    BYTE    cheksum;
    // -----------------
    BOOL    output;
    BOOL    clock;
    BYTE    state;
    BOOL    dataBit;
    BYTE    stepSendFrame;
    INT8    bitPointer;
    INT8    bytePointer;
    BYTE    bufferTemp;
    QWORD   tickClock;
    QWORD   tickFrame;
}ENCODING_CONFIG;

// --------------------------------------------------------
// **** MACRO AND STRUCTURE FOR THE DEENCODING ROUTINE ****

#define DECODING_BITRATE                0
#define DECODING_FINISHED               1
#define DECODING_PENDING                2
#define DECODING_ERROR                  4
#define CASE_HEADER                     0
#define CASE_LENGTH                     1
#define CASE_DATA1                      2
#define CASE_DATA2                      3
#define CASE_DATA3                      4
#define CASE_DATA4                      5
#define CASE_DATA5                      6
#define CASE_DATA6                      7
#define CASE_DATA7                      8
#define CASE_DATA8                      9
#define CASE_DATA9                      10
#define CASE_DATA10                     11
#define CASE_DATA11                     12
#define CASE_DATA12                     13
#define CASE_DATA13                     14
#define CASE_DATA14                     15
#define CASE_DATA15                     16
#define CASE_CHKSM                      17

typedef struct
{
    // -----------------
    WORD    bitrate;
    // -----------------
    BOOL    previousInputState;
    BYTE    numberShortPeriod;
    BYTE    byte;
    BYTE    bitPointer;
    BYTE    stepReceiveFrame;
    // -----------------
    BYTE    status;
    BYTE    length;
    BYTE    data[15];
    BYTE    cheksum;
    // -----------------
#if (DECODING_BITRATE == 0)
    WORD    timeDoublePeriod;
    WORD    timeTriplePeriod;
    WORD    timeIdle;
#endif
    QWORD   tick;
}DECODING_CONFIG;

void fCommunicationInitSendVariable(ENCODING_CONFIG *var, BYTE config, QWORD bitrate);
void fCommunicationInitReceiveVariable(DECODING_CONFIG *var);
BOOL fCommunicationEncoding(ENCODING_CONFIG *var);
BOOL fCommunicationDecoding(DECODING_CONFIG *var, BOOL input);

#endif
