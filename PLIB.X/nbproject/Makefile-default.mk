#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-default.mk)" "nbproject/Makefile-local-default.mk"
include nbproject/Makefile-local-default.mk
endif
endif

# Environment
MKDIR=mkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=a
DEBUGGABLE_SUFFIX=
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/PLIB.X.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=a
DEBUGGABLE_SUFFIX=
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/PLIB.X.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=
else
COMPARISON_BUILD=
endif

ifdef SUB_IMAGE_ADDRESS

else
SUB_IMAGE_ADDRESS_COMMAND=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=../_Experimental/_EXAMPLES_.c ../_Experimental/_LOG.c ../_External_Components/e_mcp23s17.c ../_External_Components/e_ws2812b.c ../_External_Components/e_amis30621.c ../_External_Components/e_tmc429.c ../_External_Components/e_25lc512.c ../_External_Components/e_at42qt2120.c ../_High_Level_Driver/lin.c ../_High_Level_Driver/ble.c ../_High_Level_Driver/one_wire_communication.c ../_High_Level_Driver/utilities.c ../_High_Level_Driver/string_advance.c ../_Low_Level_Driver/s14_timers.c ../_Low_Level_Driver/s08_interrupt_mapping.c ../_Low_Level_Driver/s23_spi.c ../_Low_Level_Driver/s17_adc.c ../_Low_Level_Driver/s16_output_compare.c ../_Low_Level_Driver/s24_i2c.c ../_Low_Level_Driver/s34_can.c ../_Low_Level_Driver/s35_ethernet_Applications.c ../_Low_Level_Driver/s35_ethernet_OSI-2_DataLinkLayer.c ../_Low_Level_Driver/s35_ethernet_OSI-3_NetworkLayer.c ../_Low_Level_Driver/s35_ethernet_OSI-4_TransportLayer.c ../_Low_Level_Driver/s35_ethernet_OSI-5_ApplicationLayer.c ../_Low_Level_Driver/s35_ethernet_TCPIP.c ../_Low_Level_Driver/s12_ports.c ../_Low_Level_Driver/s21_uart.c ../_External_Components/e_pca9685.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/1717005096/_EXAMPLES_.o ${OBJECTDIR}/_ext/1717005096/_LOG.o ${OBJECTDIR}/_ext/830869050/e_mcp23s17.o ${OBJECTDIR}/_ext/830869050/e_ws2812b.o ${OBJECTDIR}/_ext/830869050/e_amis30621.o ${OBJECTDIR}/_ext/830869050/e_tmc429.o ${OBJECTDIR}/_ext/830869050/e_25lc512.o ${OBJECTDIR}/_ext/830869050/e_at42qt2120.o ${OBJECTDIR}/_ext/1180237584/lin.o ${OBJECTDIR}/_ext/1180237584/ble.o ${OBJECTDIR}/_ext/1180237584/one_wire_communication.o ${OBJECTDIR}/_ext/1180237584/utilities.o ${OBJECTDIR}/_ext/1180237584/string_advance.o ${OBJECTDIR}/_ext/376376446/s14_timers.o ${OBJECTDIR}/_ext/376376446/s08_interrupt_mapping.o ${OBJECTDIR}/_ext/376376446/s23_spi.o ${OBJECTDIR}/_ext/376376446/s17_adc.o ${OBJECTDIR}/_ext/376376446/s16_output_compare.o ${OBJECTDIR}/_ext/376376446/s24_i2c.o ${OBJECTDIR}/_ext/376376446/s34_can.o ${OBJECTDIR}/_ext/376376446/s35_ethernet_Applications.o ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-2_DataLinkLayer.o ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-3_NetworkLayer.o ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-4_TransportLayer.o ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-5_ApplicationLayer.o ${OBJECTDIR}/_ext/376376446/s35_ethernet_TCPIP.o ${OBJECTDIR}/_ext/376376446/s12_ports.o ${OBJECTDIR}/_ext/376376446/s21_uart.o ${OBJECTDIR}/_ext/830869050/e_pca9685.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/1717005096/_EXAMPLES_.o.d ${OBJECTDIR}/_ext/1717005096/_LOG.o.d ${OBJECTDIR}/_ext/830869050/e_mcp23s17.o.d ${OBJECTDIR}/_ext/830869050/e_ws2812b.o.d ${OBJECTDIR}/_ext/830869050/e_amis30621.o.d ${OBJECTDIR}/_ext/830869050/e_tmc429.o.d ${OBJECTDIR}/_ext/830869050/e_25lc512.o.d ${OBJECTDIR}/_ext/830869050/e_at42qt2120.o.d ${OBJECTDIR}/_ext/1180237584/lin.o.d ${OBJECTDIR}/_ext/1180237584/ble.o.d ${OBJECTDIR}/_ext/1180237584/one_wire_communication.o.d ${OBJECTDIR}/_ext/1180237584/utilities.o.d ${OBJECTDIR}/_ext/1180237584/string_advance.o.d ${OBJECTDIR}/_ext/376376446/s14_timers.o.d ${OBJECTDIR}/_ext/376376446/s08_interrupt_mapping.o.d ${OBJECTDIR}/_ext/376376446/s23_spi.o.d ${OBJECTDIR}/_ext/376376446/s17_adc.o.d ${OBJECTDIR}/_ext/376376446/s16_output_compare.o.d ${OBJECTDIR}/_ext/376376446/s24_i2c.o.d ${OBJECTDIR}/_ext/376376446/s34_can.o.d ${OBJECTDIR}/_ext/376376446/s35_ethernet_Applications.o.d ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-2_DataLinkLayer.o.d ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-3_NetworkLayer.o.d ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-4_TransportLayer.o.d ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-5_ApplicationLayer.o.d ${OBJECTDIR}/_ext/376376446/s35_ethernet_TCPIP.o.d ${OBJECTDIR}/_ext/376376446/s12_ports.o.d ${OBJECTDIR}/_ext/376376446/s21_uart.o.d ${OBJECTDIR}/_ext/830869050/e_pca9685.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/1717005096/_EXAMPLES_.o ${OBJECTDIR}/_ext/1717005096/_LOG.o ${OBJECTDIR}/_ext/830869050/e_mcp23s17.o ${OBJECTDIR}/_ext/830869050/e_ws2812b.o ${OBJECTDIR}/_ext/830869050/e_amis30621.o ${OBJECTDIR}/_ext/830869050/e_tmc429.o ${OBJECTDIR}/_ext/830869050/e_25lc512.o ${OBJECTDIR}/_ext/830869050/e_at42qt2120.o ${OBJECTDIR}/_ext/1180237584/lin.o ${OBJECTDIR}/_ext/1180237584/ble.o ${OBJECTDIR}/_ext/1180237584/one_wire_communication.o ${OBJECTDIR}/_ext/1180237584/utilities.o ${OBJECTDIR}/_ext/1180237584/string_advance.o ${OBJECTDIR}/_ext/376376446/s14_timers.o ${OBJECTDIR}/_ext/376376446/s08_interrupt_mapping.o ${OBJECTDIR}/_ext/376376446/s23_spi.o ${OBJECTDIR}/_ext/376376446/s17_adc.o ${OBJECTDIR}/_ext/376376446/s16_output_compare.o ${OBJECTDIR}/_ext/376376446/s24_i2c.o ${OBJECTDIR}/_ext/376376446/s34_can.o ${OBJECTDIR}/_ext/376376446/s35_ethernet_Applications.o ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-2_DataLinkLayer.o ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-3_NetworkLayer.o ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-4_TransportLayer.o ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-5_ApplicationLayer.o ${OBJECTDIR}/_ext/376376446/s35_ethernet_TCPIP.o ${OBJECTDIR}/_ext/376376446/s12_ports.o ${OBJECTDIR}/_ext/376376446/s21_uart.o ${OBJECTDIR}/_ext/830869050/e_pca9685.o

