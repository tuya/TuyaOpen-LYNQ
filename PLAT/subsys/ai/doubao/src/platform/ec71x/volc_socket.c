#include DEBUG_LOG_HEADER_FILE
#include "volc_socket.h"

#include "errno.h"
//#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "volc_errno.h"
#include "volc_type.h"
#include "volc_memory.h"
#include "netdb.h"

static uint32_t _volc_ip_addr_to_socket_addr(const volc_ip_addr_t* p_ip_address, struct sockaddr_in* p_addr) {
    uint32_t ret = VOLC_STATUS_SUCCESS;
    VOLC_CHK(p_ip_address != NULL,VOLC_STATUS_NULL_ARG);
    if(p_ip_address->family == VOLC_IP_FAMILY_TYPE_IPV4) {
        p_addr->sin_family = AF_INET;
        p_addr->sin_port = (p_ip_address->port);
        memcpy(&p_addr->sin_addr, p_ip_address->address, VOLC_IPV4_ADDRESS_LENGTH);
    } else if(p_ip_address->family == VOLC_IP_FAMILY_TYPE_IPV6) {
        p_addr->sin_family = AF_INET6;
    } else {
        ret = VOLC_STATUS_INVALID_ARG;
    }
err_out_label:
    return ret;
}

static uint32_t _volc_ip_addr_from_socket_addr(volc_ip_addr_t* p_ip_address, const struct sockaddr_in* p_addr) {
    uint32_t ret = VOLC_STATUS_SUCCESS;
    VOLC_CHK(p_ip_address != NULL && p_addr != NULL, VOLC_STATUS_NULL_ARG);
    if(p_addr->sin_family == AF_INET) {
        p_ip_address->family = VOLC_IP_FAMILY_TYPE_IPV4;
        p_ip_address->port = (p_addr->sin_port);
        memcpy(p_ip_address->address, &p_addr->sin_addr, VOLC_IPV4_ADDRESS_LENGTH);
    } else if(p_addr->sin_family == AF_INET6) {
        p_ip_address->family = VOLC_IP_FAMILY_TYPE_IPV6;
        VOLC_CHK(0,VOLC_STATUS_FAILURE);
    } else {
        ret = VOLC_STATUS_INVALID_ARG;
    }
err_out_label:
    return ret;
}

int volc_socket (int __domain, int __type, int __protocol) {
	int domain = (__domain == VOLC_IP_FAMILY_TYPE_IPV4) ? AF_INET : AF_INET6;
    int type = (__type == VOLC_SOCK_DGRAM) ? SOCK_DGRAM : SOCK_STREAM;
	int fd = socket(domain, type, __protocol);
	ECPLAT_PRINTF(UNILOG_PLAT_VOLC,volc_socket,P_DEBUG,"fd(%d) type(%d)",fd,type);
    return fd;
}

int volc_bind (int __fd, const volc_ip_addr_t* __addr){
	struct sockaddr_in addr;
    _volc_ip_addr_to_socket_addr(__addr, &addr);
    return bind(__fd, (struct sockaddr*)&addr, sizeof(addr));
}

int volc_listen (int __fd, int __n) {
    return listen(__fd, __n);
}

int volc_accept (int __fd, volc_ip_addr_t* __addr, int * __addr_len){
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    int ret = accept(__fd, (struct sockaddr*)&addr, &addr_len);
    _volc_ip_addr_from_socket_addr(__addr, &addr);
    if (__addr_len) {
        *__addr_len = addr_len;
    }
    return ret;
};


int volc_connect (int __fd, volc_ip_addr_t* __addr) {
    uint32_t ret_status = VOLC_STATUS_SUCCESS;
    int r = 0;
    struct sockaddr_in addr = {0};
    _volc_ip_addr_to_socket_addr(__addr, &addr);
    do {
        char ip[INET_ADDRSTRLEN] = {0};
        inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
        r = connect(__fd, (struct sockaddr*)&addr, (socklen_t)sizeof(addr));
    } while (r == -1 && errno == EINTR);
    if(r == -1 && errno != EINPROGRESS)  {
        ret_status =  VOLC_STATUS_EVLOOP_PERFORM_FAILED;
    }

    if(r == -1 && errno == EINPROGRESS) {
        ret_status =  VOLC_STATUS_EVLOOP_PERFORM_NEED_RETRY;
    }

    return ret_status; 
}

