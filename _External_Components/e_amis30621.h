#ifndef __DEF_AMIS30621
#define	__DEF_AMIS30621

#define MAX_TAB_MOTOR                       8


// Standard Idenfitiers
#define	ID_FRAME_CDE                        0x3C
#define	ID_FRAME_BACK_CDE                   0x7D
// Dynamic Identifiers
#define ID_FRAME_GEN_PURPOSE2               0xD6	// Dynamic ID
#define ID_FRAME_GEN_PURPOSE4               0x25	// Dynamic ID
#define ID_FRAME_GET_ACTUAL_POS             0xA3	// Dynamic ID
#define ID_FRAME_SET_POSITION               0x93        // Dynamic ID
#define	ID_FRAME_SET_POS_SHORT1             0x92	// Dynamic ID
#define	ID_FRAME_SET_POS_SHORT2             0x61	// Dynamic ID
#define	ID_FRAME_SET_POS_SHORT4             0xF0	// Dynamic ID
#define ID_FRAME_PREPARING_FRAME            0x97	// Dynamic ID
// DLC
#define NB_BYTE_CDE                         0x08
#define NB_BYTE_BACK_CDE                    0x08
#define NB_BYTE_GEN_PURPOSE2                0x02
#define NB_BYTE_GEN_PURPOSE4                0x04
#define NB_BYTE_SET_POSITION                0x04
#define NB_BYTE_SET_POS_SHORT1              0x02
#define NB_BYTE_SET_POS_SHORT2              0x04
#define NB_BYTE_SET_POS_SHORT4              0x08
// ROM Pointer For Dynamic Identifiers
#define ROM_POINTER_0                       0x00        // General Purpose 2
#define ROM_POINTER_1                       0x01        // General Purpose 4
#define ROM_POINTER_2                       0x02        // GetActualPos
#define ROM_POINTER_3                       0x03        // NOT USED because GetStatus must be unique for each AMIS30621.
#define ROM_POINTER_4                       0x04        // SetPosition
#define ROM_POINTER_5                       0x05        // SetPositionShort1
#define ROM_POINTER_6                       0x06        // SetPositionShort2
#define ROM_POINTER_7                       0x07        // SetPositionShort4
#define ROM_POINTER_8                       0x08        // Preparation Frame

// Frame GET_FULL_POS
#define	FRAME_GET_FULL_POS_1                0x80
#define	FRAME_GET_FULL_POS_2                0x80
// Frame GET_FULL_STATUS
#define	FRAME_GET_FULL_STATUS_1             0x80
#define	FRAME_GET_FULL_STATUS_2             0x81
// Frame GET OTP PARAM
#define FRAME_GET_OTP_PARAM_1               0x80
#define FRAME_GET_OTP_PARAM_2               0x82
// Frame SET OTP PARAM
#define FRAME_SET_OTP_PARAM_1               0x80
#define FRAME_SET_OTP_PARAM_2               0x90
// Frame SET MOTOR PARAM
#define FRAME_SET_MOTOR_PARAM_1             0x80
#define FRAME_SET_MOTOR_PARAM_2             0x89
// Frame SET DUAL POSITIONING
#define FRAME_SET_DUAL_POSITIONING_1        0x80
#define FRAME_SET_DUAL_POSITIONING_2        0x88
// Frame DYNAMIC ID ASSIGNMENT
#define	FRAME_DYNAMIC_ID_1                  0x80
#define	FRAME_DYNAMIC_ID_2                  0x91
// Frame SET POSITION
#define	FRAME_SET_POSITION_1                0x80
#define	FRAME_SET_POSITION_2                0x8B
// Frame RESET POSITION
#define FRAME_RESET_POSITION                0x86
// Frame RESET TO DEFAULT
#define FRAME_RESET_TO_DEFAULT              0x87
// Frame HARD STOP
#define FRAME_HARD_STOP                     0x85
// Frame SOFT STOP
#define FRAME_SOFT_STOP                     0x8F

// ----------------------- Parameters for SetMotorParam ------------------------
#define I_RUN_DEFAULT                       0x0E    // 673mA
#define I_HOLD_DEFAULT                      0x00    // 59mA
#define V_MIN_DEFAULT                       0x03    // 28 step/sec
#define V_MAX_DEFAULT                       0x08    // 303 step/sec
#define ACC_DEFAULT                         0x07    // 13970 step/sec²
#define STEP_MODE_DEFAULT                   0x00    // step/2
#define SHAFT_DEFAULT                       0x01
#define ACC_SHAFT_DEFAULT                   0x01

#define STEP_MODE_1_2                       0x00    // step/2
#define STEP_MODE_1_4                       0x01    // step/4
#define STEP_MODE_1_8                       0x02    // step/8
#define STEP_MODE_1_16                      0x03    // step/16

#define SHAFT_ON                            0x01
#define SHAFT_OFF                           0x00

