#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include "tkl_network.h"
#include "cmsis_os2.h"
#include "osasys.h"
#include "cmsis_os2.h"
#include "vlog.h"

#define CANONNAME_MAX 128
#define UNW_TO_SYS_FD_SET(fds)  ((fd_set*)fds)

typedef struct NETWORK_ERRNO_TRANS {
    int sys_err;
    int priv_err;
} NETWORK_ERRNO_TRANS_S;

extern OPERATE_RET rrc_release_timer_reset(void);

CONST NETWORK_ERRNO_TRANS_S unw_errno_trans[]= {
    {EINTR,UNW_EINTR},
    {EBADF,UNW_EBADF},
    {EAGAIN,UNW_EAGAIN},
    {EFAULT,UNW_EFAULT},
    {EBUSY,UNW_EBUSY},
    {EINVAL,UNW_EINVAL},
    {ENFILE,UNW_ENFILE},
    {EMFILE,UNW_EMFILE},
    {ENOSPC,UNW_ENOSPC},
    {EPIPE,UNW_EPIPE},
    {EWOULDBLOCK,UNW_EWOULDBLOCK},
    {ENOTSOCK,UNW_ENOTSOCK},
    {ENOPROTOOPT,UNW_ENOPROTOOPT},
    {EADDRINUSE,UNW_EADDRINUSE},
    {EADDRNOTAVAIL,UNW_EADDRNOTAVAIL},
    {ENETDOWN,UNW_ENETDOWN},
    {ENETUNREACH,UNW_ENETUNREACH},
    {ENETRESET,UNW_ENETRESET},
    {ECONNRESET,UNW_ECONNRESET},
    {ENOBUFS,UNW_ENOBUFS},
    {EISCONN,UNW_EISCONN},
    {ENOTCONN,UNW_ENOTCONN},
    {ETIMEDOUT,UNW_ETIMEDOUT},
    {ECONNREFUSED,UNW_ECONNREFUSED},
    {EHOSTDOWN,UNW_EHOSTDOWN},
    {EHOSTUNREACH,UNW_EHOSTUNREACH},
    {ENOMEM ,UNW_ENOMEM},
    {EMSGSIZE,UNW_EMSGSIZE}
};


/**
 * @brief 用于获取错误序号
 *
 * @retval         errno
 */
TUYA_ERRNO tkl_net_get_errno(void)
{
    int i = 0;
    int sys_err = errno;

    for(i = 0; i < (int)sizeof(unw_errno_trans)/sizeof(unw_errno_trans[0]); i++) {
        if(unw_errno_trans[i].sys_err == sys_err) {
            return unw_errno_trans[i].priv_err;
        }
    }

    return -100 - sys_err;
}

/**
* @brief Change ip address to string
*
* @param[in] ipaddr: ip address
*
* @note This API is used to change ip address(in host byte order) to string(in IPv4 numbers-and-dots(xx.xx.xx.xx) notion).
*
* @return ip string
*/
CHAR_T* tkl_net_addr2str(CONST TUYA_IP_ADDR_T ipaddr)
{
    unsigned int addr = lwip_htonl(ipaddr);
	ip_addr_t struct_ip = {
		.u_addr = {
			.ip4 = {addr},
		},
		.type = IPADDR_TYPE_V4,
	};

	return ip_ntoa((ip_addr_t *) &struct_ip);
}

/**
 * @brief : Ascii网络字符串地址转换为主机序(4B)地址
 * @param[in]            ip_str
 * @return   主机序ip地址(4B)
 */
TUYA_IP_ADDR_T tkl_net_str2addr(CONST CHAR_T *ip_str)
{
    if(ip_str == NULL) {
        return 0xFFFFFFFF;
    }

    TUYA_IP_ADDR_T addr1 = inet_addr((char*)ip_str);
    TUYA_IP_ADDR_T addr2 = ntohl(addr1);
    return addr2;
}

/**
 * @brief : set fds
 * @param[in]      fd
 * @param[inout]      fds
 * @return  0: success  <0: fail
 */
OPERATE_RET tkl_net_fd_set(CONST INT_T fd, TUYA_FD_SET_T* fds)
{
    FD_SET(fd, UNW_TO_SYS_FD_SET(fds));
    return OPRT_OK;
}

/**
 * @brief : clear fds
 * @param[in]      fd
 * @param[inout]      fds
 * @return  0: success  <0: fail
 */
