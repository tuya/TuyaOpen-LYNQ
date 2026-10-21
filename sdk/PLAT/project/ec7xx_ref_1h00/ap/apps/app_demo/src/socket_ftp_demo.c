#include "string.h"
#include "bsp.h"
#include "osasys.h"
#include "cmsis_os2.h"
#include "debug_trace.h"
#include "sockets.h"
#include "netdb.h"
#include "ec_tcpip_api.h"
#include "ol_log.h"
#include "ol_fs_api.h"

#define FTP_TEST_IP_ADDR    "220.167.54.26"
#define FTP_TEST_USER       "ftp123"
#define FTP_TEST_PASSWD     "ftp123"
#define FTP_TEST_PORT       5021
#define FTP_TMP_BUFSIZ      1024
#define FTP_TEST_FILE       "fs_test.txt"
#define FTP_TEST_SRV_FILE   "/ftp_test.txt"

int ftp_connect_server(char* srv_addr, int srv_port)
{
    struct hostent* host = NULL;
    struct sockaddr_in sockaddr;
    int sockfd = -1;

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sockfd < 0)
    {
        OL_LOG_PRINTF("ftp_connect_server：socket fail %d", sockfd);
        return -1;
    }

    host = gethostbyname(srv_addr);
    if(host == NULL)
    {
        OL_LOG_PRINTF("ftp_connect_server：gethostbyname NULL");
        return -1;
    }

    memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sin_port = htons(srv_port);
    sockaddr.sin_family = AF_INET;
    memcpy(&(sockaddr.sin_addr.s_addr), host->h_addr, host->h_length);

    int ret = connect(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr));
    if(0 != ret)
    {
        OL_LOG_PRINTF("ftp_connect_server：connect fail %d", ret);
        return -1;
    }

    OL_LOG_INFO("ftp connect success! sockfd = %d", sockfd);

    return sockfd;
}

static int ftp_send_cmd(char* cmd, char* str, int sockfd)
{
    int ret = 0;
    char buf[FTP_TMP_BUFSIZ];

    memset(buf, 0, sizeof(buf));
    if(str == NULL)
    {
        snprintf(buf, FTP_TMP_BUFSIZ, "%s\r\n", cmd);
    }
    else
    {
        snprintf(buf, FTP_TMP_BUFSIZ, "%s %s\r\n", cmd, str);
    }

    ret = send(sockfd, buf, strlen(buf), 0);
    OL_LOG_INFO("ftp_send_cmd: %s, ret = %d", buf, ret);

    return ret;
}

int ftp_cmd_resp(int sockfd)
{
    int ret = 0;
    char buf[FTP_TMP_BUFSIZ];

    memset(buf, 0, sizeof(buf));
    ret = recv(sockfd, buf, sizeof(buf), 0);
    OL_LOG_INFO("ftp_get_resp: ret:%d, buf:%s", ret, buf);

    if(ret > 0)
    {
        sscanf(buf, "%d", &ret);
    }

    return ret;
}

void mbtk_recv_file(int sockfd)
{
    int rsize = 0;
    int wsize = 0;
    OLFILE fp = NULL;
    char temp_buf[256] = {0};

    fp = ol_fs_open(FTP_TEST_FILE, "wb+");
    if(NULL == fp)
    {
        OL_LOG_PRINTF("open local file:%s fail", FTP_TEST_FILE);
        return;
    }

    while(1)
    {
        memset(temp_buf, 0, sizeof(temp_buf));

        rsize = recv(sockfd, temp_buf, sizeof(temp_buf), 0);
        if(rsize <= 0)
        {
            OL_LOG_PRINTF("recv file end: r - %d", rsize);
            break;
        }

        wsize = ol_fs_write(temp_buf, sizeof(temp_buf), fp);
        if(wsize < rsize)
        {
            OL_LOG_PRINTF("ol_fs_write file lost data: rsize[%d] wsize[%d]", rsize, wsize);
            break;
        }
    }

    ol_fs_close(fp);
}


void ftp_download(int sockfd)
{
    int ret = 0;
    int resp_num = 0;
    int data_sockfd = -1;
    int data_port = -1;
    int addr_buf[12];
    char resp_buf[FTP_TMP_BUFSIZ];

    memset(resp_buf, 0, sizeof(resp_buf));
    memset(addr_buf, 0, sizeof(addr_buf));

    //send TYPE
    ret = ftp_send_cmd("TYPE", "I", sockfd);
    if(ret < 0)
    {
        OL_LOG_PRINTF("ftp_send_cmd TYPE fail!");
        return;
    }

    if(200 != ftp_cmd_resp(sockfd))
    {
        return;
    }

    OL_LOG_INFO("ftp_send_cmd TYPE success");

    //send PASV
    ret = ftp_send_cmd("PASV", NULL, sockfd);
    if(ret < 0)
    {
        OL_LOG_PRINTF("ftp_send_cmd PASV fail!");
        return;
    }

    ret = recv(sockfd, resp_buf, sizeof(resp_buf), 0);
    OL_LOG_INFO("ftp_get_resp: ret:%d, buf:%s", ret, resp_buf);

    if(resp_buf[0] != '2')
    {
        OL_LOG_PRINTF("ftp_send_cmd PASV resp error");
        return;
    }

    sscanf((char*)resp_buf, "%*[^(](%u,%u,%u,%u,%u,%u)", \
           &addr_buf[0], &addr_buf[1], &addr_buf[2], \
           &addr_buf[3], &addr_buf[4], &addr_buf[5]);

    data_port = (addr_buf[4] * 256 + addr_buf[5]);
    OL_LOG_INFO("ftp_send_cmd PASV success, data_port = %d", data_port);

    data_sockfd = ftp_connect_server(FTP_TEST_IP_ADDR, data_port);
    if(-1 == data_sockfd)
    {
        return;
    }

    //send RETR
    ret = ftp_send_cmd("RETR", FTP_TEST_SRV_FILE, sockfd);
    if(ret < 0)
    {
        OL_LOG_PRINTF("ftp_send_cmd STOR fail!");
        close(data_sockfd);
        return;
    }

    resp_num = ftp_cmd_resp(sockfd);
    if(125 != resp_num && 150 != resp_num)
    {
        close(data_sockfd);
        return;
    }

    OL_LOG_INFO("ftp_send_cmd RETR success");

    mbtk_recv_file(data_sockfd);

    close(data_sockfd);
}

