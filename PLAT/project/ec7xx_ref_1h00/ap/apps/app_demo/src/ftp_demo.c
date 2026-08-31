/****************************************************************************
 *
 ****************************************************************************/
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
#include "ol_ftp_api.h"


#define SERVER_IP       "220.167.54.26"
#define SERVER_PORT     5021
#define FTP_USER       "ftp123"
#define FTP_PASSWD     "ftp123"
#define FTP_SSL_MODE    2

#define TEST_DIR        "/ftp_test_dir"
#define TEST_FILE       "ftp_test_file.txt"

char ftp_ca[] =
"-----BEGIN CERTIFICATE-----\r\n"
"MIIDKjCCAhICCQCOewfZiRCiNjANBgkqhkiG9w0BAQUFADBXMQswCQYDVQQGEwJD\r\n"
"TjEQMA4GA1UECBMHU2lDaHVhbjEVMBMGA1UEChMMTU9CSUxFVEVLLkNBMQswCQYD\r\n"
"VQQLEwJJVDESMBAGA1UEAxMJTU9CSUxFVEVLMB4XDTE4MDkxODA4MDUzMloXDTMz\r\n"
"MDkxOTA4MDUzMlowVzELMAkGA1UEBhMCQ04xEDAOBgNVBAgTB1NpQ2h1YW4xFTAT\r\n"
"BgNVBAoTDE1PQklMRVRFSy5DQTELMAkGA1UECxMCSVQxEjAQBgNVBAMTCU1PQklM\r\n"
"RVRFSzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAOkdYJF1h1xjKbY0\r\n"
"ipbl88G653PiCh8ZMjmIUYeiDKC8+0wtXZtHvQIl6AncOzBy9XHVOctbKn34exC8\r\n"
"SEotMuo2T49vs9VtE8GYu2pOrf3m42NpLRnYAxfm9qw53CMHx+Jn7Oa9fnxa8haA\r\n"
"pRc2BTVadWGoS8EEwoZFk0eNb7Z2Gc7U0c+GhISI4oVTTocGvGgMzkvduu5JJbbc\r\n"
"BOcNFrii9sRO9vtOYQtqOEg01Uum2Dwp/o2bDLXNJEqAIh4WACiM4iPmmlRHWT2y\r\n"
"NjQ3vcbEdrFwbHRtO46+Vw54HnSyCoFb3uCHMNMvXObZ/8AU9E3Cgat4j0sgEeB0\r\n"
"hqA4MiMCAwEAATANBgkqhkiG9w0BAQUFAAOCAQEAtEAjf0CjsLgG9ROdmp1qXYft\r\n"
"+ndIT5l82KRK57ZQsfdFbnJOvALeF/ICKU0M2TXgJNiGOA5RxDi00YYdMbOIPwVZ\r\n"
"JH4b87J/LYdLAGf+Q+kVI6gWH3hPm4Jzfzq/40KVrf3mpa54yWz6ZYtwfxBjrMgr\r\n"
"IVe0O5SIJ99lsddgzgUkqYN2vWJW2zZ50xuXOAyo+pOnjzX0wuOcaBT3JCHWJRAb\r\n"
"VhJCf9JbswDgnddJerqFtB8pnpAOdGokLCOoM06q3s3P9mhGX+72HXdX7G8CSAuG\r\n"
"PVCGf6RaF0/G4B9R1c3du3lZRlQWfx2pxyU0LS86iFQFWqzqcWEXIcULVdcErQ==\r\n"
"-----END CERTIFICATE-----\r\n";