# Source Files
SOURCEFILES=../_Experimental/_EXAMPLES_.c ../_Experimental/_LOG.c ../_External_Components/e_mcp23s17.c ../_External_Components/e_ws2812b.c ../_External_Components/e_amis30621.c ../_External_Components/e_tmc429.c ../_External_Components/e_25lc512.c ../_External_Components/e_at42qt2120.c ../_High_Level_Driver/lin.c ../_High_Level_Driver/ble.c ../_High_Level_Driver/one_wire_communication.c ../_High_Level_Driver/utilities.c ../_High_Level_Driver/string_advance.c ../_Low_Level_Driver/s14_timers.c ../_Low_Level_Driver/s08_interrupt_mapping.c ../_Low_Level_Driver/s23_spi.c ../_Low_Level_Driver/s17_adc.c ../_Low_Level_Driver/s16_output_compare.c ../_Low_Level_Driver/s24_i2c.c ../_Low_Level_Driver/s34_can.c ../_Low_Level_Driver/s35_ethernet_Applications.c ../_Low_Level_Driver/s35_ethernet_OSI-2_DataLinkLayer.c ../_Low_Level_Driver/s35_ethernet_OSI-3_NetworkLayer.c ../_Low_Level_Driver/s35_ethernet_OSI-4_TransportLayer.c ../_Low_Level_Driver/s35_ethernet_OSI-5_ApplicationLayer.c ../_Low_Level_Driver/s35_ethernet_TCPIP.c ../_Low_Level_Driver/s12_ports.c ../_Low_Level_Driver/s21_uart.c ../_External_Components/e_pca9685.c


CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-default.mk dist/${CND_CONF}/${IMAGE_TYPE}/PLIB.X.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=32MX795F512L
MP_LINKER_FILE_OPTION=
# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assembleWithPreprocess
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/1717005096/_EXAMPLES_.o: ../_Experimental/_EXAMPLES_.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1717005096" 
	@${RM} ${OBJECTDIR}/_ext/1717005096/_EXAMPLES_.o.d 
	@${RM} ${OBJECTDIR}/_ext/1717005096/_EXAMPLES_.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1717005096/_EXAMPLES_.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/1717005096/_EXAMPLES_.o.d" -o ${OBJECTDIR}/_ext/1717005096/_EXAMPLES_.o ../_Experimental/_EXAMPLES_.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/1717005096/_LOG.o: ../_Experimental/_LOG.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1717005096" 
	@${RM} ${OBJECTDIR}/_ext/1717005096/_LOG.o.d 
	@${RM} ${OBJECTDIR}/_ext/1717005096/_LOG.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1717005096/_LOG.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/1717005096/_LOG.o.d" -o ${OBJECTDIR}/_ext/1717005096/_LOG.o ../_Experimental/_LOG.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/830869050/e_mcp23s17.o: ../_External_Components/e_mcp23s17.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/830869050" 
	@${RM} ${OBJECTDIR}/_ext/830869050/e_mcp23s17.o.d 
	@${RM} ${OBJECTDIR}/_ext/830869050/e_mcp23s17.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/830869050/e_mcp23s17.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/830869050/e_mcp23s17.o.d" -o ${OBJECTDIR}/_ext/830869050/e_mcp23s17.o ../_External_Components/e_mcp23s17.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/830869050/e_ws2812b.o: ../_External_Components/e_ws2812b.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/830869050" 
	@${RM} ${OBJECTDIR}/_ext/830869050/e_ws2812b.o.d 
	@${RM} ${OBJECTDIR}/_ext/830869050/e_ws2812b.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/830869050/e_ws2812b.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/830869050/e_ws2812b.o.d" -o ${OBJECTDIR}/_ext/830869050/e_ws2812b.o ../_External_Components/e_ws2812b.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/830869050/e_amis30621.o: ../_External_Components/e_amis30621.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/830869050" 
	@${RM} ${OBJECTDIR}/_ext/830869050/e_amis30621.o.d 
	@${RM} ${OBJECTDIR}/_ext/830869050/e_amis30621.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/830869050/e_amis30621.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/830869050/e_amis30621.o.d" -o ${OBJECTDIR}/_ext/830869050/e_amis30621.o ../_External_Components/e_amis30621.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/830869050/e_tmc429.o: ../_External_Components/e_tmc429.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/830869050" 
	@${RM} ${OBJECTDIR}/_ext/830869050/e_tmc429.o.d 
	@${RM} ${OBJECTDIR}/_ext/830869050/e_tmc429.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/830869050/e_tmc429.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/830869050/e_tmc429.o.d" -o ${OBJECTDIR}/_ext/830869050/e_tmc429.o ../_External_Components/e_tmc429.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/830869050/e_25lc512.o: ../_External_Components/e_25lc512.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/830869050" 
	@${RM} ${OBJECTDIR}/_ext/830869050/e_25lc512.o.d 
	@${RM} ${OBJECTDIR}/_ext/830869050/e_25lc512.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/830869050/e_25lc512.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/830869050/e_25lc512.o.d" -o ${OBJECTDIR}/_ext/830869050/e_25lc512.o ../_External_Components/e_25lc512.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/830869050/e_at42qt2120.o: ../_External_Components/e_at42qt2120.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/830869050" 
	@${RM} ${OBJECTDIR}/_ext/830869050/e_at42qt2120.o.d 
	@${RM} ${OBJECTDIR}/_ext/830869050/e_at42qt2120.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/830869050/e_at42qt2120.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/830869050/e_at42qt2120.o.d" -o ${OBJECTDIR}/_ext/830869050/e_at42qt2120.o ../_External_Components/e_at42qt2120.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/1180237584/lin.o: ../_High_Level_Driver/lin.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1180237584" 
	@${RM} ${OBJECTDIR}/_ext/1180237584/lin.o.d 
	@${RM} ${OBJECTDIR}/_ext/1180237584/lin.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1180237584/lin.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/1180237584/lin.o.d" -o ${OBJECTDIR}/_ext/1180237584/lin.o ../_High_Level_Driver/lin.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/1180237584/ble.o: ../_High_Level_Driver/ble.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1180237584" 
	@${RM} ${OBJECTDIR}/_ext/1180237584/ble.o.d 
	@${RM} ${OBJECTDIR}/_ext/1180237584/ble.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1180237584/ble.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/1180237584/ble.o.d" -o ${OBJECTDIR}/_ext/1180237584/ble.o ../_High_Level_Driver/ble.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/1180237584/one_wire_communication.o: ../_High_Level_Driver/one_wire_communication.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1180237584" 
	@${RM} ${OBJECTDIR}/_ext/1180237584/one_wire_communication.o.d 
	@${RM} ${OBJECTDIR}/_ext/1180237584/one_wire_communication.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1180237584/one_wire_communication.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/1180237584/one_wire_communication.o.d" -o ${OBJECTDIR}/_ext/1180237584/one_wire_communication.o ../_High_Level_Driver/one_wire_communication.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/1180237584/utilities.o: ../_High_Level_Driver/utilities.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1180237584" 
	@${RM} ${OBJECTDIR}/_ext/1180237584/utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/1180237584/utilities.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1180237584/utilities.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/1180237584/utilities.o.d" -o ${OBJECTDIR}/_ext/1180237584/utilities.o ../_High_Level_Driver/utilities.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/1180237584/string_advance.o: ../_High_Level_Driver/string_advance.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1180237584" 
	@${RM} ${OBJECTDIR}/_ext/1180237584/string_advance.o.d 
	@${RM} ${OBJECTDIR}/_ext/1180237584/string_advance.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1180237584/string_advance.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/1180237584/string_advance.o.d" -o ${OBJECTDIR}/_ext/1180237584/string_advance.o ../_High_Level_Driver/string_advance.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/376376446/s14_timers.o: ../_Low_Level_Driver/s14_timers.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/376376446" 
	@${RM} ${OBJECTDIR}/_ext/376376446/s14_timers.o.d 
	@${RM} ${OBJECTDIR}/_ext/376376446/s14_timers.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/376376446/s14_timers.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/376376446/s14_timers.o.d" -o ${OBJECTDIR}/_ext/376376446/s14_timers.o ../_Low_Level_Driver/s14_timers.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/376376446/s08_interrupt_mapping.o: ../_Low_Level_Driver/s08_interrupt_mapping.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/376376446" 
	@${RM} ${OBJECTDIR}/_ext/376376446/s08_interrupt_mapping.o.d 
	@${RM} ${OBJECTDIR}/_ext/376376446/s08_interrupt_mapping.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/376376446/s08_interrupt_mapping.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/376376446/s08_interrupt_mapping.o.d" -o ${OBJECTDIR}/_ext/376376446/s08_interrupt_mapping.o ../_Low_Level_Driver/s08_interrupt_mapping.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/376376446/s23_spi.o: ../_Low_Level_Driver/s23_spi.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/376376446" 
	@${RM} ${OBJECTDIR}/_ext/376376446/s23_spi.o.d 
	@${RM} ${OBJECTDIR}/_ext/376376446/s23_spi.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/376376446/s23_spi.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/376376446/s23_spi.o.d" -o ${OBJECTDIR}/_ext/376376446/s23_spi.o ../_Low_Level_Driver/s23_spi.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/376376446/s17_adc.o: ../_Low_Level_Driver/s17_adc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/376376446" 
	@${RM} ${OBJECTDIR}/_ext/376376446/s17_adc.o.d 
	@${RM} ${OBJECTDIR}/_ext/376376446/s17_adc.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/376376446/s17_adc.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/376376446/s17_adc.o.d" -o ${OBJECTDIR}/_ext/376376446/s17_adc.o ../_Low_Level_Driver/s17_adc.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/376376446/s16_output_compare.o: ../_Low_Level_Driver/s16_output_compare.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/376376446" 
	@${RM} ${OBJECTDIR}/_ext/376376446/s16_output_compare.o.d 
	@${RM} ${OBJECTDIR}/_ext/376376446/s16_output_compare.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/376376446/s16_output_compare.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/376376446/s16_output_compare.o.d" -o ${OBJECTDIR}/_ext/376376446/s16_output_compare.o ../_Low_Level_Driver/s16_output_compare.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/376376446/s24_i2c.o: ../_Low_Level_Driver/s24_i2c.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/376376446" 
	@${RM} ${OBJECTDIR}/_ext/376376446/s24_i2c.o.d 
	@${RM} ${OBJECTDIR}/_ext/376376446/s24_i2c.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/376376446/s24_i2c.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/376376446/s24_i2c.o.d" -o ${OBJECTDIR}/_ext/376376446/s24_i2c.o ../_Low_Level_Driver/s24_i2c.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/376376446/s34_can.o: ../_Low_Level_Driver/s34_can.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/376376446" 
	@${RM} ${OBJECTDIR}/_ext/376376446/s34_can.o.d 
	@${RM} ${OBJECTDIR}/_ext/376376446/s34_can.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/376376446/s34_can.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/376376446/s34_can.o.d" -o ${OBJECTDIR}/_ext/376376446/s34_can.o ../_Low_Level_Driver/s34_can.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/376376446/s35_ethernet_Applications.o: ../_Low_Level_Driver/s35_ethernet_Applications.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/376376446" 
	@${RM} ${OBJECTDIR}/_ext/376376446/s35_ethernet_Applications.o.d 
	@${RM} ${OBJECTDIR}/_ext/376376446/s35_ethernet_Applications.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/376376446/s35_ethernet_Applications.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/376376446/s35_ethernet_Applications.o.d" -o ${OBJECTDIR}/_ext/376376446/s35_ethernet_Applications.o ../_Low_Level_Driver/s35_ethernet_Applications.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-2_DataLinkLayer.o: ../_Low_Level_Driver/s35_ethernet_OSI-2_DataLinkLayer.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/376376446" 
	@${RM} ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-2_DataLinkLayer.o.d 
	@${RM} ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-2_DataLinkLayer.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-2_DataLinkLayer.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-2_DataLinkLayer.o.d" -o ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-2_DataLinkLayer.o ../_Low_Level_Driver/s35_ethernet_OSI-2_DataLinkLayer.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-3_NetworkLayer.o: ../_Low_Level_Driver/s35_ethernet_OSI-3_NetworkLayer.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/376376446" 
	@${RM} ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-3_NetworkLayer.o.d 
	@${RM} ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-3_NetworkLayer.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-3_NetworkLayer.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-3_NetworkLayer.o.d" -o ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-3_NetworkLayer.o ../_Low_Level_Driver/s35_ethernet_OSI-3_NetworkLayer.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-4_TransportLayer.o: ../_Low_Level_Driver/s35_ethernet_OSI-4_TransportLayer.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/376376446" 
	@${RM} ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-4_TransportLayer.o.d 
	@${RM} ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-4_TransportLayer.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-4_TransportLayer.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-4_TransportLayer.o.d" -o ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-4_TransportLayer.o ../_Low_Level_Driver/s35_ethernet_OSI-4_TransportLayer.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-5_ApplicationLayer.o: ../_Low_Level_Driver/s35_ethernet_OSI-5_ApplicationLayer.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/376376446" 
	@${RM} ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-5_ApplicationLayer.o.d 
	@${RM} ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-5_ApplicationLayer.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-5_ApplicationLayer.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-5_ApplicationLayer.o.d" -o ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-5_ApplicationLayer.o ../_Low_Level_Driver/s35_ethernet_OSI-5_ApplicationLayer.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/376376446/s35_ethernet_TCPIP.o: ../_Low_Level_Driver/s35_ethernet_TCPIP.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/376376446" 
	@${RM} ${OBJECTDIR}/_ext/376376446/s35_ethernet_TCPIP.o.d 
	@${RM} ${OBJECTDIR}/_ext/376376446/s35_ethernet_TCPIP.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/376376446/s35_ethernet_TCPIP.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/376376446/s35_ethernet_TCPIP.o.d" -o ${OBJECTDIR}/_ext/376376446/s35_ethernet_TCPIP.o ../_Low_Level_Driver/s35_ethernet_TCPIP.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/376376446/s12_ports.o: ../_Low_Level_Driver/s12_ports.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/376376446" 
	@${RM} ${OBJECTDIR}/_ext/376376446/s12_ports.o.d 
	@${RM} ${OBJECTDIR}/_ext/376376446/s12_ports.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/376376446/s12_ports.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/376376446/s12_ports.o.d" -o ${OBJECTDIR}/_ext/376376446/s12_ports.o ../_Low_Level_Driver/s12_ports.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/376376446/s21_uart.o: ../_Low_Level_Driver/s21_uart.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/376376446" 
	@${RM} ${OBJECTDIR}/_ext/376376446/s21_uart.o.d 
	@${RM} ${OBJECTDIR}/_ext/376376446/s21_uart.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/376376446/s21_uart.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/376376446/s21_uart.o.d" -o ${OBJECTDIR}/_ext/376376446/s21_uart.o ../_Low_Level_Driver/s21_uart.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/830869050/e_pca9685.o: ../_External_Components/e_pca9685.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/830869050" 
	@${RM} ${OBJECTDIR}/_ext/830869050/e_pca9685.o.d 
	@${RM} ${OBJECTDIR}/_ext/830869050/e_pca9685.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/830869050/e_pca9685.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD4=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/830869050/e_pca9685.o.d" -o ${OBJECTDIR}/_ext/830869050/e_pca9685.o ../_External_Components/e_pca9685.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
