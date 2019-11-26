#ifndef ETHERNET_APPLICATIONLAYER_H
#define	ETHERNET_APPLICATIONLAYER_H

BYTE Discovery(BOOL loop, QWORD waitingPeriod, acquisitions_params_t acquisitions);
BYTE SendPingRequest(BYTE *str_ip, QWORD *time);

#endif	/* ETHERNET_APPLICATIONLAYER_H */

