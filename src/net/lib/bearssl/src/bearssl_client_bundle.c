#include "config.h"
#include "inner.h"

/* SSL/TLS Engine */
#include "ssl/ssl_client.c"
#include "ssl/ssl_client_full.c"
#include "ssl/ssl_engine.c"
#include "ssl/ssl_hs_client.c"


#undef T0_INTERPRETED
#undef T0_ENTER
#undef T0_NEXT
#undef T0_DEFENTRY
#undef T0_LOCAL
#undef T0_POP
#undef T0_POPi
#undef T0_PEEK
#undef T0_PEEKi
#undef T0_PUSH
#undef T0_PUSHi
#undef T0_RPOP
#undef T0_RPOPi
#undef T0_RPUSH
#undef T0_RPUSHi
#undef T0_ROLL
#undef T0_SWAP
#undef T0_ROT
#undef T0_NROT
#undef T0_PICK
#undef T0_CO
#undef T0_RET
#undef T0_VBYTE
#undef T0_FBYTE
#undef T0_SBYTE
#undef T0_INT1
#undef T0_INT2
#undef T0_INT3
#undef T0_INT4
#undef T0_INT5

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
#include "hash/md5sha1.c"
#include "hash/multihash.c"
#include "hash/sha1.c"
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
#include "ec/ecdsa_i31_sign_asn1.c"
#include "ec/ecdsa_i31_vrfy_asn1.c"

/* RSA */
#include "rsa/rsa_i31_modulus.c"
#include "rsa/rsa_i31_pkcs1_sign.c"
#include "rsa/rsa_i31_pkcs1_vrfy.c"
#include "rsa/rsa_i31_priv.c"
#include "rsa/rsa_i31_pub.c"
#include "rsa/rsa_pkcs1_sig_pad.c"
#include "rsa/rsa_pkcs1_sig_unpad.c"

/* X.509 */
#include "x509/x509_decoder.c"
#undef T0_INTERPRETED
#undef T0_ENTER
#undef T0_NEXT
#undef T0_DEFENTRY
#undef T0_LOCAL
#undef T0_POP
#undef T0_POPi
#undef T0_PEEK
#undef T0_PEEKi
#undef T0_PUSH
#undef T0_PUSHi
#undef T0_RPOP
#undef T0_RPOPi
#undef T0_RPUSH
#undef T0_RPUSHi
#undef T0_ROLL
#undef T0_SWAP
#undef T0_ROT
#undef T0_NROT
#undef T0_PICK
#undef T0_CO
#undef T0_RET
#undef T0_VBYTE
#undef T0_FBYTE
#undef T0_SBYTE
#undef T0_INT1
#undef T0_INT2
#undef T0_INT3
#undef T0_INT4
#undef T0_INT5
#undef CTX
#undef CONTEXT_NAME

#include "x509/x509_knownkey.c"
#include "x509/x509_minimal.c"
#include "x509/x509_minimal_full.c"


/* UNDEF all common internal macros to prevent leaks in the bundle */
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
#undef U
#undef TLEN
#undef MSET
#undef MADD
#undef MSUB
#undef MMUL
#undef MINV
#undef MTZ
#undef ENDCODE

/* Big Integer and Core Utilities */
#include "int/i31_add.c"
#include "int/i31_decode.c"
#include "int/i31_encode.c"
#include "int/i31_modpow.c"
#include "int/i31_montmul.c"
#include "int/i31_mulacc.c"
#include "int/i31_sub.c"

