/****************************************************************************
 *
 ****************************************************************************/
#include "string.h"
#include "bsp.h"
#include "osasys.h"
#include "cmsis_os2.h"
#include "sockets.h"
#include "netdb.h"
#include "ec_tcpip_api.h"
#include "ol_log.h"
#include "ol_tcp_api.h"


#define IP_ADDR "118.114.239.159"
#define PORT 30165

int socket = 0;


int socket_select(int     fd, UINT32 timeout)
{
    fd_set readSet;
    fd_set writeSet;
    fd_set errorSet;
    FD_ZERO(&readSet);
    FD_ZERO(&writeSet);
    FD_ZERO(&errorSet);
    FD_SET(fd, &readSet);
    FD_SET(fd, &writeSet);
    FD_SET(fd, &errorSet);
    struct timeval tv;
    tv.tv_sec  = timeout;
    tv.tv_usec = 0;
    INT32 ret;

    ret = select(fd + 1, &readSet, &writeSet, &errorSet, &tv);
    if(ret < 0)
    {
        close(fd);
        OL_LOG_PRINTF("select fail");
        return -1;
    }
    else if(ret == 0)
    {
        close(fd);
        OL_LOG_PRINTF("select timeout");
        return -1;
    }
    else
    {
        if(FD_ISSET(fd, &errorSet))
        {
            int merr = sock_get_errno(fd);
            OL_LOG_PRINTF("sock_get_errno = %d", merr);
            if(merr)
            {
                close(fd);
                return -1;
            }
        }
        else if(FD_ISSET(fd, &writeSet))
        {
            OL_LOG_PRINTF("writeSet success");
            return 2;//write event(user-defined)
        }
        else if(FD_ISSET(fd, &readSet))
        {
            OL_LOG_PRINTF("readSet success");
            return 1;//read event(user-defined)
        }
    }

    return -1;
}

void dns_pars(void)
{
    int ret = 0;
    struct addrinfo hints = {0};
    struct addrinfo* res, *info;
    struct sockaddr_in* addr_in;
    struct sockaddr_in6* addr_in6;

    memset(&hints, 0x0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    ret = getaddrinfo("www.baidu.com", NULL, &hints, &res);
    if(ret != 0)
    {
        OL_LOG_PRINTF("getaddrinfo error");
        return;
    }

    for(info = res; info != NULL; info = info->ai_next)
    {
        OL_LOG_PRINTF("getaddrinfo family = %d", info->ai_family);
        if(info->ai_family == AF_INET)
        {
            addr_in = (struct sockaddr_in*)info->ai_addr;
            OL_LOG_PRINTF("IPV4 = %d", addr_in->sin_addr.s_addr);
            OL_LOG_PRINTF("IPV4 = %s",inet_ntoa(addr_in->sin_addr.s_addr));
            freeaddrinfo(res);
            return;
        }
        else if(info->ai_family == AF_INET6)
        {
            addr_in6 = (struct sockaddr_in6*)info->ai_addr;
            OL_LOG_PRINTF("IPV6 = %s", addr_in6->sin6_addr.un.u32_addr);
            OL_LOG_PRINTF("IPV6 = %s",inet6_ntoa(addr_in6->sin6_addr.un.u32_addr));
            freeaddrinfo(res);
            return;
        }
    }
}

void test_socket_tcp_client(void)
{
    int ret = 0;
    struct sockaddr_in addr;
    struct sockaddr_in6 servaddr6;
    ip6_addr_t v6;
    int errcode = 0;
    char buf[64] = {0};
    int IPV4 = 1;

    if(IPV4)
    {
        socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    }
    else
    {
        socket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    }

    OL_LOG_PRINTF("socket = %d", socket);
    if(socket < 0)
    {
        OL_LOG_PRINTF("socket error");
        return;
    }

    dns_pars();

    if(IPV4)
    {
        memset(&addr, 0x0, sizeof(struct sockaddr_in));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(PORT);
        inet_aton(IP_ADDR, &addr.sin_addr.s_addr);
        addr.sin_len = sizeof(struct sockaddr_in);
    }
    else
    {
        memset(&servaddr6, 0x0, sizeof(struct sockaddr_in6));
        memset(&v6, 0x0, sizeof(ip6_addr_t));
        servaddr6.sin6_family = AF_INET6;
        servaddr6.sin6_port = htons(PORT);
        inet6_aton("0:0:0:0:0:0:0:1", &v6);
        memcpy(&servaddr6.sin6_addr.un.u32_addr, &v6, sizeof(v6));
        servaddr6.sin6_len = sizeof(struct sockaddr_in6);
    }

    if(IPV4)
    {
        ret = connect(socket, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
    }
    else
    {
        ret = connect(socket, (struct sockaddr*)&servaddr6, sizeof(struct sockaddr_in6));
    }

    if(ret != 0)
    {
        errcode = sock_get_errno(socket);
        if(errcode == EINPROGRESS)
        {
            socket_select(socket, 9);
        }
    }

    ret = send(socket, "I am client!", 12, 0);
    OL_LOG_INFO("send = %d", ret);

    ret = recv(socket, buf, sizeof(buf), 0);
    OL_LOG_INFO("recv ret = %d, buf = %s", ret, buf);

    ret = close(socket);
}

void test_socket_udp_client(void)
{
    int ret = 0;
    struct sockaddr_in addr;
    struct sockaddr_in addr_udp;
    socklen_t addr_len = 0;
    char buf[64] = {0};

    socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(socket < 0)
    {
        OL_LOG_PRINTF("socket error");
        return;
    }

    memset(&addr, 0x0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_aton(IP_ADDR, &addr.sin_addr.s_addr);
    addr.sin_len = sizeof(struct sockaddr_in);

    ret = sendto(socket, "I am UDP client!", 16, 0, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
    OL_LOG_INFO("sendto = %d", ret);

    memset(&addr_udp, 0x0, sizeof(addr_udp));
    addr_len = sizeof(struct sockaddr_in);
    ret = recvfrom(socket, buf, sizeof(buf), 0, (struct sockaddr*)&addr_udp, &addr_len);
    OL_LOG_INFO("recv ret = %d, buf = %s", ret, buf);

    ret = close(socket);
}

void socket_demo(void)
{
    if(1)
    {
        test_socket_tcp_client();
    }
    else
    {
        test_socket_udp_client();
    }

}

