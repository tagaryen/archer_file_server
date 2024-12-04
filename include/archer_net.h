#ifndef _ARCHER_NET_H_
#define _ARCHER_NET_H_

#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <sys/types.h>

#ifndef TLS1_VERSION
#define	TLS1_VERSION      0x0301
#endif

#ifndef TLS1_1_VERSION
#define	TLS1_1_VERSION    0x0302
#endif

#ifndef TLS1_2_VERSION
#define	TLS1_2_VERSION    0x0303
#endif

#ifndef TLS1_3_VERSION
#define	TLS1_3_VERSION    0x0304
#endif

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN   65
#endif


#ifdef __cplusplus
extern "C" {
#endif

struct Channel_st;
struct ServerChannel_st;
struct SSLOption_st;
struct HttpServer_st;
struct HttpRequest_st;
struct HttpResponse_st;

typedef struct Channel_st            Channel;
typedef struct ServerChannel_st      ServerChannel;
typedef struct SSLOption_st          SSLOption;
typedef struct HttpServer_st         HttpServer;
typedef struct HttpRequest_st        HttpRequest;
typedef struct HttpResponse_st       HttpResponse;


typedef struct FairLock_st FairLock;
typedef struct BaseHandler_st BaseHandler;

typedef void (*server_channel_on_accept)(Channel *channel);
typedef void (*channel_on_connect)(Channel *channel);
typedef void (*channel_on_close)(Channel *channel);
typedef void (*channel_on_read)(Channel *channel, char *data, size_t data_len);
typedef void (*channel_on_error)(Channel *channel, const char *err_msg);
typedef void (*channel_on_peer_certificate)(Channel *channel, const char *crt, const size_t crt_len);

typedef void (*http_message_handler)(HttpRequest *req, HttpResponse *res);


//ssl
SSLOption * ssl_option_new(int is_client_mode, int is_verify_peer);
void ssl_option_free(SSLOption *opt);
int ssl_option_set_version(SSLOption *opt, int max_version, int min_version);
int ssl_option_authonrized_hostname(SSLOption *opt, const char *hostname);
int ssl_set_named_curves(SSLOption *opt, const char *named_curves);
int ssl_set_trust_ca(SSLOption *opt, const char *ca, const size_t ca_len);
int ssl_set_certificate_and_key(SSLOption *opt, const char *crt, const size_t crt_len, const char *key, const size_t key_len);
int ssl_set_encrypt_certificate_and_key(SSLOption *opt, const char *crt, const size_t crt_len, const char *key, const size_t key_len);
void ssl_option_get_errstr(SSLOption * opt, char **errstr);


// channel 
Channel * channel_new(void);
void channel_close(Channel *channel);
void channel_free(Channel *channel);
const char * channel_get_host(Channel *channel);
int channel_get_port(Channel *channel);
const char * channel_get_errstr(Channel *channel);
void * channel_get_arg(Channel *channel);
SSLOption * channel_get_ssl_option(Channel *channel);
int channel_is_client_side(Channel *channel);

void channel_set_channel_on_connect(Channel *channel, channel_on_connect);
void channel_set_channel_on_read(Channel *channel, channel_on_read);
void channel_set_channel_on_error(Channel *channel, channel_on_error);
void channel_set_channel_on_close(Channel *channel, channel_on_close);
void channel_set_channel_on_peer_cerificate(Channel *channel, channel_on_peer_certificate);
void channel_set_channel_ssl_option(Channel *channel, SSLOption *ssl_opt);
void channel_set_channel_arg(Channel *channel, void *arg);
int channel_write(Channel *channel, const char *data, const size_t data_len);
int channel_connect_to(Channel *channel, const char *host, const int port);


// server channel 
ServerChannel * server_channel_new(void);
ServerChannel * server_channel_new_with_ssl(SSLOption *ssl_opt);
void server_channel_close(ServerChannel *server);
void server_channel_free(ServerChannel *server);
const char * server_channel_get_errstr(ServerChannel *server);
void * server_channel_get_arg(ServerChannel *server);
SSLOption * server_channel_get_ssl_option(ServerChannel *server);

int server_channel_listen(ServerChannel *server, const char *host, const int port);
int server_channel_listen_ipv6(ServerChannel *server, const char *host, const int port);
void server_channel_set_eventloop_threads(ServerChannel *server, uint16_t thread_num);
void server_channel_set_read_threads(ServerChannel *server, uint16_t thread_num);
void server_channel_set_on_accept(ServerChannel *server, server_channel_on_accept);
void server_channel_set_channel_on_connect(ServerChannel *server, channel_on_connect);
void server_channel_set_channel_on_read(ServerChannel *server, channel_on_read);
void server_channel_set_channel_on_error(ServerChannel *server, channel_on_error);
void server_channel_set_channel_on_close(ServerChannel *server, channel_on_close);
void server_channel_set_channel_on_peer_cerificate(ServerChannel *server, channel_on_peer_certificate);
void server_channel_set_channel_ssl_option(ServerChannel *server, SSLOption *ssl_opt);
void server_channel_set_arg(ServerChannel *server, void *arg);


//http
const char * http_request_get_method(HttpRequest *req);
const char * http_request_get_uri(HttpRequest *req);
uint32_t http_request_get_content_length(HttpRequest *req);
const char * http_request_get_content_type(HttpRequest *req);
const char * http_request_get_header(HttpRequest *req, const char *key);
const char * http_request_get_query_param(HttpRequest *req, const char *key);
void http_request_read_all_body(HttpRequest *req, char **data, size_t *data_len);
void http_request_read_some(HttpRequest *req, char **data, size_t *data_len);

void http_response_set_status(HttpResponse *res, int code);
void http_response_set_content_length(HttpResponse *res, uint32_t len);
void http_response_set_content_type(HttpResponse *res, const char *value);
void http_response_set_header(HttpResponse *res, const char *key, const char *value);
void http_response_send_response(HttpResponse *res, const char *body, const size_t body_len);
void http_response_send_head(HttpResponse *res);
void http_response_send_some(HttpResponse *res, const char *data, const size_t data_len);
void http_response_send_end(HttpResponse *res);

HttpServer * http_server_new();
void http_server_close(HttpServer * server);
void http_server_free(HttpServer * server);
int http_server_listen(HttpServer *server, const char *host, const uint16_t port);
int http_server_listen_ipv6(HttpServer *server, const char *host, const uint16_t port);
void http_server_get_errstr(HttpServer *server, char **errstr);
void http_server_set_message_handler(HttpServer *server, http_message_handler handler_cb);
void http_server_use_threads_pool(HttpServer *server, uint16_t thread_num);


FairLock * fair_lock_new();
void fair_lock_acquire(FairLock *lock);
void fair_lock_release(FairLock *lock);
void fair_lock_destroy(FairLock* lock);


BaseHandler * base_handler_new();
void base_handler_free(BaseHandler *handler);
void base_handler_set_on_accept(BaseHandler *handler, server_channel_on_accept);
void base_handler_set_channel_on_connect(BaseHandler *handler, channel_on_connect);
void base_handler_set_channel_on_read(BaseHandler *handler, channel_on_read);
void base_handler_set_channel_on_error(BaseHandler *handler, channel_on_error);
void base_handler_set_channel_on_close(BaseHandler *handler, channel_on_close);
void base_handler_handle_channel(BaseHandler *handler, Channel *channel);
void base_handler_handle_server_channel(BaseHandler *handler, ServerChannel *server);
int base_handler_write(Channel *channel, const char *data, const size_t data_len);

#ifdef __cplusplus
}
#endif

#endif



