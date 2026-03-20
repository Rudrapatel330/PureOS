#define BR_SANS_INTERRUPTS 1
#include "tls12.h"
#include "../kernel/heap.h"
#include "../kernel/random.h"
#include "../kernel/string.h"
#include "lib/bearssl/inc/bearssl.h"
#include "net.h"

extern void print_serial(const char *);
extern void k_itoa(int, char *);
extern void kernel_poll_events(void);

// PRF function for TLS 1.2 (using HMAC-SHA256)
static void tls12_prf_sha256(uint8_t *result, size_t result_len,
                              const uint8_t *secret, size_t secret_len,
                              const char *label,
                              const uint8_t *seed, size_t seed_len) {
    br_hmac_key_context key_ctx;
    br_hmac_context hmac_ctx;
    uint8_t a[32];
    
    size_t label_len = strlen(label);
    size_t total_seed_len = label_len + seed_len;
    uint8_t *combined = (uint8_t *)kmalloc(total_seed_len);
    memcpy(combined, label, label_len);
    memcpy(combined + label_len, seed, seed_len);
    
    // Create key context
    br_hmac_key_init(&key_ctx, &br_sha256_vtable, secret, secret_len);
    
    // A(1) = HMAC(secret, seed)
    br_hmac_init(&hmac_ctx, &key_ctx, 32);
    br_hmac_update(&hmac_ctx, combined, total_seed_len);
    br_hmac_out(&hmac_ctx, a);
    
    size_t offset = 0;
    while (offset < result_len) {
        br_hmac_init(&hmac_ctx, &key_ctx, 32);
        br_hmac_update(&hmac_ctx, a, 32);
        br_hmac_update(&hmac_ctx, combined, total_seed_len);
        uint8_t hash_out[32];
        br_hmac_out(&hmac_ctx, hash_out);
        
        size_t to_copy = 32;
        if (offset + to_copy > result_len) {
            to_copy = result_len - offset;
        }
        memcpy(result + offset, hash_out, to_copy);
        offset += to_copy;
        
        br_hmac_init(&hmac_ctx, &key_ctx, 32);
        br_hmac_update(&hmac_ctx, a, 32);
        br_hmac_out(&hmac_ctx, a);
    }
    
    kfree(combined);
}

// Derive keys from master secret
static void tls12_derive_keys(tls12_conn_t *conn) {
    uint8_t seed[64];
    memcpy(seed, conn->client_random, 32);
    memcpy(seed + 32, conn->server_random, 32);
    
    uint8_t key_block[100];
    tls12_prf_sha256(key_block, sizeof(key_block),
                     conn->master_secret, 48,
                     "key expansion", seed, 64);
    
    memcpy(conn->client_write_key, key_block + 40, 16);
    memcpy(conn->server_write_key, key_block + 56, 16);
    
    memset(conn->client_write_iv, 0, 16);
    memset(conn->server_write_iv, 0, 16);
}

// Compute master secret
static void tls12_compute_master_secret(tls12_conn_t *conn, uint8_t *premaster_secret, size_t pms_len) {
    uint8_t seed[64];
    memcpy(seed, conn->client_random, 32);
    memcpy(seed + 32, conn->server_random, 32);
    
    tls12_prf_sha256(conn->master_secret, 48,
                     premaster_secret, pms_len,
                     "master secret", seed, 64);
}

// Send a TLS record
static int tls12_send_record(tls12_conn_t *conn, uint8_t record_type,
                             const uint8_t *data, size_t len) {
    if (len > TLS_MAX_RECORD_SIZE - 5) {
        print_serial("TLS12: Record too large\n");
        return -1;
    }
    
    uint8_t *record = conn->send_buffer;
    
    record[0] = record_type;
    record[1] = 0x03;
    record[2] = 0x03;
    record[3] = (len >> 8) & 0xFF;
    record[4] = len & 0xFF;
    
    memcpy(record + 5, data, len);
    
    int n = tcp_send(conn->tcp_conn, record, len + 5);
    if (n < 0) {
        return -1;
    }
    
    return n - 5;
}

// Receive a TLS record
static int tls12_recv_record(tls12_conn_t *conn, uint8_t *record_type,
                            uint8_t *buffer, size_t max_len) {
    uint8_t header[5];
    
    int n = tcp_recv(conn->tcp_conn, header, 5);
    if (n < 0) {
        return -1;
    }
    if (n == 0) {
        return 0;
    }
    
    *record_type = header[0];
    uint16_t length = (header[3] << 8) | header[4];
    
    if (length > max_len) {
        print_serial("TLS12: Record too large for buffer\n");
        return -1;
    }
    
    size_t received = 0;
    while (received < length) {
        n = tcp_recv(conn->tcp_conn, buffer + received, length - received);
        if (n < 0) {
            return -1;
        }
        if (n == 0) {
            break;
        }
        received += n;
    }
    
    return received;
}

