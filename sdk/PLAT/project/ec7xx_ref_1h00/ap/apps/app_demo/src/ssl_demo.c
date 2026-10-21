#include "stdio.h"
#include "string.h"
#include "ol_ssl_api.h"
#include "cmsis_os2.h"
#include "bsp.h"

#include "osasys.h"
#include "ol_log.h"


char ca[] =
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

char client_crt[] =
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

char client_key[] =
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


void ssl_demo(void)
{
    SSLConfig ssl_cfg ={0};
    SSLCtx ssl_ctx= {0};
    char buff[100]={0};
    int ret = -1;

    ssl_cfg.profileIdx = 0;
    ssl_cfg.dbgLevel = 3;
    ssl_cfg.protocol = 0;
    ssl_cfg.serverName = "118.114.239.159";
    ssl_cfg.serverPort = 30175;
    ssl_cfg.verify = SSL_VERIFY_MODE_REQUIRED;
    ssl_cfg.vsn = SSL_VSN_ALL;
#ifdef CERT_FROM_NVM
    ssl_cfg.cert.from = SSL_CERT_FROM_FS;
    ssl_cfg.cert.path.rootCA = "ca.crt";
    ssl_cfg.cert.path.clientCert = "client.crt";
    ssl_cfg.cert.path.clientKey = "client.key";
#else
    ssl_cfg.cert.from = SSL_CERT_FROM_BUF;
    ssl_cfg.cert.path.rootCA = ca;
    ssl_cfg.cert.path.clientCert = client_crt;
    ssl_cfg.cert.path.clientKey = client_key;
#endif

    ol_ssl_set_config(&ssl_ctx,&ssl_cfg);

    ret = ol_ssl_ctx_init(&ssl_ctx);
    OL_LOG_PRINTF("SSLCtxInit ret=%d",ret);

    ret = ol_ssl_handshake(&ssl_ctx,60*1000);
    OL_LOG_PRINTF("SSLHandshake ret=%d",ret);

    osDelay(5000);

    ret = ol_ssl_write(&ssl_ctx,"hello server",12);
    OL_LOG_PRINTF("SSLWrite ret=%d",ret);

    ret = ol_ssl_read(&ssl_ctx,buff,100);
    OL_LOG_PRINTF("SSLRead ret=%d",ret);
    OL_LOG_PRINTF("SSLRead recv data=%s",buff);

    ret = ol_ssl_shutdown(&ssl_ctx);
    OL_LOG_PRINTF("SSLShutdown ret=%d",ret);

    ol_ssl_ctx_deinit(&ssl_ctx);
}

