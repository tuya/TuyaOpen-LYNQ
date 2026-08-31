#include DEBUG_LOG_HEADER_FILE
#include "volc_network.h"

#include <unistd.h>
#include <errno.h>
#include "netdb.h"
#include "sockets.h"
#include "ps_lib_api.h"
#include "volc_type.h"

uint32_t volc_get_local_ip(volc_ip_addr_t* dest_ip_list, uint32_t* p_dest_ip_list_len, volc_network_callback_t callback, uint64_t custom_data) {
    uint32_t ret = VOLC_STATUS_FAILURE;
    uint32_t ip_count = 0;
    // iterate over active interfaces, and print out IPs of "our" netifs
	NmAtiNetifInfo netInfo;
	appGetNetInfoSync(0,&netInfo);
	VOLC_CHK(dest_ip_list != NULL && p_dest_ip_list_len != NULL, VOLC_STATUS_NULL_ARG);
    VOLC_CHK(*p_dest_ip_list_len != 0, VOLC_STATUS_INVALID_ARG);
	if(netInfo.netStatus == NM_NETIF_ACTIVATED)
	{
		if (netInfo.ipType == NM_NET_TYPE_IPV4 || netInfo.ipType == NM_NET_TYPE_IPV4_IPV6preparing)
        {
        	memcpy(dest_ip_list[ip_count].address,(uint8_t *)&netInfo.ipv4Info.ipv4Addr.addr,4*sizeof(uint8_t));
			dest_ip_list[ip_count].family = VOLC_IP_FAMILY_TYPE_IPV4;
			ip_count++;
        }
        else if (netInfo.ipType == NM_NET_TYPE_IPV6)
        {
            memcpy(dest_ip_list[ip_count].address,(uint8_t *)&netInfo.ipv6Info.ipv6Addr,16*sizeof(uint8_t));
			dest_ip_list[ip_count].family = VOLC_IP_FAMILY_TYPE_IPV6;
			ip_count++;

        }
        else if (netInfo.ipType == NM_NET_TYPE_IPV4V6)
        {
            memcpy(dest_ip_list[ip_count].address,(uint8_t *)&netInfo.ipv4Info.ipv4Addr.addr,VOLC_IPV4_ADDRESS_LENGTH);
			dest_ip_list[ip_count].family = VOLC_IP_FAMILY_TYPE_IPV4;
			ip_count++;
			memcpy(dest_ip_list[ip_count].address,(uint8_t *)&netInfo.ipv6Info.ipv6Addr,VOLC_IPV6_ADDRESS_LENGTH);
			dest_ip_list[ip_count].family = VOLC_IP_FAMILY_TYPE_IPV6;
			ip_count++;
        }
		else
			return ret;
		ret = VOLC_STATUS_SUCCESS;
		*p_dest_ip_list_len = ip_count;
	}
err_out_label:
    return ret;
}

// getIpWithHostName
uint32_t volc_get_ip_with_host_name(const char* hostname, volc_ip_addr_t* dest_ip) {
    uint32_t ret = VOLC_STATUS_SUCCESS;
    int32_t err_code;
    struct addrinfo *res = NULL, *rp;
    bool resolved = false;
    struct sockaddr_in* ipv4Addr;
    struct sockaddr_in6* ipv6Addr;
	struct addrinfo hints;
	memset(&hints,0,sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
    VOLC_CHK(hostname != NULL, VOLC_STATUS_NULL_ARG);
    err_code = getaddrinfo(hostname, NULL, &hints, &res);
    if (err_code != 0) {
        ret = VOLC_STATUS_RESOLVE_HOSTNAME_FAILED;
        goto err_out_label;
    }

    for (rp = res; rp != NULL && !resolved; rp = rp->ai_next) {
        if (rp->ai_family == AF_INET) {
            ipv4Addr = (struct sockaddr_in*) rp->ai_addr;
            dest_ip->family = VOLC_IP_FAMILY_TYPE_IPV4;
            memcpy(dest_ip->address, &ipv4Addr->sin_addr, VOLC_IPV4_ADDRESS_LENGTH);
            resolved = true;
        } else if (rp->ai_family == AF_INET6) {
            ipv6Addr = (struct sockaddr_in6*) rp->ai_addr;
            dest_ip->family = VOLC_IP_FAMILY_TYPE_IPV6;
            memcpy(dest_ip->address, &ipv6Addr->sin6_addr, VOLC_IPV6_ADDRESS_LENGTH);
            resolved = true;
        }
    }

    freeaddrinfo(res);
    if (!resolved) {
        ret = VOLC_STATUS_HOSTNAME_NOT_FOUND;
    }
err_out_label:
    return ret;
}

int volc_inet_pton (int __af, const char *__restrict __cp,void *__restrict __buf) {
    int af = AF_INET;
    if(__af == VOLC_IP_FAMILY_TYPE_IPV6 ) {
        af = AF_INET6;
    }
    return inet_pton(af,__cp,__buf);
};

 uint16_t volc_htons(uint16_t hostshort){
    return htons(hostshort);
 };
uint16_t volc_ntohs(uint16_t netshort){
    return ntohs(netshort);
}