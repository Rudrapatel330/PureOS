#include "tests.h"
#include "heap.h"
#include "memory.h"
#include "string.h"
#include "task.h"

extern void print_serial(const char *str);
extern void k_itoa(int n, char s[]);
extern void k_itoa_hex(uint32_t n, char s[]);

static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(cond)                                                      \
  do {                                                                         \
    if (!(cond)) {                                                             \
      print_serial("  [FAIL] " #cond "\n");                                    \
      tests_failed++;                                                          \
    } else {                                                                   \
      print_serial("  [PASS] " #cond "\n");                                    \
      tests_passed++;                                                          \
    }                                                                          \
    tests_run++;                                                               \
  } while (0)

// --- Memory Tests ---
static void test_memcpy() {
  print_serial("Running test_memcpy...\n");
  char src[32] = "Hello, World Test Stream!";
  char dst[32] = {0};

  memcpy(dst, src, 32);
  TEST_ASSERT(strcmp(src, dst) == 0);
}

static void test_memset() {
  print_serial("Running test_memset...\n");
  char buf[16];
  memset(buf, 'A', 15);
  buf[15] = 0;

  TEST_ASSERT(strcmp(buf, "AAAAAAAAAAAAAAA") == 0);
}

// --- Heap Tests ---
static void test_heap_alloc() {
  print_serial("Running test_heap_alloc...\n");
  void *ptr1 = kmalloc(128);
  void *ptr2 = kmalloc(256);

  TEST_ASSERT(ptr1 != 0);
  TEST_ASSERT(ptr2 != 0);
  TEST_ASSERT(ptr1 != ptr2);

  kfree(ptr1);
  kfree(ptr2);
}

static void test_heap_reuse() {
  print_serial("Running test_heap_reuse...\n");
  void *ptr1 = kmalloc(64);
  uint64_t addr1 = (uint64_t)ptr1;
  kfree(ptr1);

  void *ptr2 = kmalloc(64);
  TEST_ASSERT(ptr2 != 0);
  TEST_ASSERT((uint64_t)ptr2 == addr1);
  kfree(ptr2);
}

static void test_heap_stress() {
  print_serial("Running test_heap_stress...\n");
#define STRESS_COUNT 32
  void *ptrs[STRESS_COUNT];
  int all_ok = 1;

  for (int i = 0; i < STRESS_COUNT; i++) {
    ptrs[i] = kmalloc(16 + i * 4);
    if (!ptrs[i]) {
      all_ok = 0;
      break;
    }
  }
  TEST_ASSERT(all_ok == 1);

  for (int i = 0; i < STRESS_COUNT; i++) {
    if (ptrs[i])
      kfree(ptrs[i]);
  }

  void *check = kmalloc(512);
  TEST_ASSERT(check != 0);
  if (check)
    kfree(check);
}

// --- String Tests ---
static void test_strlen() {
  print_serial("Running test_strlen...\n");
  TEST_ASSERT(strlen("test") == 4);
  TEST_ASSERT(strlen("") == 0);
}

static void test_strcmp() {
  print_serial("Running test_strcmp...\n");
  TEST_ASSERT(strcmp("abc", "abc") == 0);
  TEST_ASSERT(strcmp("abc", "abd") != 0);
}

static void test_strncmp() {
  print_serial("Running test_strncmp...\n");
  TEST_ASSERT(strncmp("abcdef", "abcxyz", 3) == 0);
  TEST_ASSERT(strncmp("abc", "abd", 3) != 0);
  TEST_ASSERT(strncmp("abc", "abc", 0) == 0);
}

static void test_itoa() {
  print_serial("Running test_itoa...\n");
  char buf[20];

  k_itoa(0, buf);
  TEST_ASSERT(strcmp(buf, "0") == 0);

  k_itoa(12345, buf);
  TEST_ASSERT(strcmp(buf, "12345") == 0);

  k_itoa(-42, buf);
  TEST_ASSERT(strcmp(buf, "-42") == 0);
}

static void test_itoa_hex() {
  print_serial("Running test_itoa_hex...\n");
  char buf[20];

  k_itoa_hex(0xFF, buf);
  TEST_ASSERT(strlen(buf) > 0);

  k_itoa_hex(0, buf);
  TEST_ASSERT(strlen(buf) > 0);
}

// --- Task Tests ---
static void test_task_priority() {
  print_serial("Running test_task_priority...\n");
  task_t *cur = get_current_task();
  TEST_ASSERT(cur != 0);
  if (cur) {
    int old_prio = cur->priority;
    uint32_t old_slice = cur->time_slice;
    task_set_priority(cur->id, 5);
    TEST_ASSERT(cur->priority == 5);
    TEST_ASSERT(cur->time_slice == 10);
    // Restore
    task_set_priority(cur->id, old_prio);
    cur->time_slice = old_slice;
  }
}

static void test_stack_guard() {
  print_serial("Running test_stack_guard...\n");
  task_t *cur = get_current_task();
  if (cur) {
    int result = task_check_stack(cur);
    // Initial kernel task may not have canary (allocated before our change)
    TEST_ASSERT(result == 1 || result == -1);
  }
}

// --- Test Runner ---
void run_kernel_tests() {
  print_serial("\n==================================\n");
  print_serial("   STARTING KERNEL UNIT TESTS     \n");
  print_serial("==================================\n");

  tests_run = 0;
  tests_passed = 0;
  tests_failed = 0;

  // Memory
  test_memcpy();
  test_memset();

  // Strings
  test_strlen();
  test_strcmp();
  test_strncmp();
  test_itoa();
  test_itoa_hex();

  // Heap
  test_heap_alloc();
  test_heap_reuse();
  test_heap_stress();

  // Tasks
  test_task_priority();
  test_stack_guard();

  print_serial("\n==================================\n");
  char buf[16];
  print_serial("Tests Run:    ");
  k_itoa(tests_run, buf);
  print_serial(buf);
  print_serial("\n");

  print_serial("Tests Passed: ");
  k_itoa(tests_passed, buf);
  print_serial(buf);
  print_serial("\n");

  print_serial("Tests Failed: ");
  k_itoa(tests_failed, buf);
  print_serial(buf);
  print_serial("\n");
  print_serial("==================================\n\n");
}
