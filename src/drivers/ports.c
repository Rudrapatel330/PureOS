#include "ports.h"
#include "../kernel/spinlock.h"

static spinlock_irq_t serial_lock = {{0}, 0};
static int serial_lock_initialized = 0;

// Output a byte to a port
void outb(uint16_t port, uint8_t val) {
  __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Input a byte from a port
uint8_t inb(uint16_t port) {
  uint8_t ret;
  __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

// Output a word to a port
void outw(uint16_t port, uint16_t val) {
  __asm__ volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}

// Input a word from a port
uint16_t inw(uint16_t port) {
  uint16_t ret;
  __asm__ volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

// Output a long to a port
void outl(uint16_t port, uint32_t val) {
  __asm__ volatile("outl %0, %1" : : "a"(val), "Nd"(port));
}

// Input a long from a port
uint32_t inl(uint16_t port) {
  uint32_t ret;
  __asm__ volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

// Input string word (for ATA)
void insw(uint16_t port, void *addr, uint32_t count) {
  __asm__ volatile("rep insw" : "+D"(addr), "+c"(count) : "d"(port) : "memory");
}

// Output string word (for ATA)
void outsw(uint16_t port, const void *addr, uint32_t count) {
  __asm__ volatile("rep outsw" : "+S"(addr), "+c"(count) : "d"(port));
}

// SERIAL PORT DEBUGGING
#define PORT_COM1 0x3f8

void serial_init() {
  outb(PORT_COM1 + 1, 0x00); // Disable all interrupts
  outb(PORT_COM1 + 3, 0x80); // Enable DLAB (set baud rate divisor)
  outb(PORT_COM1 + 0, 0x03); // Set divisor to 3 (lo byte) 38400 baud
  outb(PORT_COM1 + 1, 0x00); //                  (hi byte)
  outb(PORT_COM1 + 3, 0x03); // 8 bits, no parity, one stop bit
  outb(PORT_COM1 + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
  outb(PORT_COM1 + 4, 0x0B); // IRQs enabled, RTS/DSR set
}

void write_serial(char c) {
  // Bochs Debug Port 0xE9 (Writes directly to emulator console/log)
  outb(0xE9, c);

  // COM1 Output (Wait for transmit empty)
  while ((inb(PORT_COM1 + 5) & 0x20) == 0)
    ;
  outb(PORT_COM1, c);
}

void print_serial(const char *str) {
  if (!serial_lock_initialized) {
    spinlock_irq_init(&serial_lock);
    serial_lock_initialized = 1;
  }
  spinlock_irq_acquire(&serial_lock);
  while (*str)
    write_serial(*str++);
  spinlock_irq_release(&serial_lock);
}
