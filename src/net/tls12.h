#ifndef TLS12_H
#define TLS12_H

#include <stdint.h>
#include <stddef.h>

// TLS Record Types
#define TLS_RECORD_CHANGE_CIPHER_SPEC 20
#define TLS_RECORD_ALERT 21
#define TLS_RECORD_HANDSHAKE 22
#define TLS_RECORD_APPLICATION_DATA 23

// TLS Handshake Types
#define TLS_HANDSHAKE_CLIENT_HELLO 1
#define TLS_HANDSHAKE_SERVER_HELLO 2
#define TLS_HANDSHAKE_CERTIFICATE 11
#define TLS_HANDSHAKE_SERVER_KEY_EXCHANGE 12
#define TLS_HANDSHAKE_CERTIFICATE_REQUEST 13
#define TLS_HANDSHAKE_SERVER_HELLO_DONE 14
#define TLS_HANDSHAKE_CERTIFICATE_VERIFY 15
#define TLS_HANDSHAKE_CLIENT_KEY_EXCHANGE 16
#define TLS_HANDSHAKE_FINISHED 20

// TLS Versions
#define TLS_VERSION_1_0 0x0301
#define TLS_VERSION_1_1 0x0302
#define TLS_VERSION_1_2 0x0303

// Cipher Suites (we'll use TLS_RSA_WITH_AES_128_CBC_SHA = 0x002F)
#define TLS_RSA_WITH_AES_128_CBC_SHA 0x002F
#define TLS_RSA_WITH_AES_128_CBC_SHA256 0x003C

// Compression methods
#define TLS_COMPRESSION_NULL 0

// Max sizes
#define TLS_MAX_HANDSHAKE_SIZE 16384
#define TLS_MAX_RECORD_SIZE 16384 + 512
#define TLS_CLIENT_RANDOM_SIZE 32
#define TLS_SERVER_RANDOM_SIZE 32
#define TLS_PREMASTER_SECRET_SIZE 48
#define TLS_MASTER_SECRET_SIZE 48
#define TLS_KEY_BLOCK_SIZE 40  // client_write_mac_key(20) + server_write_mac_key(20)
#define TLS_AES_BLOCK_SIZE 16

// TLS 1.2 Connection State
typedef struct tls12_connection {
    // TCP connection
    void* tcp_conn;
    
    // State
    int connected;
    int handshake_done;
    
    // Random values
    uint8_t client_random[TLS_CLIENT_RANDOM_SIZE];
    uint8_t server_random[TLS_SERVER_RANDOM_SIZE];
    
    // Keys
    uint8_t master_secret[TLS_MASTER_SECRET_SIZE];
    uint8_t client_write_key[16];  // AES-128
    uint8_t server_write_key[16];
    uint8_t client_write_iv[16];
    uint8_t server_write_iv[16];
    
    // Sequence numbers (for MAC)
    uint64_t client_seq;
    uint64_t server_seq;
    
    // Server certificate (for RSA)
    uint8_t* server_cert;
    size_t server_cert_len;
    uint8_t* server_public_key;
    size_t server_public_key_len;
    
    // Handshake buffer
    uint8_t handshake_buffer[TLS_MAX_HANDSHAKE_SIZE];
    size_t handshake_buffer_len;
    
    // Send buffer
    uint8_t send_buffer[TLS_MAX_RECORD_SIZE];
    size_t send_buffer_len;
    
    // Hostname for SNI
    char hostname[256];
} tls12_conn_t;

// Initialize TLS 1.2 connection
int tls12_connect(tls12_conn_t* conn, void* tcp_conn, const char* hostname);

// Send data
int tls12_send(tls12_conn_t* conn, const void* data, int len);

// Receive data  
int tls12_recv(tls12_conn_t* conn, void* buf, int max_len);

// Close connection
void tls12_close(tls12_conn_t* conn);

#endif