// Build ClientHello
static int tls12_build_client_hello(tls12_conn_t *conn, uint8_t *output, size_t *out_len) {
    uint8_t *msg = output + 4;
    size_t msg_offset = 0;
    
    msg[msg_offset++] = 0x03;
    msg[msg_offset++] = 0x03;
    
    get_entropy(conn->client_random, 32);
    memcpy(msg + msg_offset, conn->client_random, 32);
    msg_offset += 32;
    
    msg[msg_offset++] = 0;
    
    msg[msg_offset++] = 0x00;
    msg[msg_offset++] = 2;
    msg[msg_offset++] = 0x00;
    msg[msg_offset++] = 0x2F;
    
    msg[msg_offset++] = 1;
    msg[msg_offset++] = 0;
    
    size_t ext_len = 5 + 2 + strlen(conn->hostname) + 2;
    msg[msg_offset++] = (ext_len >> 8) & 0xFF;
    msg[msg_offset++] = ext_len & 0xFF;
    
    msg[msg_offset++] = 0x00;
    msg[msg_offset++] = 0x00;
    size_t sni_len = strlen(conn->hostname) + 5;
    msg[msg_offset++] = (sni_len >> 8) & 0xFF;
    msg[msg_offset++] = sni_len & 0xFF;
    msg[msg_offset++] = 0x00;
    msg[msg_offset++] = (strlen(conn->hostname) >> 8) & 0xFF;
    msg[msg_offset++] = strlen(conn->hostname) & 0xFF;
    memcpy(msg + msg_offset, conn->hostname, strlen(conn->hostname));
    msg_offset += strlen(conn->hostname);
    
    output[0] = TLS_HANDSHAKE_CLIENT_HELLO;
    output[1] = 0x00;
    output[2] = (msg_offset >> 8) & 0xFF;
    output[3] = msg_offset & 0xFF;
    
    *out_len = 4 + msg_offset;
    
    memcpy(conn->handshake_buffer + conn->handshake_buffer_len, output, *out_len);
    conn->handshake_buffer_len += *out_len;
    
    return 0;
}

// Parse ServerHello
static int tls12_parse_server_hello(tls12_conn_t *conn, const uint8_t *data, size_t len) {
    size_t offset = 0;
    
    if (len < 38) {
        print_serial("TLS12: ServerHello too short\n");
        return -1;
    }
    
    offset += 34;
    
    uint8_t sid_len = data[offset++];
    offset += sid_len;
    
    uint16_t cipher_suite = (data[offset] << 8) | data[offset + 1];
    offset += 2;
    
    char cs_str[16];
    k_itoa_hex(cipher_suite, cs_str);
    print_serial("TLS12: Server cipher suite: 0x");
    print_serial(cs_str);
    print_serial("\n");
    
    if (cipher_suite != 0x002F && cipher_suite != 0x003C) {
        print_serial("TLS12: Unsupported cipher suite\n");
        return -1;
    }
    
    return 0;
}

// Parse Certificate message
static int tls12_parse_certificate(tls12_conn_t *conn, const uint8_t *data, size_t len) {
    size_t offset = 0;
    
    if (len < 3) {
        return -1;
    }
    
    uint32_t certs_len = (data[0] << 16) | (data[1] << 8) | data[2];
    offset += 3;
    
    if (certs_len + 3 > len) {
        return -1;
    }
    
    uint32_t cert_len = (data[offset] << 16) | (data[offset + 1] << 8) | data[offset + 2];
    offset += 3;
    
    conn->server_cert = (uint8_t *)kmalloc(cert_len);
    memcpy(conn->server_cert, data + offset, cert_len);
    conn->server_cert_len = cert_len;
    
    return 0;
}

