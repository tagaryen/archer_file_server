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

struct ChannelBase_st;
struct Channel_st;
struct ServerChannel_st;
struct SSLOption_st;
struct HttpServer_st;
struct HttpRequest_st;
struct HttpResponse_st;
struct HttpHeaders_st;
struct HttpBody_st;
struct HttpClientOption_st;
struct Response_st;
struct RequestSender_st;


typedef struct ChannelBase_st        ChannelBase;
typedef struct Channel_st            Channel;
typedef struct ChannelBase_st        ChannelBase;
typedef struct ServerChannel_st      ServerChannel;
typedef struct SSLOption_st          SSLOption;
typedef struct HttpServer_st         HttpServer;
typedef struct HttpRequest_st        HttpRequest;
typedef struct HttpResponse_st       HttpResponse;

typedef struct HttpHeaders_st HttpHeaders;
typedef struct HttpBody_st HttpBody;
typedef struct HttpClientOption_st HttpClientOption;
typedef struct Response_st Response;
typedef struct RequestSender_st RequestSender;


typedef struct FairLock_st FairLock;
typedef struct BaseHandler_st BaseHandler;

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*server_channel_on_accept)(Channel *channel);
typedef void (*channel_on_connect)(Channel *channel);
typedef void (*channel_on_close)(Channel *channel);
typedef void (*channel_on_read)(Channel *channel, char *data, size_t data_len);
typedef void (*channel_on_error)(Channel *channel, const char *err_msg);
typedef void (*channel_on_peer_certificate)(Channel *channel, const char *crt, const size_t crt_len);

typedef void (*http_message_handler)(HttpRequest *req, HttpResponse *res);
typedef void (*http_client_body_sender)(RequestSender *sender, void *arg);
typedef void (*http_client_body_reader)(Response *res, const void *data, const size_t size, void *arg);

//ssl
extern SSLOption * ssl_option_new(int is_client_mode, int is_verify_peer);
extern void ssl_option_free(SSLOption *opt);
extern int ssl_option_set_version(SSLOption *opt, int max_version, int min_version);
extern int ssl_option_authonrized_hostname(SSLOption *opt, const char *hostname);
extern int ssl_option_set_named_curves(SSLOption *opt, const char *named_curves);
extern int ssl_option_set_trust_ca(SSLOption *opt, const char *ca, const size_t ca_len);
extern int ssl_option_set_certificate_and_key(SSLOption *opt, const char *crt, const size_t crt_len, const char *key, const size_t key_len);
extern int ssl_option_set_encrypt_certificate_and_key(SSLOption *opt, const char *crt, const size_t crt_len, const char *key, const size_t key_len);
extern const char * ssl_option_get_errstr(SSLOption * opt);


extern ChannelBase * channel_base_new();
extern void channel_base_stop(ChannelBase *base);
extern void channel_base_free(ChannelBase *base);
extern void channel_base_start_event_loop(ChannelBase *base);

// channel 
extern Channel * channel_new(void);
extern void channel_close(Channel *channel);
extern void channel_free(Channel *channel);
extern const char * channel_get_host(Channel *channel);
extern int channel_get_port(Channel *channel);
extern const char * channel_get_errstr(Channel *channel);
extern void * channel_get_arg(Channel *channel);
extern SSLOption * channel_get_ssl_option(Channel *channel);
extern int channel_is_client_side(Channel *channel);

extern void channel_set_channel_on_connect(Channel *channel, channel_on_connect);
extern void channel_set_channel_on_read(Channel *channel, channel_on_read);
extern void channel_set_channel_on_error(Channel *channel, channel_on_error);
extern void channel_set_channel_on_close(Channel *channel, channel_on_close);
extern void channel_set_channel_on_peer_cerificate(Channel *channel, channel_on_peer_certificate);
extern int channel_set_channel_ssl_option(Channel *channel, SSLOption *ssl_opt);
extern void channel_set_channel_arg(Channel *channel, void *arg);
extern int channel_write(Channel *channel, const char *data, const size_t data_len);
extern int channel_connect_to(Channel *channel, const char *host, const int port);
extern int channel_connect_to_with_base(Channel *channel, const char *host, const int port, ChannelBase *channel_base);


// server channel 
extern ServerChannel * server_channel_new(void);
extern ServerChannel * server_channel_new_with_ssl(SSLOption *ssl_opt);
extern void server_channel_close(ServerChannel *server);
extern void server_channel_free(ServerChannel *server);
const char * server_channel_get_errstr(ServerChannel *server);
extern void * server_channel_get_arg(ServerChannel *server);
extern SSLOption * server_channel_get_ssl_option(ServerChannel *server);

extern int server_channel_listen(ServerChannel *server, const char *host, const int port);
extern int server_channel_listen_ipv6(ServerChannel *server, const char *host, const int port);
extern void server_channel_set_eventloop_threads(ServerChannel *server, uint16_t thread_num);
extern void server_channel_set_read_threads(ServerChannel *server, uint16_t thread_num);
extern void server_channel_set_on_accept(ServerChannel *server, server_channel_on_accept);
extern void server_channel_set_channel_on_connect(ServerChannel *server, channel_on_connect);
extern void server_channel_set_channel_on_read(ServerChannel *server, channel_on_read);
extern void server_channel_set_channel_on_error(ServerChannel *server, channel_on_error);
extern void server_channel_set_channel_on_close(ServerChannel *server, channel_on_close);
extern void server_channel_set_channel_on_peer_cerificate(ServerChannel *server, channel_on_peer_certificate);
extern void server_channel_set_channel_ssl_option(ServerChannel *server, SSLOption *ssl_opt);
extern void server_channel_set_arg(ServerChannel *server, void *arg);
extern void * server_channel_get_arg(ServerChannel *server);
extern SSLOption * server_channel_get_ssl_option(ServerChannel *server);