ssize_t volc_recv_msg (int __fd, void* data, size_t size,volc_ip_addr_t* p_addr,uint32_t* p_status){
    uint32_t ret_status = VOLC_STATUS_SUCCESS;
    struct msghdr msg = {0};
    msg.msg_flags = 0;
    struct iovec iov ={.iov_base = data,.iov_len = size};
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
	msg.msg_name = NULL;
    struct sockaddr_storage peer;
	
    if(p_addr != NULL){
        msg.msg_name = (struct sockaddr *)&peer;
        msg.msg_namelen = sizeof(peer);
    }

    int r = 0;
	memset(&peer,0x00,sizeof(struct sockaddr_storage));
    do {
        //r = recvmsg(__fd, &msg, 0);
         r = recvfrom(__fd,msg.msg_iov->iov_base,msg.msg_iov->iov_len,0,msg.msg_name,&msg.msg_namelen);
    } while(r < 0 && errno == EINTR);
    if( r> 0) {		
        ret_status = VOLC_STATUS_SUCCESS;
        if(p_addr != NULL) {
            _volc_ip_addr_from_socket_addr(p_addr, (struct sockaddr_in *)&peer);
        }
    }else if(errno == EAGAIN || errno == EWOULDBLOCK) {
        ret_status = VOLC_STATUS_EVLOOP_PERFORM_NEED_RETRY;
    } else if(r < 0) {
        ret_status = VOLC_STATUS_EVLOOP_PERFORM_FAILED;
    }
    if(p_status  != NULL) {
        *p_status = ret_status;
    };
    return r;
};

ssize_t volc_send_msg (int __fd, void* data, size_t size ,volc_ip_addr_t* __addr, uint32_t *p_status){

    struct sockaddr_in addr;
    _volc_ip_addr_to_socket_addr(__addr, &addr);
    int r = 0;
    do {
        r =  sendto(__fd,data,size,0, (struct sockaddr*)&addr, (socklen_t)sizeof(addr));
    } while(r < 0 && errno == EINTR);

    uint32_t ret_status = VOLC_STATUS_SUCCESS;
    if(r > 0) {
        ret_status = VOLC_STATUS_SUCCESS;
    }else if(errno == EAGAIN || errno == EWOULDBLOCK) {
        ret_status = VOLC_STATUS_EVLOOP_PERFORM_NEED_RETRY;
    } else if(r < 0) {
        ret_status = VOLC_STATUS_EVLOOP_PERFORM_FAILED;
    };

    if(p_status  != NULL) {
        *p_status = ret_status;
    };
    return r;
}

int volc_close (int __fd){
	ECPLAT_PRINTF(UNILOG_PLAT_VOLC,volc_close,P_DEBUG,"__fd(%d) ",  __fd);
    return close(__fd);
};

int volc_set_nonblocking(int __fd) {
    uint32_t ret = VOLC_STATUS_SUCCESS;

    int flags = fcntl(__fd, F_GETFL, 0);
    VOLC_CHK(flags != -1, VOLC_STATUS_INVALID_ARG);
    flags = fcntl(__fd, F_SETFL, flags | O_NONBLOCK);
    VOLC_CHK(flags != -1, VOLC_STATUS_INVALID_ARG);
err_out_label:

    return ret;
}