OPERATE_RET tkl_net_fd_clear(CONST INT_T fd, TUYA_FD_SET_T* fds)
{
    FD_CLR(fd, UNW_TO_SYS_FD_SET(fds));
    return OPRT_OK;
}

/**
 * @brief : 判断fds是否被置位
 * @param[in]      fd
 * @param[in]      fds
 * @return  0-没有可读fd other-有可读fd
 */
OPERATE_RET tkl_net_fd_isset(CONST INT_T fd, TUYA_FD_SET_T* fds)
{
    return FD_ISSET(fd, UNW_TO_SYS_FD_SET(fds));
}

/**
 * @brief : init fds
 * @param[inout]      fds
 * @return  0: success  <0: fail
 */
OPERATE_RET tkl_net_fd_zero(TUYA_FD_SET_T* fds)
{
    if(fds == NULL) {
        return 0xFFFFFFFF;
    }

    FD_ZERO(UNW_TO_SYS_FD_SET(fds));
    return OPRT_OK;
}

/**
 * @brief : select
 * @param[in]         maxfd
 * @param[inout]      readfds
 * @param[inout]      writefds
 * @param[inout]      errorfds
 * @param[inout]      ms_timeout
 * @return  0: success  <0: fail
 */
INT_T tkl_net_select(CONST INT_T maxfd, TUYA_FD_SET_T *readfds,
                     TUYA_FD_SET_T *writefds, TUYA_FD_SET_T *errorfds,
                     CONST UINT_T ms_timeout)

{
    struct timeval *tmp;
    struct timeval timeout;

    timeout.tv_sec = ms_timeout / 1000;
    timeout.tv_usec = (ms_timeout % 1000) * 1000;
    if (0 != ms_timeout) {
        tmp = &timeout;
    } else {
        tmp = NULL;
    }

    return select(maxfd, UNW_TO_SYS_FD_SET(readfds),
                  UNW_TO_SYS_FD_SET(writefds),
                  UNW_TO_SYS_FD_SET(errorfds), tmp);
}

/**
* @brief Get no block file descriptors
*
* @param[in] fd: file descriptor
*
* @note This API is used to get no block file descriptors.
*
* @return the count of no block file descriptors.
*/
INT_T tkl_net_get_nonblock(CONST INT_T fd)
{
    if((fcntl(fd, F_GETFL, 0) & O_NONBLOCK) == O_NONBLOCK) {
        return 1;
    }
    return 0;
}
/**
* @brief Set block flag for file descriptors
*
* @param[in] fd: file descriptor
* @param[in] block: block flag
*
* @note This API is used to set block flag for file descriptors.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_net_set_block(CONST INT_T fd, CONST BOOL_T block)
{
    int flags;

    flags = fcntl(fd, F_GETFL, 0);
    if (block) {
        flags &= (~O_NONBLOCK);
    } else {
        flags |= O_NONBLOCK;
    }

    if (fcntl(fd, F_SETFL, flags) < 0) {
        return OPRT_COM_ERROR;
    }

    return OPRT_OK;
}

/**
 * @brief : close fd
 * @param[in]      fd
 * @return  0: success  <0: fail
 */
TUYA_ERRNO tkl_net_close(CONST INT_T fd)
{
    return close(fd);
}

/**
 * @brief : shutdow fd
 * @param[in]      fd
 * @param[in]      how
 * @return  OPRT_OK: success  <0: fail
 */
TUYA_ERRNO tkl_net_shutdown(CONST INT_T fd, CONST INT_T how)
{
    return shutdown(fd, how);
}

/**
 * @brief : creat fd
 * @param[in]      type
 * @return  >=0: socketfd  <0: fail
 */
INT_T tkl_net_socket_create(CONST TUYA_PROTOCOL_TYPE_E type)
{
    int fd = -1;

    if(PROTOCOL_TCP == type) {
        fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    } else if (PROTOCOL_RAW == type) {
        fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    } else {
        fd = socket(AF_INET, SOCK_DGRAM, 0);
    }

    return fd;
}

/**
 * @brief : connect
 * @param[in]      fd
 * @param[in]      addr
 * @param[in]      port
 * @return  0: success  Other: fail
 */
TUYA_ERRNO tkl_net_connect(CONST INT_T fd, CONST TUYA_IP_ADDR_T addr,
                          CONST UINT16_T port)
{
    struct sockaddr_in sock_addr;
    uint16_t tmp_port = port;
    TUYA_IP_ADDR_T tmp_addr = addr;

    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(tmp_port);
    sock_addr.sin_addr.s_addr = htonl(tmp_addr);

    // LOGI("connect fd %d addr %x port %d", fd, addr, port);
    return connect(fd, (struct sockaddr*)&sock_addr, sizeof(struct sockaddr_in));
}

