#define BR_SANS_INTERRUPTS 1
#include "../drivers/pcnet.h"
#include "../kernel/heap.h"
#include "../kernel/random.h"
#include "../kernel/string.h"
#include "lib/bearssl/inc/bearssl.h"
#include "net.h"

// Define malloc for BearSSL
#ifndef malloc
#define malloc(size) (void *)kmalloc(size)
#endif
extern void print_serial(const char *);
extern void kernel_poll_events(void);
extern uint32_t get_timer_ticks(void);

// Provide BearSSL with a system seeder
static int my_prng_seeder(const br_prng_class **ctx) {
  uint8_t buf[32];
  get_entropy(buf, sizeof(buf));
  (*ctx)->update(ctx, buf, sizeof(buf));
  return 1;
}

typedef struct {
  const br_x509_class *vtable;
  uint8_t
      dummy[4096]; // Buffer to safely hold br_x509_minimal_context during init
  br_x509_decoder_context decoder;
  int is_ee;
  br_x509_pkey *ee_pkey;
} trust_all_context;

static void ta_start_chain(const br_x509_class **ctx, const char *server_name) {
  (void)server_name;
  print_serial("TLS: X509 start_chain (Trust All Mode)\n");
  trust_all_context *t = (trust_all_context *)ctx;
  t->is_ee = 1;
  t->ee_pkey = NULL;
}

static void ta_start_cert(const br_x509_class **ctx, uint32_t length) {
  (void)length;
  trust_all_context *t = (trust_all_context *)ctx;
  if (t->is_ee) {
    print_serial("TLS: Decoding EE Certificate...\n");
    br_x509_decoder_init(&t->decoder, NULL, NULL);
  } else {
    print_serial("TLS: Ignoring Intermediate Certificate\n");
  }
}

static void ta_append(const br_x509_class **ctx, const unsigned char *buf,
                      size_t len) {
  trust_all_context *t = (trust_all_context *)ctx;
  if (t->is_ee) {
    br_x509_decoder_push(&t->decoder, buf, len);
  }
}

static void ta_end_cert(const br_x509_class **ctx) {
  trust_all_context *t = (trust_all_context *)ctx;
  if (t->is_ee) {
    int err = br_x509_decoder_last_error(&t->decoder);
    if (err == 0) {
      t->ee_pkey = br_x509_decoder_get_pkey(&t->decoder);
      print_serial("TLS: EE Public Key extracted successfully.\n");
    } else {
      print_serial("TLS: EE Decoder error: ");
      char ebuf[16];
      k_itoa(err, ebuf);
      print_serial(ebuf);
      print_serial("\n");
    }
    t->is_ee = 0;
  }
}

static unsigned int ta_end_chain(const br_x509_class **ctx) {
  (void)ctx;
  print_serial("TLS: X509 end_chain (Success forced)\n");
  return 0;
}

static const br_x509_pkey *ta_get_pkey(const br_x509_class *const *ctx,
                                       unsigned int *usages) {
  trust_all_context *t = (trust_all_context *)ctx;
  if (usages) {
    *usages = BR_KEYTYPE_KEYX | BR_KEYTYPE_SIGN;
  }
  if (!t->ee_pkey) {
    print_serial("TLS: WARNING - get_pkey called but no key extracted!\n");
  }
  return t->ee_pkey;
}

static const br_x509_class trust_all_vtable = {sizeof(trust_all_context),
                                               ta_start_chain,
                                               ta_start_cert,
                                               ta_append,
                                               ta_end_cert,
                                               ta_end_chain,
                                               ta_get_pkey};

br_prng_seeder br_prng_seeder_system(const char **name) {
  if (name != NULL) {
    *name = "custom_os";
  }
  return &my_prng_seeder;
}