int volc_make_pipe(int fds[2])  {
    uint32_t ret = VOLC_STATUS_SUCCESS;
    fds[0] = -1;
    fds[1] = -1;
    int write_fd = -1,read_fd = -1;

    read_fd =  socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    VOLC_CHK(read_fd >= 0, VOLC_STATUS_INVALID_ARG);

    struct sockaddr_in servaddr;
    int32_t addr_len = sizeof(servaddr);
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    servaddr.sin_port = 0;


    int bind_ret = bind(read_fd, (struct sockaddr*)&servaddr,sizeof(servaddr));
    VOLC_CHK(bind_ret >= 0,VOLC_STATUS_SOCKET_BIND_FAILED);

    int getsockname_ret = getsockname(read_fd, (struct sockaddr*)&servaddr, (socklen_t *)&addr_len);
    VOLC_CHK(getsockname_ret >= 0,VOLC_STATUS_SOCKET_GETSOCKETNAME_FAILED);


    if(servaddr.sin_addr.s_addr == htonl(INADDR_ANY))
        servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    write_fd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    int conn_ret = connect(write_fd, (struct sockaddr*)&servaddr,sizeof(servaddr));
    VOLC_CHK(conn_ret >= 0,VOLC_STATUS_INVALID_ARG);

    volc_set_nonblocking(write_fd);
    volc_set_nonblocking(read_fd);
    fds[0] = read_fd;
    fds[1] = write_fd;
	ECPLAT_PRINTF(UNILOG_PLAT_VOLC,volc_make_pipe,P_DEBUG,"read_fd[%d] write_fd[%d] ", read_fd, write_fd);
err_out_label:
    if(VOLC_STATUS_FAILED(ret)) {
        if (read_fd > 0) {
            close(read_fd);
        }
        if (write_fd > 0) {
            close(write_fd);
        }
    }
    return ret;
}

#if 1
// 自定义 poll 事件标志
#define POLLIN     0x01    /* There is data to read */
#define POLLPRI    0x02    /* There is urgent data to read */
#define POLLOUT    0x04    /* Writing now will not block */
#define POLLERR    0x08    /* Error condition */
#define POLLHUP    0x10    /* Hung up */
#define POLLNVAL   0x20    /* Invalid request: fd not open */


// 模拟 poll 函数
static int poll(struct pollfd *fds, int nfds, int timeout) {
	
    fd_set readfds, writefds, exceptfds;
    int max_fd = -1;
    int i;
    struct timeval tv;

    // 初始化文件描述符集合
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&exceptfds);

    // 根据 pollfd 结构体设置文件描述符集合
    for (i = 0; i < nfds; i++) {
        if (fds[i].fd >= 0) {
            if (fds[i].events & POLLIN) {
                FD_SET(fds[i].fd, &readfds);
            }
            if (fds[i].events & POLLOUT) {
                FD_SET(fds[i].fd, &writefds);
            }
            if (fds[i].events & POLLPRI) {
            FD_SET(fds[i].fd, &exceptfds);
        }
            if (fds[i].fd > max_fd) {
                max_fd = fds[i].fd;
            }
        }
    }

    // 设置超时时间
    if (timeout < 0) {
        tv.tv_sec = 0;
        tv.tv_usec = 0;
    } else {
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;
    }

    // 调用 select 函数
    int ret = select(max_fd + 1, &readfds, &writefds, &exceptfds,
                     (timeout < 0)? NULL : &tv);
	//ECPLAT_PRINTF(UNILOG_PLAT_VOLC,poll_1,P_DEBUG,"ret(%d),max_fd(%d)",ret,max_fd);
    if (ret > 0) {
        // 根据 select 的返回结果设置 pollfd 结构体的 revents 字段
        for (i = 0; i < nfds; i++) {
            fds[i].revents = 0;
            if (FD_ISSET(fds[i].fd, &readfds)) {
                fds[i].revents |= POLLIN;
            }
            if (FD_ISSET(fds[i].fd, &writefds)) {
                fds[i].revents |= POLLOUT;
            }
            if (FD_ISSET(fds[i].fd, &exceptfds)) {
                fds[i].revents |= POLLPRI;
            }
			//ECPLAT_PRINTF(UNILOG_PLAT_VOLC,poll_2,P_DEBUG,"fds[%d].revents(0x%x)",i,fds[i].revents);
        }
    }

    return ret;
}
#endif
int volc_poll(struct volc_pollfd *fds, int nfds, int timeout) {
    // 目前我们最多用3个，该值定义大于2即可
    struct pollfd inner_fds[10] = {0};
    int poll_ret = 0;
    for (int i = 0; i < nfds; i++) {
        inner_fds[i].fd = fds[i].fd;
        if (fds[i].events & VOLC_EVLOOP_POLLIN) {
            inner_fds[i].events |= POLLIN;
        }
        if (fds[i].events & VOLC_EVLOOP_POLLOUT) {
            inner_fds[i].events |= POLLOUT;
        }
        if (fds[i].events & VOLC_EVLOOP_POLLERR) {
            inner_fds[i].events |= POLLERR;
        }
        if (fds[i].events & VOLC_EVLOOP_POLLHUP) {
            inner_fds[i].events |= POLLHUP;
        }
    }
    
    poll_ret = poll(inner_fds, nfds, timeout);
    
    for (int i = 0; i < nfds; i++) {
        fds[i].revents = 0;
        if (inner_fds[i].revents & POLLIN) {
            fds[i].revents |= VOLC_EVLOOP_POLLIN;
        }
        if (inner_fds[i].revents & POLLOUT) {
            fds[i].revents |= VOLC_EVLOOP_POLLOUT;
        }
        if (inner_fds[i].revents & POLLERR) {
            fds[i].revents |= VOLC_EVLOOP_POLLERR;
        }
        if (inner_fds[i].revents & POLLHUP) {
            fds[i].revents |= VOLC_EVLOOP_POLLHUP;
        }
    }
    return poll_ret;
}

