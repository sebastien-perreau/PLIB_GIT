#ifndef DEFINE_TCPIP_H
#define	DEFINE_TCPIP_H

//	-------------------------
// 	|DHCP|SNMP|HTTP|FTP|TFTP|		ApplicationLayer (uniquement DHCP implémenté)	(couche OSI n°5/6/7 : Session/Présentation/Application)
//	-------------------------
//	|	 UDP    |    TCP    |		TransportLayer	(couche OSI n°4 : Transport)    To change the number of UDP and/or TCP socket "ethernet_OSI-4_TransportLayer.h"
//	-------------------------
//	|	 ICMP | IP | ARP	|		NetworkLayer	(couche OSI n°3 : Réseau)
//	-------------------------
//	|		  ETHERNET		|		DatalinkLayer	(couche OSI n°2 : Liaison)
//	-------------------------
//	|		   MEDIUM		|		PhysicalLayer	(couche OSI n°1 : Physique)

// =======================================================================
//   NETWORK ADDRESSES
// =======================================================================

//Name Board
#define DISCOVERY_VERSION               4
#define MY_DEFAULT_NAME                 "Prototypes Stack"
#define MY_DEFAULT_DESCT                "_version software xxx_"

//MAC address
#define MY_DEFAULT_MAC_BYTE1            (0x00)	// Use the default of 00-04-A3-00-00-00
#define MY_DEFAULT_MAC_BYTE2            (0x04)	// if using an ENCX24J600, MRF24WB0M, or
#define MY_DEFAULT_MAC_BYTE3            (0xA3)	// PIC32MX6XX/7XX internal Ethernet
#define MY_DEFAULT_MAC_BYTE4            (0x00)	// controller and wish to use the
#define MY_DEFAULT_MAC_BYTE5            (0x24)	// internal factory programmed MAC
#define MY_DEFAULT_MAC_BYTE6            (0xBC)	// address instead.

//IP address
#define MY_DEFAULT_IP_ADDR_BYTE1        (192ul)
#define MY_DEFAULT_IP_ADDR_BYTE2        (168ul)
#define MY_DEFAULT_IP_ADDR_BYTE3        (1ul)
#define MY_DEFAULT_IP_ADDR_BYTE4        (100ul)

#define MY_DEFAULT_MASK_BYTE1           (255ul)
#define MY_DEFAULT_MASK_BYTE2           (255ul)
#define MY_DEFAULT_MASK_BYTE3           (255ul)
#define MY_DEFAULT_MASK_BYTE4           (0ul)

#define MY_DEFAULT_GATE_BYTE1           (192ul)
#define MY_DEFAULT_GATE_BYTE2           (168ul)
#define MY_DEFAULT_GATE_BYTE3           (1ul)
#define MY_DEFAULT_GATE_BYTE4           (1ul)

#define MY_DEFAULT_PRIMARY_DNS_BYTE1	(169ul)
#define MY_DEFAULT_PRIMARY_DNS_BYTE2	(254ul)
#define MY_DEFAULT_PRIMARY_DNS_BYTE3	(1ul)
#define MY_DEFAULT_PRIMARY_DNS_BYTE4	(1ul)

#define MY_DEFAULT_SECONDARY_DNS_BYTE1	(0ul)
#define MY_DEFAULT_SECONDARY_DNS_BYTE2	(0ul)
#define MY_DEFAULT_SECONDARY_DNS_BYTE3	(0ul)
#define MY_DEFAULT_SECONDARY_DNS_BYTE4	(0ul)

#define IP_PROTOCOLE_ICMP               (1u)
#define IP_PROTOCOLE_TCP                (6u)
#define IP_PROTOCOLE_UDP                (17u)

#define IP_ADDR                         DWORD_VAL

// =======================================================================
//   --- HELPERS FONCTIONS
// =======================================================================
DWORD   LFSRSeedRand(DWORD dwSeed);
WORD    LFSRRand(void);
DWORD   GenerateRandomDWORD(void);
BOOL    StringToIPAddress(BYTE* str, IP_ADDR* IPAddress);
BOOL    StringToMACAddress(BYTE* str, BYTE* MACAddress);
WORD    CalcIPChecksum(BYTE* buffer, WORD len);

#endif