// Get human-readable BearSSL error name
static const char *tls_error_name(int err) {
  switch (err) {
  case 0:
    return "OK";
  case BR_ERR_BAD_PARAM:
    return "BAD_PARAM";
  case BR_ERR_BAD_STATE:
    return "BAD_STATE";
  case BR_ERR_UNSUPPORTED_VERSION:
    return "UNSUPPORTED_VERSION";
  case BR_ERR_BAD_VERSION:
    return "BAD_VERSION";
  case BR_ERR_BAD_LENGTH:
    return "BAD_LENGTH";
  case BR_ERR_TOO_LARGE:
    return "TOO_LARGE";
  case BR_ERR_BAD_MAC:
    return "BAD_MAC";
  case BR_ERR_NO_RANDOM:
    return "NO_RANDOM";
  case BR_ERR_UNKNOWN_TYPE:
    return "UNKNOWN_TYPE";
  case BR_ERR_UNEXPECTED:
    return "UNEXPECTED";
  case BR_ERR_BAD_CCS:
    return "BAD_CCS";
  case BR_ERR_BAD_ALERT:
    return "BAD_ALERT";
  case BR_ERR_BAD_HANDSHAKE:
    return "BAD_HANDSHAKE";
  case BR_ERR_OVERSIZED_ID:
    return "OVERSIZED_ID";
  case BR_ERR_BAD_CIPHER_SUITE:
    return "BAD_CIPHER_SUITE";
  case BR_ERR_BAD_COMPRESSION:
    return "BAD_COMPRESSION";
  case BR_ERR_BAD_FRAGLEN:
    return "BAD_FRAGLEN";
  case BR_ERR_BAD_SECRENEG:
    return "BAD_SECRENEG";
  case BR_ERR_EXTRA_EXTENSION:
    return "EXTRA_EXTENSION";
  case BR_ERR_BAD_SNI:
    return "BAD_SNI";
  case BR_ERR_BAD_HELLO_DONE:
    return "BAD_HELLO_DONE";
  case BR_ERR_LIMIT_EXCEEDED:
    return "LIMIT_EXCEEDED";
  case BR_ERR_BAD_FINISHED:
    return "BAD_FINISHED";
  case BR_ERR_RESUME_MISMATCH:
    return "RESUME_MISMATCH";
  case BR_ERR_INVALID_ALGORITHM:
    return "INVALID_ALGORITHM";
  case BR_ERR_BAD_SIGNATURE:
    return "BAD_SIGNATURE";
  case BR_ERR_WRONG_KEY_USAGE:
    return "WRONG_KEY_USAGE";
  case BR_ERR_NO_CLIENT_AUTH:
    return "NO_CLIENT_AUTH";
  case BR_ERR_IO:
    return "IO_ERROR";
  case BR_ERR_RECV_FATAL_ALERT:
    return "RECV_FATAL_ALERT";
  case BR_ERR_SEND_FATAL_ALERT:
    return "SEND_FATAL_ALERT";
  default:
    return "UNKNOWN";
  }
}