else
${OBJECTDIR}/_ext/1717005096/_EXAMPLES_.o: ../_Experimental/_EXAMPLES_.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1717005096" 
	@${RM} ${OBJECTDIR}/_ext/1717005096/_EXAMPLES_.o.d 
	@${RM} ${OBJECTDIR}/_ext/1717005096/_EXAMPLES_.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1717005096/_EXAMPLES_.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/1717005096/_EXAMPLES_.o.d" -o ${OBJECTDIR}/_ext/1717005096/_EXAMPLES_.o ../_Experimental/_EXAMPLES_.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/1717005096/_LOG.o: ../_Experimental/_LOG.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1717005096" 
	@${RM} ${OBJECTDIR}/_ext/1717005096/_LOG.o.d 
	@${RM} ${OBJECTDIR}/_ext/1717005096/_LOG.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1717005096/_LOG.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/1717005096/_LOG.o.d" -o ${OBJECTDIR}/_ext/1717005096/_LOG.o ../_Experimental/_LOG.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/830869050/e_mcp23s17.o: ../_External_Components/e_mcp23s17.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/830869050" 
	@${RM} ${OBJECTDIR}/_ext/830869050/e_mcp23s17.o.d 
	@${RM} ${OBJECTDIR}/_ext/830869050/e_mcp23s17.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/830869050/e_mcp23s17.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/830869050/e_mcp23s17.o.d" -o ${OBJECTDIR}/_ext/830869050/e_mcp23s17.o ../_External_Components/e_mcp23s17.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/830869050/e_ws2812b.o: ../_External_Components/e_ws2812b.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/830869050" 
	@${RM} ${OBJECTDIR}/_ext/830869050/e_ws2812b.o.d 
	@${RM} ${OBJECTDIR}/_ext/830869050/e_ws2812b.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/830869050/e_ws2812b.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/830869050/e_ws2812b.o.d" -o ${OBJECTDIR}/_ext/830869050/e_ws2812b.o ../_External_Components/e_ws2812b.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/830869050/e_amis30621.o: ../_External_Components/e_amis30621.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/830869050" 
	@${RM} ${OBJECTDIR}/_ext/830869050/e_amis30621.o.d 
	@${RM} ${OBJECTDIR}/_ext/830869050/e_amis30621.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/830869050/e_amis30621.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/830869050/e_amis30621.o.d" -o ${OBJECTDIR}/_ext/830869050/e_amis30621.o ../_External_Components/e_amis30621.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/830869050/e_tmc429.o: ../_External_Components/e_tmc429.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/830869050" 
	@${RM} ${OBJECTDIR}/_ext/830869050/e_tmc429.o.d 
	@${RM} ${OBJECTDIR}/_ext/830869050/e_tmc429.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/830869050/e_tmc429.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/830869050/e_tmc429.o.d" -o ${OBJECTDIR}/_ext/830869050/e_tmc429.o ../_External_Components/e_tmc429.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/830869050/e_25lc512.o: ../_External_Components/e_25lc512.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/830869050" 
	@${RM} ${OBJECTDIR}/_ext/830869050/e_25lc512.o.d 
	@${RM} ${OBJECTDIR}/_ext/830869050/e_25lc512.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/830869050/e_25lc512.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/830869050/e_25lc512.o.d" -o ${OBJECTDIR}/_ext/830869050/e_25lc512.o ../_External_Components/e_25lc512.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/830869050/e_at42qt2120.o: ../_External_Components/e_at42qt2120.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/830869050" 
	@${RM} ${OBJECTDIR}/_ext/830869050/e_at42qt2120.o.d 
	@${RM} ${OBJECTDIR}/_ext/830869050/e_at42qt2120.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/830869050/e_at42qt2120.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/830869050/e_at42qt2120.o.d" -o ${OBJECTDIR}/_ext/830869050/e_at42qt2120.o ../_External_Components/e_at42qt2120.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/1180237584/lin.o: ../_High_Level_Driver/lin.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1180237584" 
	@${RM} ${OBJECTDIR}/_ext/1180237584/lin.o.d 
	@${RM} ${OBJECTDIR}/_ext/1180237584/lin.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1180237584/lin.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/1180237584/lin.o.d" -o ${OBJECTDIR}/_ext/1180237584/lin.o ../_High_Level_Driver/lin.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/1180237584/ble.o: ../_High_Level_Driver/ble.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1180237584" 
	@${RM} ${OBJECTDIR}/_ext/1180237584/ble.o.d 
	@${RM} ${OBJECTDIR}/_ext/1180237584/ble.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1180237584/ble.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/1180237584/ble.o.d" -o ${OBJECTDIR}/_ext/1180237584/ble.o ../_High_Level_Driver/ble.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/1180237584/one_wire_communication.o: ../_High_Level_Driver/one_wire_communication.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1180237584" 
	@${RM} ${OBJECTDIR}/_ext/1180237584/one_wire_communication.o.d 
	@${RM} ${OBJECTDIR}/_ext/1180237584/one_wire_communication.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1180237584/one_wire_communication.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/1180237584/one_wire_communication.o.d" -o ${OBJECTDIR}/_ext/1180237584/one_wire_communication.o ../_High_Level_Driver/one_wire_communication.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/1180237584/utilities.o: ../_High_Level_Driver/utilities.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1180237584" 
	@${RM} ${OBJECTDIR}/_ext/1180237584/utilities.o.d 
	@${RM} ${OBJECTDIR}/_ext/1180237584/utilities.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1180237584/utilities.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/1180237584/utilities.o.d" -o ${OBJECTDIR}/_ext/1180237584/utilities.o ../_High_Level_Driver/utilities.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/1180237584/string_advance.o: ../_High_Level_Driver/string_advance.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/1180237584" 
	@${RM} ${OBJECTDIR}/_ext/1180237584/string_advance.o.d 
	@${RM} ${OBJECTDIR}/_ext/1180237584/string_advance.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1180237584/string_advance.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/1180237584/string_advance.o.d" -o ${OBJECTDIR}/_ext/1180237584/string_advance.o ../_High_Level_Driver/string_advance.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/376376446/s14_timers.o: ../_Low_Level_Driver/s14_timers.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/376376446" 
	@${RM} ${OBJECTDIR}/_ext/376376446/s14_timers.o.d 
	@${RM} ${OBJECTDIR}/_ext/376376446/s14_timers.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/376376446/s14_timers.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/376376446/s14_timers.o.d" -o ${OBJECTDIR}/_ext/376376446/s14_timers.o ../_Low_Level_Driver/s14_timers.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/376376446/s08_interrupt_mapping.o: ../_Low_Level_Driver/s08_interrupt_mapping.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/376376446" 
	@${RM} ${OBJECTDIR}/_ext/376376446/s08_interrupt_mapping.o.d 
	@${RM} ${OBJECTDIR}/_ext/376376446/s08_interrupt_mapping.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/376376446/s08_interrupt_mapping.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/376376446/s08_interrupt_mapping.o.d" -o ${OBJECTDIR}/_ext/376376446/s08_interrupt_mapping.o ../_Low_Level_Driver/s08_interrupt_mapping.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/376376446/s23_spi.o: ../_Low_Level_Driver/s23_spi.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/376376446" 
	@${RM} ${OBJECTDIR}/_ext/376376446/s23_spi.o.d 
	@${RM} ${OBJECTDIR}/_ext/376376446/s23_spi.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/376376446/s23_spi.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/376376446/s23_spi.o.d" -o ${OBJECTDIR}/_ext/376376446/s23_spi.o ../_Low_Level_Driver/s23_spi.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/376376446/s17_adc.o: ../_Low_Level_Driver/s17_adc.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/376376446" 
	@${RM} ${OBJECTDIR}/_ext/376376446/s17_adc.o.d 
	@${RM} ${OBJECTDIR}/_ext/376376446/s17_adc.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/376376446/s17_adc.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/376376446/s17_adc.o.d" -o ${OBJECTDIR}/_ext/376376446/s17_adc.o ../_Low_Level_Driver/s17_adc.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/376376446/s16_output_compare.o: ../_Low_Level_Driver/s16_output_compare.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/376376446" 
	@${RM} ${OBJECTDIR}/_ext/376376446/s16_output_compare.o.d 
	@${RM} ${OBJECTDIR}/_ext/376376446/s16_output_compare.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/376376446/s16_output_compare.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/376376446/s16_output_compare.o.d" -o ${OBJECTDIR}/_ext/376376446/s16_output_compare.o ../_Low_Level_Driver/s16_output_compare.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/376376446/s24_i2c.o: ../_Low_Level_Driver/s24_i2c.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/376376446" 
	@${RM} ${OBJECTDIR}/_ext/376376446/s24_i2c.o.d 
	@${RM} ${OBJECTDIR}/_ext/376376446/s24_i2c.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/376376446/s24_i2c.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/376376446/s24_i2c.o.d" -o ${OBJECTDIR}/_ext/376376446/s24_i2c.o ../_Low_Level_Driver/s24_i2c.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/376376446/s34_can.o: ../_Low_Level_Driver/s34_can.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/376376446" 
	@${RM} ${OBJECTDIR}/_ext/376376446/s34_can.o.d 
	@${RM} ${OBJECTDIR}/_ext/376376446/s34_can.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/376376446/s34_can.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/376376446/s34_can.o.d" -o ${OBJECTDIR}/_ext/376376446/s34_can.o ../_Low_Level_Driver/s34_can.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/376376446/s35_ethernet_Applications.o: ../_Low_Level_Driver/s35_ethernet_Applications.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/376376446" 
	@${RM} ${OBJECTDIR}/_ext/376376446/s35_ethernet_Applications.o.d 
	@${RM} ${OBJECTDIR}/_ext/376376446/s35_ethernet_Applications.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/376376446/s35_ethernet_Applications.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/376376446/s35_ethernet_Applications.o.d" -o ${OBJECTDIR}/_ext/376376446/s35_ethernet_Applications.o ../_Low_Level_Driver/s35_ethernet_Applications.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-2_DataLinkLayer.o: ../_Low_Level_Driver/s35_ethernet_OSI-2_DataLinkLayer.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/376376446" 
	@${RM} ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-2_DataLinkLayer.o.d 
	@${RM} ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-2_DataLinkLayer.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-2_DataLinkLayer.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-2_DataLinkLayer.o.d" -o ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-2_DataLinkLayer.o ../_Low_Level_Driver/s35_ethernet_OSI-2_DataLinkLayer.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-3_NetworkLayer.o: ../_Low_Level_Driver/s35_ethernet_OSI-3_NetworkLayer.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/376376446" 
	@${RM} ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-3_NetworkLayer.o.d 
	@${RM} ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-3_NetworkLayer.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-3_NetworkLayer.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-3_NetworkLayer.o.d" -o ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-3_NetworkLayer.o ../_Low_Level_Driver/s35_ethernet_OSI-3_NetworkLayer.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-4_TransportLayer.o: ../_Low_Level_Driver/s35_ethernet_OSI-4_TransportLayer.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/376376446" 
	@${RM} ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-4_TransportLayer.o.d 
	@${RM} ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-4_TransportLayer.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-4_TransportLayer.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-4_TransportLayer.o.d" -o ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-4_TransportLayer.o ../_Low_Level_Driver/s35_ethernet_OSI-4_TransportLayer.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-5_ApplicationLayer.o: ../_Low_Level_Driver/s35_ethernet_OSI-5_ApplicationLayer.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/376376446" 
	@${RM} ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-5_ApplicationLayer.o.d 
	@${RM} ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-5_ApplicationLayer.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-5_ApplicationLayer.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-5_ApplicationLayer.o.d" -o ${OBJECTDIR}/_ext/376376446/s35_ethernet_OSI-5_ApplicationLayer.o ../_Low_Level_Driver/s35_ethernet_OSI-5_ApplicationLayer.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/376376446/s35_ethernet_TCPIP.o: ../_Low_Level_Driver/s35_ethernet_TCPIP.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/376376446" 
	@${RM} ${OBJECTDIR}/_ext/376376446/s35_ethernet_TCPIP.o.d 
	@${RM} ${OBJECTDIR}/_ext/376376446/s35_ethernet_TCPIP.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/376376446/s35_ethernet_TCPIP.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/376376446/s35_ethernet_TCPIP.o.d" -o ${OBJECTDIR}/_ext/376376446/s35_ethernet_TCPIP.o ../_Low_Level_Driver/s35_ethernet_TCPIP.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/376376446/s12_ports.o: ../_Low_Level_Driver/s12_ports.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/376376446" 
	@${RM} ${OBJECTDIR}/_ext/376376446/s12_ports.o.d 
	@${RM} ${OBJECTDIR}/_ext/376376446/s12_ports.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/376376446/s12_ports.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/376376446/s12_ports.o.d" -o ${OBJECTDIR}/_ext/376376446/s12_ports.o ../_Low_Level_Driver/s12_ports.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/376376446/s21_uart.o: ../_Low_Level_Driver/s21_uart.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/376376446" 
	@${RM} ${OBJECTDIR}/_ext/376376446/s21_uart.o.d 
	@${RM} ${OBJECTDIR}/_ext/376376446/s21_uart.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/376376446/s21_uart.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/376376446/s21_uart.o.d" -o ${OBJECTDIR}/_ext/376376446/s21_uart.o ../_Low_Level_Driver/s21_uart.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/830869050/e_pca9685.o: ../_External_Components/e_pca9685.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/830869050" 
	@${RM} ${OBJECTDIR}/_ext/830869050/e_pca9685.o.d 
	@${RM} ${OBJECTDIR}/_ext/830869050/e_pca9685.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/830869050/e_pca9685.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O3 -MMD -MF "${OBJECTDIR}/_ext/830869050/e_pca9685.o.d" -o ${OBJECTDIR}/_ext/830869050/e_pca9685.o ../_External_Components/e_pca9685.c    -DXPRJ_default=$(CND_CONF)    $(COMPARISON_BUILD) 
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compileCPP
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: archive
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/PLIB.X.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_AR} $(MP_EXTRA_AR_PRE)  r dist/${CND_CONF}/${IMAGE_TYPE}/PLIB.X.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}    
else
dist/${CND_CONF}/${IMAGE_TYPE}/PLIB.X.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_AR} $(MP_EXTRA_AR_PRE)  r dist/${CND_CONF}/${IMAGE_TYPE}/PLIB.X.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}    
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/default
	${RM} -r dist/default

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell "${PATH_TO_IDE_BIN}"mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
