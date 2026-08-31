#ifndef LWIP_PING_H
#define LWIP_PING_H

#include "lwip/ip_addr.h"

/**
 * PING_USE_SOCKETS: Set to 1 to use sockets, otherwise the raw api is used
 */
#ifndef PING_USE_SOCKETS
#define PING_USE_SOCKETS    LWIP_SOCKET
#endif

/*ping retry count*/
#define PING_TRY_COUNT 15
#define PING_ALWAYS 0x0FFFFFFF
#define PING_PAYLOAD_MAX_LEN 1520

BOOL ping_init(const ip_addr_t* ping_addr, int32_t count, uint16_t payload_len, uint32_t timeout, BOOL raiFlag, uint16_t req_handle, uint8_t cid);
BOOL ping_url_init(const char* ping_url_addr, int32_t count, uint16_t payload_len, uint32_t timeout, BOOL raiFlag, uint16_t req_handle, uint8_t cid);

void ping_terminat(void);

//check ping whether running,TRUE->running, FALSE->not run
BOOL PingChkStatus(void);


#if !PING_USE_SOCKETS
void ping_send_now(void);
#endif /* !PING_USE_SOCKETS */

#endif /* LWIP_PING_H */