/**
 * @brief : raw connect
 * @param[in]      fd
 * @param[in]      p_socket
 * @param[in]      len
 * @return  0: success  Other: fail
 */
TUYA_ERRNO tkl_net_connect_raw(CONST INT_T fd, VOID *p_socket_addr, CONST INT_T len)
{
    return connect(fd, (struct sockaddr *)p_socket_addr, len);
}

/**
 * @brief : bind
 * @param[in]      fd
 * @param[in]      addr
 * @param[in]      port
 * @return  0: success  Other: fail
 */
TUYA_ERRNO tkl_net_bind(CONST INT_T fd, CONST TUYA_IP_ADDR_T addr, CONST UINT16_T port)
{
    unsigned short tmp_port;
    TUYA_IP_ADDR_T tmp_addr;
    struct sockaddr_in sock_addr;

    tmp_port = port;
    tmp_addr = addr;
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(tmp_port);
    sock_addr.sin_addr.s_addr = htonl(tmp_addr);

    return bind(fd, (struct sockaddr*)&sock_addr, sizeof(struct sockaddr_in));
}

/**
 * @brief : bind ip
 * @param[in]            fd
 * @param[inout]         addr
 * @param[inout]         port
 * @return  0: success  <0: fail
 */
int tuya_os_adapt_net_socket_bind(const int fd, const char *ip)
{
    struct sockaddr_in addr_client = {0};
    if (ip == NULL) {
        return OPRT_INVALID_PARM;
    }
    addr_client.sin_family = AF_INET;
    addr_client.sin_addr.s_addr = inet_addr(ip);
    addr_client.sin_port = 0;    // 0 表示由系统自动分配端口号

    return bind(fd, (struct sockaddr*)&addr_client, sizeof(addr_client));
}

/**
 * @brief : listen
 * @param[in]      fd
 * @param[in]      backlog
 * @return  0: success  < 0: fail
 */
TUYA_ERRNO tkl_net_listen(CONST INT_T fd, CONST INT_T backlog)
{
    return listen(fd, backlog);
}

/**
 * @brief : accept
 * @param[in]            fd
 * @param[inout]         addr
 * @param[inout]         port
 * @return  >=0: 新接收到的socketfd  others: fail
 */
TUYA_ERRNO tkl_net_accept(CONST INT_T fd, TUYA_IP_ADDR_T *addr, UINT16_T *port)
{
    struct sockaddr_in sock_addr;
    socklen_t len;
    int cfd;

    len = sizeof(struct sockaddr_in);
    cfd = accept(fd, (struct sockaddr *)&sock_addr,&len);
    if (cfd < 0)
        return OPRT_COM_ERROR;

    if (addr)
        *addr = ntohl((sock_addr.sin_addr.s_addr));

    if (port)
        *port = ntohs((sock_addr.sin_port));

    return cfd;
}

/**
 * @brief : send
 * @param[in]      fd
 * @param[in]      buf
 * @param[in]      nbytes
 * @return  nbytes has sended
 */
TUYA_ERRNO tkl_net_send(CONST INT_T fd, CONST VOID *buf, CONST UINT_T nbytes)
{
    if((fd < 0) || (buf == NULL) || (nbytes == 0)){
        LOGE("tkl_net_send, invalid param, fd/%d, buf/%p, nbytes/%u", fd, buf, nbytes);
        return OPRT_INVALID_PARM;
    }

    rrc_release_timer_reset();
    return send(fd,buf,nbytes,0);
}

/**
 * @brief : send to
 * @param[in]      fd
 * @param[in]      buf
 * @param[in]      nbytes
 * @param[in]      addr
 * @param[in]      port
 * @return  nbytes has sended
 */
TUYA_ERRNO tkl_net_send_to(CONST INT_T fd, CONST VOID *buf, CONST UINT_T nbytes, CONST TUYA_IP_ADDR_T addr,CONST UINT16_T port)
{
    unsigned short tmp_port = port;
    TUYA_IP_ADDR_T tmp_addr = addr;
    struct sockaddr_in sock_addr;

    if ((fd < 0) || (buf == NULL) || (nbytes == 0)) {
        LOGE("tkl_net_send_to, invalid param, fd/%d, buf/%p, nbytes/%u", fd, buf, nbytes);
        return OPRT_INVALID_PARM;
    }

    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(tmp_port);
    sock_addr.sin_addr.s_addr = htonl(tmp_addr);

    return sendto(fd, buf, nbytes, 0, (struct sockaddr *)&sock_addr,
                  sizeof(sock_addr));
}

