#include "config.h"
#include "inner.h"

/* SSL/TLS Engine - NO T0 files here */
#include "ssl/ssl_client.c"
#include "ssl/ssl_client_full.c"
#include "ssl/ssl_engine.c"

/* #include "ssl/ssl_hs_client.c" -- COMPILE SEPARATELY */
#include "ssl/ssl_hashes.c"
#include "ssl/ssl_io.c"
#include "ssl/ssl_keyexport.c"
#include "ssl/ssl_rec_cbc.c"
#include "ssl/ssl_rec_ccm.c"
#include "ssl/ssl_rec_chapol.c"
#include "ssl/ssl_rec_gcm.c"

/* Default implementations */
#include "ssl/ssl_client_default_rsapub.c"
#include "ssl/ssl_engine_default_aescbc.c"
#include "ssl/ssl_engine_default_aesccm.c"
#include "ssl/ssl_engine_default_aesgcm.c"
#include "ssl/ssl_engine_default_chapol.c"
#include "ssl/ssl_engine_default_descbc.c"
#include "ssl/ssl_engine_default_ec.c"
#include "ssl/ssl_engine_default_ecdsa.c"
#include "ssl/ssl_engine_default_rsavrfy.c"

/* Hash Functions */
#include "hash/dig_oid.c"
#include "hash/dig_size.c"
#include "hash/ghash_ctmul.c"
#include "hash/md5.c"
#undef F
#undef G
#undef H
#undef I
#include "hash/md5sha1.c"
#undef F
#undef G
#undef H
#undef I
#include "hash/multihash.c"
#include "hash/sha1.c"
#undef F
#undef G
#undef H
#undef I
#include "hash/sha2big.c"
#include "hash/sha2small.c"

/* HMAC and PRNG */
#include "mac/hmac.c"
#include "rand/hmac_drbg.c"

/* Symmetric Ciphers */
#include "symcipher/aes_common.c"
#include "symcipher/aes_ct.c"
#include "symcipher/aes_ct64.c"
#include "symcipher/aes_ct64_cbcdec.c"
#include "symcipher/aes_ct64_cbcenc.c"
#include "symcipher/aes_ct64_ctr.c"
#include "symcipher/aes_ct64_ctrcbc.c"
#include "symcipher/aes_ct_cbcdec.c"
#include "symcipher/aes_ct_cbcenc.c"
#include "symcipher/aes_ct_ctr.c"
#include "symcipher/aes_ct_ctrcbc.c"
#include "symcipher/chacha20_ct.c"
#include "symcipher/des_ct.c"
#include "symcipher/des_ct_cbcdec.c"
#include "symcipher/des_ct_cbcenc.c"
#include "symcipher/poly1305_ctmul.c"

/* EC */
#include "ec/ec_curve25519.c"
#include "ec/ec_prime_i31.c"
#undef t1
#undef t2
#undef t3
#undef t4
#undef t5
#undef t6
#undef t7
#undef t8
#undef t9
#undef t10
#undef Px
#undef Py
#undef Pz
#undef P1x
#undef P1y
#undef P1z
#undef P2x
#undef P2y
#undef P2z
#undef MSET
#undef MADD
#undef MSUB
#undef MMUL
#undef MINV
#undef MTZ
#undef ENDCODE

#include "ec/ecdsa_i31_sign_asn1.c"
#include "ec/ecdsa_i31_vrfy_asn1.c"

/* RSA */
#include "rsa/rsa_i31_modulus.c"
#include "rsa/rsa_i31_pkcs1_sign.c"
#include "rsa/rsa_i31_pkcs1_vrfy.c"
#include "rsa/rsa_i31_priv.c"
#undef U
#undef TLEN
#include "rsa/rsa_i31_pub.c"
#undef U
#undef TLEN
#include "rsa/rsa_pkcs1_sig_pad.c"
#include "rsa/rsa_pkcs1_sig_unpad.c"

/* X.509 - NO T0 files here */
/* #include "x509/x509_decoder.c" -- COMPILE SEPARATELY */
/* #include "x509/x509_minimal.c" -- COMPILE SEPARATELY */
#include "x509/x509_knownkey.c"
#include "x509/x509_minimal_full.c"

/* Big Integer and Core Utilities */
#include "int/i31_add.c"
#include "int/i31_decode.c"
#include "int/i31_encode.c"
#include "int/i31_modpow.c"
#include "int/i31_montmul.c"
#include "int/i31_mulacc.c"
#include "int/i31_sub.c"