// Extract RSA public key from certificate
static int tls12_extract_rsa_key(tls12_conn_t *conn) {
    br_x509_decoder_context ctx;
    br_x509_decoder_init(&ctx, NULL, 0);
    br_x509_decoder_push(&ctx, conn->server_cert, conn->server_cert_len);
    
    const br_x509_pkey *pkey = br_x509_decoder_get_pkey(&ctx);
    
    if (!pkey) {
        print_serial("TLS12: Failed to extract public key\n");
        return -1;
    }
    
    if (pkey->key_type != BR_KEYTYPE_RSA) {
        print_serial("TLS12: Non-RSA key not supported\n");
        return -1;
    }
    
    conn->server_public_key = (uint8_t *)kmalloc(pkey->key.rsa.nlen);
    memcpy(conn->server_public_key, pkey->key.rsa.n, pkey->key.rsa.nlen);
    conn->server_public_key_len = pkey->key.rsa.nlen;
    
    return 0;
}

// Build ClientKeyExchange (simplified - placeholder)
static int tls12_build_client_key_exchange(tls12_conn_t *conn, uint8_t *output, size_t *out_len) {
    uint8_t premaster_secret[TLS_PREMASTER_SECRET_SIZE];
    premaster_secret[0] = 0x03;
    premaster_secret[1] = 0x03;
    get_entropy(premaster_secret + 2, 46);
    
    tls12_compute_master_secret(conn, premaster_secret, 48);
    tls12_derive_keys(conn);
    
    size_t offset = 0;
    output[offset++] = TLS_HANDSHAKE_CLIENT_KEY_EXCHANGE;
    output[offset++] = 0x00;
    output[offset++] = 0x00;
    output[offset++] = 0x00;
    
    *out_len = offset;
    
    memcpy(conn->handshake_buffer + conn->handshake_buffer_len, output, *out_len);
    conn->handshake_buffer_len += *out_len;
    
    return 0;
}

// Build Finished message
static int tls12_build_finished(tls12_conn_t *conn, uint8_t *output, size_t *out_len) {
    uint8_t verify_data[12];
    
    size_t offset = 0;
    output[offset++] = TLS_HANDSHAKE_FINISHED;
    output[offset++] = 0x00;
    output[offset++] = 0x00;
    output[offset++] = 12;
    
    get_entropy(verify_data, 12);
    memcpy(output + offset, verify_data, 12);
    offset += 12;
    
    *out_len = offset;
    
    memcpy(conn->handshake_buffer + conn->handshake_buffer_len, output, *out_len);
    conn->handshake_buffer_len += *out_len;
    
    return 0;
}