/**
 * @brief : recv
 * @param[in]         fd
 * @param[inout]      buf
 * @param[in]         nbytes
 * @return  nbytes has received
 */
TUYA_ERRNO tkl_net_recv(CONST INT_T fd, VOID *buf, CONST UINT_T nbytes)
{
    if ((fd < 0) || (buf == NULL) || (nbytes == 0)) {
        LOGE("tkl_net_recv, invalid param, fd/%d, buf/%p, nbytes/%u", fd, buf, nbytes);
        return OPRT_INVALID_PARM;
    }

    return recv(fd, buf, nbytes, 0);
}

/**
 * @brief : Receive enough data to specify
 * @param[in]            fd
 * @param[inout]         buf
 * @param[in]            buf_size
 * @param[in]            nd_size
 * @return  nbytes has received
 */
INT_T tkl_net_recv_nd_size(CONST INT_T fd, VOID *buf, CONST UINT_T buf_size, CONST UINT_T nd_size)
{
    if ((fd < 0) || (NULL == buf) || (buf_size == 0) ||
       (nd_size == 0) || (buf_size < nd_size)) {
        LOGE("tkl_net_recv_nd_size, invalid param, fd/%d, buf/%p, buf_size/%u, nd_size/%u",
		     fd, buf, buf_size, nd_size);
        return OPRT_INVALID_PARM;
    }

    UINT32_T rd_size = 0;
    INT_T ret = 0;

    while(rd_size < nd_size) {
        ret = recv(fd,((UINT8_T *)buf + rd_size),nd_size-rd_size,0);
        if(ret <= 0) {
            if(EWOULDBLOCK == errno || EINTR == errno || EAGAIN == errno) {
                osDelay(10);
                continue;
            }
            break;
        }
        rd_size += ret;
    }

    if(rd_size < nd_size) {
        return -2;
    }

    return rd_size;
}

/**
 * @brief : recvfrom
 * @param[in]         fd
 * @param[inout]      buf
 * @param[in]         nbytes
 * @param[inout]         addr
 * @param[inout]         port
 * @return  nbytes has received
 */
TUYA_ERRNO tkl_net_recvfrom(CONST INT_T fd, VOID *buf, CONST UINT_T nbytes, TUYA_IP_ADDR_T *addr, UINT16_T *port)
{
    int ret = 0;
    struct sockaddr_in sock_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    if ((fd < 0) || (buf == NULL) || (nbytes == 0)) {
        LOGE("tkl_net_recvfrom, invalid param, fd/%d, buf/%p, nbytes/%u", fd, buf, nbytes);
        return OPRT_INVALID_PARM;
    }

    ret = recvfrom(fd, buf, nbytes, 0, (struct sockaddr *)&sock_addr, &addr_len);
    if (ret <= 0) {
        return ret;
    }

    if (addr) {
        *addr = ntohl(sock_addr.sin_addr.s_addr);
    }

    if (port) {
        *port = ntohs(sock_addr.sin_port);
    }

    return ret;
}

/**
 * @brief : set timeout
 * @param[in]         fd
 * @param[in]         ms_timeout
 * @param[in]         type
 * @return  0: success  <0: fail
 */
OPERATE_RET tkl_net_set_timeout(CONST INT_T fd, CONST INT_T ms_timeout,
                                CONST TUYA_TRANS_TYPE_E type)

{
    struct timeval timeout;
    int optname;

    timeout.tv_sec = ms_timeout / 1000;
    timeout.tv_usec = (ms_timeout % 1000) * 1000;
    optname = (type == TRANS_RECV) ? SO_RCVTIMEO : SO_SNDTIMEO;

    if (0 != setsockopt(fd, SOL_SOCKET, optname, (char *)&timeout, sizeof(timeout))) {
        return OPRT_COM_ERROR;
    }

    return OPRT_OK;
}

/**
 * @brief : set buf size
 * @param[in]         fd
 * @param[in]         buf_size
 * @param[in]         type
 * @return  0: success  <0: fail
 */
