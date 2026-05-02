#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H
#include <lwip/pbuf.h>
#include <stdio.h>
#include <string.h>
#include "pico/async_context.h"
#include "lwip/altcp.h"
#include "lwip/altcp_tls.h"

#include "lwip/apps/http_client.h"

#ifndef HTTP_INFO
#define HTTP_INFO printf
#endif

#ifndef HTTP_INFOC
#define HTTP_INFOC putchar
#endif

#ifndef HTTP_INFOC
#define HTTP_INFOC putchar
#endif

#ifndef HTTP_DEBUG
#ifdef NDEBUG
#define HTTP_DEBUG
#else
#define HTTP_DEBUG printf
#endif
#endif

#ifndef HTTP_ERROR
#define HTTP_ERROR printf
#endif

typedef struct http_request{
    const char* hostname;
    const char* url;
    httpc_headers_done_fn headers_fn; //function to callback with headers
    altcp_recv_fn recv_fn;//function to call when server results received
    httpc_result_fn result_fn;//final callback when request completed

    void* callback_arg;

    uint16_t port;

#if LWIP_ALTCP && LWIP_ALTCP_TLS

    struct altcp_tls_config *tls_config;
    altcp_allocator_t tls_allocator;

#endif

    httpc_connection_t settings;

    int complete;

    httpc_result_t result;
}http_request_t;

struct async_context;

typedef struct {
    char buf[2048];
    uint16_t written;
} http_response_t;

//headerst to stdout
err_t http_client_header_printf(__unused httpc_state_t* connection, __unused void* arg, struct pbuf* headers, uint16_t headers_len,__unused uint32_t content_len){
    HTTP_INFO("\nheaders %u\n", headers_len);
    uint16_t offset = 0;
    while (offset < headers->tot_len && offset < headers_len){
        char c = (char)pbuf_try_get_at(headers, offset++);
        HTTP_INFOC(c);
    }
    return ERR_OK;
}
//print body to stdout
err_t http_client_receive_print(__unused void* arg, __unused struct altcp_pcb* conn, struct pbuf* p, err_t err){
    HTTP_INFO("\ncontent err %d\n", err);
    if (!p) return ERR_OK;
    http_response_t *resp = (http_response_t*)arg;
    uint16_t offset = 0;
    while (offset < p->tot_len) {
        if (resp->written >= sizeof(resp->buf) - 1) break;
        resp->buf[resp->written++] = (char)pbuf_get_at(p, offset++);
    }
    printf("buf size %d", resp->written);
    resp->buf[resp->written] = '\0';
    return ERR_OK;
}

static err_t internal_header_fn(httpc_state_t* connection, void* arg, struct pbuf* hdr, uint16_t hdr_len, uint32_t content_len){
    assert(arg);
    http_request_t* req = (http_request_t*) arg;
    if(req->headers_fn){
        return req->headers_fn(connection, req->callback_arg, hdr, hdr_len, content_len);
    }
    return ERR_OK;
}

static void internal_result_fn(void *arg, httpc_result_t httpc_result, uint32_t rx_content_len, uint32_t srv_res, err_t err){
    assert(arg);
    http_request_t* req = (http_request_t*)arg;
    //printf("result %d len %d server response %u err %d\n", httpc_result, rx_content_len, srv_res, err);
    req->complete = true;
    req->result = httpc_result;
    if(req->result_fn){
        req->result_fn(req->callback_arg, httpc_result, rx_content_len, srv_res, err);
    }
}

static err_t internal_recv_fn(void* arg, struct altcp_pcb* conn, struct pbuf* p, err_t err){
    assert(arg);
    http_request_t* req = (http_request_t*)arg;
    if (req->recv_fn){
        return req->recv_fn(req->callback_arg, conn, p, err);
    }
    return ERR_OK;
}

int http_client_request_async(async_context_t* context, http_request_t* req){
    const uint16_t default_port = 80;

    req->complete = false;
    req->settings.headers_done_fn = req->headers_fn ? internal_header_fn : NULL;
    req->settings.result_fn = internal_result_fn;

    async_context_acquire_lock_blocking(context);
    err_t ret = httpc_get_file_dns(req->hostname, req->port ? req->port : default_port, req->url, &req->settings, internal_recv_fn, req, NULL);
    async_context_release_lock(context);
    if (ret != ERR_OK){
        HTTP_ERROR("http request failed: %d", ret);
    }
    return ret;
}

int http_client_request_sync(async_context_t* context, http_request_t* req){
    assert(req);
    int ret = http_client_request_async(context, req);
    if( ret != 0){
        return ret;
    }
    while(!req->complete){
        async_context_poll(context);
        async_context_wait_for_work_ms(context, 1000);
    }
    return req->result;
}

#endif
