#ifndef LWIP_ARCH_CC_H
#define LWIP_ARCH_CC_H

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <limits.h>

#ifndef SSIZE_MAX
#define SSIZE_MAX LONG_MAX
#endif

#include "../../../kernel/string.h" 

// Rename conflicting symbols
#define mem_init lwip_mem_init
#define tcp_connect lwip_tcp_connect
#define tcp_recv lwip_tcp_recv
#define tcp_close lwip_tcp_close
#define udp_send lwip_udp_send
#define tcp_init lwip_tcp_init
#define udp_init lwip_udp_init

extern uint32_t k_rand(void);
#define LWIP_RAND() k_rand()

#define BYTE_ORDER LITTLE_ENDIAN

typedef uint8_t u8_t;
typedef int8_t s8_t;
typedef uint16_t u16_t;
typedef int16_t s16_t;
typedef uint32_t u32_t;
typedef int32_t s32_t;
typedef uintptr_t mem_ptr_t;

#define LWIP_ERR_T int

#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_STRUCT __attribute__ ((__packed__))
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x

#define U16_F "hu"
#define S16_F "hd"
#define X16_F "hx"
#define U32_F "u"
#define S32_F "d"
#define X32_F "x"

extern void print_serial(const char*);
#define LWIP_PLATFORM_DIAG(x) do { print_serial("LWIP: "); } while(0)
#define LWIP_PLATFORM_ASSERT(x) do { print_serial("LWIP ASSERT: "); print_serial(x); print_serial("\n"); } while(0)

#endif /* LWIP_ARCH_CC_H */