OPERATE_RET tkl_net_set_bufsize(CONST INT_T fd, CONST INT_T buf_size,
                                CONST TUYA_TRANS_TYPE_E type)
{
    int size;
    int optname;

    size = buf_size;
    optname = (type == TRANS_RECV) ? SO_RCVBUF : SO_SNDBUF;
    if (0 != setsockopt(fd, SOL_SOCKET, optname, (char *)&size, sizeof(size))) {
        return OPRT_COM_ERROR;
    }

    return OPRT_OK;
}

/**
 * @brief : set reuse
 * @param[in]         fd
 * @return  0: success  <0: fail
 */
OPERATE_RET tkl_net_set_reuse(CONST INT_T fd)
{
    int flag = 1;
    if (0 != setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&flag, sizeof(int))) {
        return OPRT_COM_ERROR;
    }

    return OPRT_OK;
}

/**
 * @brief : disable nagle
 * @param[in]         fd
 * @return  0: success  <0: fail
 */
OPERATE_RET tkl_net_disable_nagle(CONST INT_T fd)
{
    int flag = 1;
    if (0 != setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&flag, sizeof(int))) {
        return OPRT_COM_ERROR;
    }

    return OPRT_OK;
}

/**
 * @brief : set broadcast
 * @param[in]         fd
 * @return  0: success  <0: fail
 */
OPERATE_RET tkl_net_set_boardcast(CONST INT_T fd)
{
    int flag = 1;
    if (0 != setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (const char*)&flag, sizeof(int))) {
        return OPRT_COM_ERROR;
    }

    return OPRT_OK;
}

/**
 * @brief : dns parse
 * @param[in]            domain
 * @param[inout]         addr
 * @return  0: success  <0: fail
 */
OPERATE_RET tkl_net_gethostbyname(CONST CHAR_T *domain, TUYA_IP_ADDR_T *addr)
{
    if ((domain == NULL) || (addr == NULL)) {
        return OPRT_OS_ADAPTER_INVALID_PARM;
    }

    struct hostent* h;
    if ((h = gethostbyname(domain)) == NULL) {
        return OPRT_COM_ERROR;
    }

    *addr = ntohl(((struct in_addr *)(h->h_addr_list[0]))->s_addr);
    return OPRT_OK;
}

OPERATE_RET tkl_net_socket_bind(CONST INT_T fd, CONST CHAR_T *ip)
{
    if(NULL == ip) {
        return -3000;
    }

    struct sockaddr_in addr_client = {0};
    addr_client.sin_family = AF_INET;
    addr_client.sin_addr.s_addr = inet_addr(ip);
    addr_client.sin_port = 0;    /// 0 表示由系统自动分配端口号
    if (0 != bind(fd, (struct sockaddr*)&addr_client, sizeof(addr_client))) {
        return OPRT_COM_ERROR;
    }

    return OPRT_OK;
}

OPERATE_RET tkl_net_set_cloexec(CONST INT_T fd)
{
    return OPRT_OK;
}

OPERATE_RET tkl_net_get_socket_ip(CONST INT_T fd, TUYA_IP_ADDR_T *addr)
{
    struct sockaddr_in sock_addr;
    memset(&sock_addr, 0, sizeof(sock_addr));
    socklen_t len = sizeof(sock_addr);
    INT_T ret = getsockname(fd, (struct sockaddr*)&sock_addr, &len);
    if (ret != 0) {
        return OPRT_COM_ERROR;
    }

    *addr = ntohl(sock_addr.sin_addr.s_addr);
    return OPRT_OK;
}