int tls_connect(tls_conn_t *conn, uint32_t ip, uint16_t port,
                const char *hostname) {
  memset(conn, 0, sizeof(tls_conn_t));

  // 1. Establish TCP connection
  if (tcp_connect(&conn->tcp, ip, port) < 0) {
    return -1001;
  }

  // 2. Allocate BearSSL contexts
  conn->ssl_ctx = kmalloc(sizeof(br_ssl_client_context));
  conn->x509_ctx = kmalloc(sizeof(trust_all_context));
  conn->io_buf_len = 65536; // 64KB for safety (bidi requires ~33KB)
  conn->io_buf = kmalloc(conn->io_buf_len);

  if (!conn->ssl_ctx || !conn->x509_ctx || !conn->io_buf) {
    print_serial("TLS: Malloc failed\n");
    return -1;
  }

  br_ssl_client_context *sc = (br_ssl_client_context *)conn->ssl_ctx;
  trust_all_context *tc = (trust_all_context *)conn->x509_ctx;

  // Initialize using the "full" client profile to ensure all modern
  // suites/PRFs/curves are correctly registered. We use our trust_all_context
  // (tc) which is large enough to be safely initialized as a minimal context.
  br_ssl_client_init_full(sc, (br_x509_minimal_context *)tc, NULL, 0);

  // Manual overrides for our custom "Trust All" validation and modern
  // constraints
  tc->vtable = &trust_all_vtable;
  br_ssl_engine_set_x509(&sc->eng, &tc->vtable);
  br_ssl_engine_set_versions(&sc->eng, BR_TLS12, BR_TLS12); // Stick to TLS 1.2

  // Set ALPN extension for HTTP/1.1
  static const char *alpn_protos[] = {"http/1.1"};
  br_ssl_engine_set_protocol_names(&sc->eng, alpn_protos, 1);

  // Buffer management and SNI
  br_ssl_engine_set_buffer(&sc->eng, conn->io_buf, conn->io_buf_len, 1);
  br_ssl_client_reset(sc, hostname, 0);

  // Inject entropy
  uint8_t entropy[32];
  get_entropy(entropy, 32);
  br_ssl_engine_inject_entropy(&sc->eng, entropy, 32);

  // 4. Perform Handshake
  print_serial("TLS: Starting handshake with ");
  print_serial(hostname);
  print_serial("...\n");

  int iterations = 0;
  int idle_count = 0;
  uint32_t handshake_start = get_timer_ticks();

  while (1) {
    // Overall handshake timeout: 45 seconds
    if (get_timer_ticks() - handshake_start > 4500) {
      print_serial("TLS: Handshake timeout (45s)\n");
      return -1003;
    }

    iterations++;
    unsigned int state = br_ssl_engine_current_state(&sc->eng);

    if (iterations % 500 == 1) { // Reduced logging frequency
      char state_str[16];
      k_itoa_hex(state, state_str);
      print_serial("TLS: State=0x");
      print_serial(state_str);
      char iter_str[16];
      k_itoa(iterations, iter_str);
      print_serial(" iter=");
      print_serial(iter_str);
      print_serial("\n");
    }

    if ((state & BR_SSL_SENDAPP) || (state & BR_SSL_RECVAPP)) {
      // Handshake complete, ready to send or receive app data
      br_ssl_session_parameters params;
      br_ssl_engine_get_session_parameters(&sc->eng, &params);
      unsigned int suite = params.cipher_suite;
      char hex[8];
      k_itoa_hex(suite, hex);
      print_serial("TLS: Handshake complete! Cipher: 0x");
      print_serial(hex);
      print_serial(" (");
      char iter_str[16];
      k_itoa(iterations, iter_str);
      print_serial(iter_str);
      char s_str[16];
      k_itoa_hex(state, s_str);
      print_serial(" iterations, state=0x");
      print_serial(s_str);
      print_serial(")\n");
      conn->connected = 1;
      return 0;
    }

    if (state == BR_SSL_CLOSED) {
      int err = br_ssl_engine_last_error(&sc->eng);
      print_serial("TLS: Engine closed. Error: ");
      print_serial(tls_error_name(err));
      print_serial(" (");
      char ebuf[16];
      k_itoa(err, ebuf);
      print_serial(ebuf);
      print_serial(")\n");
      return -(2000 + err);
    }

    if (state & BR_SSL_SENDREC) {
      idle_count = 0;
      size_t len;
      unsigned char *buf = br_ssl_engine_sendrec_buf(&sc->eng, &len);
      if (len > 0) {
        // Send in chunks to avoid overwhelming NIC
        size_t to_send = len;
        if (to_send > 1400)
          to_send = 1400;
        int n = tcp_send(&conn->tcp, buf, (int)to_send);
        if (n < 0) {
          print_serial("TLS: Send failed\n");
          break;
        }
        br_ssl_engine_sendrec_ack(&sc->eng, (size_t)to_send);
      }
      continue;
    }

    if (state & BR_SSL_RECVREC) {
      idle_count = 0;
      size_t len;
      unsigned char *buf = br_ssl_engine_recvrec_buf(&sc->eng, &len);

      // Poll NIC for incoming data
      kernel_poll_events();
      if (conn->tcp.rx_len == 0) {
        // If no data buffered, do a small poll
        static uint8_t poll_pkt[1600];
        uint16_t plen;
        int ret = pcnet_poll(poll_pkt, &plen);
        if (ret > 0) {
          net_receive(poll_pkt, plen);
        }
      }

      // Read directly from TCP buffer
      int n = 0;
      if (conn->tcp.rx_len > 0) {
        n = conn->tcp.rx_len;
        if (n > (int)len)
          n = (int)len;
        memcpy(buf, conn->tcp.rx_buf, n);
        if (n < conn->tcp.rx_len) {
          int remaining = conn->tcp.rx_len - n;
          memmove(conn->tcp.rx_buf, conn->tcp.rx_buf + n, remaining);
          conn->tcp.rx_len = remaining;
        } else {
          conn->tcp.rx_len = 0;
          conn->tcp.rx_ready = 0;
        }
      }

      if (n <= 0) {
        // No data yet, yield briefly
        for (volatile int i = 0; i < 5000; i++)
          ;
        continue;
      }
      br_ssl_engine_recvrec_ack(&sc->eng, (size_t)n);
      continue;
    }

    // No I/O pending - engine is doing internal processing
    idle_count++;

    // Poll NIC during idle
    if (idle_count % 10 == 0) {
      kernel_poll_events();
      uint8_t poll_pkt[1600];
      uint16_t plen;
      int ret = pcnet_poll(poll_pkt, &plen);
      if (ret > 0) {
        net_receive(poll_pkt, plen);
      }
    }

    if (idle_count > 50000) { // Keep an "inner" stuck check but make it larger
      int err = br_ssl_engine_last_error(&sc->eng);
      print_serial("TLS: Stuck idle. Error: ");
      print_serial(tls_error_name(err));
      print_serial(" (");
      char ebuf[16];
      k_itoa(err, ebuf);
      print_serial(ebuf);
      print_serial(")\n");
      break;
    }

    // Small yield to prevent 100% CPU usage in the handshake loop if nothing's
    // happening
    for (volatile int i = 0; i < 1000; i++)
      ;
  }

  print_serial("TLS: Handshake failed (Final iterations: ");
  char itbuf[16];
  k_itoa(iterations, itbuf);
  print_serial(itbuf);
  print_serial(")\n");
  return -1;
}