//http server
extern const char * http_request_get_method(HttpRequest *req);
extern const char * http_request_get_uri(HttpRequest *req);
extern uint32_t http_request_get_content_length(HttpRequest *req);
extern const char * http_request_get_content_type(HttpRequest *req);
extern const char * http_request_get_header(HttpRequest *req, const char *key);
extern unsigned int http_request_headers_size(HttpRequest *req);
extern const char * http_request_headers_get_key(HttpRequest *req, unsigned int i);
extern const char * http_request_headers_get_val(HttpRequest *req, unsigned int i);
extern const char * http_request_get_query(HttpRequest *req, const char *key);
extern unsigned int http_request_queries_size(HttpRequest *req);
extern const char * http_request_queries_get_key(HttpRequest *req, unsigned int i);
extern const char * http_request_queries_get_val(HttpRequest *req, unsigned int i);
extern void http_request_read_all_body(HttpRequest *req, void **data, size_t *data_len);
extern int http_request_read_some(HttpRequest *req, void *data, size_t size);

extern void http_response_set_status(HttpResponse *res, int code);
extern void http_response_set_content_length(HttpResponse *res, uint32_t len);
extern void http_response_set_content_type(HttpResponse *res, const char *value);
extern void http_response_set_header(HttpResponse *res, const char *key, const char *value);
extern const char * http_response_get_header(HttpResponse *res, const char *key);
extern void http_response_send_response(HttpResponse *res, const char *body, const size_t body_len);
extern void http_response_send_head(HttpResponse *res);
extern void http_response_send_some(HttpResponse *res, const char *data, const size_t data_len);
extern void http_response_send_end(HttpResponse *res);

extern HttpServer * http_server_new();
extern HttpServer * http_server_new_with_ssl(SSLOption *sslopt);
extern void http_server_close(HttpServer * server);
extern void http_server_free(HttpServer * server);
extern int http_server_listen(HttpServer *server, const char *host, const uint16_t port);
extern int http_server_listen_ipv6(HttpServer *server, const char *host, const uint16_t port);
extern const char * http_server_get_errstr(HttpServer *server);
extern void http_server_set_message_handler(HttpServer *server, http_message_handler handler_cb);
extern void http_server_use_threads_pool(HttpServer *server, uint16_t thread_num);


// http client
extern const char * http_headers_get(HttpHeaders *h, const char *k);
extern void http_headers_add(HttpHeaders *h, const char *k, const char *v);
extern unsigned int http_headers_size(HttpHeaders *h);
extern const char * http_headers_get_key(HttpHeaders *h, unsigned int i);
extern const char * http_headers_get_val(HttpHeaders *h, unsigned int i);


extern HttpClientOption * http_client_opt_new();
extern HttpClientOption * http_client_opt_new_method(const char *method);
extern void http_client_opt_free(HttpClientOption *opt);
extern void http_client_opt_add_method(HttpClientOption *opt, const char *method);
extern void http_client_opt_add_header(HttpClientOption *opt, const char *k, const char *v);
extern const char * http_client_opt_get_header(HttpClientOption *opt, const char *k);
extern void http_client_opt_add_body(HttpClientOption *opt, const void *body, const size_t size);
extern HttpHeaders *http_client_opt_get_headers(HttpClientOption *opt);
extern void http_client_opt_add_cb_arg(HttpClientOption *opt, void *arg);

extern void http_request_sender_send(RequestSender *sender, const void *data, const size_t size);
extern void http_request_sender_end(RequestSender *sender);
extern Response * http_client_request(const char *url, HttpClientOption *opt);
extern void http_client_stream_request(const char *url, HttpClientOption *opt, http_client_body_sender sender, http_client_body_reader reader);


extern void http_client_response_get_headers(Response *res, HttpHeaders *headers);
extern const char * http_client_response_get_header(Response *res, const char *k);
extern size_t http_client_response_get_body_size(Response *res);
extern void http_client_response_get_body_data(Response *res, void *data);
extern void http_client_response_free(Response *res);


// locks
extern FairLock * fair_lock_new();
extern void fair_lock_acquire(FairLock *lock);
extern void fair_lock_release(FairLock *lock);
extern void fair_lock_destroy(FairLock* lock);


//handlers
extern BaseHandler * base_handler_new();
extern void base_handler_free(BaseHandler *handler);
extern void base_handler_set_on_accept(BaseHandler *handler, server_channel_on_accept);
extern void base_handler_set_channel_on_connect(BaseHandler *handler, channel_on_connect);
extern void base_handler_set_channel_on_read(BaseHandler *handler, channel_on_read);
extern void base_handler_set_channel_on_error(BaseHandler *handler, channel_on_error);
extern void base_handler_set_channel_on_close(BaseHandler *handler, channel_on_close);
extern void base_handler_handle_channel(BaseHandler *handler, Channel *channel);
extern void base_handler_handle_server_channel(BaseHandler *handler, ServerChannel *server);
extern int base_handler_write(Channel *channel, const char *data, const size_t data_len);

#ifdef __cplusplus
}
#endif


#endif


