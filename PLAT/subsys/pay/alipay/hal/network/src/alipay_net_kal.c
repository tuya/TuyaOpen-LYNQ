#include "alipay_common.h"

#include <time.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "netdb.h"
#include "sockets.h"
#include "alipay_net_kal.h"

/*
* @brief: 获取UTC时间
* note:  1.精确级别,微妙级别
*        2.受系统时间修改影响
*        3.返回的秒数是从1970年1月1日0时0分0秒开始
*/
void alipay_iot_gettimeofday(alipay_iot_timeval *now){
	utc_timer_value_t utcTime;
    appGetSystemTimeUtcSync(&utcTime);
	now->tv_sec = utcTime.UTCsecs;
    now->tv_usec = (utcTime.UTCms * 1000) % 1000000;
	//alipay_log_ext("alipay_iot_gettimeofday: sec:%d,usec:%d",now->tv_sec, now->tv_usec);
}

/*
* @brief: 使任务休眠milliseconds毫秒
* @input: milliseconds 休眠的时间，单位毫秒
*/
void alipay_task_sleep(uint32_t ms){
    struct timeval tval;
    tval.tv_sec = ms / 1000;
    tval.tv_usec = (ms * 1000) % 1000000;
    select(0, NULL, NULL, NULL, &tval);
}


typedef void * alipay_iot_mutex;
/*
* @brief: 创建互斥量
* @input: mutex_name 互斥量的名称
* @return: 其他 创建成功
*          NULL 创建失败
*/
alipay_iot_mutex alipay_iot_mutex_create(const char* mutex_name){
    alipay_iot_mutex mutex = (alipay_iot_mutex)osMutexNew(NULL);
    if (!mutex) {
        return NULL;
    }
	return mutex;
}

/*
* @brief: 对互斥量加锁
* @input: mutex 要加锁的互斥量
* @return: 0 加锁成功
*          -1 加锁失败
*/
int alipay_iot_mutex_lock(alipay_iot_mutex mutex){
	int ret = -1;
    if (!mutex) {
        return ret;
    }
	ret = osMutexAcquire((osMutexId_t)mutex, osWaitForever);
	return (ret == osOK ? ret : -1);
}

/*
* @brief: 对互斥量解锁
* @input: mutex 要解锁的互斥量
* @return: 0 解锁成功
*          -1 解锁失败
*/
int alipay_iot_mutex_unlock(alipay_iot_mutex mutex){
  int ret = -1;
    if (!mutex) {
        //printf("alipay_iot_mutex_unlock: Mutex is NULL\n");
        return ret;
    }
	ret = osMutexRelease((osMutexId_t)mutex);
	return (ret == osOK ? ret : -1);
}

/*
* @brief: 删除互斥量
* @input: mutex 要删除的互斥量
* @return: 0 删除成功
*          -1 删除失败
*/
int alipay_iot_mutex_delete(alipay_iot_mutex mutex){
	int ret = -1;
    if (!mutex) {
        //printf("alipay_iot_mutex_delete: Mutex is NULL\n");
        return ret;
    }

    ret = osMutexDelete((osMutexId_t)mutex);
    return (ret == osOK ? ret : -1);

}

/*************************socket adapt*************************/
static bool alipay_socket_init_flag = false;
static int32_t alipay_sockets[ALIPAY_IOT_MAX_IP_SOCKET_NUM] = {-1};
// static aos_mutex_t alipay_socket_lock;
static alipay_iot_mutex alipay_socket_lock = NULL;

static alipay_iot_mutex get_mtx()
{
    static alipay_iot_mutex s_mtx_socket;
    static bool s_isinitialized = false;
    if (!s_isinitialized) {
        s_mtx_socket = alipay_iot_mutex_create("socket_mutex");
        s_isinitialized = true;
    }
    return s_mtx_socket;
}

static int alipay_iot_fd_init()
{
    int ret = 0;

    if (alipay_socket_init_flag == true) {
        //printf("alipay fd have already inited, should be called twice\n");
        return -1;
    }

    memset(alipay_sockets, -1, sizeof(alipay_sockets));
    alipay_socket_init_flag = true;
    return ret;
}