void mbtk_send_file(int sockfd)
{
    int rsize = 0;
    int wsize = 0;
    OLFILE fp = NULL;
    char temp_buf[256] = {0};

    fp = ol_fs_open(FTP_TEST_FILE, "rb+");
    if(NULL == fp)
    {
        OL_LOG_PRINTF("open local file:%s fail", FTP_TEST_FILE);
        return;
    }

    while(1)
    {
        memset(temp_buf, 0, sizeof(temp_buf));
        rsize = ol_fs_read(temp_buf, sizeof(temp_buf), fp);
        if(rsize <= 0)
        {
            OL_LOG_PRINTF("send file end: r - %d", rsize);
            break;
        }

        wsize = send(sockfd, temp_buf, rsize, 0);
        if(wsize < rsize)
        {
            OL_LOG_PRINTF("send file lost data: rsize[%d] wsize[%d]", rsize, wsize);
            break;
        }
    }

    ol_fs_close(fp);
}

void ftp_upload(int sockfd)
{
    int ret = 0;
    int resp_num = 0;
    int data_sockfd = -1;
    int data_port = -1;
    int addr_buf[12];
    char resp_buf[FTP_TMP_BUFSIZ];

    memset(resp_buf, 0, sizeof(resp_buf));
    memset(addr_buf, 0, sizeof(addr_buf));

    //send TYPE
    ret = ftp_send_cmd("TYPE", "I", sockfd);
    if(ret < 0)
    {
        OL_LOG_PRINTF("ftp_send_cmd TYPE fail!");
        return;
    }

    if(200 != ftp_cmd_resp(sockfd))
    {
        return;
    }

    OL_LOG_INFO("ftp_send_cmd TYPE success");

    //send PASV
    ret = ftp_send_cmd("PASV", NULL, sockfd);
    if(ret < 0)
    {
        OL_LOG_PRINTF("ftp_send_cmd PASV fail!");
        return;
    }

    ret = recv(sockfd, resp_buf, sizeof(resp_buf), 0);
    OL_LOG_INFO("ftp_get_resp: ret:%d, buf:%s", ret, resp_buf);

    if(resp_buf[0] != '2')
    {
        OL_LOG_PRINTF("ftp_send_cmd PASV resp error");
        return;
    }

    sscanf((char*)resp_buf, "%*[^(](%u,%u,%u,%u,%u,%u)", \
           &addr_buf[0], &addr_buf[1], &addr_buf[2], \
           &addr_buf[3], &addr_buf[4], &addr_buf[5]);

    data_port = (addr_buf[4] * 256 + addr_buf[5]);
    OL_LOG_INFO("ftp_send_cmd PASV success, data_port = %d", data_port);

    data_sockfd = ftp_connect_server(FTP_TEST_IP_ADDR, data_port);
    if(-1 == data_sockfd)
    {
        return;
    }

    //send STOR
    ret = ftp_send_cmd("STOR", FTP_TEST_SRV_FILE, sockfd);
    if(ret < 0)
    {
        OL_LOG_PRINTF("ftp_send_cmd STOR fail!");
        close(data_sockfd);
        return;
    }

    resp_num = ftp_cmd_resp(sockfd);
    if(125 != resp_num && 150 != resp_num && 226 != resp_num)
    {
        close(data_sockfd);
        return;
    }

    OL_LOG_INFO("ftp_send_cmd STOR success");

    mbtk_send_file(data_sockfd);

    close(data_sockfd);
}


int ftp_login(int sockfd)
{
    int ret = 0;

    // send USER
    ret = ftp_send_cmd("USER", FTP_TEST_USER, sockfd);
    if(ret < 0)
    {
        OL_LOG_PRINTF("ftp_send_cmd USER fail!");
        return -1;
    }

    if(331 != ftp_cmd_resp(sockfd))
    {
        return -1;
    }

    OL_LOG_INFO("ftp_send_cmd USER success");

    //send PASS
    ret = ftp_send_cmd("PASS", FTP_TEST_PASSWD, sockfd);
    if(ret < 0)
    {
        OL_LOG_PRINTF("ftp_send_cmd PASS fail!");
        return -1;
    }

    if(230 != ftp_cmd_resp(sockfd))
    {
        return -1;
    }

    OL_LOG_INFO("ftp_send_cmd PASS success");

    return 0;
}

void ftp_demo(void)
{
    int ctrl_sockfd = -1;

    ctrl_sockfd = ftp_connect_server(FTP_TEST_IP_ADDR, FTP_TEST_PORT);
    if(-1 == ctrl_sockfd)
    {
        return;
    }

    if(220 != ftp_cmd_resp(ctrl_sockfd))
    {
        OL_LOG_PRINTF("ftp: get welcome msg error!");
        close(ctrl_sockfd);
        return;
    }

    OL_LOG_INFO("ftp welcome success !");

    if(0 == ftp_login(ctrl_sockfd))
    {
        if(1)
        {
            ftp_upload(ctrl_sockfd);
        }
        else
        {
            ftp_download(ctrl_sockfd);
        }
    }

    close(ctrl_sockfd);
}