/**
* @brief Set socket options
*
* @param[in] fd: file descriptor
* @param[in] level: setting level
* @param[in] optname: the name of the option
* @param[in] optval: the value of option
* @param[in] optlen: the length of the option value
*
* @note This API is used for setting socket options.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_net_setsockopt(CONST INT_T fd, CONST TUYA_OPT_LEVEL level, CONST TUYA_OPT_NAME optname, CONST VOID_T *optval, CONST INT_T optlen)
{
    return setsockopt((INT_T)fd, (int)level, (int)optname, optval, (INT_T)optlen);
}

/**
* @brief Get socket options
*
* @param[in] fd: file descriptor
* @param[in] level: getting level
* @param[in] optname: the name of the option
* @param[out] optval: the value of option
* @param[out] optlen: the length of the option value
*
* @note This API is used for getting socket options.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_net_getsockopt(CONST INT_T fd, CONST TUYA_OPT_LEVEL level, CONST TUYA_OPT_NAME optname, VOID_T *optval, INT_T *optlen)
{
    if (0 != getsockopt((INT_T)fd, (int)level, (int)optname, (VOID*)optval, (socklen_t*)optlen)) {
        return OPRT_COM_ERROR;
    }

    return OPRT_OK;
}

OPERATE_RET tkl_net_set_broadcast(CONST INT_T fd)
{
    int broadcast_enabled = 1;
    int ret = tkl_net_setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &broadcast_enabled , sizeof(broadcast_enabled));
    if (ret < 0) {
        LOGE("tkl_net_set_broadcast failed, ret: %d", ret);
        return OPRT_OS_ADAPTER_NETWORK_ERROR;
    }
    return OPRT_OK;
}

OPERATE_RET tkl_net_set_keepalive(INT_T fd, CONST BOOL_T alive, CONST UINT_T idle, CONST UINT_T intr, CONST UINT_T cnt)
{
    int ret;

    // 启用或禁用SO_KEEPALIVE选项
    ret = tkl_net_setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &alive, sizeof(alive));
    if (ret < 0) {
        LOGE("tkl_net_set_keepalive failed, SO_KEEPALIVE, ret: %d", ret);
        return ret;
    }

    // 设置TCP_KEEPIDLE选项（在开始发送保持活动探测之前的空闲时间）
    ret = tkl_net_setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(idle));
    if (ret < 0) {
        LOGE("tkl_net_set_keepalive failed, TCP_KEEPIDLE, ret: %d", ret);
        return ret;
    }
    
	// 设置TCP_KEEPINTVL选项（两次保持活动探测之间的时间间隔）
    ret = tkl_net_setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &intr, sizeof(intr));
    if (ret < 0) {
        LOGE("tkl_net_set_keepalive failed, TCP_KEEPINTVL, ret: %d", ret);
        return ret;
    }

    // 设置TCP_KEEPINTVL选项（两次保持活动探测之间的时间间隔）
    ret = tkl_net_setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &cnt, sizeof(cnt));
    if (ret < 0) {
        LOGE("tkl_net_set_keepalive failed, TCP_KEEPCNT, ret: %d", ret);
        return ret;
    }
    return OPRT_OK;
}

OPERATE_RET tkl_net_getsockname(INT_T fd, TUYA_IP_ADDR_T *addr, UINT16_T *port)
{
    struct sockaddr_in sock_addr;
    memset(&sock_addr, 0, sizeof(sock_addr));
    socklen_t len = sizeof(sock_addr);
    int ret = getsockname(fd, (struct sockaddr *)&sock_addr, &len);
    if (ret != 0) {
        return OPRT_COM_ERROR;
    }
    if (addr) {
        *addr = ntohl(sock_addr.sin_addr.s_addr);
    }
    if (port) {
        *port = (UINT16_T)ntohs(sock_addr.sin_port);
    }
    // LOGD("%d getsockname %x %d %s",fd, *addr, *port, tkl_net_addr2str(*addr));
    return OPRT_OK;
}

OPERATE_RET tkl_net_getpeername(INT_T fd, TUYA_IP_ADDR_T *addr, UINT16_T *port)
{
    LOGE("getpeername");
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET tkl_net_sethostname(CONST CHAR_T *hostname)
{
    LOGE("sethostname");
    return OPRT_NOT_SUPPORTED;
}

#if 0
void tcp_test(char *ip, uint16_t port)
{
    INT_T fd = -1;
    TUYA_IP_ADDR_T local_addr;
    UINT16_T local_port = 0;
    OPERATE_RET ret;

    fd = tkl_net_socket_create(PROTOCOL_TCP);
    if (fd < 0) {
        LOGE("tcp_test: socket create failed, fd=%d", fd);
        return;
    }

    // 修改为目标 TCP 服务器的 IP 和端口
    TUYA_IP_ADDR_T server_addr = tkl_net_str2addr(ip);
    ret = tkl_net_connect(fd, server_addr, port);
    if (ret != OPRT_OK) {
        LOGE("tcp_test: connect failed, ret=%d", ret);
        tkl_net_close(fd);
        return;
    }

    ret = tkl_net_getsockname(fd, &local_addr, &local_port);
    if (ret != OPRT_OK) {
        LOGE("tcp_test: getsockname failed, ret=%d", ret);
        tkl_net_close(fd);
        return;
    }

    LOGI("tcp_test: local addr=%s, port=%u", tkl_net_addr2str(local_addr), local_port);
    // tkl_net_close(fd);
}
#endif 