static int alloc_alipay_socket(int origin_fd)
{
    int i;

    if (alipay_socket_init_flag != true) {
        alipay_iot_fd_init();
        // //printf("alipay fd haven't init yet\n");
        // return -1;
    }
    
    alipay_iot_mutex_lock(get_mtx());
    
    for (i = 0; i < ALIPAY_IOT_MAX_IP_SOCKET_NUM; i++) {
        if (alipay_sockets[i] == -1) {
            alipay_sockets[i] = origin_fd;
            alipay_iot_mutex_unlock(get_mtx());
            return i;
        }
    }

    alipay_iot_mutex_unlock(get_mtx());
    return -1;
}

static int free_alipay_socket(int s)
{
    if (alipay_socket_init_flag != true) {
        // //printf("alipay fd haven't init yet\n");
        // return -1;
        alipay_iot_fd_init();
    }

    if (s >= ALIPAY_IOT_MAX_IP_SOCKET_NUM || s < 0) {
        return -1;
    }

    alipay_iot_mutex_lock(get_mtx());
    alipay_sockets[s] = -1;
    alipay_iot_mutex_unlock(get_mtx());
    
    return 0;
}

static int get_origin_fd_by_alipay_socket(int socket)
{
    int fd = -1;
    if (alipay_socket_init_flag != true) {
        // //printf("alipay fd haven't init yet\n");
        // return -1;
        alipay_iot_fd_init();
    }
    
    if (socket >= ALIPAY_IOT_MAX_IP_SOCKET_NUM || socket < 0){
        //printf("invalid input socket %d\n", socket);
        return -1;
    }

    alipay_iot_mutex_lock(get_mtx());
    fd = alipay_sockets[socket];
    alipay_iot_mutex_unlock(get_mtx());
    return fd;
}

static void alipay_iot_fd_setResultbit(int fd, alipay_iot_fd_set* fdset)
{
    if(fdset != NULL && fd >=0 && fd < ALIPAY_IOT_MAX_IP_SOCKET_NUM )
    {
        fdset->fds_bits[fd] |= 0x02;
    }
}

static int alipay_iot_fd_check(int fd, alipay_iot_fd_set* fdset)
{
    if(fdset != NULL && fd >=0 && fd < ALIPAY_IOT_MAX_IP_SOCKET_NUM )
    {
        return fdset->fds_bits[fd] & 0x01;
    }
    return 0;
}

int alipay_iot_fd_isset(int fd, alipay_iot_fd_set* fdset)
{
    //printf("isset fd: %d\n", fd);

    if(fdset != NULL && fd >=0 && fd < ALIPAY_IOT_MAX_IP_SOCKET_NUM )
    {
        return fdset->fds_bits[fd] & 0x02;
    }
    return 0;
}

void alipay_iot_fd_setbit(int fd, alipay_iot_fd_set* fdset)
{
    //printf("setbit fd: %d\n", fd);

    if(fdset != NULL && fd >=0 && fd < ALIPAY_IOT_MAX_IP_SOCKET_NUM )
    {
        fdset->fds_bits[fd] |= 0x01;
    }
}

void alipay_iot_fd_zero(alipay_iot_fd_set* fdset)
{
    if(fdset != NULL)
    {
        memset(fdset, 0, sizeof(alipay_iot_fd_set));
    }
}

int alipay_iot_select(int maxfdp1,
                      alipay_iot_fd_set* readset,
                      alipay_iot_fd_set* writeset,
                      alipay_iot_fd_set* exceptset,
                      alipay_iot_timeval* timeout)
{
    fd_set  set_r, set_w, set_e;
    int i = 0;
    int origin_fd = -1;
    int origin_max = -1;
    int count = 0;
    
    FD_ZERO(&set_r);
    FD_ZERO(&set_w);
    FD_ZERO(&set_e);

    if(maxfdp1 < 0)
    {
        //printf("maxfdp1 error %d\n", maxfdp1);
        return -1;
    }

    //printf("maxfdp1 is %d, timeout sec %d, usec %d\r\n ", maxfdp1, timeout->tv_sec, timeout->tv_usec);
    for(i = 0; i < maxfdp1 + 1; i++)
    {
        origin_fd = get_origin_fd_by_alipay_socket(i);
        if (origin_fd == -1) {
            //printf("fail to get alipay sock %d origin sock\r\n", i);
            continue;
        }
        
        if (origin_fd > origin_max - 1) {
            origin_max = origin_fd + 1;
        }
        
        if(alipay_iot_fd_check(i, readset))
        {
            //printf("set fd %d read\n", origin_fd);
            FD_SET(origin_fd, &set_r);
        }

        if(alipay_iot_fd_check(i, writeset))
        {
            //printf("set fd %d write\n", origin_fd);
            FD_SET(origin_fd, &set_w);
        }

        if(alipay_iot_fd_check(i, exceptset))
        {
            //printf("set fd %d except\n", origin_fd);
            FD_SET(origin_fd, &set_e);
        }
        
        //printf("maxfp1 is %d , i is %d, fd is %d maxfd is %d\n",maxfdp1, i, origin_fd, origin_max);
    }

    struct timeval iTimeout = {0};
    if (timeout != NULL) {
        iTimeout.tv_sec = timeout->tv_sec;
        iTimeout.tv_usec = timeout->tv_usec;
    }
    count = select(origin_max, &set_r, &set_w, &set_e, &iTimeout);
    //printf("select return count is 0x%x\n", count);

    for(i = 0; i < maxfdp1 + 1; i++)
    {
        origin_fd = get_origin_fd_by_alipay_socket(i);
        if (origin_fd == -1) {
            continue;
        }
        
        if(FD_ISSET(origin_fd, &set_r))
        {
            alipay_iot_fd_setResultbit(i, readset);
        }
        if(FD_ISSET(origin_fd, &set_w))
        {
            alipay_iot_fd_setResultbit(i, writeset);
        }
        if(FD_ISSET(origin_fd, &set_e))
        {
            alipay_iot_fd_setResultbit(i, exceptset);
        }
    }

    return count;
}