int tls_send(tls_conn_t *conn, const void *data, int len) {
  print_serial("TLS: Sending ");
  char lbuf[16];
  k_itoa(len, lbuf);
  print_serial(lbuf);
  print_serial(" bytes...\n");

  if (!conn->connected)
    return -1;

  br_ssl_client_context *sc = (br_ssl_client_context *)conn->ssl_ctx;
  const uint8_t *p = (const uint8_t *)data;
  int remaining = len;

  while (remaining > 0) {
    size_t n;
    unsigned char *buf = br_ssl_engine_sendapp_buf(&sc->eng, &n);
    if (n == 0) {
      // Flush engine buffer to TCP
      unsigned int state = br_ssl_engine_current_state(&sc->eng);
      if (state & BR_SSL_SENDREC) {
        size_t plen;
        unsigned char *pbuf = br_ssl_engine_sendrec_buf(&sc->eng, &plen);
        tcp_send(&conn->tcp, pbuf, (int)plen);
        br_ssl_engine_sendrec_ack(&sc->eng, (size_t)plen);
      }
      continue;
    }

    int chunk = (remaining > (int)n) ? (int)n : remaining;
    memcpy(buf, p, chunk);
    br_ssl_engine_sendapp_ack(&sc->eng, (size_t)chunk);
    br_ssl_engine_flush(&sc->eng, 0);

    p += chunk;
    remaining -= chunk;

    // Push packets out
    size_t plen;
    unsigned char *pbuf = br_ssl_engine_sendrec_buf(&sc->eng, &plen);
    if (plen > 0) {
      tcp_send(&conn->tcp, pbuf, (int)plen);
      br_ssl_engine_sendrec_ack(&sc->eng, (size_t)plen);
    }
  }

  return len;
}

int tls_recv(tls_conn_t *conn, void *buf, int max_len) {
  if (!conn->connected)
    return -1;

  br_ssl_client_context *sc = (br_ssl_client_context *)conn->ssl_ctx;
  uint32_t start = get_timer_ticks();

  while (get_timer_ticks() - start < 2000) { // 20s timeout
    kernel_poll_events();
    unsigned int state = br_ssl_engine_current_state(&sc->eng);

    if (state & BR_SSL_RECVAPP) {
      size_t n;
      unsigned char *src = br_ssl_engine_recvapp_buf(&sc->eng, &n);
      if (n > 0) {
        int copy = (max_len > (int)n) ? (int)n : max_len;
        memcpy(buf, src, copy);
        br_ssl_engine_recvapp_ack(&sc->eng, (size_t)copy);
        return copy;
      }
    }

    if (state & BR_SSL_RECVREC) {
      size_t plen;
      unsigned char *pbuf = br_ssl_engine_recvrec_buf(&sc->eng, &plen);
      if (plen > 0) {
        // Use the blocking tcp_recv
        int n = tcp_recv(&conn->tcp, pbuf, (int)plen);
        if (n > 0) {
          br_ssl_engine_recvrec_ack(&sc->eng, (size_t)n);
          start = get_timer_ticks(); // Reset timeout on activity
          continue;                  // Check state again immediately
        } else if (n < 0) {
          print_serial("TLS: recvREC TCP error\n");
          return -1;
        }
        // n == 0 means timeout or closed from TCP's perspective
      }
    }

    if (state & BR_SSL_SENDREC) {
      size_t plen;
      unsigned char *pbuf = br_ssl_engine_sendrec_buf(&sc->eng, &plen);
      if (plen > 0) {
        tcp_send(&conn->tcp, pbuf, (int)plen);
        br_ssl_engine_sendrec_ack(&sc->eng, (size_t)plen);
        continue;
      }
    }

    if (state == BR_SSL_CLOSED) {
      return 0;
    }

    // Small sleep to avoid eating CPU while waiting
    for (int i = 0; i < 1000; i++)
      __asm__("nop");
  }

  print_serial("TLS: Receive timeout\n");
  return 0;
}

void tls_close(tls_conn_t *conn) {
  if (conn->ssl_ctx) {
    br_ssl_engine_close(&((br_ssl_client_context *)conn->ssl_ctx)->eng);
    kfree(conn->ssl_ctx);
  }
  if (conn->x509_ctx)
    kfree(conn->x509_ctx);
  if (conn->io_buf)
    kfree(conn->io_buf);
  tcp_close(&conn->tcp);
  conn->connected = 0;
}
