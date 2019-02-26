#ifndef ETHERNET_APPLICATIONLAYER_H
#define	ETHERNET_APPLICATIONLAYER_H

BYTE Discovery(BOOL loop, QWORD waitingPeriod, ACQUISITIONS_VAR acquisitions);
BYTE SendPingRequest(BYTE *str_ip, QWORD *time);

#endif	/* ETHERNET_APPLICATIONLAYER_H */