char ftp_client_crt[] =
"-----BEGIN CERTIFICATE-----\r\n"
"MIIDKTCCAhECCQDpY/MI6IXYcTANBgkqhkiG9w0BAQUFADBXMQswCQYDVQQGEwJD\r\n"
"TjEQMA4GA1UECBMHU2lDaHVhbjEVMBMGA1UEChMMTU9CSUxFVEVLLkNBMQswCQYD\r\n"
"VQQLEwJJVDESMBAGA1UEAxMJTU9CSUxFVEVLMB4XDTE4MDkxODA4MDcxNloXDTMz\r\n"
"MDkxOTA4MDcxNlowVjELMAkGA1UEBhMCQ04xEDAOBgNVBAgTB1NpQ2h1YW4xFDAS\r\n"
"BgNVBAoTC01PQklMRVRFSy5DMQswCQYDVQQLEwJJVDESMBAGA1UEAxMJTU9CSUxF\r\n"
"VEVLMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAk1EdX++SIEgEnJVm\r\n"
"T//7U8xBJBT4JL61A0MAIFSE+xLcFjKQ492ev19Y6XmrcrlrXfvaO+d9cCMvfjxc\r\n"
"xQ+aBOiNs0Sy2jHLdZJfLAKRo8Pt9eM1cX8ALaymigpmgL+7zeXNhW0ejzzCrsay\r\n"
"XLVWKZyOHNyv5kdDfRyfM1kGnPxU9qV6YNgwELz0+Nhf+b4DSNEUHDDT+eu2ZiVs\r\n"
"VoiC7Duwen2794yka1xyPrDQazW4uZMqUOZzZhrBptjjNXQ6PCHqcV5FDzFzXM7h\r\n"
"lklnl4ZOVOCSVoTCNLS3M35N+P70e4uSXM3SMJuMhFURUWrJ+BRFBBa/Oqv97lJZ\r\n"
"DLnnpQIDAQABMA0GCSqGSIb3DQEBBQUAA4IBAQAyUETpOQS6p6RAxDuX8/O7r5M0\r\n"
"ZYtzd+eUyx5tAt7H2YPCXRPBEpEppExayCihoWoEYsItfs8fQ7a6dA5goltK7bXv\r\n"
"XT3uVJnF5aBmVY3fdXiOmH5bTD7SUv7QWmXoA8qIfNcOgPL4wCwYaGdGwfP+x5+W\r\n"
"lHZK79BKli7MiABaMCmn/Ivzi1aq5iLFvCRs054ibuiz/LgVtRG912LHJbmRin0R\r\n"
"jtIzguJ7ViGW2eCFhlnpA8oACh2G7JQp6ifw8Mzi0N7rCVV8BjIc2Dfh0zWjbQ6D\r\n"
"uhiuzXYukHoEM/DRIbn66ffxb/R/tU6u2UewW3+Bv/h1vWzfKJdbqIIgwNbY\r\n"
"-----END CERTIFICATE-----\r\n";