#define ACC_SHAFT_ON                        0x01
#define ACC_SHAFT_OFF                       0x00
// -----------------------------------------------------------------------------
// --------------------- Parameters for SetDualPositioning ---------------------
#define POS1_DEFAULT                        0xED20
#define POS2_DEFAULT                        0xECE0
// -----------------------------------------------------------------------------
// ---------------------------- Current parameters -----------------------------
#define I_59_MA                             0x00
#define I_71_MA                             0x01
#define I_84_MA                             0x02
#define I_100_MA                            0x03
#define I_119_MA                            0x04
#define I_141_MA                            0x05
#define I_168_MA                            0x06
#define I_200_MA                            0x07
#define I_238_MA                            0x08
#define I_283_MA                            0x09
#define I_336_MA                            0x0A
#define I_400_MA                            0x0B
#define I_476_MA                            0x0C
#define I_566_MA                            0x0D
#define I_673_MA                            0x0E
#define I_800_MA                            0x0F
// -----------------------------------------------------------------------------
// ------------------- Velocity Max parameters (full step/s)--------------------
#define V_MAX_99_FULL_STEP_SEC              0x00
#define V_MAX_136_FULL_STEP_SEC             0x01
#define V_MAX_167_FULL_STEP_SEC             0x02
#define V_MAX_197_FULL_STEP_SEC             0x03
#define V_MAX_213_FULL_STEP_SEC             0x04
#define V_MAX_228_FULL_STEP_SEC             0x05
#define V_MAX_243_FULL_STEP_SEC             0x06
#define V_MAX_273_FULL_STEP_SEC             0x07
#define V_MAX_303_FULL_STEP_SEC             0x08
#define V_MAX_334_FULL_STEP_SEC             0x09
#define V_MAX_364_FULL_STEP_SEC             0x0A
#define V_MAX_395_FULL_STEP_SEC             0x0B
#define V_MAX_456_FULL_STEP_SEC             0x0C
#define V_MAX_546_FULL_STEP_SEC             0x0D
#define V_MAX_729_FULL_STEP_SEC             0x0E
#define V_MAX_973_FULL_STEP_SEC             0x0F
// -----------------------------------------------------------------------------
// ------------------- Velocity Min parameters (full step/s)--------------------
#define V_MIN_V_MAX_1                       0x00
#define V_MIN_V_MAX_1_32                    0x01
#define V_MIN_V_MAX_2_32                    0x02
#define V_MIN_V_MAX_3_32                    0x03
#define V_MIN_V_MAX_4_32                    0x04
#define V_MIN_V_MAX_5_32                    0x05
#define V_MIN_V_MAX_6_32                    0x06
#define V_MIN_V_MAX_7_32                    0x07
#define V_MIN_V_MAX_8_32                    0x08
#define V_MIN_V_MAX_9_32                    0x09
#define V_MIN_V_MAX_10_32                   0x0A
#define V_MIN_V_MAX_11_32                   0x0B
#define V_MIN_V_MAX_12_32                   0x0C
#define V_MIN_V_MAX_13_32                   0x0D
#define V_MIN_V_MAX_14_32                   0x0E
#define V_MIN_V_MAX_15_32                   0x0F
// -----------------------------------------------------------------------------
// ------------------- Acceleration parameters (full step²)---------------------
#define ACC_49                              0x00        // * VMAX <= 243
#define ACC_218                             0x01        // * VMAX <= 456
#define ACC_1004                            0x02
#define ACC_3609                            0x03
#define ACC_6228                            0x04
#define ACC_8848                            0x05
#define ACC_11409                           0x06
#define ACC_13970                           0x07
#define ACC_16531                           0x08
#define ACC_19092                           0x09        // * VMAX >= 136
#define ACC_21886                           0x0A        // * VMAX >= 136
#define ACC_24447                           0x0B        // * VMAX >= 136
#define ACC_27008                           0x0C        // * VMAX >= 136
#define ACC_29570                           0x0D        // * VMAX >= 136
#define ACC_34925                           0x0E        // * VMAX >= 273
#define ACC_40047                           0x0F        // * VMAX >= 273
// -----------------------------------------------------------------------------
#define AMIS_DEAMON_COMMAND_CONTINUE        1
#define AMIS_DEAMON_NORMAL                  0

typedef struct
{
    unsigned iHold:4;
    unsigned iRun:4;
}_CURRENT_PARAM;

typedef struct
{
    unsigned vMin:4;
    unsigned vMax:4;
}_VELOCITY_PARAM;

typedef struct
{
    union
    {
        BYTE iParam;
        _CURRENT_PARAM iParamBits;
    };
    union
    {
        BYTE vParam;
        _VELOCITY_PARAM vParamBits;
    };
    unsigned acc:4;
    unsigned shaft:1;
    unsigned stepMode:2;
    unsigned accShaft:1;
}_MOTOR_PARAM;

typedef struct
{
    // Frame 1
    BYTE adress;
    _MOTOR_PARAM param;
    unsigned tInfo:2;
    unsigned tw:1;
    unsigned tsd:1;
    unsigned uv2:1;
    unsigned eidef:1;
    unsigned stepLoss:1;
    unsigned vddReset:1;
    // Frame 2
    UINT actPos;
    UINT tagPos;
    UINT secPos;
}_GET_FULL_STATUS;

