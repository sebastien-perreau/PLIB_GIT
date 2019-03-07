/*********************************************************************
*	External peripheral AMIS30621 (Micro Stepping Motor Driver)
*	Author : Sébastien PERREAU
*
*	Revision history	:
*		17/03/2014		- Initial release
*********************************************************************/

#include "../PLIB.h"

///*******************************************************************************
//  Function:
//    void eAMIS30621DeamonMotor(LIN_MODULE mLinModule, AMIS30621_PARAM *motor, AMIS30621_DETECTED *busInformations, BOOL mode, BYTE nbMotor);
//
//  Description:
//    This function is a deamon for all stepper which are using the AMIS30621.
//
//  Parameters:
//    mLinModule          - The LIN module which will be used.
//
//    *motor              - A pointer on a AMIS30621_PARAM table which contain all motor parameters.
//
//    *busInformation     - A pointer on a AMIS30621_DETECTED variable which contain all AMIS informations
//                        on the LIN bus.
//
//    mode                - The selected mode for the state machine.
//
//    nbMotor             - The size of AMIS30621_PARAM table.
//
//  Returns:
//
//  Example:
//    <code>
//    AMIS30621_PARAM motor[3];
//    AMIS30621_DETECTED busInformations;
//    BYTE ind = 0;
//    ...
//    // Init motor 0, 1, 2...
//    motor[0]flags.isInitDone = 0;
//    motor[0]flags.getOtpParam = 0;
//    motor[0]flags.setMotorParam = 0;
//    motor[0]flags.command = 0;
//    motor[0]getFullStatus.adress = 0xF0;
//    motor[0]setMotorParam.param.iParamBits.iHold = I_59_MA;
//    motor[0]setMotorParam.param.iParamBits.iRun = I_566_MA;
//    motor[0]setMotorParam.param.vParamBits.vMin = V_MIN_V_MAX_4_32;
//    motor[0]setMotorParam.param.vParamBits.vMax = V_MAX_546_FULL_STEP_SEC;
//    motor[0]setMotorParam.param.acc = ACC_16531;
//    motor[0]setMotorParam.param.shaft = SHAFT_ON;
//    motor[0]setMotorParam.param.accShaft = ACC_SHAFT_OFF;
//    motor[0]setMotorParam.param.stepMode = STEP_MODE_1_2;
//    motor[0]setDualPositioning.vMin = V_MIN_DEFAULT;
//    motor[0]setDualPositioning.vMax = V_MAX_DEFAULT;
//    motor[0]setDualPositioning.pos1 = 0;
//    motor[0]setDualPositioning.pos2 = 0;
//    motor[0]position16step = 0;
//    ...
//    while(1)
//    {
//        static QWORD tick1;
//        static BOOL t1 = 0;
//
//        if(mTickCompare(tick1) >= TICK_1S*3)
//        {
//            tick1 = mGetTick();
//            if(t1)
//            {
//                if(motor[2].flags.isInitDone)
//                {
//                    setParam1(&motor[2]);
//                    motor[2].position16step += 25000;
//                    t1 = 0;
//                }
//                else
//                {
//                    initMotor(&motor[2], 0xF7);
//                }
//            }
//            else
//            {
//                if(motor[2].flags.isInitDone)
//                {
//                    setParam2(&motor[2]);
//                    motor[2].position16step += 15000;
//                    t1 = 1;
//                }
//                else
//                {
//                    initMotor(&motor[2], 0xF7);
//                }
//            }
//        }
//        eAMIS30621DeamonMotor(LIN2, motor, &busInformations, AMIS_DEAMON_NORMAL, sizeof(motor)/sizeof(AMIS30621_PARAM));
//    }
//    
//    </code>
//  *****************************************************************************/
//void eAMIS30621DeamonMotor(LIN_MODULE mLinModule, AMIS30621_PARAM *motor, AMIS30621_DETECTED *busInformations, BOOL mode, BYTE nbMotor)
//{
//    static BYTE ptrTarget[6] = {0};
//    static BYTE ptrCommand[6] = {0};
//    static state_machine_t smMotor[6];
//    AMIS30621_PARAM *ptrMotor = motor;
//    BYTE i, j;
//
//    enum
//    {
//        SM_GET_FULL_STATUS,
//        SM_INIT,
//        SM_STOP_AND_RESET_POSITION,
//        SM_GET_OTP_PARAM,
//        SM_SET_MOTOR_PARAM,
//        SM_SET_COMMAND,
//        SM_DEFAULT
//    };
//
//    for(i = 0 ; i < nbMotor ; i++, ptrMotor++)
//    {
//        if((ptrMotor->flags.stopAndResetPosition) && (ptrMotor->flags.isInitDone))
//        {
//            motor = ptrMotor;
//            smMotor[mLinModule].index = SM_STOP_AND_RESET_POSITION;
//            ptrTarget[mLinModule] = 0;
//            break;
//        }
//        else if((ptrMotor->flags.setMotorParam) && (ptrMotor->flags.isInitDone))
//        {
//            motor = ptrMotor;
//            smMotor[mLinModule].index = SM_SET_MOTOR_PARAM;
//            ptrTarget[mLinModule] = 0;
//            break;
//        }
//    }
//
//    switch (smMotor[mLinModule].index)
//    {
//        case SM_GET_FULL_STATUS:
//            motor += ptrTarget[mLinModule];
//            switch(eAMIS30621GetFullStatus(mLinModule, motor->getFullStatus.adress, &motor->getFullStatus))
//            {
//                case 0:
//                    busInformations->isNewDetection = TRUE;
//                    for(i = 0 ; i < busInformations->numberMotorDetected ; i++)
//                    {
//                        if(busInformations->tabAdressDetected[i] == motor->getFullStatus.adress)
//                        {
//                            busInformations->isNewDetection = FALSE;
//                        }
//                    }
//                    if(busInformations->isNewDetection)
//                    {
//                        busInformations->tabAdressDetected[busInformations->numberMotorDetected++] = motor->getFullStatus.adress;
//                    }
//                    if(motor->flags.isInitDone)
//                    {
//                        if(motor->flags.getOtpParam)
//                        {
//                            smMotor[mLinModule].index = SM_GET_OTP_PARAM;
//                        }
//                        else
//                        {
//                            smMotor[mLinModule].index = SM_SET_COMMAND;
//                            ptrCommand[mLinModule] = 0;
//                        }
//                    }
//                    else
//                    {
//                        smMotor[mLinModule].index = SM_INIT;
//                    }
//                    break;
//                case 1:
//                    // Do Nothing
//                    break;
//                case 2:
//                    for(i = 0 ; i < busInformations->numberMotorDetected ; i++)
//                    {
//                        if(busInformations->tabAdressDetected[i] == motor->getFullStatus.adress)
//                        {
//                            for(j = i ; j < busInformations->numberMotorDetected ; j++)
//                            {
//                                busInformations->tabAdressDetected[j] = busInformations->tabAdressDetected[j+1];
//                            }
//                            busInformations->numberMotorDetected--;
//                        }
//                    }
//                    motor->flags.isInitDone = 0;
//                    smMotor[mLinModule].index = SM_DEFAULT;
//                    break;
//            }
//            break;
//        case SM_INIT:
//            motor += ptrTarget[mLinModule];
//            switch(eAMIS30621Init(mLinModule, motor->getFullStatus.adress, &motor->getFullStatus, motor->setMotorParam, motor->setDualPositioning))
//            {
//                case 0:
//                    motor->flags.isInitDone = 1;
//                    smMotor[mLinModule].index = SM_SET_COMMAND;
//                    ptrCommand[mLinModule] = 0;
//                    break;
//                case 1:
//                    // Do Nothing
//                    break;
//                case 2:
//                    smMotor[mLinModule].index = SM_DEFAULT;
//                    break;
//            }
//            break;
//        case SM_STOP_AND_RESET_POSITION:
//            motor += ptrTarget[mLinModule];
//            if(!eAMIS30621SoftStopAndResetPosition(mLinModule, motor->getFullStatus.adress))
//            {
//                motor->flags.stopAndResetPosition = 0;
//                smMotor[mLinModule].index = SM_SET_COMMAND;
//                ptrCommand[mLinModule] = 0;
//            }
//            break;
//        case SM_GET_OTP_PARAM:
//            motor += ptrTarget[mLinModule];
//            if(!eAMIS30621GetOTPParam(mLinModule, motor->getFullStatus.adress, &motor->getOtpParam))
//            {
//                motor->flags.getOtpParam = 0;
//                smMotor[mLinModule].index = SM_SET_COMMAND;
//                ptrCommand[mLinModule] = 0;
//            }
//            break;
//        case SM_SET_MOTOR_PARAM:
//            motor += ptrTarget[mLinModule];
//            if(!eAMIS30621SetMotorParam(mLinModule, motor->getFullStatus.adress, motor->setMotorParam))
//            {
//                motor->flags.setMotorParam = 0;
//                smMotor[mLinModule].index = SM_SET_COMMAND;
//                ptrCommand[mLinModule] = 0;
//            }
//            break;
//        case SM_SET_COMMAND:
//            if(mode)
//            {
//                motor += ptrCommand[mLinModule];
//            }
//            else
//            {
//                motor += ptrTarget[mLinModule];
//            }
//            if(motor->flags.isInitDone)
//            {
//                if(!motor->flags.command)
//                {
//                    if(!eAMIS30621SetPosition1M(mLinModule, motor->getFullStatus.adress, motor->position16step))
//                    {
//                        if(++ptrCommand[mLinModule] >= nbMotor)
//                        {
//                            ptrCommand[mLinModule] = 0;
//                            smMotor[mLinModule].index = SM_DEFAULT;
//                        }
//                        if(!mode)
//                        {
//                            smMotor[mLinModule].index = SM_DEFAULT;
//                        }
//                    }
//                }
//                else
//                {
//                    if(!eAMIS30621SetPositionShort1M(mLinModule, motor->getFullStatus.adress, motor->position16step))
//                    {
//                        if(++ptrCommand[mLinModule] >= nbMotor)
//                        {
//                            ptrCommand[mLinModule] = 0;
//                            smMotor[mLinModule].index = SM_DEFAULT;
//                        }
//                        if(!mode)
//                        {
//                            smMotor[mLinModule].index = SM_DEFAULT;
//                        }
//                    }
//                }
//            }
//            else
//            {
//                if(++ptrCommand[mLinModule] >= nbMotor)
//                {
//                    ptrCommand[mLinModule] = 0;
//                    smMotor[mLinModule].index = SM_DEFAULT;
//                }
//                if(!mode)
//                {
//                    smMotor[mLinModule].index = SM_DEFAULT;
//                }
//            }
//            break;
//        default:
//            if(++ptrTarget[mLinModule] >= nbMotor)
//            {
//                ptrTarget[mLinModule] = 0;
//            }
//            smMotor[mLinModule].index = SM_GET_FULL_STATUS;
//            break;
//    }
//}
//
///*******************************************************************************
//  Function:
//    BYTE eAMIS30621Init(LIN_MODULE mLinModule, BYTE adress, _GET_FULL_STATUS *getFullStatus, _SET_MOTOR_PARAM setMotorParam, _SET_DUAL_POSITIONING_PARAM setDualPositioning);
//
//  Description:
//    This routine return the full status of an AMIS30621.
//
//  Parameters:
//    mLinModule          - The LIN module which will be used.
//
//    adress              - Adress of the AMIS30621.
//
//    *getFullStatus      - The variable containing the full status of the motor.
//
//    setMotorParam       - The variable containing the motor parameters.
//
//    setDualPositioning  - The variable containing the dual positioning parameters.
//
//  Returns:
//    0x00                - Function over.
//    0x01                - Function in progress.
//    0x02                - Adress not found / function over.
//
//  Example:
//    <code>
//    _SET_MOTOR_PARAM setMotorParam;
//    _SET_DUAL_POSITIONING_PARAM setDualPositioning;
//    _GET_FULL_STATUS getFullStatus;
//    eAMIS30621Init(LIN2, 0xF0, &getFullStatus, setMotorParam, setDualPositioning);
//    </code>
//  *****************************************************************************/
//BYTE eAMIS30621Init(LIN_MODULE mLinModule, BYTE adress, _GET_FULL_STATUS *getFullStatus, _SET_MOTOR_PARAM setMotorParam, _SET_DUAL_POSITIONING_PARAM setDualPositioning)
//{
//    static LIN_FRAME *linPtrFrame;
//    static state_machine_t smInit = {0};
//
//    if(LINGetStatusBitsAdress(mLinModule)->readyForNextTransmission)
//    {
//        BYTE ret;
//        linPtrFrame = LINGetFrameAdress(mLinModule);
//        switch(smInit.index)
//        {
//            case 0:
//                ret = eAMIS30621GetFullStatus(mLinModule, adress, getFullStatus);
//                if(!ret)
//                {
//                    smInit.index++;
//                }
//                else if(ret == 2)
//                {
//                    smInit.index = 0;
//                    return 2;
//                }
//                break;
//            case 1:
//                if(!eAMIS30621SetDynamicIdentifiers(mLinModule, adress))
//                {
//                    smInit.index++;
//                }
//                break;
//            case 2:
//                if(!eAMIS30621ResetToDefault(mLinModule, adress))
//                {
//                    smInit.index++;
//                }
//                break;
//            case 3:
//                ret = eAMIS30621GetFullStatus(mLinModule, adress, getFullStatus);
//                if(!ret)
//                {
//                    smInit.index++;
//                }
//                else if(ret == 2)
//                {
//                    smInit.index = 0;
//                    return 2;
//                }
//                break;
//            case 4:
//                if(!eAMIS30621SetDynamicIdentifiers(mLinModule, adress))
//                {
//                    smInit.index++;
//                }
//                break;
//            case 5:
//                if(!eAMIS30621SetMotorParam(mLinModule, adress, setMotorParam))
//                {
//                    smInit.index++;
//                }
//                break;
//            case 6:
//                if(!eAMIS30621SoftStop(mLinModule, adress))
//                {
//                    smInit.index++;
//                }
//                break;
//            case 7:
//                if(!eAMIS30621ResetPosition(mLinModule, adress))
//                {
//                    smInit.index++;
//                    if(setDualPositioning.pos1 == 0)
//                    {
//                        smInit.index++;
//                    }
//                }
//                break;
//            case 8:
//                if(!eAMIS30621SetDualPositioning(mLinModule, adress, setDualPositioning))
//                {
//                    smInit.index++;
//                }
//                break;
//            default:
//                smInit.index = 0;
//                return 0;
//        }
//    }
//    return 1;
//}
//
///*******************************************************************************
//  Function:
//    BYTE eAMIS30621GetFullStatus(LIN_MODULE mLinModule, BYTE adress, _GET_FULL_STATUS *getFullStatus);
//
//  Description:
//    This routine return the full status of an AMIS30621.
//
//  Parameters:
//    mLinModule      - The LIN module which will be used.
//
//    adress          - Adress of the AMIS30621.
//
//    *getFullStatus  - The variable to fill with the data return by slave.
//
//  Returns:
//    0x00            - Function over.
//    0x01            - Function in progress.
//    0x02            - Adress not found / function over.
//
//  Example:
//    <code>
//    static _GET_FULL_STATUS getFullStatus;
//    eAMIS30621GetFullStatus(LIN2, 0xF0, &getFullStatus);
//    </code>
//  *****************************************************************************/
//BYTE eAMIS30621GetFullStatus(LIN_MODULE mLinModule, BYTE adress, _GET_FULL_STATUS *getFullStatus)
//{
//    static LIN_FRAME *linPtrFrame;
//    static state_machine_t smGetFullStatus = {0};
//
//    if(LINGetStatusBitsAdress(mLinModule)->readyForNextTransmission)
//    {
//        if(mTickCompare(smGetFullStatus.tick) >= TICK_20MS)
//        {
//            smGetFullStatus.index = 0;
//        }
//
//        linPtrFrame = LINGetFrameAdress(mLinModule);
//        switch(smGetFullStatus.index)
//        {
//            case 0:
//                linPtrFrame->id = ID_FRAME_CDE;
//                linPtrFrame->dlc = NB_BYTE_CDE;
//                linPtrFrame->data[0] = FRAME_GET_FULL_STATUS_1;
//                linPtrFrame->data[1] = FRAME_GET_FULL_STATUS_2;
//                linPtrFrame->data[2] = adress;
//                linPtrFrame->data[3] = 0xFF;
//                linPtrFrame->data[4] = 0xFF;
//                linPtrFrame->data[5] = 0xFF;
//                linPtrFrame->data[6] = 0xFF;
//                linPtrFrame->data[7] = 0xFF;
//
//                LINFlush(mLinModule, LIN_TRANSMISSION_REQUEST);
//                smGetFullStatus.index++;
//                smGetFullStatus.tick = mGetTick();
//                break;
//            case 1:
//                linPtrFrame->id = ID_FRAME_BACK_CDE;
//                linPtrFrame->dlc = NB_BYTE_BACK_CDE;
//
//                LINFlush(mLinModule, LIN_RECEPTION_REQUEST);
//                smGetFullStatus.index++;
//                smGetFullStatus.tick = mGetTick();
//                break;
//            case 2:
//                if(linPtrFrame->data[0] != adress)
//                {
//                    smGetFullStatus.index = 0;
//                    LINCleanup(mLinModule);
//                    return 2;
//                }
//
//                getFullStatus->adress = linPtrFrame->data[0];
//                getFullStatus->param.iParam = linPtrFrame->data[1];
//                getFullStatus->param.vParam = linPtrFrame->data[2];
//                getFullStatus->param.acc = (linPtrFrame->data[3] >> 0)&0x0F;
//                getFullStatus->param.shaft = (linPtrFrame->data[3] >> 4)&0x01;
//                getFullStatus->param.stepMode = (linPtrFrame->data[3] >> 5)&0x03;
//                getFullStatus->param.accShaft = (linPtrFrame->data[3] >> 7)&0x01;
//                getFullStatus->tInfo = (linPtrFrame->data[4] >> 0)&0x03;
//                getFullStatus->tw = (linPtrFrame->data[4] >> 2)&0x01;
//                getFullStatus->tsd = (linPtrFrame->data[4] >> 3)&0x01;
//                getFullStatus->uv2 = (linPtrFrame->data[4] >> 4)&0x01;
//                getFullStatus->eidef = (linPtrFrame->data[4] >> 5)&0x01;
//                getFullStatus->stepLoss = (linPtrFrame->data[4] >> 6)&0x01;
//                getFullStatus->vddReset = (linPtrFrame->data[4] >> 7)&0x01;
//
//                LINFlush(mLinModule, LIN_RECEPTION_REQUEST);
//                smGetFullStatus.index++;
//                smGetFullStatus.tick = mGetTick();
//                break;
//            default:
//                getFullStatus->actPos = (linPtrFrame->data[1] << 8) + linPtrFrame->data[2];
//                getFullStatus->tagPos = (linPtrFrame->data[3] << 8) + linPtrFrame->data[4];
//                getFullStatus->secPos = ((linPtrFrame->data[6] << 8) & 0x03) + linPtrFrame->data[5];
//                smGetFullStatus.index = 0;
//                return 0;
//        }
//    }
//    return 1;
//}
//
///*******************************************************************************
//  Function:
//    BYTE eAMIS30621GetActualPos(LIN_MODULE mLinModule, BYTE adress, UINT *getActualPos);
//
//  Description:
//    This routine return the actual position of the stepper.
//
//  Parameters:
//    mLinModule      - The LIN module which will be used.
//
//    adress          - Adress of the AMIS30621.
//
//    *getFullStatus  - The variable to fill with the data return by slave.
//
//  Returns:
//    0x00            - Function over.
//    0x01            - Function in progress.
//    0x02            - Adress not found / function over.
//
//  Example:
//    <code>
//    static UINT getActPos;
//    eAMIS30621GetActualPos(LIN2, 0xF0, &getActPos);
//    </code>
//  *****************************************************************************/
//BYTE eAMIS30621GetActualPos(LIN_MODULE mLinModule, BYTE adress, UINT *getActualPos)
//{
//    static LIN_FRAME *linPtrFrame;
//    static state_machine_t smGetActualPos = {0};
//
//    if(LINGetStatusBitsAdress(mLinModule)->readyForNextTransmission)
//    {
//        linPtrFrame = LINGetFrameAdress(mLinModule);
//        switch(smGetActualPos.index)
//        {
//            case 0:
//                linPtrFrame->id = ID_FRAME_CDE;
//                linPtrFrame->dlc = NB_BYTE_CDE;
//                linPtrFrame->data[0] = FRAME_GET_FULL_POS_1;
//                linPtrFrame->data[1] = FRAME_GET_FULL_POS_2;
//                linPtrFrame->data[2] = adress;
//                linPtrFrame->data[3] = 0xFF;
//                linPtrFrame->data[4] = 0xFF;
//                linPtrFrame->data[5] = 0xFF;
//                linPtrFrame->data[6] = 0xFF;
//                linPtrFrame->data[7] = 0xFF;
//
//                LINFlush(mLinModule, LIN_TRANSMISSION_REQUEST);
//                smGetActualPos.index++;
//                break;
//            case 1:
//                linPtrFrame->id = ID_FRAME_BACK_CDE;
//                linPtrFrame->dlc = NB_BYTE_BACK_CDE;
//
//                LINFlush(mLinModule, LIN_RECEPTION_REQUEST);
//                smGetActualPos.index++;
//                break;
//            default:
//                if(linPtrFrame->data[0] != adress)
//                {
//                    smGetActualPos.index = 0;
//                    LINCleanup(mLinModule);
//                    return 2;
//                }
//
//                *getActualPos = (linPtrFrame->data[1] << 8) + linPtrFrame->data[2];
//                smGetActualPos.index = 0;
//                return 0;
//        }
//    }
//    return 1;
//}
//
///*******************************************************************************
//  Function:
//    BOOL eAMIS30621GetOTPParam(LIN_MODULE mLinModule, BYTE adress, _GET_OTP_PARAM *getOtpParam);
//
//  Description:
//    This routine return the value of the OTP Memory.
//
//  Parameters:
//    mLinModule      - The LIN module which will be used.
//
//    adress          - Adress of the AMIS30621.
//
//    *getFullStatus  - The variable to fill with the data return by slave.
//
//  Returns:
//    0x00            - Function over.
//    0x01            - Function in progress.
//
//  Example:
//    <code>
//    static _GET_OTP_PARAM getOtpParam;
//    eAMIS30621GetOTPParam(LIN2, 0xF0, &getOtpParam);
//    </code>
//  *****************************************************************************/
//BOOL eAMIS30621GetOTPParam(LIN_MODULE mLinModule, BYTE adress, _GET_OTP_PARAM *getOtpParam)
//{
//    static LIN_FRAME *linPtrFrame;
//    static state_machine_t smGetOtpParam = {0};
//
//    if(LINGetStatusBitsAdress(mLinModule)->readyForNextTransmission)
//    {
//        linPtrFrame = LINGetFrameAdress(mLinModule);
//        switch(smGetOtpParam.index)
//        {
//            case 0:
//                linPtrFrame->id = ID_FRAME_CDE;
//                linPtrFrame->dlc = NB_BYTE_CDE;
//                linPtrFrame->data[0] = FRAME_GET_OTP_PARAM_1;
//                linPtrFrame->data[1] = FRAME_GET_OTP_PARAM_2;
//                linPtrFrame->data[2] = adress;
//                linPtrFrame->data[3] = 0xFF;
//                linPtrFrame->data[4] = 0xFF;
//                linPtrFrame->data[5] = 0xFF;
//                linPtrFrame->data[6] = 0xFF;
//                linPtrFrame->data[7] = 0xFF;
//
//                LINFlush(mLinModule, LIN_TRANSMISSION_REQUEST);
//                smGetOtpParam.index++;
//                break;
//            case 1:
//                linPtrFrame->id = ID_FRAME_BACK_CDE;
//                linPtrFrame->dlc = NB_BYTE_BACK_CDE;
//
//                LINFlush(mLinModule, LIN_RECEPTION_REQUEST);
//                smGetOtpParam.index++;
//                break;
//            default:
//                getOtpParam->iRef = (linPtrFrame->data[0] >> 0) & 0x0F;
//                getOtpParam->osc = (linPtrFrame->data[0] >> 4) & 0x0F;
//                getOtpParam->bg = (linPtrFrame->data[1] >> 0) & 0x0F;
//                getOtpParam->tsd = (linPtrFrame->data[1] >> 4) & 0x07;
//                getOtpParam->pa = (linPtrFrame->data[2] >> 0) & 0x0F;
//                getOtpParam->hw = (linPtrFrame->data[2] >> 4) & 0x07;
//                getOtpParam->adm = (linPtrFrame->data[2] >> 7) & 0x01;
//                getOtpParam->param.iParam = linPtrFrame->data[3];
//                getOtpParam->param.vParam = linPtrFrame->data[4];
//                getOtpParam->param.acc = (linPtrFrame->data[5] >> 0) & 0x0F;
//                getOtpParam->param.shaft = (linPtrFrame->data[5] >> 4) & 0x01;
//                getOtpParam->secPos = (linPtrFrame->data[5] << 3) + linPtrFrame->data[6];
//                getOtpParam->param.stepMode = (linPtrFrame->data[7] >> 2) & 0x03;
//
//                smGetOtpParam.index = 0;
//                return 0;
//        }
//    }
//    return 1;
//}
//
///*******************************************************************************
//  Function:
//    BOOL eAMIS30621SetOTPParam(LIN_MODULE mLinModule, BYTE adress, _SET_OTP_PARAM setOtpParam);
//
//  Description:
//    This command is provided to the circuit by the LIN master to program the content
//    D of the OTP memory byte OTPA and to zap it.
//
//    IMPORTANT: THIS COMMAND MUST BE SENT UNDER A SPECIFIC VBB VOLTAGE VALUE (min 9.0V / max 10.0v).
//
//  Parameters:
//    mLinModule      - The LIN module which will be used.
//
//    adress          - Adress of the AMIS30621.
//
//    setOtpParam     - Parameters for OTP memory.
//
//  Returns:
//    0x00            - Function over.
//    0x01            - Function in progress.
//
//  Example:
//    <code>
//    </code>
//  *****************************************************************************/
//BOOL eAMIS30621SetOTPParam(LIN_MODULE mLinModule, BYTE adress, _SET_OTP_PARAM setOtpParam)
//{
//    LIN_FRAME *linPtrFrame;
//
//    if(LINGetStatusBitsAdress(mLinModule)->readyForNextTransmission)
//    {
//        linPtrFrame = LINGetFrameAdress(mLinModule);
//
//        linPtrFrame->id = ID_FRAME_CDE;
//        linPtrFrame->dlc = NB_BYTE_CDE;
//        linPtrFrame->data[0] = FRAME_SET_OTP_PARAM_1;
//        linPtrFrame->data[1] = FRAME_SET_OTP_PARAM_2;
//        linPtrFrame->data[2] = adress;
//        linPtrFrame->data[3] = 0xF8 | (setOtpParam.otpMemoryAdress & 0x07);
//        linPtrFrame->data[4] = setOtpParam.data;
//        linPtrFrame->data[5] = 0xFF;
//        linPtrFrame->data[6] = 0xFF;
//        linPtrFrame->data[7] = 0xFF;
//
//        LINFlush(mLinModule, LIN_TRANSMISSION_REQUEST);
//        return 0;
//    }
//    return 1;
//}
//
///*******************************************************************************
//  Function:
//    BOOL eAMIS30621SetMotorParam(LIN_MODULE mLinModule, BYTE adress, _SET_MOTOR_PARAM setMotorParam);
//
//  Description:
//    This routine set all parameters for the stepper.
//
//  Parameters:
//    mLinModule      - The LIN module which will be used.
//
//    adress          - Adress of the AMIS30621.
//
//    setMotorParam   - The variable which contain all parameters for the stepper.
//
//  Returns:
//    0x00            - Function over.
//    0x01            - Function in progress.
//
//  Example:
//    <code>
//    _SET_MOTOR_PARAM setMotorParam;
//    eAMIS30621SetDefaultMotorParam(&setMotorParam);
//    eAMIS30621SetMotorParam(LIN2, 0xF0, setMotorParam);
//    </code>
//  *****************************************************************************/
//BOOL eAMIS30621SetMotorParam(LIN_MODULE mLinModule, BYTE adress, _SET_MOTOR_PARAM setMotorParam)
//{
//    LIN_FRAME *linPtrFrame;
//
//    if(LINGetStatusBitsAdress(mLinModule)->readyForNextTransmission)
//    {
//        linPtrFrame = LINGetFrameAdress(mLinModule);
//
//        linPtrFrame->id = ID_FRAME_CDE;
//        linPtrFrame->dlc = NB_BYTE_CDE;
//        linPtrFrame->data[0] = FRAME_SET_MOTOR_PARAM_1;
//        linPtrFrame->data[1] = FRAME_SET_MOTOR_PARAM_2;
//        linPtrFrame->data[2] = adress;
//        linPtrFrame->data[3] = (BYTE) (setMotorParam.param.iParam);
//        linPtrFrame->data[4] = (BYTE) (setMotorParam.param.vParam);
//        linPtrFrame->data[5] = (BYTE) (setMotorParam.param.acc & 0x0F) | ((setMotorParam.param.shaft & 0x01) << 4) | ((setMotorParam.secPos >> 3) & 0xE0);
//        linPtrFrame->data[6] = (BYTE) (setMotorParam.secPos & 0xFF);
//        linPtrFrame->data[7] = (BYTE) ((setMotorParam.param.stepMode & 0x03) << 2) | ((setMotorParam.param.accShaft & 0x01) << 4);
//
//        LINFlush(mLinModule, LIN_TRANSMISSION_REQUEST);
//        return 0;
//    }
//    return 1;
//}
//
///*******************************************************************************
//  Function:
//    BOOL eAMIS30621SetDualPositioning(LIN_MODULE mLinModule, BYTE adress, _SET_DUAL_POSITIONING_PARAM setDualPositioning);
//
//  Description:
//    This command is provided to the circuit by the LIN master in order to perform
//    a positioning of the motor using two different velocities (<Vmax> is the max velocity
//    for first motion and <Vmin> is the velocity for first motion and velocity for second motion).
//    After dual positioning the internal flag <Reference Done> is set.
//
//    NOTE: This sequence cannot be interrupted by another positioning command.
//
//    IMPORTANT: If for some reason <ActPos> equals <pos1> at the moment the SetDualPositioning
//    command is issued, the circuit will enter in deadlock state. That's why the application
//    should check the actual position by GetPosition or GetFullStatus command prior to start
//    a dual positioning. For the same reason, <pos2> sould not be equal to <pos1>.
//
//
//  Parameters:
//    mLinModule          - The LIN module which will be used.
//
//    adress              - Adress of the AMIS30621.
//
//    setDualPositioning  - The variable which contain all parameters for the stepper.
//
//  Returns:
//    0x00                - Function over.
//    0x01                - Function in progress.
//
//  Example:
//    <code>
//    _SET_DUAL_POSITIONING_PARAM setDualPositioning;
//    eAMIS30621SetDefaultDualPositioning(&setDualPositioning);
//    eAMIS30621SetDualPositioning(LIN2, 0xF0, setDualPositioning);
//    </code>
//  *****************************************************************************/
//BOOL eAMIS30621SetDualPositioning(LIN_MODULE mLinModule, BYTE adress, _SET_DUAL_POSITIONING_PARAM setDualPositioning)
//{
//    LIN_FRAME *linPtrFrame;
//
//    if(LINGetStatusBitsAdress(mLinModule)->readyForNextTransmission)
//    {
//        linPtrFrame = LINGetFrameAdress(mLinModule);
//
//        linPtrFrame->id = ID_FRAME_CDE;
//        linPtrFrame->dlc = NB_BYTE_CDE;
//        linPtrFrame->data[0] = FRAME_SET_DUAL_POSITIONING_1;
//        linPtrFrame->data[1] = FRAME_SET_DUAL_POSITIONING_2;
//        linPtrFrame->data[2] = adress;
//        linPtrFrame->data[3] = 0x83;//((setDualPositioning.vMax & 0x0F) << 4) | ((setDualPositioning.vMin & 0x0F) << 0);
//        linPtrFrame->data[4] = (setDualPositioning.pos1 >> 8) & 0xFF;
//        linPtrFrame->data[5] = (setDualPositioning.pos1 >> 0) & 0xFF;
//        linPtrFrame->data[6] = (setDualPositioning.pos2 >> 8) & 0xFF;
//        linPtrFrame->data[7] = (setDualPositioning.pos2 >> 0) & 0xFF;
//
//        LINFlush(mLinModule, LIN_TRANSMISSION_REQUEST);
//        return 0;
//    }
//    return 1;
//}
//
///*******************************************************************************
//  Function:
//    BOOL eAMIS30621SetDynamicIdentifiers(LIN_MODULE mLinModule, BYTE adress);
//
//  Description:
//    The identifier field in the LIN datagram denotes the content of the message.
//    Six identifier bits and two parity bits are used to represent the content.
//    The identifiers 0x3C abd 0x3F are reserved for command frames and extended frames.
//    Slave nodes need to be very flexible to adapt itself to a given LIN network in
//    order to avoid conflicts with slave nodes from different manufacturers.
//    Dynamic assignment of the identifiers will futfill this requirement by writing
//    identifiers into the circuits RAM. ROM pointers are linking commands and dynamic
//    identifiers together.
//
//    NOTE: Identifiers are declared in e_amis30621.h header file. 
//
//  Parameters:
//    mLinModule      - The LIN module which will be used.
//
//    adress          - Adress of the AMIS30621.
//
//  Returns:
//    0x00            - Function over.
//    0x01            - Function in progress.
//
//  Example:
//    <code>
//    while(!eAMIS30621SetDynamicIdentifiers(LIN2, 0xF0));
//    </code>
//  *****************************************************************************/
//BOOL eAMIS30621SetDynamicIdentifiers(LIN_MODULE mLinModule, BYTE adress)
//{
//    static LIN_FRAME *linPtrFrame;
//    static state_machine_t smSetDynamicId = {0};
//
//    if(LINGetStatusBitsAdress(mLinModule)->readyForNextTransmission)
//    {
//        linPtrFrame = LINGetFrameAdress(mLinModule);
//
//        switch(smSetDynamicId.index)
//        {
//            case 0:
//            linPtrFrame->id = ID_FRAME_CDE;
//            linPtrFrame->dlc = NB_BYTE_CDE;
//            linPtrFrame->data[0] = FRAME_DYNAMIC_ID_1;
//            linPtrFrame->data[1] = FRAME_DYNAMIC_ID_2;
//            linPtrFrame->data[2] = adress;
//            linPtrFrame->data[3] = (BYTE) ((ID_FRAME_GEN_PURPOSE2 & 0x3F) << 4 ) | (ROM_POINTER_0 << 0);
//            linPtrFrame->data[4] = (BYTE) ((ID_FRAME_GEN_PURPOSE4 & 0x3F) << 6 ) | (ROM_POINTER_1 << 2) | ((ID_FRAME_GEN_PURPOSE2 & 0x3F) >> 4 );
//            linPtrFrame->data[5] = (BYTE) (ROM_POINTER_2 << 4) | ((ID_FRAME_GEN_PURPOSE4 & 0x3F) >> 2 );
//            linPtrFrame->data[6] = (BYTE) (ROM_POINTER_4 << 6) | ((ID_FRAME_GET_ACTUAL_POS & 0x3F) << 0);
//            linPtrFrame->data[7] = (BYTE) ((ID_FRAME_SET_POSITION & 0x3F) << 6 ) | (ROM_POINTER_4 >> 2);
//
//            LINFlush(mLinModule, LIN_TRANSMISSION_REQUEST);
//            smSetDynamicId.index++;
//            break;
//        case 1:
//            linPtrFrame->id = ID_FRAME_CDE;
//            linPtrFrame->dlc = NB_BYTE_CDE;
//            linPtrFrame->data[0] = FRAME_DYNAMIC_ID_1;
//            linPtrFrame->data[1] = FRAME_DYNAMIC_ID_2;
//            linPtrFrame->data[2] = adress;
//            linPtrFrame->data[3] = (BYTE) ((ID_FRAME_SET_POS_SHORT1 & 0x3F) << 4 ) | (ROM_POINTER_5 << 0);
//            linPtrFrame->data[4] = (BYTE) ((ID_FRAME_SET_POS_SHORT2 & 0x3F) << 6 ) | (ROM_POINTER_6 << 2) | ((ID_FRAME_SET_POS_SHORT1 & 0x3F) >> 4 );
//            linPtrFrame->data[5] = (BYTE) (ROM_POINTER_7 << 4) | ((ID_FRAME_SET_POS_SHORT2 & 0x3F) >> 2 );
//            linPtrFrame->data[6] = (BYTE) (ROM_POINTER_8 << 6) | ((ID_FRAME_SET_POS_SHORT4 & 0x3F) << 0);
//            linPtrFrame->data[7] = (BYTE) ((ID_FRAME_PREPARING_FRAME & 0x3F) << 6 ) | (ROM_POINTER_8 >> 2);
//
//            LINFlush(mLinModule, LIN_TRANSMISSION_REQUEST);
//            smSetDynamicId.index++;
//            break;
//        default:
//            smSetDynamicId.index = 0;
//            return 0;
//                break;
//        }
//    }
//    return 1;
//}
//
///*******************************************************************************
//  Function:
//    BOOL eAMIS30621SetPositionShort1M(LIN_MODULE mLinModule, BYTE adress, UINT position16steps);
//
//  Description:
//    This command is provided to the circuit by the LIN master to driver one motor to a given
//    absolute position.
//
//  Parameters:
//    mLinModule          - The LIN module which will be used.
//
//    adress              - Adress of the AMIS30621.
//
//    position16steps     - Position in 1/16e steps.
//
//  Returns:
//    0x00                - Function over.
//    0x01                - Function in progress.
//
//  Example:
//    <code>
//    eAMIS30621SetPositionShort1M(LIN2, 0xF0, 10000);
//    </code>
//  *****************************************************************************/
//BOOL eAMIS30621SetPositionShort1M(LIN_MODULE mLinModule, BYTE adress, UINT position16steps)
//{
//    LIN_FRAME *linPtrFrame;
//
//    if(LINGetStatusBitsAdress(mLinModule)->readyForNextTransmission)
//    {
//        linPtrFrame = LINGetFrameAdress(mLinModule);
//
//        linPtrFrame->id = ID_FRAME_SET_POS_SHORT1;
//        linPtrFrame->dlc = NB_BYTE_SET_POS_SHORT1;
//        linPtrFrame->data[0] = ((position16steps >> 3) & 0xE0) | ((adress >> 3) & 0x10) | (adress & 0x0F);
//        linPtrFrame->data[1] = position16steps & 0xFF;
//
//        LINFlush(mLinModule, LIN_TRANSMISSION_REQUEST);
//        return 0;
//    }
//    return 1;
//}
//
///*******************************************************************************
//  Function:
//    BOOL eAMIS30621SetPositionShort2M(LIN_MODULE mLinModule, BYTE adressA, UINT position16stepsA, BYTE adressB, UINT position16stepsB);
//
//  Description:
//    This command is provided to the circuit by the LIN master to driver two motors to a given
//    absolute position.
//
//  Parameters:
//    mLinModule          - The LIN module which will be used.
//
//    adressA             - Adress 1 of the AMIS30621.
//
//    position16stepsA    - Position 1 in 1/16e steps.
//
//    adressB             - Adress 2 of the AMIS30621.
//
//    position16stepsB    - Position 2 in 1/16e steps.
//
//  Returns:
//    0x00                - Function over.
//    0x01                - Function in progress.
//
//  Example:
//    <code>
//    eAMIS30621SetPositionShort2M(LIN2, 0xF0, 10000, 0xF7, 3000);
//    </code>
//  *****************************************************************************/
//BOOL eAMIS30621SetPositionShort2M(LIN_MODULE mLinModule, BYTE adressA, UINT position16stepsA, BYTE adressB, UINT position16stepsB)
//{
//    LIN_FRAME *linPtrFrame;
//
//    if(LINGetStatusBitsAdress(mLinModule)->readyForNextTransmission)
//    {
//        linPtrFrame = LINGetFrameAdress(mLinModule);
//
//        linPtrFrame->id = ID_FRAME_SET_POS_SHORT2;
//        linPtrFrame->dlc = NB_BYTE_SET_POS_SHORT2;
//        linPtrFrame->data[0] = ((position16stepsA >> 3) & 0xE0) | 0x10 | (adressA & 0x0F);
//        linPtrFrame->data[1] = position16stepsA & 0xFF;
//        linPtrFrame->data[2] = ((position16stepsB >> 3) & 0xE0) | 0x10 | (adressB & 0x0F);
//        linPtrFrame->data[3] = position16stepsB & 0xFF;
//
//        LINFlush(mLinModule, LIN_TRANSMISSION_REQUEST);
//        return 0;
//    }
//    return 1;
//}
//
///*******************************************************************************
//  Function:
//    BOOL eAMIS30621SetPositionShort4M(LIN_MODULE mLinModule, BYTE adressA, UINT position16stepsA, BYTE adressB, UINT position16stepsB, BYTE adressC, UINT position16stepsC, BYTE adressD, UINT position16stepsD);
//
//  Description:
//    This command is provided to the circuit by the LIN master to driver four motors to a given
//    absolute position.
//
//  Parameters:
//    mLinModule          - The LIN module which will be used.
//
//    adressA             - Adress 1 of the AMIS30621.
//
//    position16stepsA    - Position 1 in 1/16e steps.
//
//    adressB             - Adress 2 of the AMIS30621.
//
//    position16stepsB    - Position 2 in 1/16e steps.
//
//    adressC             - Adress 3 of the AMIS30621.
//
//    position16stepsC    - Position 3 in 1/16e steps.
//
//    adressD             - Adress 4 of the AMIS30621.
//
//    position16stepsD    - Position 4 in 1/16e steps.
//
//  Returns:
//    0x00                - Function over.
//    0x01                - Function in progress.
//
//  Example:
//    <code>
//    eAMIS30621SetPositionShort4M(LIN2, 0xF0, 10000, 0xF1, 3000, 0xF7, 100, 0xF8, 18000);
//    </code>
//  *****************************************************************************/
//BOOL eAMIS30621SetPositionShort4M(LIN_MODULE mLinModule, BYTE adressA, UINT position16stepsA, BYTE adressB, UINT position16stepsB, BYTE adressC, UINT position16stepsC, BYTE adressD, UINT position16stepsD)
//{
//    LIN_FRAME *linPtrFrame;
//
//    if(LINGetStatusBitsAdress(mLinModule)->readyForNextTransmission)
//    {
//        linPtrFrame = LINGetFrameAdress(mLinModule);
//
//        linPtrFrame->id = ID_FRAME_SET_POS_SHORT4;
//        linPtrFrame->dlc = NB_BYTE_SET_POS_SHORT4;
//        linPtrFrame->data[0] = ((position16stepsA >> 3) & 0xE0) | 0x10 | (adressA & 0x0F);
//        linPtrFrame->data[1] = position16stepsA & 0xFF;
//        linPtrFrame->data[2] = ((position16stepsB >> 3) & 0xE0) | 0x10 | (adressB & 0x0F);
//        linPtrFrame->data[3] = position16stepsB & 0xFF;
//        linPtrFrame->data[4] = ((position16stepsC >> 3) & 0xE0) | 0x10 | (adressC & 0x0F);
//        linPtrFrame->data[5] = position16stepsC & 0xFF;
//        linPtrFrame->data[6] = ((position16stepsD >> 3) & 0xE0) | 0x10 | (adressD & 0x0F);
//        linPtrFrame->data[7] = position16stepsD & 0xFF;
//
//        LINFlush(mLinModule, LIN_TRANSMISSION_REQUEST);
//        return 0;
//    }
//    return 1;
//}
//
///*******************************************************************************
//  Function:
//    BOOL eAMIS30621SetPosition1M(LIN_MODULE mLinModule, BYTE adress, UINT position16steps);
//
//  Description:
//    This command is provided to the circuit by the LIN master to driver one motor to a given
//    absolute position.
//
//  Parameters:
//    mLinModule          - The LIN module which will be used.
//
//    adress              - Adress of the AMIS30621.
//
//    position16steps     - Position in 1/16e steps.
//
//  Returns:
//    0x00                - Function over.
//    0x01                - Function in progress.
//
//  Example:
//    <code>
//    eAMIS30621SetPosition1M(LIN2, 0xF0, 10000);
//    </code>
//  *****************************************************************************/
//BOOL eAMIS30621SetPosition1M(LIN_MODULE mLinModule, BYTE adress, UINT position16steps)
//{
//    LIN_FRAME *linPtrFrame;
//
//    if(LINGetStatusBitsAdress(mLinModule)->readyForNextTransmission)
//    {
//        linPtrFrame = LINGetFrameAdress(mLinModule);
//
//        linPtrFrame->id = ID_FRAME_GEN_PURPOSE4;
//        linPtrFrame->dlc = NB_BYTE_GEN_PURPOSE4;
//        linPtrFrame->data[0] = FRAME_SET_POSITION_2;
//        linPtrFrame->data[1] = adress;
//        linPtrFrame->data[2] = (position16steps >> 8) & 0xFF;
//        linPtrFrame->data[3] = (position16steps >> 0) & 0xFF;
//
//        LINFlush(mLinModule, LIN_TRANSMISSION_REQUEST);
//        return 0;
//    }
//    return 1;
//}
//
///*******************************************************************************
//  Function:
//    BOOL eAMIS30621SetPosition2M(LIN_MODULE mLinModule, BYTE adressA, UINT position16stepsA, BYTE adressB, UINT position16stepsB);
//
//  Description:
//    This command is provided to the circuit by the LIN master to driver two motors to a given
//    absolute position.
//
//  Parameters:
//    mLinModule          - The LIN module which will be used.
//
//    adressA             - Adress 1 of the AMIS30621.
//
//    position16stepsA    - Position 1 in 1/16e steps.
//
//    adressB             - Adress 2 of the AMIS30621.
//
//    position16stepsB    - Position 2 in 1/16e steps.
//
//  Returns:
//    0x00                - Function over.
//    0x01                - Function in progress.
//
//  Example:
//    <code>
//    eAMIS30621SetPosition2M(LIN2, 0xF0, 10000, 0xF7, 4500);
//    </code>
//  *****************************************************************************/
//BOOL eAMIS30621SetPosition2M(LIN_MODULE mLinModule, BYTE adressA, UINT position16stepsA, BYTE adressB, UINT position16stepsB)
//{
//    LIN_FRAME *linPtrFrame;
//
//    if(LINGetStatusBitsAdress(mLinModule)->readyForNextTransmission)
//    {
//        linPtrFrame = LINGetFrameAdress(mLinModule);
//
//        linPtrFrame->id = ID_FRAME_SET_POSITION;
//        linPtrFrame->dlc = NB_BYTE_SET_POSITION;
//        linPtrFrame->data[0] = FRAME_SET_POSITION_1;
//        linPtrFrame->data[1] = FRAME_SET_POSITION_2;
//        linPtrFrame->data[2] = adressA;
//        linPtrFrame->data[3] = (position16stepsA >> 8) & 0xFF;
//        linPtrFrame->data[4] = (position16stepsA >> 0) & 0xFF;
//        linPtrFrame->data[5] = adressB;
//        linPtrFrame->data[6] = (position16stepsB >> 8) & 0xFF;
//        linPtrFrame->data[7] = (position16stepsB >> 0) & 0xFF;
//
//        LINFlush(mLinModule, LIN_TRANSMISSION_REQUEST);
//        return 0;
//    }
//    return 1;
//}
//
///*******************************************************************************
//  Function:
//    BOOL eAMIS30621ResetPosition(LIN_MODULE mLinModule, BYTE adress);
//
//  Description:
//    This routine reset the ActPos and TagPos register to zero.
//
//  Parameters:
//    mLinModule      - The LIN module which will be used.
//
//    adress          - Adress of the AMIS30621.
//
//  Returns:
//    0x00            - Function over.
//    0x01            - Function in progress.
//
//  Example:
//    <code>
//    eAMIS30621ResetPosition(LIN2, 0xF0);
//    </code>
//  *****************************************************************************/
//BOOL eAMIS30621ResetPosition(LIN_MODULE mLinModule, BYTE adress)
//{
//    LIN_FRAME *linPtrFrame;
//
//    if(LINGetStatusBitsAdress(mLinModule)->readyForNextTransmission)
//    {
//        linPtrFrame = LINGetFrameAdress(mLinModule);
//
//        linPtrFrame->id = ID_FRAME_GEN_PURPOSE2;
//        linPtrFrame->dlc = NB_BYTE_GEN_PURPOSE2;
//        linPtrFrame->data[0] = FRAME_RESET_POSITION;
//        linPtrFrame->data[1] = adress;
//
//        LINFlush(mLinModule, LIN_TRANSMISSION_REQUEST);
//        return 0;
//    }
//    return 1;
//}
//
///*******************************************************************************
//  Function:
//    BOOL eAMIS30621ResetToDefault(LIN_MODULE mLinModule, BYTE adress);
//
//  Description:
//    This command is provided to the circuit by the LIN master in order to reset the
//    whole slave node into the initial state. ResetToDefault wille, for instance,
//    overwrite the RAM with the reset state of the registers parameters. This is another
//    way for the master to initialize a slave node in case of emergency or simply to
//    refresh the RAM content.
//
//  Parameters:
//    mLinModule      - The LIN module which will be used.
//
//    adress          - Adress of the AMIS30621.
//
//  Returns:
//    0x00            - Function over.
//    0x01            - Function in progress.
//
//  Example:
//    <code>
//    eAMIS30621ResetToDefault(LIN2, 0xF0);
//    </code>
//  *****************************************************************************/
//BOOL eAMIS30621ResetToDefault(LIN_MODULE mLinModule, BYTE adress)
//{
//    LIN_FRAME *linPtrFrame;
//
//    if(LINGetStatusBitsAdress(mLinModule)->readyForNextTransmission)
//    {
//        linPtrFrame = LINGetFrameAdress(mLinModule);
//
//        linPtrFrame->id = ID_FRAME_GEN_PURPOSE2;
//        linPtrFrame->dlc = NB_BYTE_GEN_PURPOSE2;
//        linPtrFrame->data[0] = FRAME_RESET_TO_DEFAULT;
//        linPtrFrame->data[1] = adress;
//
//        LINFlush(mLinModule, LIN_TRANSMISSION_REQUEST);
//        return 0;
//    }
//    return 1;
//}
//
///*******************************************************************************
//  Function:
//    BOOL eAMIS30621HardStop(LIN_MODULE mLinModule, BYTE adress);
//
//  Description:
//    This command will be internally triggered when an electrical problem is
//    detected in one or both coils, leading to shutdown mode. If this occurs
//    while the motor is moving, the <stepLoss> flag is raised to allow warning
//    of the LIN master at the next GetFullStatus command. Once the motor is stopped,
//    <ActPos> register is copied into <TagPos> register to ensure keeping the stop
//    position.
//
//  Parameters:
//    mLinModule      - The LIN module which will be used.
//
//    adress          - Adress of the AMIS30621.
//
//  Returns:
//    0x00            - Function over.
//    0x01            - Function in progress.
//
//  Example:
//    <code>
//    eAMIS30621HardStop(LIN2, 0xF0);
//    </code>
//  *****************************************************************************/
//BOOL eAMIS30621HardStop(LIN_MODULE mLinModule, BYTE adress)
//{
//    LIN_FRAME *linPtrFrame;
//
//    if(LINGetStatusBitsAdress(mLinModule)->readyForNextTransmission)
//    {
//        linPtrFrame = LINGetFrameAdress(mLinModule);
//
//        linPtrFrame->id = ID_FRAME_GEN_PURPOSE2;
//        linPtrFrame->dlc = NB_BYTE_GEN_PURPOSE2;
//        linPtrFrame->data[0] = FRAME_HARD_STOP;
//        linPtrFrame->data[1] = adress;
//
//        LINFlush(mLinModule, LIN_TRANSMISSION_REQUEST);
//        return 0;
//    }
//    return 1;
//}
//
///*******************************************************************************
//  Function:
//    BOOL eAMIS30621SoftStop(LIN_MODULE mLinModule, BYTE adress);
//
//  Description:
//     If a soft Stop command occur during a motion of the stepper motor, it provokes
//     an immediate deceleration to Vmin followed by a stop (regardless of the position
//     reached). Once the motor is stopped, TagPos register is overwritten with value in
//     ActPos register to ensure keeping the stop position.
//
//  Parameters:
//    mLinModule      - The LIN module which will be used.
//
//    adress          - Adress of the AMIS30621.
//
//  Returns:
//    0x00            - Function over.
//    0x01            - Function in progress.
//
//  Example:
//    <code>
//    eAMIS30621SoftStop(LIN2, 0xF0);
//    </code>
//  *****************************************************************************/
//BOOL eAMIS30621SoftStop(LIN_MODULE mLinModule, BYTE adress)
//{
//    LIN_FRAME *linPtrFrame;
//
//    if(LINGetStatusBitsAdress(mLinModule)->readyForNextTransmission)
//    {
//        linPtrFrame = LINGetFrameAdress(mLinModule);
//
//        linPtrFrame->id = ID_FRAME_GEN_PURPOSE2;
//        linPtrFrame->dlc = NB_BYTE_GEN_PURPOSE2;
//        linPtrFrame->data[0] = FRAME_SOFT_STOP;
//        linPtrFrame->data[1] = adress;
//
//        LINFlush(mLinModule, LIN_TRANSMISSION_REQUEST);
//        return 0;
//    }
//    return 1;
//}
//
///*******************************************************************************
//  Function:
//    BOOL eAMIS30621SoftStopAndResetPosition(LIN_MODULE mLinModule, BYTE adress);
//
//  Description:
//    cf. softStop function and resetPosition function.
//
//  Parameters:
//    mLinModule      - The LIN module which will be used.
//
//    adress          - Adress of the AMIS30621.
//
//  Returns:
//    0x00            - Function over.
//    0x01            - Function in progress.
//
//  Example:
//    <code>
//    eAMIS30621SoftStopAndResetPosition(LIN2, 0xF0);
//    </code>
//  *****************************************************************************/
//BOOL eAMIS30621SoftStopAndResetPosition(LIN_MODULE mLinModule, BYTE adress)
//{
//    static state_machine_t smInit = {0};
//
//    if(LINGetStatusBitsAdress(mLinModule)->readyForNextTransmission)
//    {
//        switch(smInit.index)
//        {
//            case 0:
//                if(!eAMIS30621SoftStop(mLinModule, adress))
//                {
//                    smInit.index++;
//                }
//                break;
//            case 1:
//                if(!eAMIS30621ResetPosition(mLinModule, adress))
//                {
//                    smInit.index++;
//                }
//                break;
//            default:
//                smInit.index = 0;
//                return 0;
//                break;
//        }
//    }
//    return 1;
//}
//
///*******************************************************************************
//  Function:
//    BOOL eAMIS30621ChangeAdress(LIN_MODULE mLinModule, BYTE currentAdress, BYTE newAdress);
//
//  Description:
//    This routine allow the user to modify the adress of an AMIS30621 by modifying the OTP memory.
//
//    IMPORTANT: THIS COMMAND MUST BE SENT UNDER A SPECIFIC VBB VOLTAGE VALUE (min 9.0V / max 10.0v).
//    Once an adress has been changed, it is impossible to go back.
//
//  Parameters:
//    mLinModule      - The LIN module which will be used.
//
//    currentAdress   - Current adress of the AMIS30621.
//
//    newAdress       - New adress for the AMIS30621.
//
//  Returns:
//    0x00            - Adress changed.
//    0x01            - Adress cannot be changed.
//
//  Example:
//    <code>
//     eAMIS30621ChangeAdress(LIN2, 0xF0, 0xF7);
//    </code>
//  *****************************************************************************/
//BOOL eAMIS30621ChangeAdress(LIN_MODULE mLinModule, BYTE currentAdress, BYTE newAdress)
//{
//    _GET_OTP_PARAM getOtpParam;
//    _SET_OTP_PARAM setOtpParam;
//    BYTE getOtpParamByte;
//
//    setOtpParam.otpMemoryAdress = 0x02;
//    setOtpParam.data = 0x00;
//
//    while(eAMIS30621GetOTPParam(mLinModule, currentAdress, &getOtpParam) != 0);
//    // ADM HW2 HW1 HW0 PA3 PA2 PA1 PA0
//    getOtpParamByte = (BYTE) ((getOtpParam.adm << 7) | (getOtpParam.hw << 4) | (getOtpParam.pa << 0));
//
//    if(getOtpParam.adm) // Adress is defined as 1 PA0 HW0 HW1 HW2 PA3 PA2 PA1
//    {
//        setOtpParam.data = 0x80 | ((newAdress << 1) & 0x0E) | ((newAdress >> 6) & 0x01) | ((newAdress << 3) & 0x40) | ((newAdress << 1) & 0x20) | ((newAdress >> 1) & 0x10);
//    }
//    else                // Adress is defined as 1 HW0 HW1 HW2 PA3 PA2 PA1 PA0
//    {
//        setOtpParam.data = (newAdress & 0x0F) | ((newAdress << 2) & 0x40) | ((newAdress >> 2) & 0x10);
//    }
//    
//    if((setOtpParam.data & (getOtpParamByte | 0x0F)) > getOtpParamByte)
//    {
//        eAMIS30621SetOTPParam(mLinModule, currentAdress, setOtpParam);
//        while(eAMIS30621GetOTPParam(mLinModule, newAdress, &getOtpParam) != 0);
//        return 0;
//    }
//    else
//    {
//        return 1;
//    }
//}
//
///*******************************************************************************
//  Function:
//    void eAMIS30621SetDefaultParam(_SET_MOTOR_PARAM *setMotorParam);
//
//  Description:
//    This routine set the motor param variable to default.
//
//  Parameters:
//    setMotorParam   - The variable which contain all parameters for the stepper.
//
//  Returns:
//
//  Example:
//    <code>
//    _SET_MOTOR_PARAM setMotorParam;
//    eAMIS30621SetDefaultMotorParam(&setMotorParam);
//    </code>
//  *****************************************************************************/
//void eAMIS30621SetDefaultMotorParam(_SET_MOTOR_PARAM *setMotorParam)
//{
//    setMotorParam->param.iParamBits.iHold = I_HOLD_DEFAULT;
//    setMotorParam->param.iParamBits.iRun = I_RUN_DEFAULT;
//    setMotorParam->param.vParamBits.vMin = V_MIN_DEFAULT;
//    setMotorParam->param.vParamBits.vMax = V_MAX_DEFAULT;
//    setMotorParam->param.acc = ACC_DEFAULT;
//    setMotorParam->param.shaft = SHAFT_ON;
//    setMotorParam->param.stepMode = STEP_MODE_1_2;
//    setMotorParam->param.accShaft = ACC_SHAFT_OFF;
//    setMotorParam->secPos = 0;
//}
//
///*******************************************************************************
//  Function:
//    void eAMIS30621SetDefaultDualPositioning(_SET_DUAL_POSITIONING_PARAM *setDualPositioning);
//
//  Description:
//    This routine set the dual positioning variable to default.
//
//  Parameters:
//    setDualPositioning   - The variable which contain all parameters for the stepper.
//
//  Returns:
//
//  Example:
//    <code>
//    _SET_DUAL_POSITIONING_PARAM setDualPositioning;
//    eAMIS30621SetDefaultDualPositioning(&setDualPositioning);
//    </code>
//  *****************************************************************************/
//void eAMIS30621SetDefaultDualPositioning(_SET_DUAL_POSITIONING_PARAM *setDualPositioning)
//{
//    setDualPositioning->vMin = V_MIN_DEFAULT;
//    setDualPositioning->vMax = V_MAX_DEFAULT;
//    setDualPositioning->pos1 = POS1_DEFAULT;
//    setDualPositioning->pos2 = POS2_DEFAULT;
//}
