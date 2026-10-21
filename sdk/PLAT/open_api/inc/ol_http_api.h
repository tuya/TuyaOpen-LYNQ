#ifndef __OL_HTTP_API__
#define __OL_HTTP_API__

typedef void* http_client;
typedef void (*client_response_cb)(char *headbuf, char *buffer, int size);


typedef struct HttpClientCfgTag
{
    char* url;
    int timeout_s;
    int timeout_r;
    char *caCert;
    char *clientCert;
    char *clientPk;
    uint8_t seclevel;//0:no verify; 1:verify server; 2:both verify
    client_response_cb callback;
}ol_http_cfg;

///HTTP client results
typedef enum {
    OL_HTTP_ERR_PARAM = -1,
    OL_HTTP_ERR_CLIENT = -2,
    OL_HTTP_OK = 0,        ///<Success
    OL_HTTP_PARSE,         ///1<url Parse error
    OL_HTTP_DNS,           ///2<Could not resolve name
    OL_HTTP_PRTCL,         ///3<Protocol error
    OL_HTTP_SOCKET_FAIL,   ///4<create socket fail
    OL_HTTP_BIND_FAIL,     ///5<bind fail  
    OL_HTTP_TIMEOUT,       ///6<Connection timeout
    OL_HTTP_CONN,          ///7<Connection error
    OL_HTTP_CLOSED,        ///8<Connection was closed by remote host
    OL_HTTP_MBEDTLS_ERR,   ///9<meet ssl error
    OL_HTTP_MOREDATA,      ///10<Need get more data
    OL_HTTP_OVERFLOW,      ///11<Buffer overflow
    OL_HTTP_REQ_TIMEOUT,   ///12<HTTP request timeout waittime is 60s
    OL_HTTP_NO_MEMORY,     ///13<memory not enough
    OL_HTTP_INTERNAL       ///14<Internal error
}OL_HTTP_Result;

typedef enum  {
    OL_HTTP_GET,
    OL_HTTP_POST,
    OL_HTTP_PUT,
    OL_HTTP_DELETE,
    OL_HTTP_HEAD
}OL_HTTP_METH;

/*****************************************************************************
 *
 * FUNCTIO
 *      ol_http_client_init
 * DESCRIPTION 
 *      This API is to init http client
 * PARAMETERS 
 * RETURN VALUES
 *      NULL            : init error
 *      http_client     :   http client
 *
 *****************************************************************************/
http_client ol_http_client_init(void);

/*****************************************************************************
 *
 * FUNCTIO
 *      ol_http_set_cfg
 * DESCRIPTION 
 *      This API is used to config http client
 * PARAMETERS
 *      client          :http client
 *      cfg             :a pointer type variable that points to the config function for http client
 * RETURN VALUES
 *      OL_HTTP_Result
 *
 *****************************************************************************/
int ol_http_set_cfg(http_client client, ol_http_cfg cfg);

/*****************************************************************************
 *
 * FUNCTIO
 *      ol_http_connect
 * DESCRIPTION 
 *      This API is used to connect the http server
 * PARAMETERS
 *      client          :http client
 * RETURN VALUES
 *      OL_HTTP_Result
 *
 *****************************************************************************/
int ol_http_connect(http_client client);

/*****************************************************************************
 *
 * FUNCTIO
 *      ol_http_get
 * DESCRIPTION 
 *      This API is used to send get request to the http url
 * PARAMETERS
 *      client          :http client
 * RETURN VALUES
 *      OL_HTTP_Result
 *
 *****************************************************************************/
int ol_http_get(http_client client);

/*****************************************************************************
 *
 * FUNCTIO
 *      ol_http_post
 * DESCRIPTION 
 *      This API is used to send post request to the http url
 * PARAMETERS
 *      client          :http client
 *      postbuf         :a pointer to the post data buffer
 * RETURN VALUES
 *      OL_HTTP_Result
 *
 *****************************************************************************/
int ol_http_post(http_client client, char* postbuf);

/*****************************************************************************
 *
 * FUNCTIO
 *      ol_http_post
 * DESCRIPTION 
 *      This API is used to send request to the http url
 * PARAMETERS
 *      client          :http client
 *      method          :OL_HTTP_METH
 *      postbuf         :if method set OL_HTTP_POST,set a pointer to the post data buffer
                         others set NULL
 * RETURN VALUES
 *      OL_HTTP_Result
 *
 *****************************************************************************/
int ol_http_send_request(http_client client, int method, char* postbuf);

/*****************************************************************************
 *
 * FUNCTIO
 *      ol_http_post
 * DESCRIPTION 
 *      This API is used to close http client
 * PARAMETERS
 *      client          :http client
 * RETURN VALUES
 *      OL_HTTP_Result
 *
 *****************************************************************************/
int ol_http_close(http_client client);


#endif