typedef struct
{
    unsigned iRef:4;
    unsigned osc:4;
    unsigned bg:4;
    unsigned tsd:3;
    unsigned pa:4;
    unsigned hw:3;
    unsigned adm:1;
    _MOTOR_PARAM param;
    UINT secPos;
}_GET_OTP_PARAM;

typedef struct
{
    unsigned otpMemoryAdress:3;
    BYTE data;
}_SET_OTP_PARAM;

typedef struct
{
    _MOTOR_PARAM param;
    UINT secPos;
}_SET_MOTOR_PARAM;

typedef struct
{
    unsigned vMin:4;
    unsigned vMax:4;
    WORD pos1;
    WORD pos2;
}_SET_DUAL_POSITIONING_PARAM;

typedef struct
{
    unsigned isInitDone:1;
    unsigned getOtpParam:1;
    unsigned setMotorParam:1;
    unsigned command:1;
    unsigned stopAndResetPosition:1;
    unsigned reserved:3;
}_FLAGS_MOTOR;

typedef struct
{
    _FLAGS_MOTOR flags;
    UINT position16step;
    _GET_FULL_STATUS getFullStatus;
    _GET_OTP_PARAM getOtpParam;
    _SET_MOTOR_PARAM setMotorParam;
    _SET_DUAL_POSITIONING_PARAM setDualPositioning;
}AMIS30621_PARAM;

typedef struct
{
    BOOL isNewDetection;
    BYTE loopAdress;
    BYTE tabAdressDetected[15];
    BYTE numberMotorDetected;
}AMIS30621_DETECTED;

void eAMIS30621DeamonMotor(LIN_MODULE mLinModule, AMIS30621_PARAM *motor, AMIS30621_DETECTED *busInformations, BOOL mode, BYTE nbMotor);
BYTE eAMIS30621Init(LIN_MODULE mLinModule, BYTE adress, _GET_FULL_STATUS *getFullStatus, _SET_MOTOR_PARAM setMotorParam, _SET_DUAL_POSITIONING_PARAM setDualPositioning);
BYTE eAMIS30621GetFullStatus(LIN_MODULE mLinModule, BYTE adress, _GET_FULL_STATUS *getFullStatus);
BYTE eAMIS30621GetActualPos(LIN_MODULE mLinModule, BYTE adress, UINT *getActualPos);
BOOL eAMIS30621GetOTPParam(LIN_MODULE mLinModule, BYTE adress, _GET_OTP_PARAM *getOtpParam);
BOOL eAMIS30621SetOTPParam(LIN_MODULE mLinModule, BYTE adress, _SET_OTP_PARAM setOtpParam);
BOOL eAMIS30621SetMotorParam(LIN_MODULE mLinModule, BYTE adress, _SET_MOTOR_PARAM setMotorParam);
BOOL eAMIS30621SetDualPositioning(LIN_MODULE mLinModule, BYTE adress, _SET_DUAL_POSITIONING_PARAM setDualPositioning);
BOOL eAMIS30621SetDynamicIdentifiers(LIN_MODULE mLinModule, BYTE adress);
BOOL eAMIS30621SetPositionShort1M(LIN_MODULE mLinModule, BYTE adress, UINT position16steps);
BOOL eAMIS30621SetPositionShort2M(LIN_MODULE mLinModule, BYTE adressA, UINT position16stepsA, BYTE adressB, UINT position16stepsB);
BOOL eAMIS30621SetPositionShort4M(LIN_MODULE mLinModule, BYTE adressA, UINT position16stepsA, BYTE adressB, UINT position16stepsB, BYTE adressC, UINT position16stepsC, BYTE adressD, UINT position16stepsD);
BOOL eAMIS30621SetPosition1M(LIN_MODULE mLinModule, BYTE adress, UINT position16steps);
BOOL eAMIS30621SetPosition2M(LIN_MODULE mLinModule, BYTE adressA, UINT position16stepsA, BYTE adressB, UINT position16stepsB);
BOOL eAMIS30621ResetPosition(LIN_MODULE mLinModule, BYTE adress);
BOOL eAMIS30621ResetToDefault(LIN_MODULE mLinModule, BYTE adress);
BOOL eAMIS30621HardStop(LIN_MODULE mLinModule, BYTE adress);
BOOL eAMIS30621SoftStop(LIN_MODULE mLinModule, BYTE adress);
BOOL eAMIS30621SoftStopAndResetPosition(LIN_MODULE mLinModule, BYTE adress);

BOOL eAMIS30621ChangeAdress(LIN_MODULE mLinModule, BYTE currentAdress, BYTE newAdress);
void eAMIS30621SetDefaultMotorParam(_SET_MOTOR_PARAM *setMotorParam);
void eAMIS30621SetDefaultDualPositioning(_SET_DUAL_POSITIONING_PARAM *setDualPositioning);

#endif
