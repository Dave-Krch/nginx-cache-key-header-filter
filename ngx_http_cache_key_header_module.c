#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_http_file_cache.c>

static ngx_int_t ngx_http_cache_key_header(ngx_http_request_t *r);
static ngx_int_t ngx_http_cache_key_header_init(ngx_conf_t *cf);
void key_to_hex(u_char* input, u_char* output);



static ngx_http_module_t  ngx_http_cache_key_header_module_ctx = {
        NULL,                                   /* preconfiguration */
        ngx_http_cache_key_header_init,        /* postconfiguration */

        NULL,                                   /* create main configuration */
        NULL,                                   /* init main configuration */

        NULL,                                   /* create server configuration */
        NULL,                                   /* merge server configuration */

        NULL,                                   /* create location configuration */
        NULL                                    /* merge location configuration */
};


ngx_module_t  ngx_http_cache_key_header_module = {
        NGX_MODULE_V1,
        &ngx_http_cache_key_header_module_ctx, /* module context */
        NULL,                                   /* module directives */
        NGX_HTTP_MODULE,                        /* module type */
        NULL,                                   /* init master */
        NULL,                                   /* init module */
        NULL,                                   /* init process */
        NULL,                                   /* init thread */
        NULL,                                   /* exit thread */
        NULL,                                   /* exit process */
        NULL,                                   /* exit master */
        NGX_MODULE_V1_PADDING
};


static ngx_http_output_header_filter_pt  ngx_http_next_header_filter;


static ngx_int_t
ngx_http_cache_key_header(ngx_http_request_t *r)
{
    ngx_table_elt_t  *h;

    if (r->headers_out.status != NGX_HTTP_OK) {
        return ngx_http_next_header_filter(r);
    }

#if (NGX_HTTP_CACHE)

    //Add header
    h = ngx_list_push(&r->headers_out.headers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    h->hash = 1;
    ngx_str_set(&h->key, "X-Cache-Key");
    ngx_str_set(&h->value, r->cache->key);


    //Add header key hexadec
    u_char key[NGX_HTTP_CACHE_KEY_LEN];
    memcpy(key, r->cache->key, NGX_HTTP_CACHE_KEY_LEN);

    u_char key_hex[(NGX_HTTP_CACHE_KEY_LEN * 2) + 1];

    key_to_hex(key, key_hex);

    h = ngx_list_push(&r->headers_out.headers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    h->hash = 1;
    ngx_str_set(&h->key, "X-Cache-Key-Hexadec");
    ngx_str_set(&h->value, key_hex);

#endif

    return ngx_http_next_header_filter(r);
}

static ngx_int_t
ngx_http_cache_key_header_init(ngx_conf_t *cf)
{
    ngx_http_next_header_filter = ngx_http_top_header_filter;
    ngx_http_top_header_filter = ngx_http_cache_key_header;

    return NGX_OK;
}

void key_to_hex(u_char* input, u_char* output) {
    for (int i = 0; i < NGX_HTTP_CACHE_KEY_LEN; i++) {
        sprintf((char*)&output[i*2], "%02X", input[i]);
    }
    input[(NGX_HTTP_CACHE_KEY_LEN * 2) + 1] = '\0';
}