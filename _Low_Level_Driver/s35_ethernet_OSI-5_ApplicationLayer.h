#ifndef __DHCP_H
#define __DHCP_H

typedef struct __attribute__((__packed__)) appConfigStruct 
{
    IP_ADDR MyIPAddr; // IP address
    IP_ADDR MyMask; // Subnet mask
    IP_ADDR MyGateway; // Default Gateway
    IP_ADDR PrimaryDNSServer; // Primary DNS Server
    IP_ADDR SecondaryDNSServer; // Secondary DNS Server
    IP_ADDR DefaultIPAddr; // Default IP address
    IP_ADDR DefaultMask; // Default subnet mask
    BYTE NetBIOSName[16]; // NetBIOS name
    BOOL bIsDHCPEnabled;
    BOOL bInConfigMode;
    MAC_ADDR MyMACAddr; // Application MAC address
}
APP_CONFIG;

#define DHCP_ENABLED                        1
#define DHCP_DISABLED                       0
// Defines how long to wait before a DHCP request times out
#define DHCP_TIMEOUT                        (TICK_2S)
// UDP client port for DHCP Client transactions
#define DHCP_CLIENT_PORT                    (68u)
// UDP listening port for DHCP Server messages
#define DHCP_SERVER_PORT                    (67u)

#define BOOT_REQUEST                        (1u)	// BOOT_REQUEST DHCP type
#define BOOT_REPLY                          (2u)	// BOOT_REPLY DHCP type
#define BOOT_HW_TYPE                        (1u)	// BOOT_HW_TYPE DHCP type
#define BOOT_LEN_OF_HW_TYPE                 (6u)	// BOOT_LEN_OF_HW_TYPE DHCP type

#define DHCP_MESSAGE_TYPE                   (53u)	// DHCP Message Type constant
#define DHCP_MESSAGE_TYPE_LEN               (1u)	// Length of DHCP Message Type

#define DHCP_UNKNOWN_MESSAGE                (0u)	// Code for unknown DHCP message
    
#define DHCP_DISCOVER_MESSAGE               (1u)	// DCHP Discover Message
#define DHCP_OFFER_MESSAGE                  (2u)	// DHCP Offer Message
#define DHCP_REQUEST_MESSAGE                (3u)	// DHCP Request message
#define DHCP_DECLINE_MESSAGE                (4u)	// DHCP Decline Message
#define DHCP_ACK_MESSAGE                    (5u)	// DHCP ACK Message
#define DHCP_NAK_MESSAGE                    (6u)	// DHCP NAK Message
#define DHCP_RELEASE_MESSAGE                (7u)	// DCHP Release message

#define DHCP_SERVER_IDENTIFIER              (54u)	// DHCP Server Identifier
#define DHCP_SERVER_IDENTIFIER_LEN          (4u)	// DHCP Server Identifier length

#define DHCP_OPTION_ACK_MESSAGE             (53u)	// DHCP_OPTION_ACK_MESSAGE Type
#define DHCP_PARAM_REQUEST_LIST             (55u)	// DHCP_PARAM_REQUEST_LIST Type
#define DHCP_PARAM_REQUEST_LIST_LEN         (4u)	// DHCP_PARAM_REQUEST_LIST_LEN Type
#define DHCP_PARAM_REQUEST_IP_ADDRESS       (50u)	// DHCP_PARAM_REQUEST_IP_ADDRESS Type
#define DHCP_PARAM_REQUEST_IP_ADDRESS_LEN   (4u)	// DHCP_PARAM_REQUEST_IP_ADDRESS_LEN Type
#define DHCP_SUBNET_MASK                    (1u)	// DHCP_SUBNET_MASK Type
#define DHCP_ROUTER                         (3u)	// DHCP_ROUTER Type
#define DHCP_DNS                            (6u)	// DHCP_DNS Type
#define DHCP_HOST_NAME                      (12u)	// DHCP_HOST_NAME Type
#define DHCP_IP_LEASE_TIME                  (51u)	// DHCP_IP_LEASE_TIME Type
#define DHCP_END_OPTION                     (255u)	// DHCP_END_OPTION Type

typedef enum {
    SM_DHCP_DISABLED = 0, // DHCP is not currently enabled
    SM_DHCP_GET_SOCKET, // DHCP is trying to obtain a socket
    SM_DHCP_SEND_DISCOVERY, // DHCP is sending a DHCP Discover message
    SM_DHCP_GET_OFFER, // DHCP is waiting for a DHCP Offer
    SM_DHCP_SEND_REQUEST, // DHCP is sending a DHCP Send Reequest message
    SM_DHCP_GET_REQUEST_ACK, // DCHP is waiting for a Request ACK message
    SM_DHCP_BOUND, // DHCP is bound
    SM_DHCP_SEND_RENEW, // DHCP is sending a DHCP renew message (first try)
    SM_DHCP_GET_RENEW_ACK, // DHCP is waiting for a renew ACK
    SM_DHCP_SEND_RENEW2, // DHCP is sending a DHCP renew message (second try)
    SM_DHCP_GET_RENEW_ACK2, // DHCP is waiting for a renew ACK
    SM_DHCP_SEND_RENEW3, // DHCP is sending a DHCP renew message (third try)
    SM_DHCP_GET_RENEW_ACK3 // DHCP is waiting for a renew ACK
} SM_DHCP;

typedef struct 
{
    UDP_SOCKET hDHCPSocket; // Handle to DHCP client socket
    SM_DHCP smState; // DHCP client state machine variable
    BYTE bIsBound; // Whether or not DHCP is currently bound
    BYTE bEvent; // Indicates to an external module that the DHCP client has been reset, has obtained new parameters via the DHCP client, or has refreshed a lease on existing ones
    BYTE bOfferReceived; // Whether or not an offer has been received
    BYTE bDHCPServerDetected; // Indicates if a DCHP server has been detected
    BYTE bUseUnicastMode; // Indicates if the
    QWORD dwTimer; // Tick timer value used for triggering future events after a certain wait period.
    DWORD dwLeaseTime; // DHCP lease time remaining, in seconds
    DWORD dwServerID; // DHCP Server ID cache
    IP_ADDR tempIPAddress; // Temporary IP address to use when no DHCP lease
    IP_ADDR tempGateway; // Temporary gateway to use when no DHCP lease
    IP_ADDR tempMask; // Temporary mask to use when no DHCP lease

    struct _validValues 
    {
        char IPAddress; // Leased IP address is valid
        char Gateway; // Gateway address is valid
        char Mask; // Subnet mask is valid
    } validValues;
} DHCP_CLIENT_VARS;

extern APP_CONFIG AppConfig;

void ETH_StackInit(BYTE *str_mac, BYTE *str_ip, BOOL dhcpEnabled);
void ETH_StackTask(void);

void DHCPInit(void);
void DHCPTask(void);
void DHCPServerTask(void);
void DHCPDisable(void);
void DHCPEnable(void);
BOOL DHCPIsEnabled(void);
BOOL DHCPIsBound(void);
BOOL DHCPStateChanged(void);
BOOL DHCPIsServerDetected(void);

#endif