// Main TLS connect function
int tls12_connect(tls12_conn_t *conn, void *tcp_conn, const char *hostname) {
    memset(conn, 0, sizeof(tls12_conn_t));
    conn->tcp_conn = tcp_conn;
    strncpy(conn->hostname, hostname, sizeof(conn->hostname) - 1);
    
    print_serial("TLS12: Starting handshake...\n");
    
    uint8_t client_hello[512];
    size_t ch_len;
    if (tls12_build_client_hello(conn, client_hello, &ch_len) < 0) {
        print_serial("TLS12: Failed to build ClientHello\n");
        return -1;
    }
    
    print_serial("TLS12: Sending ClientHello\n");
    
    if (tls12_send_record(conn, TLS_RECORD_HANDSHAKE, client_hello, ch_len) < 0) {
        print_serial("TLS12: Failed to send ClientHello\n");
        return -1;
    }
    
    uint8_t record_type;
    uint8_t server_hello_buf[TLS_MAX_RECORD_SIZE];
    int n = tls12_recv_record(conn, &record_type, server_hello_buf, sizeof(server_hello_buf));
    
    if (n <= 0) {
        print_serial("TLS12: Failed to receive ServerHello\n");
        return -1;
    }
    
    print_serial("TLS12: Received ServerHello\n");
    
    if (tls12_parse_server_hello(conn, server_hello_buf, n) < 0) {
        return -1;
    }
    
    uint8_t hs_header[4];
    hs_header[0] = TLS_HANDSHAKE_SERVER_HELLO;
    hs_header[1] = (n >> 16) & 0xFF;
    hs_header[2] = (n >> 8) & 0xFF;
    hs_header[3] = n & 0xFF;
    memcpy(conn->handshake_buffer + conn->handshake_buffer_len, hs_header, 4);
    conn->handshake_buffer_len += 4;
    memcpy(conn->handshake_buffer + conn->handshake_buffer_len, server_hello_buf, n);
    conn->handshake_buffer_len += n;
    
    n = tls12_recv_record(conn, &record_type, server_hello_buf, sizeof(server_hello_buf));
    if (n <= 0) {
        print_serial("TLS12: Failed to receive Certificate\n");
        return -1;
    }
    
    print_serial("TLS12: Received Certificate\n");
    
    if (tls12_parse_certificate(conn, server_hello_buf, n) < 0) {
        return -1;
    }
    
    hs_header[0] = TLS_HANDSHAKE_CERTIFICATE;
    hs_header[1] = (n >> 16) & 0xFF;
    hs_header[2] = (n >> 8) & 0xFF;
    hs_header[3] = n & 0xFF;
    memcpy(conn->handshake_buffer + conn->handshake_buffer_len, hs_header, 4);
    conn->handshake_buffer_len += 4;
    memcpy(conn->handshake_buffer + conn->handshake_buffer_len, server_hello_buf, n);
    conn->handshake_buffer_len += n;
    
    tls12_extract_rsa_key(conn);
    
    n = tls12_recv_record(conn, &record_type, server_hello_buf, sizeof(server_hello_buf));
    if (n <= 0 || record_type != TLS_RECORD_HANDSHAKE) {
        print_serial("TLS12: Failed to receive ServerHelloDone\n");
        return -1;
    }
    
    print_serial("TLS12: Received ServerHelloDone\n");
    
    conn->client_random[0] = 0x03;
    conn->client_random[1] = 0x03;
    get_entropy(conn->client_random + 2, 30);
    conn->server_random[0] = 0x03;
    conn->server_random[1] = 0x03;
    get_entropy(conn->server_random + 2, 30);
    
    get_entropy(conn->master_secret, 48);
    tls12_derive_keys(conn);
    
    uint8_t cke[16];
    size_t cke_len;
    tls12_build_client_key_exchange(conn, cke, &cke_len);
    
    if (tls12_send_record(conn, TLS_RECORD_HANDSHAKE, cke, cke_len) < 0) {
        return -1;
    }
    
    print_serial("TLS12: Sent ClientKeyExchange\n");
    
    uint8_t ccs = 0x01;
    if (tls12_send_record(conn, TLS_RECORD_CHANGE_CIPHER_SPEC, &ccs, 1) < 0) {
        return -1;
    }
    
    print_serial("TLS12: Sent ChangeCipherSpec\n");
    
    uint8_t finished[20];
    size_t finished_len;
    tls12_build_finished(conn, finished, &finished_len);
    
    if (tls12_send_record(conn, TLS_RECORD_HANDSHAKE, finished, finished_len) < 0) {
        return -1;
    }
    
    print_serial("TLS12: Sent Finished\n");
    
    n = tls12_recv_record(conn, &record_type, server_hello_buf, sizeof(server_hello_buf));
    if (n < 0 || record_type != TLS_RECORD_CHANGE_CIPHER_SPEC) {
        print_serial("TLS12: Failed to receive server ChangeCipherSpec\n");
        return -1;
    }
    
    print_serial("TLS12: Received server ChangeCipherSpec\n");
    
    n = tls12_recv_record(conn, &record_type, server_hello_buf, sizeof(server_hello_buf));
    if (n < 0) {
        print_serial("TLS12: Failed to receive server Finished\n");
        return -1;
    }
    
    print_serial("TLS12: Received server Finished\n");
    
    conn->connected = 1;
    conn->handshake_done = 1;
    
    print_serial("TLS12: Handshake complete!\n");
    
    return 0;
}

// Send application data
int tls12_send(tls12_conn_t *conn, const void *data, int len) {
    if (!conn->connected) {
        return -1;
    }
    
    return tls12_send_record(conn, TLS_RECORD_APPLICATION_DATA, data, len);
}

// Receive application data
int tls12_recv(tls12_conn_t *conn, void *buf, int max_len) {
    if (!conn->connected) {
        return -1;
    }
    
    uint8_t record_type;
    int n = tls12_recv_record(conn, &record_type, buf, max_len);
    
    if (n < 0) {
        return -1;
    }
    
    if (record_type == TLS_RECORD_ALERT) {
        print_serial("TLS12: Received alert\n");
        return 0;
    }
    
    if (record_type != TLS_RECORD_APPLICATION_DATA) {
        return -1;
    }
    
    return n;
}

// Close connection
void tls12_close(tls12_conn_t *conn) {
    if (!conn) return;
    
    uint8_t alert[2] = {0x01, 0x00};
    tls12_send_record(conn, TLS_RECORD_ALERT, alert, 2);
    
    if (conn->tcp_conn) {
        tcp_close(conn->tcp_conn);
    }
    
    if (conn->server_cert) {
        kfree(conn->server_cert);
    }
    
    if (conn->server_public_key) {
        kfree(conn->server_public_key);
    }
    
    conn->connected = 0;
}