int alipay_iot_dns(const char *name, unsigned char ip[4]) {
    struct hostent *he;
	struct in_addr **addr_list;

    if ((he = gethostbyname(name)) == NULL) return -1;

	addr_list = (struct in_addr **) he->h_addr_list;
    if (addr_list[0] == NULL) return -1;

    char *p_tmp = inet_ntoa(*addr_list[0]);
    sscanf(p_tmp, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
    return 0;
}


int alipay_iot_socket_create(int domain, alipay_socket_type_enum type, int protocol) {
    int socket_type = -1;
    int fd = -1;
	int socket_protocol;
    switch (type) {
        case ALIPAY_IOT_SOC_SOCK_STREAM: {
            socket_type = SOCK_STREAM;
			socket_protocol = IPPROTO_TCP;
            break;
        }
        case ALIPAY_IOT_SOC_SOCK_DGRAM: {
            socket_type = SOCK_DGRAM;
			socket_protocol = IPPROTO_UDP;
            break;
        }
        default: {
            return -1;
        }
    }

    int socket_fd = -1;
    if ((socket_fd = socket(PF_INET, socket_type, socket_protocol)) < 0)
      return -1;

    //printf("socket_fd: %d\n", socket_fd);
    fd = alloc_alipay_socket(socket_fd); 
    if (fd == -1) {
        //printf("fail to create alipay socket 0x%x\n", fd);
        close(socket_fd);
        return -1;
    }

    //printf("fd: %d\n", fd);
    return fd;
}

int alipay_iot_socket_close(int s) {
    int fd = -1;
    int ret  = -1;
    
    fd = get_origin_fd_by_alipay_socket(s); 
    if (fd == -1) {
        return -1;
    }

    ret = close(fd);

    ret |= free_alipay_socket(s);
    if (ret != 0) {
        return -1;
    }

    return ret;
}


int alipay_iot_socket_bind(int                        s,
                    const struct alipay_iot_sockaddr *name,
                    unsigned int               namelen) {
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(name->data.sin_data.port);
    memcpy(&serv_addr.sin_addr, name->data.sin_data.ip, 4);

    int fd = -1;
    fd = get_origin_fd_by_alipay_socket(s); 
    if (fd == -1) {
        //printf("fail to connect alipay sock %d\n", s);
        return -1;
    }
    int val = bind(fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (val < 0) {
        //printf("alipay_iot_socket_bind failed, %d", val);
        return -1;
    }

    return 0;
}

int alipay_iot_socket_connect(int                            s,
                        const struct alipay_iot_sockaddr *name,
                        unsigned int                    namelen) {
    struct sockaddr_in serv_addr; 
    serv_addr.sin_family = AF_INET;
	serv_addr.sin_port=htons(443);

    memcpy(&serv_addr.sin_addr, name->data.sin_data.ip, 4);
    
    char str[100] = {0};
    inet_ntop(AF_INET, &(serv_addr.sin_addr), str, INET_ADDRSTRLEN);
    //printf("ip address: %s\n", str);
    //printf("port: %d - %d\n", serv_addr.sin_port, name->data.sin_data.port);

    int fd = -1;
    fd = get_origin_fd_by_alipay_socket(s);
    if (fd == -1) {
        //printf("fail to connect alipay sock %d\n", s);
        return -1;
    }
	int flags = fcntl( fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags|O_NONBLOCK);
    if (connect(fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <0)
	{
		fcntl(fd, F_SETFL, flags&~O_NONBLOCK);
  		return -1;
	}
	fcntl(fd, F_SETFL, flags&~O_NONBLOCK);
    return 0;
}

int alipay_iot_socket_sendto(int                           s,
                        const void                  *dataptr,
                        int                          size,
                        int                            flags,
                        const struct alipay_iot_sockaddr *to,
                        unsigned int                    tolen) {
    //printf("send socket fd: %d\n", s);
    struct sockaddr_in server_addr;
    server_addr.sin_family = to->sa_family;
    server_addr.sin_port = htons(443);
    // server_addr.sin_port = htons(to->data.sin_data.port);
    //printf("[test]sin_port %d\n",server_addr.sin_port);
    memcpy(&server_addr.sin_addr, to->data.sin_data.ip, 4);

    int fd = -1;
    fd = get_origin_fd_by_alipay_socket(s);
    if (fd == -1) {
        //printf("fail to connect alipay sock %d\n", s);
        return -1;
    }
    int bytes_send = sendto(fd, dataptr, size, 0, &server_addr, sizeof(server_addr));
        //printf("bytes send: %d\n", bytes_send);    
    return bytes_send;
}

int alipay_iot_socket_write(int s, const void *dataptr, int len) {
        //alipay_log_ext("write fd: %d - len: %d\n", s, len);
    int fd = -1;
    fd = get_origin_fd_by_alipay_socket(s);
    if (fd == -1) {
    	//alipay_log_ext("fail to connect alipay sock %d\n", s);
        return -1;
    }
    return write(fd, dataptr, len);
}

int alipay_iot_socket_recvfrom(int                     s,
                        void                        *mem,
                        int                       len,
                        int                        flags,
                        struct alipay_iot_sockaddr *from,
                        unsigned int            *fromlen) {
	//alipay_log_ext("socket recvfrom: %d\n", s);
    int bytes_received = -1;
    
    int fd = -1;
    fd = get_origin_fd_by_alipay_socket(s);
    if (fd == -1) {
    	//alipay_log_ext("fail to connect alipay sock %d\n", s);
        return -1;
    }

    if (from) {
        struct sockaddr_in server_addr;
        server_addr.sin_family = from->sa_family;
            /* server_addr.sin_port = htons(443); */
		//alipay_log_ext("alipay_iot_socket_recvfrom,port is %d",from->data.sin_data.port);
        server_addr.sin_port = htons(from->data.sin_data.port);
        memcpy(&server_addr.sin_addr, from->data.sin_data.ip, 4);
    
        int addrlen = sizeof(server_addr);
        bytes_received = recvfrom(fd, mem, len, flags, &server_addr, &addrlen);
            //alipay_log_ext("bytes_received: %d\n", bytes_received);
    } else {
        fcntl(fd, F_SETFL, fcntl(fd, F_GETFL,0) | O_NONBLOCK);
        bytes_received = recv(fd, mem, len, flags);
            //alipay_log_ext("bytes_received: %d\n", bytes_received);
    }
            
    return bytes_received;
}

int alipay_iot_socket_read(int s, void *mem, int len) {
        //alipay_log_ext("read fd: %d - len: %d\n", s, len);
    int n = alipay_iot_socket_recvfrom(s, mem, len, 0, NULL, NULL);
        /* int n =  read(s, mem, len); */
        //alipay_log_ext("socket read %d\n", n);
    if (n <= 0 || n > len)
      return -2;

    return n;
}


int alipay_iot_socket_setsockopt(int          s,
                          int          level,
                          int          optname,
                          const void  *opval,
                          unsigned int optlen) {

	//return setsockopt(s, level, optname, opval, optlen);
	//alipay_log_ext("alipay_iot_socket_setsockopt,optname %d",optname);
	//setsockopt(s, level, optname, opval, optlen);
	return 0;
}

int alipay_iot_socket_getsockopt(int         s,
                          int            level,
                          int          optname,
                          void          *opval,
                          unsigned int *optlen) {
    //return getsockopt(s, level, optname, opval, optlen);
    //alipay_log_ext("alipay_iot_socket_getsockopt,optname %d",optname);
    return 0;
}