int volc_write (int __fd,void* data, size_t size) {
    uint32_t ret_status = VOLC_STATUS_SUCCESS;
    int r = 0;
    do {
        errno = 0;
        r = write(__fd, data,size);
    } while (r == -1 && errno == EINTR);
    if(r == size) {
        ret_status = VOLC_STATUS_SUCCESS;
    }

    if(r == -1 && errno != EINPROGRESS)  {
        ret_status =  VOLC_STATUS_EVLOOP_PERFORM_FAILED;
    }

    if(r == -1 && errno == EINPROGRESS) {
        ret_status =  VOLC_STATUS_EVLOOP_PERFORM_NEED_RETRY;
    }
    return ret_status; 
}

int volc_read (int __fd,void* data, size_t size) {
    int r = 0;
    do {
        errno = 0;
     	r = read(__fd, data,size);
    } while (r == -1 && errno == EINTR);
    return r; 
}

int volc_sockopt_set_buffer_size(int __fd, bool _is_send_buffer,int buffer_size) {
    int len = sizeof(buffer_size);
    int opt_name = SO_SNDBUF;
    if( !_is_send_buffer ) {
        opt_name = SO_RCVBUF;
    } 
    return setsockopt(__fd,SOL_SOCKET,opt_name,&buffer_size,len);
};
 
 int volc_sockopt_get_buffer_size(int __fd, bool _is_send_buffer){
    int opt_name = SO_SNDBUF;
    if( !_is_send_buffer ) {
        opt_name = SO_RCVBUF;
    } 
    int buffer_size = 0;
    int len = sizeof(buffer_size);
    return getsockopt(__fd,SOL_SOCKET,opt_name,&buffer_size,(socklen_t *)&len);
 } ;

int volc_getaddrinfo(const char* host, uint16_t port, volc_ip_addr_t** addrs, int* count) {
    int index = 0;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    struct addrinfo *res = NULL;
    struct addrinfo *cur = NULL;

    hints.ai_family     = AF_UNSPEC;
    hints.ai_socktype   = SOCK_STREAM;
    hints.ai_protocol   = IPPROTO_TCP;
    hints.ai_flags = AI_CANONNAME | AI_ADDRCONFIG;
    hints.ai_socktype = SOCK_STREAM;

    char port_str[10] = {0};
    if (NULL == count) {
        return VOLC_FAILED;
    }

    snprintf(port_str, sizeof(port_str), "%d", (int)port);
    int ret = getaddrinfo(host, port_str, &hints, &res);
    if (ret != 0) {
        return VOLC_FAILED;
    }
    cur = res;
    while(cur) {
        (*count)++;
        cur = cur->ai_next;
    }
    *addrs = (volc_ip_addr_t*)volc_malloc(sizeof(volc_ip_addr_t) * (*count));
    cur = res;
    while (cur) {
        _volc_ip_addr_from_socket_addr(&(*addrs)[index], (struct sockaddr_in*)cur->ai_addr);
        index++;
        cur = cur->ai_next;
    }
	freeaddrinfo(res);
    return VOLC_SUCCESS;
}

int volc_freeaddrinfo(volc_ip_addr_t* addrs) {
	if (addrs) {
	volc_free(addrs);
	}
	return 0;
}