char ftp_client_key[] =
"-----BEGIN RSA PRIVATE KEY-----\r\n"
"MIIEpAIBAAKCAQEAk1EdX++SIEgEnJVmT//7U8xBJBT4JL61A0MAIFSE+xLcFjKQ\r\n"
"492ev19Y6XmrcrlrXfvaO+d9cCMvfjxcxQ+aBOiNs0Sy2jHLdZJfLAKRo8Pt9eM1\r\n"
"cX8ALaymigpmgL+7zeXNhW0ejzzCrsayXLVWKZyOHNyv5kdDfRyfM1kGnPxU9qV6\r\n"
"YNgwELz0+Nhf+b4DSNEUHDDT+eu2ZiVsVoiC7Duwen2794yka1xyPrDQazW4uZMq\r\n"
"UOZzZhrBptjjNXQ6PCHqcV5FDzFzXM7hlklnl4ZOVOCSVoTCNLS3M35N+P70e4uS\r\n"
"XM3SMJuMhFURUWrJ+BRFBBa/Oqv97lJZDLnnpQIDAQABAoIBADv70kcm6EwGpwIO\r\n"
"/xrh8zb6uydy9lsX3+KVHG8NmxwUYNMVAdQWgDYSH8TpSrsq7qHVE9ZCetv3J7Zx\r\n"
"46BFqwQGoPP2rBCWJ+8Wi3QDE8Qn2jO9sRwmhy8R1rnsZDJWX6UFxjpm5QOTzP2X\r\n"
"YDbJi2zkFyV/YowURlGiHtysijnPEq0PcYlKrOMp7ee4znCKmcyEWU9EjvkVvBBS\r\n"
"WSgGYsMCePh4AzlfyhoxcO8NLWM7XTl32ov5IjUEqwV0oIVqB5yFFRxPS1gN5nSn\r\n"
"N6uggVrzwLjK4bgTGwbMAHkZhfXPeNvpSVbWLXqsgTYeyqLyikigzZrMF+PmMbKT\r\n"
"hgsOmr0CgYEAxFhLo8zMkoPmKc3Rfv3lpE6AtSIqdQfaJhVxbE+tiRzt4xpbfFtb\r\n"
"LbWGAUwcewesxMWyMFzl2nBixBPkIJkDYMkAo8dR26oL4QorWtb96xLQceDCF3L5\r\n"
"uI6Tm/3tCd1iNCEzg4yqCTtljelkcIyLwOClcscBlqIYe+ac6tj56PcCgYEAwBNr\r\n"
"CI/+s3QUiT7b9Na6WZhCItRmnA8Z3sUCdcDirm48UEwaWLFzuvolpxj9h9YrP/GQ\r\n"
"6ODnqZpaMgeTr0yW8c/LrXkW5o6RmRcnOLTWHHvphlaEgrU33xpmr0XbabOCl+TS\r\n"
"yKyrArYpyIL5wPAvCtOKHzDuQqRk+9lwBp3YyUMCgYEAramjDXghIR5Ev4jp2Tbc\r\n"
"nN1KfeuAPg755mFk9vXqebH6vrobXPy4ws8hfZhCQJdjOo/ZsWkZbIGm+eLWDfcI\r\n"
"w9xFEpdDUdUvuJX0Dt7Fq9vyPicbxP3O3mxNJtYLiIQlThJnq90IsC22/zQFwH6v\r\n"
"RaPs8n4Oa5tnqH8rH5VWRbUCgYAY/+4EdUl/bfJmUqoqWfBzTN/+zIp2cXi6iAXj\r\n"
"8bEPZwWupdkgBii8A2b3Msd88KE8d2KXDP7aEl7++AF+5YcX/iXSdFpIs/G7bUoL\r\n"
"lW3w0yf39jbVFGUrQrJuybbfMSAkSQIuYFr5xV/22yVKuXhF/naRzLqWLfN+3DQ6\r\n"
"iDz5JQKBgQCC+SK63r5sXt8xKdHy7KXNo9WcjBgDumiF3VuaBunqSasQHpYlUBol\r\n"
"M/ZvR2RLOuZWCM6XZcU84bowdSXMfWEwhMPbXvuVgMiYVfVGe9unomtRVOZdUqct\r\n"
"ULwV8cGeNE6fwrLN8hkXTmEjmL3ESNXKjinJp46U0IJ/4iVtdzfiNg==\r\n"
"-----END RSA PRIVATE KEY-----\r\n";

void set_ftp_param(ftp_config_t* config)
{
    config->hostname = SERVER_IP;
    config->port = SERVER_PORT;
    config->is_ipv6 = 0;
    config->username = FTP_USER;
    config->password = FTP_PASSWD;
    config->ftp_type = 0;
    config->sslmode = 2;
    config->seclevel = 0;
    config->ca_crt = ftp_ca;
    config->client_crt = ftp_client_crt;
    config->client_key = ftp_client_key;
}

int create_test_file(char* file)
{
    int ret = 0;
    OLFILE fp = NULL;
    char* test_str = "123456789";

    fp = ol_fs_open(file, "wb+",FS_TYPE_INTERNAL);
    if(fp == NULL)
    {
        return -1;
    }

    ret = ol_fs_write(test_str, strlen(test_str), fp);
    if(ret < 0)
    {
        return -1;
    }

    ret = ol_fs_close(fp);
    if(ret < 0)
    {
        return -1;
    }

    return 0;
}

