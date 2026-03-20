/**
 * user_settings.h - PureOS Platform Configuration for wolfSSL
 * 
 * This file configures wolfSSL to work with PureOS:
 * - 32-bit x86 environment
 * - No filesystem  
 * - Custom TCP stack
 * - Custom memory allocation
 */

#ifndef PUREOS_USER_SETTINGS_H
#define PUREOS_USER_SETTINGS_H

/*============================================================================*
 * Platform Basics
 *============================================================================*/

/* PureOS is 32-bit x86 */
#define SIZEOF_LONG 4
#define SIZEOF_LONG_LONG 8

/* x86 is little endian */
#define LITTLE_ENDIAN_ORDER

/* No filesystem - we'll load certificates from memory */
#define NO_FILESYSTEM

/* Single-threaded OS */
#define SINGLE_THREADED

/*============================================================================*
 * I/O Configuration
 *============================================================================*/

/* Use custom I/O callbacks (we'll implement these) */
#define WOLFSSL_USER_IO

/* No writev support in our OS */
#define NO_WRITEV

/*============================================================================*
 * TLS Configuration
 *============================================================================*/

/* TLS 1.3 support */
#define WOLFSSL_TLS13
#define HAVE_TLS_EXTENSIONS
#define HAVE_SUPPORTED_CURVES
#define HAVE_HKDF
#define HAVE_AEAD
#define WC_RSA_PSS

/* Enable modern cipher suites */
#define HAVE_AESGCM
#define HAVE_AESCCM
#define GCM_SMALL

/* SHA-256/SHA-384 for TLS 1.3 */
#define WOLFSSL_SHA384
#define WOLFSSL_SHA512

/*============================================================================*
 * Cryptography Configuration  
 *============================================================================*/

/* RSA support */
#define RSA_LOW_MEM
#define WC_RSA_BLINDING

/* ECC support (for ECDHE - needed for forward secrecy) */
#define HAVE_ECC
#define ECC_TIMING_RESISTANT
#define HAVE_ECC_ENCRYPT
#define USE_ECC_SECP256R1

/* DH support for key exchange */
#define HAVE_DH
#define HAVE_FFDHE_2048

/*============================================================================*
 * Memory Configuration
 *============================================================================*/

/* Use small stack allocations */
#define WOLFSSL_SMALL_STACK

/* Custom memory functions - we'll provide these */
#define XMALLOC_USER
#define XFREE_USER  
#define XREALLOC_USER

/* Alignment - 4 bytes for 32-bit */
#define WOLFSSL_GENERAL_ALIGNMENT 4

/*============================================================================*
 * Disable Unneeded Features
 *============================================================================*/

/* Disable older protocols - use TLS 1.2+ only */
#define NO_OLD_TLSNAME
#define NO_TLSv1
#define NO_TLSv1_1

/* Disable insecure algorithms */
#define NO_RC4
#define NO_DES3
#define NO_MD4
#define NO_HC128
#define NO_RABBIT
#define NO_DSA

/* Disable features we don't need */
#define NO_SESSION_CACHE
#define NO_HANDSHAKE_DONE_CB

/*============================================================================*
 * Debug (disable in production)
 *============================================================================*/

/* Uncomment for debugging: */
/* #define DEBUG_WOLFSSL */

#endif /* PUREOS_USER_SETTINGS_H */
