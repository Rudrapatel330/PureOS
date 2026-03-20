/**
 * wolfssl_io.c - Custom I/O callbacks for PureOS
 * 
 * This file connects wolfSSL to PureOS's custom TCP stack.
 * wolfSSL uses callbacks for all network I/O, allowing us to use
 * our existing tcp_send/tcp_recv functions.
 */

#include <stdint.h>
#include <string.h>
#include "../kernel/heap.h"
#include "../kernel/string.h"
#include "../net/net.h"
#include "../net/tcp.h"

extern void print_serial(const char *);
extern void k_itoa(int, char *);

/*============================================================================*
 * I/O Context Structure
 *============================================================================*/

/* Context structure passed to wolfSSL I/O callbacks */
typedef struct {
    tcp_conn_t *tcp_conn;      /* Our TCP connection */
    uint8_t *recv_buffer;      /* Optional receive buffer */
    int recv_buffer_len;
    int recv_buffer_offset;
} wolfssl_io_ctx_t;

/*============================================================================*
 * Receive Callback
 *============================================================================*/

/**
 * wolfSSL calls this to receive data from the network.
 * 
 * @param ssl   - WolfSSL session (unused)
 * @param buf   - Buffer to receive data into
 * @param sz    - Maximum bytes to receive
 * @param ctx   - Our context (wolfssl_io_ctx_t)
 * @return      - Bytes received, or error code
 */
int pureos_wolfssl_recv(WOLFSSL *ssl, char *buf, int sz, void *ctx) {
    wolfssl_io_ctx_t *io_ctx = (wolfssl_io_ctx_t *)ctx;
    
    if (!io_ctx || !io_ctx->tcp_conn) {
        print_serial("WOLFSSL: Recv called with null context\n");
        return WOLFSSL_CBIO_ERR_WANT_READ;
    }
    
    /* Try to receive data */
    int received = tcp_recv(io_ctx->tcp_conn, buf, sz);
    
    if (received > 0) {
        return received;  /* Success */
    } else if (received == 0) {
        /* No data available yet - would block */
        return WOLFSSL_CBIO_ERR_WANT_READ;
    } else {
        /* Error */
        print_serial("WOLFSSL: Recv error\n");
        return WOLFSSL_CBIO_ERR_CONN_RST;
    }
}

/*============================================================================*
 * Send Callback  
 *============================================================================*/

/**
 * wolfSSL calls this to send data over the network.
 * 
 * @param ssl   - WolfSSL session (unused)
 * @param buf   - Buffer containing data to send
 * @param sz    - Bytes to send
 * @param ctx   - Our context (wolfssl_io_ctx_t)
 * @return      - Bytes sent, or error code
 */
int pureos_wolfssl_send(WOLFSSL *ssl, const char *buf, int sz, void *ctx) {
    wolfssl_io_ctx_t *io_ctx = (wolfssl_io_ctx_t *)ctx;
    
    if (!io_ctx || !io_ctx->tcp_conn) {
        print_serial("WOLFSSL: Send called with null context\n");
        return WOLFSSL_CBIO_ERR_WANT_WRITE;
    }
    
    /* Try to send data */
    int sent = tcp_send(io_ctx->tcp_conn, buf, sz);
    
    if (sent == sz) {
        return sent;  /* Success */
    } else if (sent == 0) {
        /* Would block */
        return WOLFSSL_CBIO_ERR_WANT_WRITE;
    } else {
        /* Error */
        print_serial("WOLFSSL: Send error\n");
        return WOLFSSL_CBIO_ERR_CONN_RST;
    }
}

/*============================================================================*
 * Time Callback (for certificate validation)
 *============================================================================*/

/**
 * wolfSSL calls this to get current time (for certificate validation)
 * 
 * @param time - Output: current time in seconds since epoch
 * @return     - 0 on success
 */
int pureos_wolfssl_time_cb(void *time) {
    /* TODO: Use RTC to get actual time */
    /* For now, return a fixed time to allow cert validation to proceed */
    *(uint32_t *)time = 1700000000;  /* Approximate time */
    return 0;
}

/*============================================================================*
 * Entropy Callback (for random number generation)
 *============================================================================*/

/**
 * wolfSSL calls this to get entropy for random number generation
 * 
 * @param fd    - File descriptor (unused)
 * @param buf   - Buffer to receive entropy
 * @param len   - Bytes requested
 * @return      - Bytes received, or error
 */
int pureos_wolfssl_entropy_cb(void *fd, unsigned char *buf, int len) {
    /* Use our existing entropy function */
    extern void get_entropy(uint8_t *buffer, uint32_t length);
    get_entropy(buf, len);
    return len;
}

/*============================================================================*
 * Memory Callbacks (optional - can use system malloc)
 *============================================================================*/

/* wolfSSL can use system malloc/free by default, but we can override if needed */

/*============================================================================*
 * Public API
 *============================================================================*/

/**
 * Create a new wolfSSL I/O context
 * 
 * @param tcp - Our TCP connection
 * @return    - Context structure, or NULL on error
 */
wolfssl_io_ctx_t *wolfssl_io_create(tcp_conn_t *tcp) {
    wolfssl_io_ctx_t *ctx = (wolfssl_io_ctx_t *)kmalloc(sizeof(wolfssl_io_ctx_t));
    if (!ctx) {
        return NULL;
    }
    
    ctx->tcp_conn = tcp;
    ctx->recv_buffer = NULL;
    ctx->recv_buffer_len = 0;
    ctx->recv_buffer_offset = 0;
    
    return ctx;
}

/**
 * Free a wolfSSL I/O context
 * 
 * @param ctx - Context to free
 */
void wolfssl_io_free(wolfssl_io_ctx_t *ctx) {
    if (ctx) {
        if (ctx->recv_buffer) {
            kfree(ctx->recv_buffer);
        }
        kfree(ctx);
    }
}