void ftp_demo_get_list(void)
{
    int ret = 0;
    char buf[256] = {0};
    int len = sizeof(buf);
    ftp_config_t config = {0};

    set_ftp_param(&config);

    ret = ol_ftp_list(TEST_DIR, buf, len, &config);  
    if(ret < 0)
    {
        OL_LOG_PRINTF("ftp_demo: ol_ftp_list fail");
    }
    else
    {
        if(ret == 0)
        {
            OL_LOG_PRINTF("ftp_demo: buf_len: %d", strlen(buf));
            OL_LOG_PRINTF("ftp_demo: buf: %s", buf);
        }
        else
        {
            OL_LOG_PRINTF("ftp_demo: ol_ftp_list buffer too small, ret = %d", ret);
        }
    }
}

void ftp_demo_get_files(void)
{
    int ret = 0;
    ftp_config_t config = {0};

    set_ftp_param(&config);

    ret = ol_ftp_getfile(TEST_FILE, TEST_DIR, 0, &config);
    if(ret < 0)
    {
        OL_LOG_PRINTF("ftp_demo: ol_ftp_getfile fail: ret: %d", ret);
    }
    else
    {
        OL_LOG_PRINTF("ftp_demo: ol_ftp_getfile OK");
        OL_LOG_PRINTF("ftp_demo: ret: %d", ret);
    }
}

void ftp_demo_put_files(void)
{
    int ret = 0;
    ftp_config_t config = {0};

    ret = create_test_file(TEST_FILE);
    if(ret < 0)
    {
        return;
    }
    set_ftp_param(&config);
    
    ret = ol_ftp_putfile(TEST_FILE, TEST_DIR, 0, &config);
    if(ret < 0)
    {
        OL_LOG_PRINTF("ftp_demo: ol_ftp_putfile fail, ret: %d", ret);
    }
    else
    {
        OL_LOG_PRINTF("ftp_demo: ol_ftp_putfile OK");
    }
    
    ret = ol_fs_remove(TEST_FILE,FS_TYPE_INTERNAL);
    if(ret < 0)
    {
        return;
    }
}

void ftp_demo_mkdir(void)
{
    int ret = 0;
    ftp_config_t config = {0};

    set_ftp_param(&config);

    ret = ol_ftp_mkdir(TEST_DIR, &config);
    if(ret < 0)
    {
        OL_LOG_PRINTF("ftp_demo: ol_ftp_mkdir fail, ret: %d", ret);
    }
    else
    {
        OL_LOG_PRINTF("ftp_demo: ol_ftp_mkdir OK");
    }
}

void ftp_demo_rmdir(void)
{
    int ret = 0;
    ftp_config_t config = {0};

    set_ftp_param(&config);

    ret = ol_ftp_rmdir(TEST_DIR, &config);
    if(ret < 0)
    {
        OL_LOG_PRINTF("ftp_demo: ol_ftp_rmdir fail, ret: %d", ret);
    }
    else
    {
        OL_LOG_PRINTF("ftp_demo: ol_ftp_rmdir OK");
    }

}

void ftp_demo_delete(void)
{
    int ret = 0;
    ftp_config_t config = {0};

    set_ftp_param(&config);

    ret = ol_ftp_delete(TEST_FILE, TEST_DIR, &config);
    if(ret < 0)
    {
        OL_LOG_PRINTF("ftp_demo: ol_ftp_delete fail, ret: %d", ret);
    }
    else
    {
        OL_LOG_PRINTF("ftp_demo: ol_ftp_delete OK");
    }
}

void ftp_demo_file_size(void)
{
    int ret = 0;
    ftp_config_t config = {0};

    set_ftp_param(&config);

    ret = ol_ftp_size(TEST_FILE, TEST_DIR, &config);
    if(ret < 0)
    {
        OL_LOG_PRINTF("ftp_demo: ol_ftp_size fail, ret: %d", ret);
    }
    else
    {
        OL_LOG_PRINTF("ftp_demo: ol_ftp_size OK, size: %d", ret);
    }
}

void ftp_demo()
{
    ftp_demo_mkdir();

    ftp_demo_put_files();

    ftp_demo_file_size();

    ftp_demo_get_list();

    ftp_demo_get_files();

    ftp_demo_delete();

    ftp_demo_rmdir();
}
