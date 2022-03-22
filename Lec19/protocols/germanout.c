#define NDEBUG 1

/* Setting a POSIX version on FreeBSD causes other functions to become hidden.
 */
#ifndef __FreeBSD__
#define _POSIX_C_SOURCE 200809L
#endif

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <pthread.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __linux__
#include <linux/version.h>
#endif

#ifdef __APPLE__
#include <sandbox.h>
#elif defined(__FreeBSD__)
#include <sys/capsicum.h>
#elif defined(__linux__)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 5, 0)
#include <linux/audit.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#endif
#endif

enum { SET_CAPACITY = 8388608ul };

enum { SET_EXPAND_THRESHOLD = 75 };

static const enum { OFF, ON, AUTO } COLOR = AUTO;

enum trace_category_t {
  TC_HANDLE_READS       = 1,
  TC_HANDLE_WRITES      = 2,
  TC_MEMORY_USAGE       = 4,
  TC_QUEUE              = 8,
  TC_SET                = 16,
  TC_SYMMETRY_REDUCTION = 32,
};
static const uint64_t TRACES_ENABLED = UINT64_C(0);

static const enum {
  DEADLOCK_DETECTION_OFF,
  DEADLOCK_DETECTION_STUCK,
  DEADLOCK_DETECTION_STUTTERING,
} DEADLOCK_DETECTION = DEADLOCK_DETECTION_STUTTERING;

enum {
  SYMMETRY_REDUCTION_OFF = 0,
  SYMMETRY_REDUCTION_HEURISTIC = 1,
  SYMMETRY_REDUCTION_EXHAUSTIVE = 2,
};
#define SYMMETRY_REDUCTION SYMMETRY_REDUCTION_HEURISTIC

enum { SANDBOX_ENABLED = 0 };

enum { MAX_ERRORS = 1ul };

enum { THREADS = 64ul };

enum { STATE_SIZE_BITS = 57ul };

enum { ASSUME_STATEMENTS_COUNT = 0ul };

#define LIVENESS_COUNT 0

#define CEX_OFF 0
#define DIFF 1
#define FULL 2
#define COUNTEREXAMPLE_TRACE DIFF

enum { MACHINE_READABLE_OUTPUT = 0 };

enum { MAX_SIMPLE_WIDTH = 3 };

#define BOUND 0

typedef uint_fast8_t value_t;
#define VALUE_MIN ((uint_fast8_t)0)
#define VALUE_MAX UINT_FAST8_MAX
#define VALUE_C(x) ((value_t)UINT8_C(x))
#define PRIVAL PRIuFAST8
typedef uint_fast8_t raw_value_t;
#define RAW_VALUE_MIN ((uint_fast8_t)0)
#define RAW_VALUE_MAX UINT_FAST8_MAX
#define PRIRAWVAL PRIuFAST8

#define RULE_TAKEN_LIMIT 26
#define PACK_STATE 1
#define SCHEDULE_BITS 2ul
#define PRINTS_SCALARSETS 0

/* whether scalarset schedules should be computed and used during printing */
#define USE_SCALARSET_SCHEDULES (1 && SYMMETRY_REDUCTION != SYMMETRY_REDUCTION_OFF && \
  (COUNTEREXAMPLE_TRACE != CEX_OFF || PRINTS_SCALARSETS))
#define POINTER_BITS 0
enum {
  /* Dummy entry in case the above generated list is empty to avoid an empty enum. */
  COVER_INVALID = -1,
};

static const char *COVER_MESSAGES[] = {
};

static uintmax_t covers[0];

#ifndef __OPTIMIZE__
#ifdef __clang__
#ifdef __x86_64__
#warning you are compiling without optimizations enabled. I would suggest -march=native -O3 -mcx16.
#else
#warning you are compiling without optimizations enabled. I would suggest -march=native -O3.
#endif
#else
#ifdef __x86_64__
#warning you are compiling without optimizations enabled. I would suggest -march=native -O3 -fwhole-program -mcx16.
#else
#warning you are compiling without optimizations enabled. I would suggest -march=native -O3 -fwhole-program.
#endif
#endif
#endif

#define value_to_string(v) ((value_t)(v))
#define raw_value_to_string(v) ((raw_value_t)(v))

/* A more powerful assert that treats the assertion as an assumption when
 * assertions are disabled.
 */
#ifndef NDEBUG
#define ASSERT(expr) assert(expr)
#elif defined(__clang__)
#define ASSERT(expr) __builtin_assume(expr)
#else
/* GCC doesn't have __builtin_assume, so we need something else. */
#define ASSERT(expr)                                                           \
  do {                                                                         \
    /* The following is an idiom for teaching the compiler an assumption. */   \
    if (!(expr)) {                                                             \
      __builtin_unreachable();                                                 \
    }                                                                          \
  } while (0)
#endif

#define BITS_TO_BYTES(size) ((size) / 8 + ((size) % 8 == 0 ? 0 : 1))
#define BITS_FOR(value)                                                        \
  ((value) == 0 ? 0 : (sizeof(unsigned long long) * 8 - __builtin_clzll(value)))

/* The size of the compressed state data in bytes. */
enum { STATE_SIZE_BYTES = BITS_TO_BYTES(STATE_SIZE_BITS) };

/* the size of auxliary members of the state struct */
enum { BOUND_BITS = BITS_FOR(BOUND) };
#if COUNTEREXAMPLE_TRACE != CEX_OFF || LIVENESS_COUNT > 0
#if POINTER_BITS != 0
enum { PREVIOUS_BITS = POINTER_BITS };
#elif defined(__linux__) && defined(__x86_64__) && !defined(__ILP32__)
/* assume 5-level paging, and hence the top 2 bytes of any user pointer are
 * always 0 and not required.
 * https://www.kernel.org/doc/Documentation/x86/x86_64/mm.txt
 */
enum { PREVIOUS_BITS = 56 };
#else
enum { PREVIOUS_BITS = sizeof(void *) * 8 };
#endif
#else
enum { PREVIOUS_BITS = 0 };
#endif
#if COUNTEREXAMPLE_TRACE != CEX_OFF
enum { RULE_TAKEN_BITS = BITS_FOR(RULE_TAKEN_LIMIT) };
#else
enum { RULE_TAKEN_BITS = 0 };
#endif
enum {
  STATE_OTHER_BYTES =
      BITS_TO_BYTES(BOUND_BITS + PREVIOUS_BITS + RULE_TAKEN_BITS +
                    (USE_SCALARSET_SCHEDULES ? SCHEDULE_BITS : 0))
};

/* Implement _Thread_local for GCC <4.9, which is missing this. */
#if defined(__GNUC__) && defined(__GNUC_MINOR__)
#if __GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 9)
#define _Thread_local __thread
#endif
#endif

#ifdef __clang__
#define NONNULL _Nonnull
#else
#define NONNULL /* nothing; other compilers don't have _Nonnull */
#endif

/* A word about atomics... There are two different atomic operation mechanisms
 * used in this code and it may not immediately be obvious why one was not
 * sufficient. The two are:
 *
 *   1. GCC __atomic built-ins: Used for variables that are sometimes accessed
 *      with atomic semantics and sometimes as regular memory operations. The
 *      C11 atomics cannot give us this and the __atomic built-ins are
 *      implemented by the major compilers.
 *   2. GCC __sync built-ins: used for 128-bit atomic accesses on x86-64. It
 *      seems the __atomic built-ins do not result in a CMPXCHG instruction, but
 *      rather in a less efficient library call. See
 *      https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80878.
 *
 * Though this is intended to be a C11 program, we avoid the C11 atomics to be
 * compatible with GCC <4.9.
 */

/* Identifier of the current thread. This counts up from 0 and thus is suitable
 * to use for, e.g., indexing into arrays. The initial thread has ID 0.
 */
static _Thread_local size_t thread_id;

/* The threads themselves. Note that we have no element for the initial thread,
 * so *your* thread is 'threads[thread_id - 1]'.
 */
static pthread_t threads[THREADS - 1];

/* What we are currently doing. Either "warming up" (running single threaded
 * building up queue occupancy) or "free running" (running multithreaded).
 */
static enum { WARMUP, RUN } phase = WARMUP;

/* Number of errors we've noted so far. If a thread sees this hit or exceed
 * MAX_ERRORS, they should attempt to exit gracefully as soon as possible.
 */
static unsigned long error_count;

/* Number of rules that have been processed. There are two representations of
 * this: a thread-local count of how many rules we have fired thus far and a
 * global array of *final* counts of fired rules per-thread that is updated and
 * used as threads are exiting. The purpose of this duplication is to let the
 * compiler layout the thread-local variable in a cache-friendly way and use
 * this during checking, rather than having all threads contending on the global
 * array whose entries are likely all within the same cache line.
 */
static _Thread_local uintmax_t rules_fired_local;
static uintmax_t rules_fired[THREADS];

/* Checkpoint to restore to after reporting an error. This is only used if we
 * are tolerating more than one error before exiting.
 */
static _Thread_local sigjmp_buf checkpoint;

_Static_assert(MAX_ERRORS > 0, "illegal MAX_ERRORS value");

/* Whether we need to save and restore checkpoints. This is determined by
 * whether we ever need to perform the action "discard the current state and
 * skip to checking the next." This scenario can occur for two reasons:
 *   1. We are running multithreaded, have just found an error and have not yet
 *      hit MAX_ERRORS. In this case we want to longjmp back to resume checking.
 *   2. We failed an assume statement. In this case we want to mark the current
 *      state as invalid and resume checking with the next state.
 * In either scenario the actual longjmp performed is the same, but by knowing
 * statically whether either can occur we can avoid calling setjmp if both are
 * impossible.
 */
enum { JMP_BUF_NEEDED = MAX_ERRORS > 1 || ASSUME_STATEMENTS_COUNT > 0 };

/*******************************************************************************
 * Sandbox support.                                                            *
 *                                                                             *
 * Because we're running generated code, it seems wise to use OS mechanisms to *
 * reduce our privileges, where possible.                                      *
 ******************************************************************************/

static void sandbox(void) {

  if (!SANDBOX_ENABLED) {
    return;
  }

#ifdef __APPLE__
  {
    char *err;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    int r = sandbox_init(kSBXProfilePureComputation, SANDBOX_NAMED, &err);
#pragma clang diagnostic pop

    if (__builtin_expect(r != 0, 0)) {
      fprintf(stderr, "sandbox_init failed: %s\n", err);
      free(err);
      exit(EXIT_FAILURE);
    }

    return;
  }
#endif

#ifdef __FreeBSD__
  {
    if (__builtin_expect(cap_enter() != 0, 0)) {
      perror("cap_enter");
      exit(EXIT_FAILURE);
    }
    return;
  }
#endif

#if defined(__linux__)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 5, 0)
  {
    /* Disable the addition of new privileges via execve and friends. */
    int r = prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0);
    if (__builtin_expect(r != 0, 0)) {
      perror("prctl(PR_SET_NO_NEW_PRIVS) failed");
      exit(EXIT_FAILURE);
    }

    /* A BPF program that traps on any syscall we want to disallow. */
    static struct sock_filter filter[] = {

        /* Load syscall number. */
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offsetof(struct seccomp_data, nr)),

    /* Enable exiting. */
#ifdef __NR_exit_group
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_exit_group, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
#endif

    /* Enable syscalls used by printf. */
#ifdef __NR_fstat
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_fstat, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
#endif
#ifdef __NR_fstat64
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_fstat64, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
#endif
#ifdef __NR_newfstatat
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_newfstatat, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
#endif
#ifdef __NR_write
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_write, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
#endif

    /* Enable syscalls used by malloc. */
#ifdef __NR_brk
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_brk, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
#endif
#ifdef __NR_mmap
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_mmap, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
#endif
#ifdef __NR_mmap2
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_mmap2, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
#endif
#ifdef __NR_munmap
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_munmap, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
#endif

    /* If we're running multithreaded, enable syscalls used by pthreads. */
#ifdef __NR_clone
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_clone, 0, 1),
        BPF_STMT(BPF_RET | BPF_K,
                 THREADS > 1 ? SECCOMP_RET_ALLOW : SECCOMP_RET_TRAP),
#endif
#ifdef __NR_close
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_close, 0, 1),
        BPF_STMT(BPF_RET | BPF_K,
                 THREADS > 1 ? SECCOMP_RET_ALLOW : SECCOMP_RET_TRAP),
#endif
#ifdef __NR_exit
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_exit, 0, 1),
        BPF_STMT(BPF_RET | BPF_K,
                 THREADS > 1 ? SECCOMP_RET_ALLOW : SECCOMP_RET_TRAP),
#endif
#ifdef __NR_futex
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_futex, 0, 1),
        BPF_STMT(BPF_RET | BPF_K,
                 THREADS > 1 ? SECCOMP_RET_ALLOW : SECCOMP_RET_TRAP),
#endif
#ifdef __NR_get_robust_list
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_get_robust_list, 0, 1),
        BPF_STMT(BPF_RET | BPF_K,
                 THREADS > 1 ? SECCOMP_RET_ALLOW : SECCOMP_RET_TRAP),
#endif
#ifdef __NR_madvise
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_madvise, 0, 1),
        BPF_STMT(BPF_RET | BPF_K,
                 THREADS > 1 ? SECCOMP_RET_ALLOW : SECCOMP_RET_TRAP),
#endif
#ifdef __NR_mprotect
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_mprotect, 0, 1),
        BPF_STMT(BPF_RET | BPF_K,
                 THREADS > 1 ? SECCOMP_RET_ALLOW : SECCOMP_RET_TRAP),
#endif
#ifdef __NR_open
        /* XXX: it would be nice to avoid open() but pthreads seems to open
         * libgcc.
         */
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_open, 0, 1),
        BPF_STMT(BPF_RET | BPF_K,
                 THREADS > 1 ? SECCOMP_RET_ALLOW : SECCOMP_RET_TRAP),
#endif
#ifdef __NR_read
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_read, 0, 1),
        BPF_STMT(BPF_RET | BPF_K,
                 THREADS > 1 ? SECCOMP_RET_ALLOW : SECCOMP_RET_TRAP),
#endif
#ifdef __NR_set_robust_list
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_set_robust_list, 0, 1),
        BPF_STMT(BPF_RET | BPF_K,
                 THREADS > 1 ? SECCOMP_RET_ALLOW : SECCOMP_RET_TRAP),
#endif

    /* on platforms without vDSO support, time() makes an actual syscall, so
     * we need to allow them
     */
#ifdef __NR_clock_gettime
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_clock_gettime, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
#endif
#ifdef __NR_clock_gettime64
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_clock_gettime64, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
#endif
#ifdef __NR_gettimeofday
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_gettimeofday, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
#endif
#ifdef __NR_time
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_time, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
#endif

        /* Deny everything else. On a disallowed syscall, we trap instead of
         * killing to allow the user to debug the failure. If you are debugging
         * seccomp denials, strace the checker and find the number of the denied
         * syscall in the first si_value parameter reported in the terminating
         * SIG_SYS.
         */
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_TRAP),
    };

    static const struct sock_fprog filter_program = {
        .len = sizeof(filter) / sizeof(filter[0]),
        .filter = filter,
    };

    /* Apply the above filter to ourselves. */
    r = prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &filter_program, 0, 0);
    if (__builtin_expect(r != 0, 0)) {
      perror("prctl(PR_SET_SECCOMP) failed");
      exit(EXIT_FAILURE);
    }

    return;
  }
#endif
#endif

#ifdef __OpenBSD__
  {
    if (__builtin_expect(pledge("stdio", "") != 0, 0)) {
      perror("pledge");
      exit(EXIT_FAILURE);
    }
    return;
  }
#endif

  /* No sandbox available. */
  fprintf(stderr, "no sandboxing facilities available\n");
  exit(EXIT_FAILURE);
}

/******************************************************************************/

/* ANSI colour code support */

static bool istty;

static const char *green() {
  if (COLOR == ON || (COLOR == AUTO && istty))
    return "\033[32m";
  return "";
}

static const char *red() {
  if (COLOR == ON || (COLOR == AUTO && istty))
    return "\033[31m";
  return "";
}

static const char *yellow() {
  if (COLOR == ON || (COLOR == AUTO && istty))
    return "\033[33m";
  return "";
}

static const char *bold() {
  if (COLOR == ON || (COLOR == AUTO && istty))
    return "\033[1m";
  return "";
}

static const char *reset() {
  if (COLOR == ON || (COLOR == AUTO && istty))
    return "\033[0m";
  return "";
}

#ifdef __SIZEOF_INT128__ /* if we have the type `__int128` */

#define UINT128_MAX                                                            \
  ((((unsigned __int128)UINT64_MAX) << 64) | ((unsigned __int128)UINT64_MAX))

#define INT128_MAX ((((__int128)INT64_MAX) << 64) | ((__int128)UINT64_MAX))
#define INT128_MIN (-INT128_MAX - 1)

struct string_buffer {
  char data[41];
};

static struct string_buffer value_u128_to_string(unsigned __int128 v) {

  struct string_buffer buffer;

  if (v == 0) {
    buffer.data[0] = '0';
    buffer.data[1] = '\0';
    return buffer;
  }

  size_t i = sizeof(buffer.data);
  while (v != 0) {
    i--;
    buffer.data[i] = '0' + v % 10;
    v /= 10;
  }

  memmove(buffer.data, &buffer.data[i], sizeof(buffer) - i);
  buffer.data[sizeof(buffer) - i] = '\0';

  return buffer;
}
static __attribute__((unused)) struct string_buffer
value_128_to_string(__int128 v) {

  if (v == INT128_MIN) {
    struct string_buffer buffer;
    strcpy(buffer.data, "-170141183460469231731687303715884105728");
    return buffer;
  }

  bool negative = v < 0;
  if (negative) {
    v = -v;
  }

  struct string_buffer buffer = value_u128_to_string(v);

  if (negative) {
    memmove(&buffer.data[1], buffer.data, strlen(buffer.data) + 1);
    buffer.data[0] = '-';
  }

  return buffer;
}
#endif

/* Is value_t a signed type? We need this as a function because value_t is a
 * typedef that can be tweaked by the user.
 */
static __attribute__((const)) bool value_is_signed(void) {
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-compare"
#pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  return (value_t)-1 < 0;
#ifdef __clang__
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
}

/*******************************************************************************
 * MurmurHash by Austin Appleby                                                *
 *                                                                             *
 * More information on this at https://github.com/aappleby/smhasher/           *
 ******************************************************************************/

static uint64_t MurmurHash64A(const void *NONNULL key, size_t len) {

  static const uint64_t seed = 0;

  static const uint64_t m = UINT64_C(0xc6a4a7935bd1e995);
  static const unsigned r = 47;

  uint64_t h = seed ^ (len * m);

  const unsigned char *data = key;
  const unsigned char *end = data + len / sizeof(uint64_t) * sizeof(uint64_t);

  while (data != end) {

    uint64_t k;
    memcpy(&k, data, sizeof(k));
    data += sizeof(k);

    k *= m;
    k ^= k >> r;
    k *= m;

    h ^= k;
    h *= m;
  }

  const unsigned char *data2 = data;

  switch (len & 7) {
    case 7: h ^= (uint64_t)data2[6] << 48; /* fall through */
    case 6: h ^= (uint64_t)data2[5] << 40; /* fall through */
    case 5: h ^= (uint64_t)data2[4] << 32; /* fall through */
    case 4: h ^= (uint64_t)data2[3] << 24; /* fall through */
    case 3: h ^= (uint64_t)data2[2] << 16; /* fall through */
    case 2: h ^= (uint64_t)data2[1] << 8; /* fall through */
    case 1: h ^= (uint64_t)data2[0];
    h *= m;
  }

  h ^= h >> r;
  h *= m;
  h ^= h >> r;

  return h;
}

/******************************************************************************/

/* Signal an out-of-memory condition and terminate abruptly. */
static _Noreturn void oom(void) {
  fputs("out of memory", stderr);
  exit(EXIT_FAILURE);
}

static void *xmalloc(size_t size) {
  void *p = malloc(size);
  if (__builtin_expect(p == NULL, 0)) {
    oom();
  }
  return p;
}

static void *xcalloc(size_t count, size_t size) {
  void *p = calloc(count, size);
  if (__builtin_expect(p == NULL, 0)) {
    oom();
  }
  return p;
}

static void put(const char *NONNULL s) {
  for (; *s != '\0'; ++s) {
    putchar_unlocked(*s);
  }
}

static void put_int(intmax_t u) {
  char buffer[128] = {0};
  snprintf(buffer, sizeof(buffer), "%" PRIdMAX, u);
  put(buffer);
}

static void put_uint(uintmax_t u) {
  char buffer[128] = {0};
  snprintf(buffer, sizeof(buffer), "%" PRIuMAX, u);
  put(buffer);
}

static __attribute__((unused)) void put_val(value_t v) {
  if (value_is_signed()) {
    put_int((intmax_t)v);
  } else {
    put_uint((uintmax_t)v);
  }
}

static void xml_printf(const char *NONNULL s) {
  while (*s != '\0') {
    switch (*s) {
      case '"': put("&quot;"); break;
      case '<': put("&lt;");   break;
      case '>': put("&gt;");   break;
      case '&': put("&amp;");  break;
      default:  putchar_unlocked(*s); break;
    }
    s++;
  }
}

/* Support for tracing specific operations. This can be enabled during checker
 * generation with '--trace ...' and is useful for debugging Rumur itself.
 */
static __attribute__((format(printf, 1, 2))) void trace(const char *NONNULL fmt,
                                                        ...) {

  va_list ap;
  va_start(ap, fmt);

  flockfile(stdout);

  (void)fprintf(stderr, "%sTRACE%s:", yellow(), reset());
  (void)vfprintf(stderr, fmt, ap);
  (void)fprintf(stderr, "\n");

  funlockfile(stdout);
  va_end(ap);
}

/* Wrap up trace() as a macro. It looks as if the following could just be
 * incorporated into trace(). However, present compilers seem unwilling to
 * inline varargs functions or do interprocedural analysis across a call to one.
 * As a result, the compiler does not notice when tracing is disabled and a call
 * to trace() would be a no-op that can be elided. By making the call a macro we
 * make the category comparison visible to the compiler's optimising passes.
 */
#define TRACE(category, args...)                                               \
  do {                                                                         \
    if ((category) & TRACES_ENABLED) {                                         \
      trace(args);                                                             \
    }                                                                          \
  } while (0)

/*******************************************************************************
 * Arithmetic wrappers                                                         *
 *                                                                             *
 * For compilers that support them, we call the overflow built-ins to check    *
 * undefined operations during arithmetic. For others, we just emit the bare   *
 * operation.                                                                  *
 ******************************************************************************/

#if defined(__clang__) || (defined(__GNUC__) && __GNUC__ >= 5)

#define ADD(a, b, c) __builtin_add_overflow((a), (b), (c))
#define MUL(a, b, c) __builtin_mul_overflow((a), (b), (c))
#define SUB(a, b, c) __builtin_sub_overflow((a), (b), (c))

#else

  #define ADD(a, b, c) ({ *(c) = (a) + (b); false; })
  #define MUL(a, b, c) ({ *(c) = (a) * (b); false; })
  #define SUB(a, b, c) ({ *(c) = (a) - (b); false; })

#endif

/******************************************************************************/

/* The state of the current model. */
struct state {

#if LIVENESS_COUNT > 0
  uintptr_t liveness[LIVENESS_COUNT / sizeof(uintptr_t) / CHAR_BIT +
                     (LIVENESS_COUNT % sizeof(uintptr_t) == 0 &&
                              LIVENESS_COUNT / sizeof(uintptr_t) % CHAR_BIT == 0
                          ? 0
                          : 1)];
#endif

  uint8_t data[STATE_SIZE_BYTES];

#if PACK_STATE
  /* the following effective fields are packed into here:
   *
   *  * uint64_t bound;
   *  * const struct state *previous;
   *  * uint64_t rule_taken;
   *  * uint8_t schedules[BITS_TO_BYTES(SCHEDULE_BITS)];
   *
   * They are bit-packed, so may take up less space. E.g. if the maximum value
   * of `bound` is known to be 5, it will be stored in 3 bits instead of 64.
   */
  uint8_t other[STATE_OTHER_BYTES];
#else
  uint64_t bound;
  const struct state *previous;
  uint64_t rule_taken;
  uint8_t schedules[USE_SCALARSET_SCHEDULES ? BITS_TO_BYTES(SCHEDULE_BITS) : 0];
#endif
};

struct handle {
  uint8_t *base;
  size_t offset;
  size_t width;
};

static struct handle handle_align(struct handle h) {

  ASSERT(h.offset < CHAR_BIT && "handle has an offset outside the base byte");

  size_t width = h.width + h.offset;
  if (width % 8 != 0) {
    width += 8 - width % 8;
  }

  return (struct handle){
      .base = h.base,
      .offset = 0,
      .width = width,
  };
}

static bool is_big_endian(void) {

  union {
    uint32_t x;
    uint8_t y[sizeof(uint32_t)];
  } z;

  z.y[0] = 1;
  z.y[1] = 2;
  z.y[2] = 3;
  z.y[3] = 4;

  return z.x == 0x01020304;
}

static uint64_t copy_out64(const uint8_t *p, size_t extent) {

  ASSERT(extent <= sizeof(uint64_t));

  uint64_t x = 0;
  memcpy(&x, p, extent);

  if (is_big_endian()) {
    x = __builtin_bswap64(x);
  }

  return x;
}

#ifdef __SIZEOF_INT128__ /* if we have the type `__int128` */
static unsigned __int128 byte_swap128(unsigned __int128 x) {

  union {
    unsigned __int128 a;
    uint64_t b[2];
  } in, out;

  in.a = x;
  out.b[0] = __builtin_bswap64(in.b[1]);
  out.b[1] = __builtin_bswap64(in.b[0]);

  return out.a;
}

static unsigned __int128 copy_out128(const uint8_t *p, size_t extent) {

  ASSERT(extent <= sizeof(unsigned __int128));

  unsigned __int128 x = 0;
  memcpy(&x, p, extent);

  if (is_big_endian()) {
    x = byte_swap128(x);
  }

  return x;
}
#endif

/* If you are in the Rumur repository modifying the following function, remember
 * to also update ../../misc/read-raw.smt2.
 */
static __attribute__((pure)) uint64_t read_raw(struct handle h) {

  /* a uint64_t is the maximum value we support reading */
  ASSERT(h.width <= 64 && "read of too wide value");

  if (h.width == 0) {
    return 0;
  }

  /* Generate a handle that is offset- and width-aligned on byte boundaries.
   * Essentially, we widen the handle to align it. The motivation for this is
   * that we can only do byte-granularity reads, so we need to "over-read" if we
   * have an unaligned handle.
   */
  struct handle aligned = handle_align(h);
  ASSERT(aligned.offset == 0);

  /* The code below attempts to provide four alternatives for reading out the
   * bits corresponding to a value of simple type referenced by a handle, and to
   * give the compiler enough hints to steer it towards picking one of these and
   * removing the other three as dead code:
   *
   *   1. Read into a single 64-bit variable. Enabled when the maximum width for
   *      an unaligned handle spans 0 - 8 bytes.
   *   2. Read into two 64-bit variables and then combine the result using
   *      shifts and ORs. Enabled when the maximum width for an unaligned handle
   *      spans 9 - 16 bytes and the compiler does not provide the `__int128`
   *      type.
   *   3. Read into a single 128-bit variable. Enabled when the compiler does
   *      provide the `__int128` type and the maximum width for an unaligned
   *      handle spans 9 - 16 bytes.
   *   4. Read into two 128-bit chunks, and then combine the result using shifts
   *      and ORs. Enabled when the compiler provides the `__int128` type and
   *      the maximum width for an unaligned handle spans 17 - 32 bytes.
   */

#ifdef __SIZEOF_INT128__ /* if we have the type `__int128` */

  /* If a byte-unaligned value_t cannot be fully read into a single uint64_t
   * using byte-aligned reads...
   */
  if (aligned.width > (sizeof(uint64_t) - 1) * 8) {

    /* Read the low double-word of this (possibly quad-word-sized) value. */
    unsigned __int128 low = 0;
    size_t low_size = aligned.width / 8;
    /* optimisation hint: */
    ASSERT(low_size <= sizeof(low) || aligned.width > (sizeof(low) - 1) * 8);
    if (low_size > sizeof(low)) {
      low_size = sizeof(low);
    }
    {
      const uint8_t *src = aligned.base;
      low = copy_out128(src, low_size);
    }

    low >>= h.offset;

    size_t high_size = aligned.width / 8 - low_size;

    /* If the value could not be read into a single double-word... */
    ASSERT(high_size == 0 || aligned.width > (sizeof(low) - 1) * 8);
    if (high_size != 0) {
      unsigned __int128 high = 0;
      const uint8_t *src = aligned.base + sizeof(low);
      high = copy_out128(src, high_size);

      high <<= sizeof(low) * 8 - h.offset;

      /* Combine the two halves into a single double-word. */
      low |= high;
    }

    if (h.width < sizeof(low) * 8) {
      unsigned __int128 mask = (((unsigned __int128)1) << h.width) - 1;
      low &= mask;
    }

    ASSERT(low <= UINT64_MAX && "read of value larger than a uint64_t");

    return (uint64_t)low;
  }
#endif

  /* Read the low word of this (possibly two-word-sized) value. */
  uint64_t low = 0;
  size_t low_size = aligned.width / 8;
  /* optimisation hint: */
  ASSERT(low_size <= sizeof(low) || aligned.width > (sizeof(low) - 1) * 8);
  if (low_size > sizeof(low)) {
    low_size = sizeof(low);
  }
  {
    const uint8_t *src = aligned.base;
    low = copy_out64(src, low_size);
  }

  low >>= h.offset;

  size_t high_size = aligned.width / 8 - low_size;

  /* If the value could not be read into a single word... */
  ASSERT(high_size == 0 || aligned.width > (sizeof(low) - 1) * 8);
  if (high_size != 0) {
    const uint8_t *src = aligned.base + sizeof(low);
    uint64_t high = copy_out64(src, high_size);

    high <<= sizeof(low) * 8 - h.offset;

    /* Combine the high and low words. Note that we know we can store the final
     * result in a single word because the width is guaranteed to be <= 64.
     */
    low |= high;
  }

  if (h.width < sizeof(low) * 8) {
    uint64_t mask = (UINT64_C(1) << h.width) - 1;
    low &= mask;
  }

  return low;
}

static void copy_in64(uint8_t *p, uint64_t v, size_t extent) {
  ASSERT(extent <= sizeof(v));

  if (is_big_endian()) {
    v = __builtin_bswap64(v);
  }

  memcpy(p, &v, extent);
}

#ifdef __SIZEOF_INT128__ /* if we have the type `__int128` */
static void copy_in128(uint8_t *p, unsigned __int128 v, size_t extent) {
  ASSERT(extent <= sizeof(v));

  if (is_big_endian()) {
    v = byte_swap128(v);
  }

  memcpy(p, &v, extent);
}
#endif

/* If you are in the Rumur repository modifying the following function, remember
 * to also update ../../misc/write-raw.smt2.
 */
static void write_raw(struct handle h, uint64_t v) {

  if (h.width == 0) {
    return;
  }

  /* sanitise input value */
  if (h.width < sizeof(v) * 8) {
    v &= (UINT64_C(1) << h.width) - 1;
  }

  /* Generate a offset- and width-aligned handle on byte boundaries. */
  struct handle aligned = handle_align(h);
  ASSERT(aligned.offset == 0);

#ifdef __SIZEOF_INT128__ /* if we have the type `__int128` */

  /* If a byte-unaligned value_t cannot be fully written within a single
   * byte-aligned uint64_t...
   */
  if (aligned.width > (sizeof(uint64_t) - 1) * 8) {

    /* Read the low double-word of this region. */
    unsigned __int128 low = 0;
    size_t low_size = aligned.width / 8;
    ASSERT(low_size <= sizeof(low) || aligned.width > (sizeof(low) - 1) * 8);
    if (low_size > sizeof(low)) {
      low_size = sizeof(low);
    }
    {
      const uint8_t *src = aligned.base;
      low = copy_out128(src, low_size);
    }

    {
      unsigned __int128 or_mask = ((unsigned __int128)v) << h.offset;
      if (low_size < sizeof(low)) {
        or_mask &= (((unsigned __int128)1) << (low_size * 8)) - 1;
      }
      unsigned __int128 and_mask = (((unsigned __int128)1) << h.offset) - 1;
      if (h.width + h.offset < sizeof(low) * 8) {
        size_t high_bits = aligned.width - h.offset - h.width;
        and_mask |= ((((unsigned __int128)1) << high_bits) - 1)
                    << (low_size * 8 - high_bits);
      }

      low = (low & and_mask) | or_mask;
    }

    {
      uint8_t *dest = aligned.base;
      copy_in128(dest, low, low_size);
    }

    /* Now do the second double-word if necessary. */

    size_t high_size = aligned.width / 8 - low_size;

    ASSERT(high_size == 0 || aligned.width > (sizeof(low) - 1) * 8);
    if (high_size != 0) {
      unsigned __int128 high = 0;
      {
        const uint8_t *src = aligned.base + sizeof(low);
        high = copy_out128(src, high_size);
      }

      {
        unsigned __int128 or_mask =
            ((unsigned __int128)v) >> (sizeof(low) * 8 - h.offset);
        unsigned __int128 and_mask = ~(
            (((unsigned __int128)1) << (h.width + h.offset - sizeof(low) * 8)) -
            1);

        high = (high & and_mask) | or_mask;
      }

      {
        uint8_t *dest = aligned.base + sizeof(low);
        copy_in128(dest, high, high_size);
      }
    }

    return;
  }
#endif

  /* Replicate the above logic for uint64_t. */

  uint64_t low = 0;
  size_t low_size = aligned.width / 8;
  ASSERT(low_size <= sizeof(low) || aligned.width > (sizeof(low) - 1) * 8);
  if (low_size > sizeof(low)) {
    low_size = sizeof(low);
  }
  {
    const uint8_t *src = aligned.base;
    low = copy_out64(src, low_size);
  }

  {
    uint64_t or_mask = ((uint64_t)v) << h.offset;
    if (low_size < sizeof(low)) {
      or_mask &= (UINT64_C(1) << (low_size * 8)) - 1;
    }
    uint64_t and_mask = (UINT64_C(1) << h.offset) - 1;
    if (h.width + h.offset < sizeof(low) * 8) {
      size_t high_bits = aligned.width - h.offset - h.width;
      and_mask |= ((UINT64_C(1) << high_bits) - 1)
                  << (low_size * 8 - high_bits);
    }

    low = (low & and_mask) | or_mask;
  }

  {
    uint8_t *dest = aligned.base;
    copy_in64(dest, low, low_size);
  }

  size_t high_size = aligned.width / 8 - low_size;

  ASSERT(high_size == 0 || aligned.width > (sizeof(low) - 1) * 8);
  if (high_size != 0) {
    uint64_t high = 0;
    {
      const uint8_t *src = aligned.base + sizeof(low);
      high = copy_out64(src, high_size);
    }

    {
      uint64_t or_mask = ((uint64_t)v) >> (sizeof(low) * 8 - h.offset);
      uint64_t and_mask =
          ~((UINT64_C(1) << (h.width + h.offset - sizeof(low) * 8)) - 1);

      high = (high & and_mask) | or_mask;
    }

    {
      uint8_t *dest = aligned.base + sizeof(low);
      copy_in64(dest, high, high_size);
    }
  }
}

#if BOUND > 0
#if PACK_STATE
static struct handle state_bound_handle(const struct state *NONNULL s) {

  struct handle h = (struct handle){
      .base = (uint8_t *)s->other,
      .offset = 0,
      .width = BITS_FOR(BOUND),
  };

  return h;
}
#endif

_Static_assert((uintmax_t)BOUND <= UINT64_MAX,
               "bound limit does not fit in a uint64_t");

static __attribute__((pure)) uint64_t
state_bound_get(const struct state *NONNULL s) {
  assert(s != NULL);

#if PACK_STATE
  struct handle h = state_bound_handle(s);
  return read_raw(h);
#else
  return s->bound;
#endif
}

static void state_bound_set(struct state *NONNULL s, uint64_t bound) {
  assert(s != NULL);

#if PACK_STATE
  struct handle h = state_bound_handle(s);
  write_raw(h, bound);
#else
  s->bound = bound;
#endif
}
#endif

#if COUNTEREXAMPLE_TRACE != CEX_OFF || LIVENESS_COUNT > 0
#if PACK_STATE
static struct handle state_previous_handle(const struct state *NONNULL s) {

  size_t offset = BOUND_BITS;

  struct handle h = (struct handle){
      .base = (uint8_t *)s->other + offset / 8,
      .offset = offset % 8,
      .width = PREVIOUS_BITS,
  };

  return h;
}
#endif

static __attribute__((pure)) const struct state *
state_previous_get(const struct state *NONNULL s) {
#if PACK_STATE
  struct handle h = state_previous_handle(s);
  return (const struct state *)(uintptr_t)read_raw(h);
#else
  return s->previous;
#endif
}

static void state_previous_set(struct state *NONNULL s,
                               const struct state *previous) {
#if PACK_STATE
  ASSERT(
      (PREVIOUS_BITS == sizeof(void *) * 8 ||
       ((uintptr_t)previous >> PREVIOUS_BITS) == 0) &&
      "upper bits of pointer are non-zero (incorrect --pointer-bits setting?)");
  struct handle h = state_previous_handle(s);
  write_raw(h, (uint64_t)(uintptr_t)previous);
#else
  s->previous = previous;
#endif
}
#endif

#if COUNTEREXAMPLE_TRACE != CEX_OFF
#if PACK_STATE
static struct handle state_rule_taken_handle(const struct state *NONNULL s) {

  size_t offset = BOUND_BITS + PREVIOUS_BITS;

  struct handle h = (struct handle){
      .base = (uint8_t *)s->other + offset / 8,
      .offset = offset % 8,
      .width = RULE_TAKEN_BITS,
  };

  return h;
}
#endif

static __attribute__((pure)) uint64_t
state_rule_taken_get(const struct state *NONNULL s) {
  assert(s != NULL);
#if PACK_STATE
  struct handle h = state_rule_taken_handle(s);
  return read_raw(h);
#else
  return s->rule_taken;
#endif
}

static void state_rule_taken_set(struct state *NONNULL s, uint64_t rule_taken) {
  assert(s != NULL);
#if PACK_STATE
  struct handle h = state_rule_taken_handle(s);
  write_raw(h, rule_taken);
#else
  s->rule_taken = rule_taken;
#endif
}
#endif

static struct handle state_schedule_handle(const struct state *NONNULL s,
                                           size_t offset, size_t width) {

  /* the maximum schedule width handle ever derived should be SCHEDULE_BITS, and
   * should only ever occur when writing the entire schedule
   */
#if !defined(__clang__) && defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(width <= SCHEDULE_BITS && (width < SCHEDULE_BITS || offset == 0) &&
         "out-of-bounds handle derived to access schedule data");
#if !defined(__clang__) && defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

  uint8_t *b;
  size_t o;

#if PACK_STATE
  b = (uint8_t *)s->other;
  o = BOUND_BITS + PREVIOUS_BITS + RULE_TAKEN_BITS + offset;
#else
  b = (uint8_t *)s->schedules;
  o = offset;
#endif

  struct handle h = (struct handle){
      .base = (uint8_t *)b + o / 8,
      .offset = o % 8,
      .width = width,
  };

  return h;
}

static __attribute__((pure, unused)) size_t
state_schedule_get(const struct state *NONNULL s, size_t offset, size_t width) {
  assert(s != NULL);

  struct handle h = state_schedule_handle(s, offset, width);
  return (size_t)read_raw(h);
}

static __attribute__((unused)) void state_schedule_set(struct state *NONNULL s,
                                                       size_t offset,
                                                       size_t width, size_t v) {
  assert(s != NULL);

  /* we should never attempt to write an invalid schedule index that will be
   * truncated
   */
  if (width < 64) {
    ASSERT((((UINT64_C(1) << width) - 1) & (uint64_t)v) == (uint64_t)v &&
           "truncation in writing schedule data to state");
  }

  struct handle h = state_schedule_handle(s, offset, width);
  write_raw(h, (uint64_t)v);
}

/*******************************************************************************
 * State allocator.                                                            *
 *                                                                             *
 * The following implements a simple bump allocator for states. The purpose of *
 * this (rather than simply mallocing individual states) is to speed up        *
 * allocation by taking global locks less frequently and decrease allocator    *
 * metadata overhead.                                                          *
 ******************************************************************************/

/* An initial size of thread-local allocator pools ~8MB. */
static _Thread_local size_t arena_count =
    (sizeof(struct state) > 8 * 1024 * 1024)
        ? 1
        : (8 * 1024 * 1024 / sizeof(struct state));

static _Thread_local struct state *arena_base;
static _Thread_local struct state *arena_limit;

static struct state *state_new(void) {

  if (arena_base == arena_limit) {
    /* Allocation pool is empty. We need to set up a new pool. */
    for (;;) {
      if (arena_count == 1) {
        arena_base = xmalloc(sizeof(*arena_base));
      } else {
        arena_base = calloc(arena_count, sizeof(*arena_base));
        if (__builtin_expect(arena_base == NULL, 0)) {
          /* Memory pressure high. Decrease our attempted allocation and try
           * again.
           */
          arena_count /= 2;
          continue;
        }
      }

      arena_limit = arena_base + arena_count;
      break;
    }
  }

  assert(arena_base != NULL);
  assert(arena_base != arena_limit);

  struct state *s = arena_base;
  arena_base++;
  return s;
}

static void state_free(struct state *s) {

  if (s == NULL) {
    return;
  }

  assert(s + 1 == arena_base);
  arena_base--;
}

/*******************************************************************************
 * statistics for memory usage                                                 *
 *                                                                             *
 * This functionality is only used when `--trace memory_usage` is given on the *
 * command line.                                                               *
 ******************************************************************************/

/* number of allocated state structs per depth of expansion */
static size_t allocated[BOUND == 0 ? 1 : (BOUND + 1)];

/* note a new allocation of a state struct at the given depth */
static void register_allocation(size_t depth) {

  /* if we are not tracing memory usage, make this a no-op */
  if (!(TC_MEMORY_USAGE & TRACES_ENABLED)) {
    return;
  }

  ASSERT(depth < sizeof(allocated) / sizeof(allocated[0]) &&
         "out of range access to allocated array");

  /* increment the number of known allocated states, avoiding an expensive
   * atomic if we are single-threaded
   */
  if (THREADS == 1) {
    allocated[depth]++;
  } else {
    (void)__sync_add_and_fetch(&allocated[depth], 1);
  }
}

/* print a summary of the allocation results we have accrued */
static void print_allocation_summary(void) {

  /* it is assumed we are running single-threaded here and do not need atomic
   * accesses
   */

  if (BOUND == 0) {
    TRACE(TC_MEMORY_USAGE,
          "allocated %zu state structure(s), totaling %zu bytes",
          allocated[0], allocated[0] * sizeof(struct state));
  } else {
    for (size_t i = 0; i < sizeof(allocated) / sizeof(allocated[0]); i++) {

      if (allocated[i] == 0) {
        /* no state at this depth was reached, therefore no states at deeper
         * depths were reached either and we are done
         */
        for (size_t j = i + 1; j < sizeof(allocated) / sizeof(allocated[0]);
             j++) {
          assert(allocated[j] == 0 &&
                 "state allocated at a deeper depth than an empty level");
        }
        break;
      }

      TRACE(TC_MEMORY_USAGE,
            "depth %zu: allocated %zu state structure(s), totaling %zu bytes",
            i, allocated[i], allocated[i] * sizeof(struct state));
    }
  }
}

/******************************************************************************/

/* Print a counterexample trace terminating at the given state. This function
 * assumes that the caller already holds a lock on stdout.
 */
static void print_counterexample(const struct state *NONNULL s
                                 __attribute__((unused)));

/* "Exit" the current thread. This takes into account which thread we are. I.e.
 * the correct way to exit the checker is for every thread to eventually call
 * this function.
 */
static _Noreturn int exit_with(int status);

static __attribute__((format(printf, 2, 3))) _Noreturn void
error(const struct state *NONNULL s, const char *NONNULL fmt, ...) {

  unsigned long prior_errors =
      __atomic_fetch_add(&error_count, 1, __ATOMIC_SEQ_CST);

  if (__builtin_expect(prior_errors < MAX_ERRORS, 1)) {

    flockfile(stdout);

    va_list ap;
    va_start(ap, fmt);

    if (MACHINE_READABLE_OUTPUT) {
      put("<error includes_trace=\"");
      put((s == NULL || COUNTEREXAMPLE_TRACE == CEX_OFF) ? "false" : "true");
      put("\">\n");

      put("<message>");
      {
        va_list ap2;
        va_copy(ap2, ap);

        int size = vsnprintf(NULL, 0, fmt, ap2);
        va_end(ap2);
        if (__builtin_expect(size < 0, 0)) {
          fputs("vsnprintf failed", stderr);
          exit(EXIT_FAILURE);
        }

        char *buffer = xmalloc(size + 1);
        if (__builtin_expect(vsnprintf(buffer, size + 1, fmt, ap) != size, 0)) {
          fputs("vsnprintf failed", stderr);
          exit(EXIT_FAILURE);
        }

        xml_printf(buffer);
        free(buffer);
      }
      put("</message>\n");

      if (s != NULL && COUNTEREXAMPLE_TRACE != CEX_OFF) {
        print_counterexample(s);
      }

      put("</error>\n");

    } else {
      if (s != NULL) {
        put("The following is the error trace for the error:\n\n");
      } else {
        put("Result:\n\n");
      }

      put("\t");
      put(red());
      put(bold());
      {
        va_list ap2;
        va_copy(ap2, ap);

        int size = vsnprintf(NULL, 0, fmt, ap2);
        va_end(ap2);
        if (__builtin_expect(size < 0, 0)) {
          fputs("vsnprintf failed", stderr);
          exit(EXIT_FAILURE);
        }

        char *buffer = xmalloc(size + 1);
        if (__builtin_expect(vsnprintf(buffer, size + 1, fmt, ap) != size, 0)) {
          fputs("vsnprintf failed", stderr);
          exit(EXIT_FAILURE);
        }

        put(buffer);
        free(buffer);
      }
      put(reset());
      put("\n\n");

      if (s != NULL && COUNTEREXAMPLE_TRACE != CEX_OFF) {
        print_counterexample(s);
        put("End of the error trace.\n\n");
      }
    }

    va_end(ap);

    funlockfile(stdout);
  }

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-compare"
#pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if (prior_errors < MAX_ERRORS - 1) {
#ifdef __clang__
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
    assert(JMP_BUF_NEEDED && "longjmping without a setup jmp_buf");
    siglongjmp(checkpoint, 1);
  }

  exit_with(EXIT_FAILURE);
}

static void deadlock(const struct state *NONNULL s) {
  if (JMP_BUF_NEEDED) {
    if (sigsetjmp(checkpoint, 0)) {
      /* error() longjmped back to us. */
      return;
    }
  }
  error(s, "deadlock");
}

static int state_cmp(const struct state *NONNULL a,
                     const struct state *NONNULL b) {
  return memcmp(a->data, b->data, sizeof(a->data));
}

static bool state_eq(const struct state *NONNULL a,
                     const struct state *NONNULL b) {
  return state_cmp(a, b) == 0;
}

static void handle_copy(struct handle a, struct handle b);

static struct state *state_dup(const struct state *NONNULL s) {
  struct state *n = state_new();
  memcpy(n->data, s->data, sizeof(n->data));
#if COUNTEREXAMPLE_TRACE != CEX_OFF || LIVENESS_COUNT > 0
  state_previous_set(n, s);
#endif
#if BOUND > 0
  assert(state_bound_get(s) < BOUND && "exceeding bounded exploration depth");
  state_bound_set(n, state_bound_get(s) + 1);
#endif
#if LIVENESS_COUNT > 0
  memset(n->liveness, 0, sizeof(n->liveness));
#endif

  if (USE_SCALARSET_SCHEDULES) {
    /* copy schedule data related to past scalarset permutations */
    struct handle sch_src = state_schedule_handle(s, 0, SCHEDULE_BITS);
    struct handle sch_dst = state_schedule_handle(n, 0, SCHEDULE_BITS);
    handle_copy(sch_dst, sch_src);
  }

  return n;
}

static size_t state_hash(const struct state *NONNULL s) {
  return (size_t)MurmurHash64A(s->data, sizeof(s->data));
}

#if COUNTEREXAMPLE_TRACE != CEX_OFF
static __attribute__((unused)) size_t
state_depth(const struct state *NONNULL s) {
#if BOUND > 0
  uint64_t bound = state_bound_get(s);
  ASSERT(bound <= BOUND && "claimed state bound exceeds limit");
  return (size_t)bound + 1;
#else
  size_t d = 0;
  while (s != NULL) {
    d++;
    s = state_previous_get(s);
  }
  return d;
#endif
}
#endif

/* A type-safe const cast. */
static __attribute__((unused)) struct state *
state_drop_const(const struct state *s) {
  return (struct state *)s;
}

/* These functions are generated. */
static void state_canonicalise_heuristic(struct state *NONNULL s);
static void state_canonicalise_exhaustive(struct state *NONNULL s);

static void state_canonicalise(struct state *NONNULL s) {

  assert(s != NULL && "attempt to canonicalise NULL state");

  switch (SYMMETRY_REDUCTION) {

  case SYMMETRY_REDUCTION_OFF:
    break;

  case SYMMETRY_REDUCTION_HEURISTIC:
    state_canonicalise_heuristic(s);
    break;

  case SYMMETRY_REDUCTION_EXHAUSTIVE:
    state_canonicalise_exhaustive(s);
    break;
  }
}

/* This function is generated. */
static __attribute__((unused)) void state_print_field_offsets(void);

/* Print a state to stderr. This function is generated. This function assumes
 * that the caller already holds a lock on stdout.
 */
static __attribute__((unused)) void state_print(const struct state *previous,
                                                const struct state *NONNULL s);

/* Print the first rule that resulted in s. This function is generated. This
 * function assumes that the caller holds a lock on stdout.
 */
static __attribute__((unused)) void
print_transition(const struct state *NONNULL s);

static void print_counterexample(const struct state *NONNULL s
                                 __attribute__((unused))) {

  assert(s != NULL && "missing state in request for counterexample trace");

#if COUNTEREXAMPLE_TRACE != CEX_OFF
  /* Construct an array of the states we need to print by walking backwards to
   * the initial starting state.
   */
  size_t trace_length = state_depth(s);

  const struct state **cex = xcalloc(trace_length, sizeof(cex[0]));

  {
    size_t i = trace_length - 1;
    for (const struct state *p = s; p != NULL; p = state_previous_get(p)) {
      assert(i < trace_length &&
             "error in counterexample trace traversal logic");
      cex[i] = p;
      i--;
    }
  }

  for (size_t i = 0; i < trace_length; i++) {

    const struct state *current = cex[i];
    const struct state *previous = i == 0 ? NULL : cex[i - 1];

    print_transition(current);

    if (MACHINE_READABLE_OUTPUT) {
      put("<state>\n");
    }
    state_print(COUNTEREXAMPLE_TRACE == FULL ? NULL : previous, current);
    if (MACHINE_READABLE_OUTPUT) {
      put("</state>\n");
    } else {
      put("----------\n\n");
    }
  }

  free(cex);
#endif
}

static __attribute__((unused)) struct handle
state_handle(const struct state *NONNULL s, size_t offset, size_t width) {

  assert(sizeof(s->data) * CHAR_BIT - width >= offset &&
         "generating an out of bounds handle in state_handle()");

  return (struct handle){
      .base = (uint8_t *)s->data + offset / CHAR_BIT,
      .offset = offset % CHAR_BIT,
      .width = width,
  };
}

static raw_value_t handle_read_raw(const struct state *NONNULL s,
                                   struct handle h) {

  /* Check if this read is larger than the variable we will store it in. This
   * can only occur if the user has manually overridden value_t with the
   * --value-type command line argument.
   */
  if (__builtin_expect(h.width > sizeof(raw_value_t) * 8, 0)) {
    error(s, "read of a handle that is wider than the value type");
  }

  ASSERT(h.width <= MAX_SIMPLE_WIDTH &&
         "read of a handle that is larger than "
         "the maximum width of a simple type in this model");

  uint64_t raw = (raw_value_t)read_raw(h);

  TRACE(TC_HANDLE_READS,
        "read value %" PRIRAWVAL " from handle { %p, %zu, %zu }",
        raw_value_to_string(raw), h.base, h.offset, h.width);

  return raw;
}

static value_t decode_value(value_t lb, value_t ub, raw_value_t v) {

  value_t dest = 0;

  bool r __attribute__((unused)) =
      SUB(v, 1, &v) || ADD(v, lb, &dest) || dest < lb || dest > ub;

  ASSERT(!r && "read of out-of-range value");

  return dest;
}

static __attribute__((unused)) value_t
handle_read(const char *NONNULL context, const char *rule_name,
            const char *NONNULL name, const struct state *NONNULL s, value_t lb,
            value_t ub, struct handle h) {

  assert(context != NULL);
  assert(name != NULL);

  /* If we happen to be reading from the current state, do a sanity check that
   * we're only reading within bounds.
   */
  assert((h.base != (uint8_t *)s->data /* not a read from the current state */
          || sizeof(s->data) * CHAR_BIT - h.width >= h.offset) /* in bounds */
         && "out of bounds read in handle_read()");

  raw_value_t dest = handle_read_raw(s, h);

  if (__builtin_expect(dest == 0, 0)) {
    error(s, "%sread of undefined value in %s%s%s", context, name,
          rule_name == NULL ? "" : " within ",
          rule_name == NULL ? "" : rule_name);
  }

  return decode_value(lb, ub, dest);
}

static void handle_write_raw(const struct state *NONNULL s, struct handle h,
                             raw_value_t value) {

  /* Check if this write is larger than the variable we will are reading from.
   * This can only occur if the user has manually overridden value_t with the
   * --value-type command line argument.
   */
  if (__builtin_expect(h.width > sizeof(raw_value_t) * 8, 0)) {
    error(s, "write of a handle that is wider than the value type");
  }

  ASSERT(h.width <= MAX_SIMPLE_WIDTH &&
         "write to a handle that is larger than "
         "the maximum width of a simple type in this model");

  TRACE(TC_HANDLE_WRITES,
        "writing value %" PRIRAWVAL " to handle { %p, %zu, %zu }",
        raw_value_to_string(value), h.base, h.offset, h.width);

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-compare"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT((uintmax_t)value <= UINT64_MAX &&
         "truncating value during handle_write_raw");
#ifdef __clang__
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

  write_raw(h, (uint64_t)value);
}

static __attribute__((unused)) void
handle_write(const char *NONNULL context, const char *rule_name,
             const char *NONNULL name, const struct state *NONNULL s,
             value_t lb, value_t ub, struct handle h, value_t value) {

  assert(context != NULL);
  assert(name != NULL);

  /* If we happen to be writing to the current state, do a sanity check that
   * we're only writing within bounds.
   */
  assert((h.base != (uint8_t *)s->data /* not a write to the current state */
          || sizeof(s->data) * CHAR_BIT - h.width >= h.offset) /* in bounds */
         && "out of bounds write in handle_write()");

  raw_value_t r;
  if (__builtin_expect(
          value < lb || value > ub || SUB(value, lb, &r) || ADD(r, 1, &r), 0)) {
    error(s, "%swrite of out-of-range value into %s%s%s", context, name,
          rule_name == NULL ? "" : " within ",
          rule_name == NULL ? "" : rule_name);
  }

  handle_write_raw(s, h, r);
}

static __attribute__((unused)) void handle_zero(struct handle h) {

  uint8_t *p = h.base + h.offset / 8;

  /* Zero out up to a byte-aligned offset. */
  if (h.offset % 8 != 0) {
    uint8_t mask = (UINT8_C(1) << (h.offset % 8)) - 1;
    if (h.width < 8 - h.offset % 8) {
      mask |= UINT8_MAX & ~((UINT8_C(1) << (h.offset % 8 + h.width)) - 1);
    }
    *p &= mask;
    p++;
    if (h.width < 8 - h.offset % 8) {
      return;
    }
    h.width -= 8 - h.offset % 8;
  }

  /* Zero out as many bytes as we can. */
  memset(p, 0, h.width / 8);
  p += h.width / 8;
  h.width -= h.width / 8 * 8;

  /* Zero out the trailing bits in the final byte. */
  if (h.width > 0) {
    uint8_t mask = ~((UINT8_C(1) << h.width) - 1);
    *p &= mask;
  }
}

static void handle_copy(struct handle a, struct handle b) {

  ASSERT(a.width == b.width && "copying between handles of different sizes");

  /* FIXME: This does a bit-by-bit copy which almost certainly could be
   * accelerated by detecting byte-boundaries and complementary alignment and
   * then calling memcpy when possible.
   */

  for (size_t i = 0; i < a.width; i++) {

    uint8_t *dst = a.base + (a.offset + i) / 8;
    size_t dst_off = (a.offset + i) % 8;

    const uint8_t *src = b.base + (b.offset + i) / 8;
    size_t src_off = (b.offset + i) % 8;

    uint8_t or_mask = ((*src >> src_off) & UINT8_C(1)) << dst_off;
    uint8_t and_mask = ~(UINT8_C(1) << dst_off);

    *dst = (*dst & and_mask) | or_mask;
  }
}

static __attribute__((unused)) bool handle_eq(struct handle a,
                                              struct handle b) {

  ASSERT(a.width == b.width && "comparing handles of different sizes");

  /* FIXME: as with handle_copy, we do a bit-by-bit comparison which could be
   * made more efficient.
   */

  for (size_t i = 0; i < a.width; i++) {

    uint8_t *x = a.base + (a.offset + i) / 8;
    size_t x_off = (a.offset + i) % 8;
    bool x_bit = (*x >> x_off) & 0x1;

    const uint8_t *y = b.base + (b.offset + i) / 8;
    size_t y_off = (b.offset + i) % 8;
    bool y_bit = (*y >> y_off) & 0x1;

    if (x_bit != y_bit) {
      return false;
    }
  }

  return true;
}

static __attribute__((unused)) struct handle
handle_narrow(struct handle h, size_t offset, size_t width) {

  ASSERT(h.offset + offset + width <= h.offset + h.width &&
         "narrowing a handle with values that actually expand it");

  size_t r __attribute__((unused));
  assert(!ADD(h.offset, offset, &r) && "narrowing handle overflows a size_t");

  return (struct handle){
      .base = h.base + (h.offset + offset) / CHAR_BIT,
      .offset = (h.offset + offset) % CHAR_BIT,
      .width = width,
  };
}

static __attribute__((unused)) struct handle
handle_index(const char *NONNULL context, const char *rule_name,
             const char *NONNULL expr, const struct state *NONNULL s,
             size_t element_width, value_t index_min, value_t index_max,
             struct handle root, value_t index) {

  assert(expr != NULL);

  if (__builtin_expect(index < index_min || index > index_max, 0)) {
    error(s, "%sindex out of range in expression %s%s%s", context, expr,
          rule_name == NULL ? "" : " within ",
          rule_name == NULL ? "" : rule_name);
  }

  size_t r1, r2;
  if (__builtin_expect(
          SUB(index, index_min, &r1) || MUL(r1, element_width, &r2), 0)) {
    error(s, "%soverflow when indexing array in expression %s%s%s", context,
          expr, rule_name == NULL ? "" : " within ",
          rule_name == NULL ? "" : rule_name);
  }

  size_t r __attribute__((unused));
  assert(!ADD(root.offset, r2, &r) && "indexing handle overflows a size_t");

  return (struct handle){
      .base = root.base + (root.offset + r2) / CHAR_BIT,
      .offset = (root.offset + r2) % CHAR_BIT,
      .width = element_width,
  };
}

static __attribute__((unused)) value_t
handle_isundefined(const struct state *NONNULL s, struct handle h) {
  raw_value_t v = handle_read_raw(s, h);

  return v == 0;
}

/* Overflow-safe helpers for doing bounded arithmetic. */

static __attribute__((unused)) value_t add(const char *NONNULL context,
                                           const char *rule_name,
                                           const char *NONNULL expr,
                                           const struct state *NONNULL s,
                                           value_t a, value_t b) {

  assert(context != NULL);
  assert(expr != NULL);

  value_t r;
  if (__builtin_expect(ADD(a, b, &r), 0)) {
    error(s, "%sinteger overflow in addition in expression %s%s%s", context,
          expr, rule_name == NULL ? "" : " within ",
          rule_name == NULL ? "" : rule_name);
  }
  return r;
}

static __attribute__((unused)) value_t sub(const char *NONNULL context,
                                           const char *rule_name,
                                           const char *NONNULL expr,
                                           const struct state *NONNULL s,
                                           value_t a, value_t b) {

  assert(context != NULL);
  assert(expr != NULL);

  value_t r;
  if (__builtin_expect(SUB(a, b, &r), 0)) {
    error(s, "%sinteger overflow in subtraction in expression %s%s%s", context,
          expr, rule_name == NULL ? "" : " within ",
          rule_name == NULL ? "" : rule_name);
  }
  return r;
}

static __attribute__((unused)) value_t mul(const char *NONNULL context,
                                           const char *rule_name,
                                           const char *NONNULL expr,
                                           const struct state *NONNULL s,
                                           value_t a, value_t b) {

  assert(context != NULL);
  assert(expr != NULL);

  value_t r;
  if (__builtin_expect(MUL(a, b, &r), 0)) {
    error(s, "%sinteger overflow in multiplication in expression %s%s%s",
          context, expr, rule_name == NULL ? "" : " within ",
          rule_name == NULL ? "" : rule_name);
  }
  return r;
}

static __attribute__((unused)) value_t divide(const char *NONNULL context,
                                              const char *rule_name,
                                              const char *NONNULL expr,
                                              const struct state *NONNULL s,
                                              value_t a, value_t b) {

  assert(context != NULL);
  assert(expr != NULL);

  if (__builtin_expect(b == 0, 0)) {
    error(s, "%sdivision by zero in expression %s%s%s", context, expr,
          rule_name == NULL ? "" : " within ",
          rule_name == NULL ? "" : rule_name);
  }

  if (__builtin_expect(VALUE_MIN != 0 && a == VALUE_MIN && b == (value_t)-1,
                       0)) {
    error(s, "%sinteger overflow in division in expression %s%s%s", context,
          expr, rule_name == NULL ? "" : " within ",
          rule_name == NULL ? "" : rule_name);
  }

  return a / b;
}

static __attribute__((unused)) value_t mod(const char *NONNULL context,
                                           const char *rule_name,
                                           const char *NONNULL expr,
                                           const struct state *NONNULL s,
                                           value_t a, value_t b) {

  assert(context != NULL);
  assert(expr != NULL);

  if (__builtin_expect(b == 0, 0)) {
    error(s, "%smodulus by zero in expression %s%s%s", context, expr,
          rule_name == NULL ? "" : " within ",
          rule_name == NULL ? "" : rule_name);
  }

  /* Is INT_MIN % -1 UD? Reading the C spec I am not sure. */
  if (__builtin_expect(VALUE_MIN != 0 && a == VALUE_MIN && b == (value_t)-1,
                       0)) {
    error(s, "%sinteger overflow in modulo in expression %s%s%s", context, expr,
          rule_name == NULL ? "" : " within ",
          rule_name == NULL ? "" : rule_name);
  }

  return a % b;
}

static __attribute__((unused)) value_t
negate(const char *NONNULL context, const char *rule_name,
       const char *NONNULL expr, const struct state *NONNULL s, value_t a) {

  assert(context != NULL);
  assert(expr != NULL);

  if (__builtin_expect(VALUE_MIN != 0 && a == VALUE_MIN, 0)) {
    error(s, "%sinteger overflow in negation in expression %s%s%s", context,
          expr, rule_name == NULL ? "" : " within ",
          rule_name == NULL ? "" : rule_name);
  }

  return -a;
}

/* wrappers for << and >> to avoid C undefined behaviour */
static __attribute__((unused)) value_t rsh(value_t a, value_t b);
static __attribute__((unused)) value_t lsh(value_t a, value_t b) {

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-compare"
#pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if (value_is_signed() && b <= -(value_t)(sizeof(a) * 8)) {
    return 0;
  }

  if (b >= (value_t)(sizeof(a) * 8)) {
    return 0;
  }

  if (b < 0) {
    return rsh(a, -b);
  }
#ifdef __clang__
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

  return (value_t)((raw_value_t)a << b);
}

static value_t rsh(value_t a, value_t b) {

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-compare"
#pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if (value_is_signed() && b <= -(value_t)(sizeof(a) * 8)) {
    return 0;
  }

  if (b >= (value_t)(sizeof(a) * 8)) {
    return 0;
  }

  if (b < 0) {
    return lsh(a, -b);
  }
#ifdef __clang__
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

  return a >> b;
}

/* Bitwise NOT wrapper. We only need this to suppress some spurious GCC
 * warnings:
 *   https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
 *   https://gcc.gnu.org/bugzilla/show_bug.cgi?id=59098
 */
static __attribute__((unused)) value_t bnot(value_t v) {
  return (value_t)~(raw_value_t)v;
}

/* use Heap's algorithm for generating permutations to implement a
 * permutation-to-number mapping
 */
static __attribute__((unused)) size_t
permutation_to_index(const size_t *NONNULL permutation, size_t *NONNULL stack,
                     size_t *NONNULL working, size_t count) {

  /* byte extent of the permutation arrays */
  size_t bytes = sizeof(permutation[0]) * count;

  size_t index = 0;

  /* clear our stack */
  memset(stack, 0, bytes);

  /* initialise the first (identity) permutation */
  for (size_t i = 0; i < count; ++i) {
    working[i] = i;
  }

  if (memcmp(permutation, working, bytes) == 0) {
    return index;
  }

  size_t i = 0;
  while (i < count) {
    if (stack[i] < i) {
      ++index;
      if (i % 2 == 0) {
        /* swap working[0] and working[i] */
        size_t tmp = working[0];
        working[0] = working[i];
        working[i] = tmp;
      } else {
        /* swap working[stack[i]] and working[i] */
        size_t tmp = working[stack[i]];
        working[stack[i]] = working[i];
        working[i] = tmp;
      }
      if (memcmp(permutation, working, bytes) == 0) {
        return index;
      }
      ++stack[i];
      i = 0;
    } else {
      stack[i] = 0;
      ++i;
    }
  }

  /* the permutation should have been one of the possible ones */
  ASSERT(!"invalid permutation passed to permutation_to_index");
}
static __attribute__((unused)) void
index_to_permutation(size_t index, size_t *NONNULL permutation,
                     size_t *NONNULL stack, size_t count) {

  /* byte extent of the permutation arrays */
  size_t bytes = sizeof(permutation[0]) * count;

  size_t ind = 0;

  /* clear our stack */
  memset(stack, 0, bytes);

  /* initialise the first (identity) permutation */
  for (size_t i = 0; i < count; ++i) {
    permutation[i] = i;
  }

  if (ind == index) {
    return;
  }

  size_t i = 0;
  while (i < count) {
    if (stack[i] < i) {
      ++ind;
      if (i % 2 == 0) {
        /* swap permutation[0] and permutation[i] */
        size_t tmp = permutation[0];
        permutation[0] = permutation[i];
        permutation[i] = tmp;
      } else {
        /* swap permutation[stack[i]] and permutation[i] */
        size_t tmp = permutation[stack[i]];
        permutation[stack[i]] = permutation[i];
        permutation[i] = tmp;
      }
      if (ind == index) {
        return;
      }
      ++stack[i];
      i = 0;
    } else {
      stack[i] = 0;
      ++i;
    }
  }

  /* the target permutation should have been one of the possible ones */
  ASSERT(!"invalid index passed to index_to_permutation");
}

/*******************************************************************************
 * State queue node                                                            *
 *                                                                             *
 * Queue nodes are 4K-sized, 4K-aligned linked-list nodes. They contain        *
 * pending states and then a pointer to the next node in the queue.            *
 ******************************************************************************/

struct queue_node {
  struct state
      *s[(4096 - sizeof(struct queue_node *)) / sizeof(struct state *)];
  struct queue_node *next;
};

_Static_assert(sizeof(struct queue_node) == 4096,
               "incorrect queue_node size calculation");

static struct queue_node *queue_node_new(void) {
  struct queue_node *p = NULL;

  int r = posix_memalign((void **)&p, sizeof(*p), sizeof(*p));

  assert((r == 0 || r == ENOMEM) && "invalid alignment to posix_memalign");

  if (__builtin_expect(r != 0, 0)) {
    oom();
  }

  memset(p, 0, sizeof(*p));

  return p;
}

static void queue_node_free(struct queue_node *p) { free(p); }

/******************************************************************************/

/*******************************************************************************
 * Queue node handles                                                          *
 *                                                                             *
 * These are pointers to a member-aligned address within a queue_node. The     *
 * idea is that you can have a queue_handle_t pointing at either one of the    *
 * elements of the `s` member or at the chained `next` pointer. Since          *
 * queue_node pointers are always 4K-aligned, you can examine the low 12 bits  *
 * of the queue node handle to determine which member you are pointing at.     *
 ******************************************************************************/

typedef uintptr_t queue_handle_t;

static queue_handle_t queue_handle_from_node_ptr(const struct queue_node *n) {
  return (queue_handle_t)n;
}

static struct queue_node *queue_handle_base(queue_handle_t h) {
  return (struct queue_node *)(h - h % sizeof(struct queue_node));
}

static bool queue_handle_is_state_pptr(queue_handle_t h) {
  return h % sizeof(struct queue_node) <
         __builtin_offsetof(struct queue_node, next);
}

static struct state **queue_handle_to_state_pptr(queue_handle_t h) {
  assert(queue_handle_is_state_pptr(h) &&
         "invalid use of queue_handle_to_state_pptr");

  return (struct state **)h;
}

static struct queue_node **queue_handle_to_node_pptr(queue_handle_t h) {
  assert(!queue_handle_is_state_pptr(h) &&
         "invalid use of queue_handle_to_node_pptr");

  return (struct queue_node **)h;
}

static queue_handle_t queue_handle_next(queue_handle_t h) {
  return h + sizeof(struct state *);
}

/******************************************************************************/

/*******************************************************************************
 * Hazard pointers                                                             *
 *                                                                             *
 * The idea of "hazard pointers" comes from Maged Michael, "Hazard Pointers:   *
 * Safe Memory Reclamation for Lock-Free Objects" in TPDS 15(8) 2004. The      *
 * basic concept is to maintain a collection of safe-to-dereference pointers.  *
 * Before freeing a pointer, you look in this collection to see if it is in    *
 * use and you must always add a pointer to this collection before             *
 * dereferencing it. The finer details of how we keep this consistent and why  *
 * the size of this collection can be statically known ahead of time are a     *
 * little complicated, but the paper explains this in further detail.          *
 ******************************************************************************/

/* Queue node pointers currently safe to dereference. */
static const struct queue_node *hazarded[THREADS];

/* Protect a pointer that we wish to dereference. */
static void hazard(queue_handle_t h) {

  /* Find the queue node this handle lies within. */
  const struct queue_node *p = queue_handle_base(h);

  /* You can't protect the null pointer because it is invalid to dereference it.
   */
  assert(p != NULL && "attempt to hazard an invalid pointer");

  /* Each thread is only allowed a single hazarded pointer at a time. */
  assert(__atomic_load_n(&hazarded[thread_id], __ATOMIC_SEQ_CST) == NULL &&
         "hazarding multiple pointers at once");

  __atomic_store_n(&hazarded[thread_id], p, __ATOMIC_SEQ_CST);
}

/* Drop protection on a pointer whose target we are done accessing. */
static void unhazard(queue_handle_t h) {

  /* Find the queue node this handle lies within. */
  const struct queue_node *p __attribute__((unused)) = queue_handle_base(h);

  assert(p != NULL && "attempt to unhazard an invalid pointer");

  assert(__atomic_load_n(&hazarded[thread_id], __ATOMIC_SEQ_CST) != NULL &&
         "unhazarding a pointer when none are hazarded");

  assert(__atomic_load_n(&hazarded[thread_id], __ATOMIC_SEQ_CST) == p &&
         "unhazarding a pointer that differs from the one hazarded");

  __atomic_store_n(&hazarded[thread_id], NULL, __ATOMIC_SEQ_CST);
}

/* Free a pointer or, if not possible, defer this to later. */
static void reclaim(queue_handle_t h) {

  /* Find the queue node this handle lies within. */
  struct queue_node *p = queue_handle_base(h);

  assert(p != NULL && "reclaiming a null pointer");

  /* The reclaimer is not allowed to be freeing something while also holding a
   * hazarded pointer.
   */
  assert(__atomic_load_n(&hazarded[thread_id], __ATOMIC_SEQ_CST) == NULL &&
         "reclaiming a pointer while holding a hazarded pointer");

  /* Pointers that we failed to free initially because they were in use
   * (hazarded) at the time they were passed to reclaim().
   *
   * Why are we sure we will only ever have a maximum of `THREADS - 1` pointers
   * outstanding? Anything passed to reclaim() is expected to be
   * now-unreachable, so the only outstanding references to such are threads
   * racing with us. Because each thread can only have one hazarded pointer at a
   * time, the maximum number of in use pointers right now is `THREADS - 1`
   * (because the current thread does not have one).
   *
   * There is an edge case where another thread (1) held a hazarded pointer we
   * previously tried to reclaim and thus ended up on our deferred list, then
   * (2) in-between that time and now dropped this reference and acquired a
   * hazarded pointer to `p`. This still will not exceed our count of
   * `THREADS - 1` as the order in which we scan the deferred list and then add
   * `p` to it below ensures that we will discover the originally hazarded
   * pointer (now no longer conflicted) and clear its slot, leaving this
   * available for `p`.
   */
  static _Thread_local struct queue_node *deferred[THREADS - 1];
  static const size_t DEFERRED_SIZE = sizeof(deferred) / sizeof(deferred[0]);

  /* First try to free any previously deferred pointers. */
  for (size_t i = 0; i < DEFERRED_SIZE; i++) {
    if (deferred[i] != NULL) {
      bool conflict = false;
      for (size_t j = 0; j < sizeof(hazarded) / sizeof(hazarded[0]); j++) {
        if (j == thread_id) {
          /* No need to check for conflicts with ourself. */
          assert(__atomic_load_n(&hazarded[j], __ATOMIC_SEQ_CST) == NULL);
          continue;
        }
        if (deferred[i] == __atomic_load_n(&hazarded[j], __ATOMIC_SEQ_CST)) {
          /* This pointer is in use by thread j. */
          conflict = true;
          break;
        }
      }
      if (!conflict) {
        queue_node_free(deferred[i]);
        deferred[i] = NULL;
      }
    }
  }

  /* Now deal with the pointer we were passed. The most likely case is that no
   * one else is using this pointer, so try this first.
   */
  bool conflict = false;
  for (size_t i = 0; i < sizeof(hazarded) / sizeof(hazarded[i]); i++) {
    if (i == thread_id) {
      /* No need to check for conflicts with ourself. */
      assert(__atomic_load_n(&hazarded[i], __ATOMIC_SEQ_CST) == NULL);
      continue;
    }
    if (p == __atomic_load_n(&hazarded[i], __ATOMIC_SEQ_CST)) {
      /* Bad luck :( */
      conflict = true;
      break;
    }
  }

  if (!conflict) {
    /* We're done! */
    queue_node_free(p);
    return;
  }

  /* If we reached here, we need to defer this reclamation to later. */
  for (size_t i = 0; i < DEFERRED_SIZE; i++) {
    if (deferred[i] == NULL) {
      deferred[i] = p;
      return;
    }
  }

  ASSERT(!"deferred more than `THREADS` reclamations");
}

/******************************************************************************/

/*******************************************************************************
 * Atomic operations on double word values                                     *
 ******************************************************************************/

#if __SIZEOF_POINTER__ <= 4
typedef uint64_t dword_t;
#elif __SIZEOF_POINTER__ <= 8
typedef unsigned __int128 dword_t;
#else
#error "unexpected pointer size; what scalar type to use for dword_t?"
#endif

static dword_t atomic_read(dword_t *p) {

  if (THREADS == 1) {
    return *p;
  }

#if defined(__x86_64__) || defined(__i386__)
  /* x86-64: MOV is not guaranteed to be atomic on 128-bit naturally aligned
   *   memory. The way to work around this is apparently the following
   *   degenerate CMPXCHG16B.
   * i386: __atomic_load_n emits code calling a libatomic function that takes a
   *   lock, making this no longer lock free. Force a CMPXCHG8B by using the
   *   __sync built-in instead.
   */
  return __sync_val_compare_and_swap(p, 0, 0);
#endif

  return __atomic_load_n(p, __ATOMIC_SEQ_CST);
}

static void atomic_write(dword_t *p, dword_t v) {

  if (THREADS == 1) {
    *p = v;
    return;
  }

#if defined(__x86_64__) || defined(__i386__)
  /* As explained above, we need some extra gymnastics to avoid a call to
   * libatomic on x86-64 and i386.
   */
  dword_t expected;
  dword_t old = 0;
  do {
    expected = old;
    old = __sync_val_compare_and_swap(p, expected, v);
  } while (expected != old);
  return;
#endif

  __atomic_store_n(p, v, __ATOMIC_SEQ_CST);
}

static bool atomic_cas(dword_t *p, dword_t expected, dword_t new) {

  if (THREADS == 1) {
    if (*p == expected) {
      *p = new;
      return true;
    }
    return false;
  }

#if defined(__x86_64__) || defined(__i386__)
  /* Make GCC >= 7.1 emit cmpxchg on x86-64 and i386. See
   * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80878.
   */
  return __sync_bool_compare_and_swap(p, expected, new);
#endif

  return __atomic_compare_exchange_n(p, &expected, new, false, __ATOMIC_SEQ_CST,
                                     __ATOMIC_SEQ_CST);
}

static dword_t atomic_cas_val(dword_t *p, dword_t expected, dword_t new) {

  if (THREADS == 1) {
    dword_t old = *p;
    if (old == expected) {
      *p = new;
    }
    return old;
  }

#if defined(__x86_64__) || defined(__i386__)
  /* Make GCC >= 7.1 emit cmpxchg on x86-64 and i386. See
   * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80878.
   */
  return __sync_val_compare_and_swap(p, expected, new);
#endif

  (void)__atomic_compare_exchange_n(p, &expected, new, false, __ATOMIC_SEQ_CST,
                                    __ATOMIC_SEQ_CST);
  return expected;
}

/******************************************************************************/

/*******************************************************************************
 * Double pointers                                                             *
 *                                                                             *
 * A scalar type that can store the value of two pointers.                     *
 ******************************************************************************/

#if __SIZEOF_POINTER__ <= 4
typedef uint64_t double_ptr_t;
#elif __SIZEOF_POINTER__ <= 8
typedef unsigned __int128 double_ptr_t;
#else
#error "unexpected pointer size; what scalar type to use for double_ptr_t?"
#endif

static uintptr_t double_ptr_extract1(double_ptr_t p) {

  _Static_assert(sizeof(p) > sizeof(uintptr_t),
                 "double_ptr_t is not big enough to fit a pointer");

  uintptr_t q;
  memcpy(&q, &p, sizeof(q));

  return q;
}

static uintptr_t double_ptr_extract2(double_ptr_t p) {

  _Static_assert(sizeof(p) >= 2 * sizeof(uintptr_t),
                 "double_ptr_t is not big enough to fit two pointers");

  uintptr_t q;
  memcpy(&q, (unsigned char *)&p + sizeof(void *), sizeof(q));

  return q;
}

static double_ptr_t double_ptr_make(uintptr_t q1, uintptr_t q2) {

  double_ptr_t p = 0;

  _Static_assert(sizeof(p) >= 2 * sizeof(uintptr_t),
                 "double_ptr_t is not big enough to fit two pointers");

  memcpy(&p, &q1, sizeof(q1));
  memcpy((unsigned char *)&p + sizeof(q1), &q2, sizeof(q2));

  return p;
}

/******************************************************************************/

/*******************************************************************************
 * State queue                                                                 *
 *                                                                             *
 * The following implements a per-thread queue for pending states. The only    *
 * supported operations are enqueueing and dequeueing states. A property we    *
 * maintain is that all states within all queues pass the current model's      *
 * invariants.                                                                 *
 ******************************************************************************/

static struct {
  double_ptr_t ends;
  size_t count;
} q[THREADS];

static size_t queue_enqueue(struct state *NONNULL s, size_t queue_id) {
  assert(queue_id < sizeof(q) / sizeof(q[0]) && "out of bounds queue access");

  /* Look up the tail of the queue. */

  double_ptr_t ends = atomic_read(&q[queue_id].ends);

retry:;
  queue_handle_t tail = double_ptr_extract2(ends);

  if (tail == 0) {
    /* There's nothing currently in the queue. */

    assert(double_ptr_extract1(ends) == 0 &&
           "tail of queue 0 while head is non-0");

    struct queue_node *n = queue_node_new();
    n->s[0] = s;

    double_ptr_t new = double_ptr_make(queue_handle_from_node_ptr(n),
                                       queue_handle_from_node_ptr(n));

    double_ptr_t old = atomic_cas_val(&q[queue_id].ends, ends, new);
    if (old != ends) {
      /* Failed. */
      queue_node_free(n);
      ends = old;
      goto retry;
    }

  } else {
    /* The queue is non-empty, so we'll need to access the last element. */

    /* Try to protect our upcoming access to the tail. */
    hazard(tail);
    {
      double_ptr_t ends_check = atomic_read(&q[queue_id].ends);
      if (ends != ends_check) {
        /* Failed. Someone else modified the queue in the meantime. */
        unhazard(tail);
        ends = ends_check;
        goto retry;
      }
    }

    /* We've now notified other threads that we're going to be accessing the
     * tail, so we can safely dereference its pointer.
     */

    struct queue_node *new_node = NULL;
    queue_handle_t next_tail = queue_handle_next(tail);

    if (queue_handle_is_state_pptr(next_tail)) {
      /* There's an available slot in this queue node; no need to create a new
       * one.
       */

      {
        struct state **target = queue_handle_to_state_pptr(next_tail);
        struct state *null = NULL;
        if (!__atomic_compare_exchange_n(target, &null, s, false,
                                         __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
          /* Failed. Someone else enqueued before we could. */
          unhazard(tail);
          goto retry;
        }
      }

    } else {
      /* There's no remaining slot in this queue node. We'll need to create a
       * new (empty) queue node, add our state to this one and then append this
       * node to the queue.
       */

      /* Create the new node. */
      new_node = queue_node_new();
      new_node->s[0] = s;

      /* Try to update the chained pointer of the current tail to point to this
       * new node.
       */
      struct queue_node **target = queue_handle_to_node_pptr(next_tail);
      struct queue_node *null = NULL;
      if (!__atomic_compare_exchange_n(target, &null, new_node, false,
                                       __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
        /* Failed. Someone else enqueued before we could. */
        queue_node_free(new_node);
        unhazard(tail);
        goto retry;
      }

      /* We now need the tail to point at our new node. */
      next_tail = queue_handle_from_node_ptr(new_node);
    }

    queue_handle_t head = double_ptr_extract1(ends);
    double_ptr_t new = double_ptr_make(head, next_tail);

    /* Try to update the queue. */
    {
      double_ptr_t old = atomic_cas_val(&q[queue_id].ends, ends, new);
      if (old != ends) {
        /* Failed. Someone else dequeued before we could finish. We know the
         * operation that beat us was a dequeue and not an enqueue, because by
         * writing to next_tail we have prevented any other enqueue from
         * succeeding.
         */

        /* Undo the update of next_tail. We know this is safe (non-racy) because
         * no other enqueue can proceed and a dequeue will never look into
         * next_tail due to the way it handles the case when head == tail.
         */
        next_tail = queue_handle_next(tail);
        if (queue_handle_is_state_pptr(next_tail)) {
          /* We previously wrote into an existing queue node. */
          struct state **target = queue_handle_to_state_pptr(next_tail);
          struct state *temp = s;
          bool r __attribute__((unused)) = __atomic_compare_exchange_n(
              target, &temp, NULL, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
          assert(r && "undo of write to next_tail failed");
        } else {
          /* We previously wrote into a new queue node. */
          struct queue_node **target = queue_handle_to_node_pptr(next_tail);
          struct queue_node *temp = new_node;
          bool r __attribute__((unused)) = __atomic_compare_exchange_n(
              target, &temp, NULL, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
          assert(r && "undo of write to next_tail failed");

          queue_node_free(new_node);
        }

        unhazard(tail);
        ends = old;
        goto retry;
      }
    }

    /* Success! */

    unhazard(tail);
  }

  size_t count = __atomic_add_fetch(&q[queue_id].count, 1, __ATOMIC_SEQ_CST);

  TRACE(TC_QUEUE, "enqueued state %p into queue %zu, queue length is now %zu",
        s, queue_id, count);

  return count;
}

static const struct state *queue_dequeue(size_t *NONNULL queue_id) {
  assert(queue_id != NULL && *queue_id < sizeof(q) / sizeof(q[0]) &&
         "out of bounds queue access");

  const struct state *s = NULL;

  for (size_t attempts = 0; attempts < sizeof(q) / sizeof(q[0]); attempts++) {

    double_ptr_t ends = atomic_read(&q[*queue_id].ends);

  retry:;
    queue_handle_t head = double_ptr_extract1(ends);

    if (head != 0) {
      /* This queue is non-empty. */

      /* Try to protect our upcoming accesses to the head. */
      hazard(head);
      {
        double_ptr_t ends_check = atomic_read(&q[*queue_id].ends);
        if (ends != ends_check) {
          /* Failed. Someone else updated the queue. */
          unhazard(head);
          ends = ends_check;
          goto retry;
        }
      }

      queue_handle_t tail = double_ptr_extract2(ends);

      double_ptr_t new;
      if (head == tail) {
        /* There is only a single element in the queue. We will need to update
         * both head and tail.
         */
        new = double_ptr_make(0, 0);
      } else if (queue_handle_is_state_pptr(head)) {
        /* There are multiple elements in the queue; we can deal only with the
         * head.
         */
        new = double_ptr_make(queue_handle_next(head), tail);
      } else {
        /* The head of the queue is the end of a queue node. I.e. the only thing
         * remaining in this queue node is the chained pointer to the next queue
         * node.
         */

        /* Load the next queue node. */
        struct queue_node **n = queue_handle_to_node_pptr(head);
        struct queue_node *new_head = __atomic_load_n(n, __ATOMIC_SEQ_CST);
        new = double_ptr_make(queue_handle_from_node_ptr(new_head), tail);

        /* Try to replace the current head with the next node. */
        double_ptr_t old = atomic_cas_val(&q[*queue_id].ends, ends, new);
        /* Either way, now we'll need to retry, but if we succeeded we also need
         * to free the queue node we just removed.
         */
        unhazard(head);
        if (old == ends) {
          /* Succeeded. */
          reclaim(head);
        }
        ends = old;
        goto retry;
      }

      /* Try to remove the head. */
      {
        double_ptr_t old = atomic_cas_val(&q[*queue_id].ends, ends, new);
        if (old != ends) {
          /* Failed. Someone else either enqueued or dequeued. */
          unhazard(head);
          ends = old;
          goto retry;
        }
      }

      /* We now have either a pointer to a state or we've just removed an empty
       * queue node that was the last in the queue.
       */

      if (queue_handle_is_state_pptr(head)) {
        struct state **st = queue_handle_to_state_pptr(head);
        s = *st;
      }

      unhazard(head);

      if (head == tail || !queue_handle_is_state_pptr(head)) {
        reclaim(head);
      }

      if (s == NULL) {
        /* Move to the next queue to try. */
        *queue_id = (*queue_id + 1) % (sizeof(q) / sizeof(q[0]));
        continue;
      }

      size_t count =
          __atomic_sub_fetch(&q[*queue_id].count, 1, __ATOMIC_SEQ_CST);

      TRACE(TC_QUEUE,
            "dequeued state %p from queue %zu, queue length is now %zu",
            s, *queue_id, count);

      return s;
    }

    /* Move to the next queue to try. */
    *queue_id = (*queue_id + 1) % (sizeof(q) / sizeof(q[0]));
  }

  return s;
}

/******************************************************************************/

/*******************************************************************************
 * Reference counted pointers                                                  *
 *                                                                             *
 * These are capable of encapsulating any generic pointer (void*). Note that   *
 * we rely on the existence of double-word atomics. On x86-64, you need to     *
 * use compiler flag '-mcx16' to get an efficient 128-bit cmpxchg.             *
 *                                                                             *
 * Of these functions, only the following are thread safe:                     *
 *                                                                             *
 *   * refcounted_ptr_get                                                      *
 *   * refcounted_ptr_peek                                                     *
 *   * refcounted_ptr_put                                                      *
 *   * refcounted_ptr_set                                                      *
 *                                                                             *
 * The caller is expected to coordinate with other threads to exclude them     *
 * operating on the relevant refcounted_ptr_t when using one of the other      *
 * functions:                                                                  *
 *                                                                             *
 *   * refcounted_ptr_shift                                                    *
 *                                                                             *
 ******************************************************************************/

struct refcounted_ptr {
  void *ptr;
  size_t count;
};

#if __SIZEOF_POINTER__ <= 4
typedef uint64_t refcounted_ptr_t;
#elif __SIZEOF_POINTER__ <= 8
typedef unsigned __int128 refcounted_ptr_t;
#else
#error "unexpected pointer size; what scalar type to use for refcounted_ptr_t?"
#endif

_Static_assert(sizeof(struct refcounted_ptr) <= sizeof(refcounted_ptr_t),
               "refcounted_ptr does not fit in a refcounted_ptr_t, which we "
               "need to operate on it atomically");

static void refcounted_ptr_set(refcounted_ptr_t *NONNULL p, void *ptr) {

#ifndef NDEBUG
  /* Read the current state of the pointer. */
  refcounted_ptr_t old = atomic_read(p);
  struct refcounted_ptr p2;
  memcpy(&p2, &old, sizeof(old));
  assert(p2.count == 0 && "overwriting a pointer source while someone still "
                          "has a reference to this pointer");
#endif

  /* Set the current source pointer with no outstanding references. */
  struct refcounted_ptr p3;
  p3.ptr = ptr;
  p3.count = 0;

  /* Commit the result. */
  refcounted_ptr_t new;
  memcpy(&new, &p3, sizeof(p3));
  atomic_write(p, new);
}

static void *refcounted_ptr_get(refcounted_ptr_t *NONNULL p) {

  refcounted_ptr_t old, new;
  void *ret;
  bool r;

  do {

    /* Read the current state of the pointer. */
    old = atomic_read(p);
    struct refcounted_ptr p2;
    memcpy(&p2, &old, sizeof(old));

    /* Take a reference to it. */
    p2.count++;
    ret = p2.ptr;

    /* Try to commit our results. */
    memcpy(&new, &p2, sizeof(new));
    r = atomic_cas(p, old, new);
  } while (!r);

  return ret;
}

static void refcounted_ptr_put(refcounted_ptr_t *NONNULL p,
                               void *ptr __attribute__((unused))) {

  refcounted_ptr_t old, new;
  bool r;

  do {

    /* Read the current state of the pointer. */
    old = atomic_read(p);
    struct refcounted_ptr p2;
    memcpy(&p2, &old, sizeof(old));

    /* Release our reference to it. */
    ASSERT(p2.ptr == ptr && "releasing a reference to a pointer after someone "
                            "has changed the pointer source");
    ASSERT(p2.count > 0 && "releasing a reference to a pointer when it had no "
                           "outstanding references");
    p2.count--;

    /* Try to commit our results. */
    memcpy(&new, &p2, sizeof(new));
    r = atomic_cas(p, old, new);
  } while (!r);
}

static void *refcounted_ptr_peek(refcounted_ptr_t *NONNULL p) {

  /* Read out the state of the pointer. This rather unpleasant expression is
   * designed to emit an atomic load at a smaller granularity than the entire
   * refcounted_ptr structure. Because we only need the pointer -- and not the
   * count -- we can afford to just atomically read the first word.
   */
  void *ptr = __atomic_load_n(
      (void **)((void *)p + __builtin_offsetof(struct refcounted_ptr, ptr)),
      __ATOMIC_SEQ_CST);

  return ptr;
}

static void refcounted_ptr_shift(refcounted_ptr_t *NONNULL current,
                                 refcounted_ptr_t *NONNULL next) {

  /* None of the operations in this function are performed atomically because we
   * assume the caller has synchronised with other threads via other means.
   */

  /* The pointer we're about to overwrite should not be referenced. */
  struct refcounted_ptr p __attribute__((unused));
  memcpy(&p, current, sizeof(*current));
  ASSERT(p.count == 0 && "overwriting a pointer that still has outstanding "
                         "references");

  /* Shift the next value into the current pointer. */
  *current = *next;

  /* Blank the value we just shifted over. */
  *next = 0;
}

/******************************************************************************/

/*******************************************************************************
 * Thread rendezvous support                                                   *
 ******************************************************************************/

static pthread_mutex_t rendezvous_lock; /* mutual exclusion mechanism for below. */
static pthread_cond_t rendezvous_cond;  /* sleep mechanism for below. */
static size_t running_count = 1;        /* how many threads are opted in to rendezvous? */
static size_t rendezvous_pending = 1;   /* how many threads are opted in and not sleeping? */

static void rendezvous_init(void) {
  int r = pthread_mutex_init(&rendezvous_lock, NULL);
  if (__builtin_expect(r != 0, 0)) {
    fprintf(stderr, "pthread_mutex_init failed: %s\n", strerror(r));
    exit(EXIT_FAILURE);
  }

  r = pthread_cond_init(&rendezvous_cond, NULL);
  if (__builtin_expect(r != 0, 0)) {
    fprintf(stderr, "pthread_cond_init failed: %s\n", strerror(r));
    exit(EXIT_FAILURE);
  }
}

/** call this at the start of a rendezvous point
 *
 * This is a low level function, not expected to be directly used outside of the
 * context of the rendezvous implementation.
 *
 * \return True if the caller was the last to arrive and henceforth dubbed the
 *   'leader'.
 */
static bool rendezvous_arrive(void) {
  int r __attribute__((unused)) = pthread_mutex_lock(&rendezvous_lock);
  assert(r == 0);

  /* Take a token from the rendezvous down-counter. */
  assert(rendezvous_pending > 0);
  rendezvous_pending--;

  /* If we were the last to arrive then it was our arrival that dropped the
   * counter to zero.
   */
  return rendezvous_pending == 0;
}

/** call this at the end of a rendezvous point
 *
 * This is a low level function, not expected to be directly used outside of the
 * context of the rendezvous implementation.
 *
 * \param leader Whether the caller is the 'leader'. If you call this when you
 *   are the 'leader' it will unblock all 'followers' at the rendezvous point.
 * \param action Optional code for the leader to run.
 */
static void rendezvous_depart(bool leader, void (*action)(void)) {
  int r __attribute((unused));

  if (leader) {
    if (action != NULL) {
      action();
    }

    /* Reset the counter for the next rendezvous. */
    assert(rendezvous_pending == 0 &&
           "a rendezvous point is being exited "
           "while some participating threads have yet to arrive");
    rendezvous_pending = running_count;

    /* Wake up the 'followers'. */
    r = pthread_cond_broadcast(&rendezvous_cond);
    assert(r == 0);

  } else {

    /* Wait on the 'leader' to wake us up. */
    r = pthread_cond_wait(&rendezvous_cond, &rendezvous_lock);
    assert(r == 0);
  }

  r = pthread_mutex_unlock(&rendezvous_lock);
  assert(r == 0);
}

/* Exposed friendly function for performing a rendezvous. */
static void rendezvous(void (*action)(void)) {
  bool leader = rendezvous_arrive();
  if (leader) {
    TRACE(TC_SET, "arrived at rendezvous point as leader");
  }
  rendezvous_depart(leader, action);
}

/* Remove the caller from the pool of threads who participate in this
 * rendezvous.
 */
static void rendezvous_opt_out(void (*action)(void)) {

retry:;

  /* "Arrive" at the rendezvous to decrement the count of outstanding threads.
   */
  bool leader = rendezvous_arrive();

  if (leader && running_count > 1) {
    /* We unfortunately opted out of this rendezvous while the remaining threads
     * were arriving at one and we were the last to arrive. Let's pretend we are
     * participating in the rendezvous and unblock them.
     */
    rendezvous_depart(true, action);

    /* Re-attempt opting-out. */
    goto retry;
  }

  /* Remove ourselves from the known threads. */
  assert(running_count > 0);
  running_count--;

  int r __attribute__((unused)) = pthread_mutex_unlock(&rendezvous_lock);
  assert(r == 0);
}

/******************************************************************************/

/*******************************************************************************
 * 'Slots', an opaque wrapper around a state pointer                           *
 *                                                                             *
 * See usage of this in the state set below for its purpose.                   *
 ******************************************************************************/

typedef uintptr_t slot_t;

static __attribute__((const)) slot_t slot_empty(void) { return 0; }

static __attribute__((const)) bool slot_is_empty(slot_t s) {
  return s == slot_empty();
}

static __attribute__((const)) slot_t slot_tombstone(void) {
  static const slot_t TOMBSTONE = ~(slot_t)0;
  return TOMBSTONE;
}

static __attribute__((const)) bool slot_is_tombstone(slot_t s) {
  return s == slot_tombstone();
}

static struct state *slot_to_state(slot_t s) {
  ASSERT(!slot_is_empty(s));
  ASSERT(!slot_is_tombstone(s));
  return (struct state *)s;
}

static slot_t state_to_slot(const struct state *s) { return (slot_t)s; }

/******************************************************************************/

/*******************************************************************************
 * State set                                                                   *
 *                                                                             *
 * The following implementation provides a set for storing the seen states.    *
 * There is no support for removing elements, only thread-safe insertion of    *
 * elements.                                                                   *
 ******************************************************************************/

enum {
  INITIAL_SET_SIZE_EXPONENT =
      sizeof(unsigned long long) * 8 - 1 -
      __builtin_clzll(SET_CAPACITY / sizeof(struct state *) /
                      sizeof(struct state))
};

struct set {
  slot_t *bucket;
  size_t size_exponent;
};

/* Some utility functions for dealing with exponents. */

static size_t set_size(const struct set *NONNULL set) {
  return ((size_t)1) << set->size_exponent;
}

static size_t set_index(const struct set *NONNULL set, size_t index) {
  return index & (set_size(set) - 1);
}

/* The states we have encountered. This collection will only ever grow while
 * checking the model. Note that we have a global reference-counted pointer and
 * a local bare pointer. See below for an explanation.
 */
static refcounted_ptr_t global_seen;
static _Thread_local struct set *local_seen;

/* Number of elements in the global set (i.e. occupancy). */
static size_t seen_count;

/* The "next" 'global_seen' value. See below for an explanation. */
static refcounted_ptr_t next_global_seen;

/* Now the explanation I teased... When the set capacity exceeds a threshold
 * (see 'set_expand' related logic below) it is expanded and the reference
 * tracking within the 'refcounted_ptr_t's comes in to play. We need to allocate
 * a new seen set ('next_global_seen'), copy over all the elements (done in
 * 'set_migrate'), and then "shift" the new set to become the current set. The
 * role of the reference counts of both 'global_seen' and 'next_global_seen' in
 * all of this is to detect when the last thread releases its reference to the
 * old seen set and hence can deallocate it.
 */

/* The next chunk to migrate from the old set to the new set. What exactly a
 * "chunk" is is covered in 'set_migrate'.
 */
static size_t next_migration;

/* A mechanism for synchronisation in 'set_expand'. */
static pthread_mutex_t set_expand_mutex;

static void set_expand_lock(void) {
  if (THREADS > 1) {
    int r __attribute__((unused)) = pthread_mutex_lock(&set_expand_mutex);
    ASSERT(r == 0);
  }
}

static void set_expand_unlock(void) {
  if (THREADS > 1) {
    int r __attribute__((unused)) = pthread_mutex_unlock(&set_expand_mutex);
    ASSERT(r == 0);
  }
}

static void set_init(void) {

  if (THREADS > 1) {
    int r = pthread_mutex_init(&set_expand_mutex, NULL);
    if (__builtin_expect(r < 0, 0)) {
      fprintf(stderr, "pthread_mutex_init failed: %s\n", strerror(r));
      exit(EXIT_FAILURE);
    }
  }

  /* Allocate the set we'll store seen states in at some conservative initial
   * size.
   */
  struct set *set = xmalloc(sizeof(*set));
  set->size_exponent = INITIAL_SET_SIZE_EXPONENT;
  set->bucket = xcalloc(set_size(set), sizeof(set->bucket[0]));

  /* Stash this somewhere for threads to later retrieve it from. Note that we
   * initialize its reference count to zero as we (the setup logic) are not
   * using it beyond this function.
   */
  refcounted_ptr_set(&global_seen, set);
}

static void set_thread_init(void) {
  /* Take a local reference to the global seen set. */
  local_seen = refcounted_ptr_get(&global_seen);
}

static void set_update(void) {
  /* Guard against the case where we've been called from exit_with() and we're
   * not finishing a migration, but just opting out of the rendezvous protocol.
   */
  if (refcounted_ptr_peek(&next_global_seen) != NULL) {

    /* Clean up the old set. Note that we are using a pointer we have already
     * given up our reference count to here, but we rely on the caller to ensure
     * this access is safe.
     */
    free(local_seen->bucket);
    free(local_seen);

    /* Reset migration state for the next time we expand the set. */
    next_migration = 0;

    /* Update the global pointer to the new set. We know all the above
     * migrations have completed and no one needs the old set.
     */
    refcounted_ptr_shift(&global_seen, &next_global_seen);
  }
}

static void set_migrate(void) {

  TRACE(TC_SET, "assisting in set migration...");

  /* Size of a migration chunk. Threads in this function grab a chunk at a time
   * to migrate.
   */
  enum { CHUNK_SIZE = 4096 / sizeof(local_seen->bucket[0]) /* slots */ };

  /* Take a pointer to the target set for the migration. */
  struct set *next = refcounted_ptr_get(&next_global_seen);

  for (;;) {

    size_t chunk = __atomic_fetch_add(&next_migration, 1, __ATOMIC_SEQ_CST);
    size_t start = chunk * CHUNK_SIZE;
    size_t end = start + CHUNK_SIZE;

    /* Bail out if we've finished migrating all of the set. */
    if (start >= set_size(local_seen)) {
      break;
    }

    /* TODO: The following algorithm assumes insertions can collide. That is, it
     * operates atomically on slots because another thread could be migrating
     * and also targeting the same slot. If we were to more closely wick to the
     * Maier design this would not be required.
     */

    for (size_t i = start; i < end; i++) {

      /* retrieve the slot element and mark it as migrated */
      slot_t s = __atomic_exchange_n(&local_seen->bucket[i], slot_tombstone(),
                                     __ATOMIC_SEQ_CST);
      ASSERT(!slot_is_tombstone(s) && "attempted double slot migration");

      /* If the current slot contained a state, rehash it and insert it into the
       * new set. Note we don't need to do any state comparisons because we know
       * everything in the old set is unique.
       */
      if (!slot_is_empty(s)) {
        size_t index = set_index(next, state_hash(slot_to_state(s)));
        /* insert and shuffle any colliding entries one along */
        for (size_t j = index; !slot_is_empty(s); j = set_index(next, j + 1)) {
          s = __atomic_exchange_n(&next->bucket[j], s, __ATOMIC_SEQ_CST);
        }
      }
    }
  }

  /* Release our reference to the old set now we're done with it. */
  refcounted_ptr_put(&global_seen, local_seen);

  /* Now we need to make sure all the threads get to this point before any one
   * thread leaves. The purpose of this is to guarantee we only ever have at
   * most two seen sets "in flight". Without this rendezvous, one thread could
   * race ahead, fill the new set, and then decide to expand again while some
   * are still working on the old set. It's possible to make such a scheme work
   * but the synchronisation requirements just seem too complicated.
   */
  rendezvous(set_update);

  /* We're now ready to resume model checking. Note that we already have a
   * (reference counted) pointer to the now-current global seen set, so we don't
   * need to take a fresh reference to it.
   */
  local_seen = next;
}

static void set_expand(void) {

  /* Using double-checked locking, we look to see if someone else has already
   * started expanding the set. We do this by first checking before acquiring
   * the set mutex, and then later again checking after we've acquired the
   * mutex. The idea here is that, with multiple threads, you'll frequently find
   * someone else beat you to expansion and you can jump straight to helping
   * them with migration without having the expense of acquiring the set mutex.
   */
  if (THREADS > 1 && refcounted_ptr_peek(&next_global_seen) != NULL) {
    /* Someone else already expanded it. Join them in the migration effort. */
    TRACE(TC_SET, "attempted expansion failed because another thread got there "
                  "first");
    set_migrate();
    return;
  }

  set_expand_lock();

  /* Check again, as described above. */
  if (THREADS > 1 && refcounted_ptr_peek(&next_global_seen) != NULL) {
    set_expand_unlock();
    TRACE(TC_SET, "attempted expansion failed because another thread got there "
                  "first");
    set_migrate();
    return;
  }

  TRACE(TC_SET, "expanding set from %zu slots to %zu slots...",
        (((size_t)1) << local_seen->size_exponent) / sizeof(slot_t),
        (((size_t)1) << (local_seen->size_exponent + 1)) / sizeof(slot_t));

  /* Create a set of double the size. */
  struct set *set = xmalloc(sizeof(*set));
  set->size_exponent = local_seen->size_exponent + 1;
  set->bucket = xcalloc(set_size(set), sizeof(set->bucket[0]));

  /* Advertise this as the newly expanded global set. */
  refcounted_ptr_set(&next_global_seen, set);

  /* We now need to migrate all slots from the old set to the new one, but we
   * can do this multithreaded.
   */
  set_expand_unlock();
  set_migrate();
}

static bool set_insert(struct state *NONNULL s, size_t *NONNULL count) {

restart:;

  if (__atomic_load_n(&seen_count, __ATOMIC_SEQ_CST) * 100 /
          set_size(local_seen) >=
      SET_EXPAND_THRESHOLD)
    set_expand();

  size_t index = set_index(local_seen, state_hash(s));

  size_t attempts = 0;
  for (size_t i = index; attempts < set_size(local_seen);
       i = set_index(local_seen, i + 1)) {

    /* Guess that the current slot is empty and try to insert here. */
    slot_t c = slot_empty();
    if (__atomic_compare_exchange_n(&local_seen->bucket[i], &c,
                                    state_to_slot(s), false, __ATOMIC_SEQ_CST,
                                    __ATOMIC_SEQ_CST)) {
      /* Success */
      *count = __atomic_add_fetch(&seen_count, 1, __ATOMIC_SEQ_CST);
      TRACE(TC_SET, "added state %p, set size is now %zu", s, *count);

      /* The maximum possible size of the seen state set should be constrained
       * by the number of possible states based on how many bits we are using to
       * represent the state data.
       */
      if (STATE_SIZE_BITS < sizeof(size_t) * CHAR_BIT) {
        assert(*count <= ((size_t)1) << STATE_SIZE_BITS &&
               "seen set size "
               "exceeds total possible number of states");
      }

      /* Update statistics if `--trace memory_usage` is in effect. Note that we
       * do this here (when a state is being added to the seen set) rather than
       * when the state was originally allocated to ensure that the final
       * allocation figures do not include transient states that we allocated
       * and then discarded as duplicates.
       */
      size_t depth = 0;
#if BOUND > 0
      depth = (size_t)state_bound_get(s);
#endif
      register_allocation(depth);

      return true;
    }

    if (slot_is_tombstone(c)) {
      /* This slot has been migrated. We need to rendezvous with other migrating
       * threads and restart our insertion attempt on the newly expanded set.
       */
      set_migrate();
      goto restart;
    }

    /* If we find this already in the set, we're done. */
    if (state_eq(s, slot_to_state(c))) {
      TRACE(TC_SET, "skipped adding state %p that was already in set", s);
      return false;
    }

    attempts++;
  }

  /* If we reach here, the set is full. Expand it and retry the insertion. */
  set_expand();
  return set_insert(s, count);
}

/* Find an existing element in the set.
 *
 * Why would you ever want to do this? If you already have the state, why do you
 * want to find a copy of it? The answer is for liveness information. When
 * checking liveness properties, a duplicate of your current state that is
 * already contained in the state set might know some of the liveness properties
 * are satisfied that your current state considers unknown.
 */
static __attribute__((unused)) const struct state *
set_find(const struct state *NONNULL s) {

  assert(s != NULL);

  size_t index = set_index(local_seen, state_hash(s));

  size_t attempts = 0;
  for (size_t i = index; attempts < set_size(local_seen);
       i = set_index(local_seen, i + 1)) {

    slot_t slot = __atomic_load_n(&local_seen->bucket[i], __ATOMIC_SEQ_CST);

    /* This function is only expected to be called during the final liveness
     * scan, in which all threads participate. So we should never encounter a
     * set undergoing migration.
     */
    ASSERT(!slot_is_tombstone(slot) &&
           "tombstone encountered during final phase");

    if (slot_is_empty(slot)) {
      /* reached the end of the linear block in which this state could lie */
      break;
    }

    const struct state *n = slot_to_state(slot);
    ASSERT(n != NULL && "null pointer stored in state set");

    if (state_eq(s, n)) {
      /* found */
      return n;
    }

    attempts++;
  }

  /* not found */
  return NULL;
}

/******************************************************************************/

static time_t START_TIME;

static unsigned long long gettime() {
  return (unsigned long long)(time(NULL) - START_TIME);
}

#if LIVENESS_COUNT > 0
/* Set one of the liveness bits (i.e. mark the matching property as 'hit') in a
 * state and all its predecessors.
 */
static __attribute__((unused)) void mark_liveness(struct state *NONNULL s,
                                                  size_t index, bool shared) {

  assert(s != NULL);
  ASSERT(index < sizeof(s->liveness) * CHAR_BIT &&
         "out of range liveness write");

  size_t word_index = index / (sizeof(s->liveness[0]) * CHAR_BIT);
  size_t bit_index = index % (sizeof(s->liveness[0]) * CHAR_BIT);

  uintptr_t previous_value;
  uintptr_t *target = &s->liveness[word_index];
  uintptr_t mask = ((uintptr_t)1) << bit_index;

  if (shared) {
    /* If this state is shared (accessible by other threads) we need to operate
     * on its liveness data atomically.
     */
    previous_value = __atomic_fetch_or(target, mask, __ATOMIC_SEQ_CST);
  } else {
    /* Otherwise we can use a cheaper ordinary OR. */
    previous_value = *target;
    *target |= mask;
  }

  /* Cheat a little and cast away the constness of the previous state for which
   * we may need to update liveness data.
   */
  struct state *previous = state_drop_const(state_previous_get(s));

  /* If the given bit was already set, we know all the predecessors of this
   * state have already had their corresponding bit marked. However, if it was
   * not we now need to recurse to mark them. Note that we assume any
   * predecessors of this state are globally visible and hence shared. The
   * recursion depth here can be indeterminately deep, but we assume the
   * compiler can tail-optimise this call.
   */
  if ((previous_value & mask) != mask && previous != NULL) {
    mark_liveness(previous, index, true);
  }
}

/* number of unknown liveness properties for a given state */
static unsigned long unknown_liveness(const struct state *NONNULL s) {

  assert(s != NULL);

  unsigned long unknown = 0;

  for (size_t i = 0; i < sizeof(s->liveness) / sizeof(s->liveness[0]); i++) {

    uintptr_t word = __atomic_load_n(&s->liveness[i], __ATOMIC_SEQ_CST);

    for (size_t j = 0; j < sizeof(s->liveness[0]) * CHAR_BIT; j++) {
      if (i * sizeof(s->liveness[0]) * CHAR_BIT + j >= LIVENESS_COUNT) {
        break;
      }

      if (!((word >> j) & 0x1)) {
        unknown++;
      }
    }
  }

  return unknown;
}

/** learn new liveness information about the state `s` from its successor
 *
 * Note that typically `state_previous_get(successor) != s` because `successor`
 * is actually one of the de-duped aliases of the original successor to `s`.
 *
 * \param s State to learn information about
 * \param successor Successor to s
 * \return Number of new liveness information facts learnt
 */
static unsigned long learn_liveness(struct state *NONNULL s,
                                    const struct state *NONNULL successor) {

  assert(s != NULL);
  assert(successor != NULL);

  unsigned long new_info = 0;

  for (size_t i = 0; i < sizeof(s->liveness) / sizeof(s->liveness[0]); i++) {

    uintptr_t word_src =
        __atomic_load_n(&successor->liveness[i], __ATOMIC_SEQ_CST);
    uintptr_t word_dst = __atomic_load_n(&s->liveness[i], __ATOMIC_SEQ_CST);

    for (size_t j = 0; j < sizeof(s->liveness[0]) * CHAR_BIT; j++) {
      if (i * sizeof(s->liveness[0]) * CHAR_BIT + j >= LIVENESS_COUNT) {
        break;
      }

      bool live_src = !!((word_src >> j) & 0x1);
      bool live_dst = !!((word_dst >> j) & 0x1);

      if (!live_dst && live_src) {
        mark_liveness(s, i * sizeof(s->liveness[0]) * CHAR_BIT + j, true);
        new_info++;
      }
    }
  }

  return new_info;
}
#endif

/* Prototypes for generated functions. */
static void init(void);
static _Noreturn void explore(void);
#if LIVENESS_COUNT > 0
static void check_liveness_final(void);
static unsigned long check_liveness_summarise(void);
#endif

static int exit_with(int status) {

  /* Opt out of the thread-wide rendezvous protocol. */
  refcounted_ptr_put(&global_seen, local_seen);
  rendezvous_opt_out(set_update);
  local_seen = NULL;

  /* Make fired rule count visible globally. */
  rules_fired[thread_id] = rules_fired_local;

  if (thread_id == 0) {
    /* We are the initial thread. Wait on the others before exiting. */
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-compare"
#pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif
    for (size_t i = 0; phase == RUN && i < sizeof(threads) / sizeof(threads[0]);
         i++) {
#ifdef __clang__
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
      void *ret;
      int r = pthread_join(threads[i], &ret);
      if (__builtin_expect(r != 0, 0)) {
        flockfile(stdout);
        fprintf(stderr, "failed to join thread: %s\n", strerror(r));
        funlockfile(stdout);
        continue;
      }
      status |= (int)(intptr_t)ret;
    }

    /* We're now single-threaded again. */

    /* Reacquire a pointer to the seen set. Note that this may not be the same
     * value as what we previously had in local_seen because the other threads
     * may have expanded and migrated the seen set in the meantime.
     */
    local_seen = refcounted_ptr_get(&global_seen);

    if (error_count == 0) {
      /* If we didn't see any other errors, print cover information. */
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-compare"
#pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif
      for (size_t i = 0; i < sizeof(covers) / sizeof(covers[0]); i++) {
#ifdef __clang__
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
        if (MACHINE_READABLE_OUTPUT) {
          put("<cover_result message=\"");
          xml_printf(COVER_MESSAGES[i]);
          put("\" count=\"");
          put_uint(covers[i]);
          put("\"/>\n");
        }
        if (covers[i] == 0) {
          if (!MACHINE_READABLE_OUTPUT) {
            put("\t");
            put(red());
            put(bold());
            put("cover \"");
            put(COVER_MESSAGES[i]);
            put("\" not hit");
            put(reset());
            put("\n");
          }
          error_count++;
          status = EXIT_FAILURE;
        } else if (!MACHINE_READABLE_OUTPUT) {
          put("\t");
          put(green());
          put(bold());
          put("cover \"");
          put(COVER_MESSAGES[i]);
          put("\" hit ");
          put_uint(covers[i]);
          put(" times");
          put(reset());
          put("\n");
        }
      }
    }

#if LIVENESS_COUNT > 0
    /* If we have liveness properties to assess and have seen no previous
     * errors, do a final check of them now.
     */
    if (error_count == 0) {
      check_liveness_final();

      unsigned long failed = check_liveness_summarise();
      if (failed > 0) {
        error_count += failed;
        status = EXIT_FAILURE;
      }
    }
#endif

    if (!MACHINE_READABLE_OUTPUT) {
      put("\n"
          "===================================================================="
          "======\n"
          "\n"
          "Status:\n"
          "\n");
      if (error_count == 0) {
        put("\t");
        put(green());
        put(bold());
        put("No error found.");
        put(reset());
        put("\n");
      } else {
        put("\t");
        put(red());
        put(bold());
        put_uint(error_count);
        put(" error(s) found.");
        put(reset());
        put("\n");
      }
      put("\n");
    }

    /* Calculate the total number of rules fired. */
    uintmax_t fire_count = 0;
    for (size_t i = 0; i < sizeof(rules_fired) / sizeof(rules_fired[0]); i++) {
      fire_count += rules_fired[i];
    }

    /* Paranoid check that we didn't miscount during set insertions/expansions.
     */
#ifndef NDEBUG
    size_t count = 0;
    for (size_t i = 0; i < set_size(local_seen); i++) {
      if (!slot_is_empty(local_seen->bucket[i])) {
        count++;
      }
    }
#endif
    assert(count == seen_count && "seen set count is inconsistent at exit");

    if (MACHINE_READABLE_OUTPUT) {
      put("<summary states=\"");
      put_uint(seen_count);
      put("\" rules_fired=\"");
      put_uint(fire_count);
      put("\" errors=\"");
      put_uint(error_count);
      put("\" duration_seconds=\"");
      put_uint(gettime());
      put("\"/>\n");
      put("</rumur_run>\n");
    } else {
      put("State Space Explored:\n"
          "\n"
          "\t");
      put_uint(seen_count);
      put(" states, ");
      put_uint(fire_count);
      put(" rules fired in ");
      put_uint(gettime());
      put("s.\n");
    }

    /* print memory usage statistics if `--trace memory_usage` is in effect */
    print_allocation_summary();

    exit(status);
  } else {
    pthread_exit((void *)(intptr_t)status);
  }
}

static void *thread_main(void *arg) {

  /* Initialize (thread-local) thread identifier. */
  thread_id = (size_t)(uintptr_t)arg;

  set_thread_init();

  explore();
}

static void start_secondary_threads(void) {

  /* XXX: Kind of hacky. We've left the rendezvous down-counter at 1 until now
   * in case we triggered a rendezvous before starting the other threads (it
   * *can* happen). We bump it immediately -- i.e. before starting *any* of the
   * secondary threads -- because any of them could race us and trigger a
   * rendezvous before we exit the below loop and they need to know about the
   * thundering herd bearing down on them. It is safe to do this without holding
   * rendezvous_lock because we are still single threaded at this point.
   */
  assert(running_count == 1);
  running_count = THREADS;
  assert(rendezvous_pending == 1);
  rendezvous_pending = THREADS;

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-compare"
#pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  for (size_t i = 0; i < sizeof(threads) / sizeof(threads[0]); i++) {
#ifdef __clang__
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
    int r = pthread_create(&threads[i], NULL, thread_main,
                           (void *)(uintptr_t)(i + 1));
    if (__builtin_expect(r != 0, 0)) {
      fprintf(stderr, "pthread_create failed: %s\n", strerror(r));
      exit(EXIT_FAILURE);
    }
  }
}

int main(void) {

  if (COLOR == AUTO)
    istty = isatty(STDOUT_FILENO) != 0;

  /* We don't need to read anything from stdin, so discard it. */
  (void)fclose(stdin);

  sandbox();

  if (MACHINE_READABLE_OUTPUT) {
    put("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
        "<rumur_run>\n"
        "<information state_size_bits=\"");
    put_uint(STATE_SIZE_BITS);
    put("\" state_size_bytes=\"");
    put_uint(STATE_SIZE_BYTES);
    put("\" hash_table_slots=\"");
    put_uint(((size_t)1) << INITIAL_SET_SIZE_EXPONENT);
    put("\"/>\n");
  } else {
    put("Memory usage:\n"
        "\n"
        "\t* The size of each state is ");
    put_uint(STATE_SIZE_BITS);
    put(" bits (rounded up to ");
    put_uint(STATE_SIZE_BYTES);
    put(" bytes).\n"
        "\t* The size of the hash table is ");
    put_uint(((size_t)1) << INITIAL_SET_SIZE_EXPONENT);
    put(" slots.\n"
        "\n");
  }

#ifndef NDEBUG
  state_print_field_offsets();
#endif

  START_TIME = time(NULL);

  rendezvous_init();

  set_init();

  set_thread_init();

  init();

  if (!MACHINE_READABLE_OUTPUT) {
    put("Progress Report:\n\n");
  }

  explore();
}

static void swap_NODE(struct state *s __attribute__((unused)), size_t x __attribute__((unused)), size_t y __attribute__((unused))) {
  if (((size_t)0ull) + x * ((size_t)4ull) != ((size_t)0ull) + y * ((size_t)4ull)) {
    raw_value_t a = handle_read_raw(s, state_handle(s, ((size_t)0ull) + x * ((size_t)4ull), 2ull));
    raw_value_t b = handle_read_raw(s, state_handle(s, ((size_t)0ull) + y * ((size_t)4ull), 2ull));
    handle_write_raw(s, state_handle(s, ((size_t)0ull) + y * ((size_t)4ull), 2ull), a);
    handle_write_raw(s, state_handle(s, ((size_t)0ull) + x * ((size_t)4ull), 2ull), b);
  }
  if (((size_t)0ull) + x * ((size_t)4ull) + ((size_t)2ull) != ((size_t)0ull) + y * ((size_t)4ull) + ((size_t)2ull)) {
    raw_value_t a = handle_read_raw(s, state_handle(s, ((size_t)0ull) + x * ((size_t)4ull) + ((size_t)2ull), 2ull));
    raw_value_t b = handle_read_raw(s, state_handle(s, ((size_t)0ull) + y * ((size_t)4ull) + ((size_t)2ull), 2ull));
    handle_write_raw(s, state_handle(s, ((size_t)0ull) + y * ((size_t)4ull) + ((size_t)2ull), 2ull), a);
    handle_write_raw(s, state_handle(s, ((size_t)0ull) + x * ((size_t)4ull) + ((size_t)2ull), 2ull), b);
  }
  for (size_t i0 = 0; i0 < ((size_t)2ull); i0++) {
  }
  if (((size_t)24ull) + x * ((size_t)5ull) != ((size_t)24ull) + y * ((size_t)5ull)) {
    raw_value_t a = handle_read_raw(s, state_handle(s, ((size_t)24ull) + x * ((size_t)5ull), 2ull));
    raw_value_t b = handle_read_raw(s, state_handle(s, ((size_t)24ull) + y * ((size_t)5ull), 2ull));
    handle_write_raw(s, state_handle(s, ((size_t)24ull) + y * ((size_t)5ull), 2ull), a);
    handle_write_raw(s, state_handle(s, ((size_t)24ull) + x * ((size_t)5ull), 2ull), b);
  }
  if (((size_t)24ull) + x * ((size_t)5ull) + ((size_t)2ull) != ((size_t)24ull) + y * ((size_t)5ull) + ((size_t)2ull)) {
    raw_value_t a = handle_read_raw(s, state_handle(s, ((size_t)24ull) + x * ((size_t)5ull) + ((size_t)2ull), 3ull));
    raw_value_t b = handle_read_raw(s, state_handle(s, ((size_t)24ull) + y * ((size_t)5ull) + ((size_t)2ull), 3ull));
    handle_write_raw(s, state_handle(s, ((size_t)24ull) + y * ((size_t)5ull) + ((size_t)2ull), 3ull), a);
    handle_write_raw(s, state_handle(s, ((size_t)24ull) + x * ((size_t)5ull) + ((size_t)2ull), 3ull), b);
  }
  for (size_t i0 = 0; i0 < ((size_t)2ull); i0++) {
  }
  if (((size_t)34ull) + x * ((size_t)5ull) != ((size_t)34ull) + y * ((size_t)5ull)) {
    raw_value_t a = handle_read_raw(s, state_handle(s, ((size_t)34ull) + x * ((size_t)5ull), 2ull));
    raw_value_t b = handle_read_raw(s, state_handle(s, ((size_t)34ull) + y * ((size_t)5ull), 2ull));
    handle_write_raw(s, state_handle(s, ((size_t)34ull) + y * ((size_t)5ull), 2ull), a);
    handle_write_raw(s, state_handle(s, ((size_t)34ull) + x * ((size_t)5ull), 2ull), b);
  }
  if (((size_t)34ull) + x * ((size_t)5ull) + ((size_t)2ull) != ((size_t)34ull) + y * ((size_t)5ull) + ((size_t)2ull)) {
    raw_value_t a = handle_read_raw(s, state_handle(s, ((size_t)34ull) + x * ((size_t)5ull) + ((size_t)2ull), 3ull));
    raw_value_t b = handle_read_raw(s, state_handle(s, ((size_t)34ull) + y * ((size_t)5ull) + ((size_t)2ull), 3ull));
    handle_write_raw(s, state_handle(s, ((size_t)34ull) + y * ((size_t)5ull) + ((size_t)2ull), 3ull), a);
    handle_write_raw(s, state_handle(s, ((size_t)34ull) + x * ((size_t)5ull) + ((size_t)2ull), 3ull), b);
  }
  for (size_t i0 = 0; i0 < ((size_t)2ull); i0++) {
  }
  if (((size_t)44ull) + x * ((size_t)5ull) != ((size_t)44ull) + y * ((size_t)5ull)) {
    raw_value_t a = handle_read_raw(s, state_handle(s, ((size_t)44ull) + x * ((size_t)5ull), 2ull));
    raw_value_t b = handle_read_raw(s, state_handle(s, ((size_t)44ull) + y * ((size_t)5ull), 2ull));
    handle_write_raw(s, state_handle(s, ((size_t)44ull) + y * ((size_t)5ull), 2ull), a);
    handle_write_raw(s, state_handle(s, ((size_t)44ull) + x * ((size_t)5ull), 2ull), b);
  }
  if (((size_t)44ull) + x * ((size_t)5ull) + ((size_t)2ull) != ((size_t)44ull) + y * ((size_t)5ull) + ((size_t)2ull)) {
    raw_value_t a = handle_read_raw(s, state_handle(s, ((size_t)44ull) + x * ((size_t)5ull) + ((size_t)2ull), 3ull));
    raw_value_t b = handle_read_raw(s, state_handle(s, ((size_t)44ull) + y * ((size_t)5ull) + ((size_t)2ull), 3ull));
    handle_write_raw(s, state_handle(s, ((size_t)44ull) + y * ((size_t)5ull) + ((size_t)2ull), 3ull), a);
    handle_write_raw(s, state_handle(s, ((size_t)44ull) + x * ((size_t)5ull) + ((size_t)2ull), 3ull), b);
  }
  for (size_t i0 = 0; i0 < ((size_t)2ull); i0++) {
  }
  if (((size_t)8ull) + x * ((size_t)2ull) != ((size_t)8ull) + y * ((size_t)2ull)) {
    raw_value_t a = handle_read_raw(s, state_handle(s, ((size_t)8ull) + x * ((size_t)2ull), 2ull));
    raw_value_t b = handle_read_raw(s, state_handle(s, ((size_t)8ull) + y * ((size_t)2ull), 2ull));
    handle_write_raw(s, state_handle(s, ((size_t)8ull) + y * ((size_t)2ull), 2ull), a);
    handle_write_raw(s, state_handle(s, ((size_t)8ull) + x * ((size_t)2ull), 2ull), b);
  }
  for (size_t i0 = 0; i0 < ((size_t)2ull); i0++) {
  }
  if (((size_t)12ull) + x * ((size_t)2ull) != ((size_t)12ull) + y * ((size_t)2ull)) {
    raw_value_t a = handle_read_raw(s, state_handle(s, ((size_t)12ull) + x * ((size_t)2ull), 2ull));
    raw_value_t b = handle_read_raw(s, state_handle(s, ((size_t)12ull) + y * ((size_t)2ull), 2ull));
    handle_write_raw(s, state_handle(s, ((size_t)12ull) + y * ((size_t)2ull), 2ull), a);
    handle_write_raw(s, state_handle(s, ((size_t)12ull) + x * ((size_t)2ull), 2ull), b);
  }
  for (size_t i0 = 0; i0 < ((size_t)2ull); i0++) {
  }
  if (x != y) {
    raw_value_t v = handle_read_raw(s, state_handle(s, ((size_t)18ull), ((size_t)2ull)));
    if (v != 0) {
      if (v - 1 == (raw_value_t)x) {
        handle_write_raw(s, state_handle(s, ((size_t)18ull), ((size_t)2ull)), y + 1);
      } else if (v - 1 == (raw_value_t)y) {
        handle_write_raw(s, state_handle(s, ((size_t)18ull), ((size_t)2ull)), x + 1);
      }
    }
  }
}

static void swap_DATA(struct state *s __attribute__((unused)), size_t x __attribute__((unused)), size_t y __attribute__((unused))) {
  for (size_t i0 = 0; i0 < ((size_t)2ull); i0++) {
    if (x != y) {
      raw_value_t v = handle_read_raw(s, state_handle(s, ((size_t)0ull) + i0 * ((size_t)4ull) + ((size_t)2ull), ((size_t)2ull)));
      if (v != 0) {
        if (v - 1 == (raw_value_t)x) {
          handle_write_raw(s, state_handle(s, ((size_t)0ull) + i0 * ((size_t)4ull) + ((size_t)2ull), ((size_t)2ull)), y + 1);
        } else if (v - 1 == (raw_value_t)y) {
          handle_write_raw(s, state_handle(s, ((size_t)0ull) + i0 * ((size_t)4ull) + ((size_t)2ull), ((size_t)2ull)), x + 1);
        }
      }
    }
  }
  for (size_t i0 = 0; i0 < ((size_t)2ull); i0++) {
    if (x != y) {
      raw_value_t v = handle_read_raw(s, state_handle(s, ((size_t)24ull) + i0 * ((size_t)5ull), ((size_t)2ull)));
      if (v != 0) {
        if (v - 1 == (raw_value_t)x) {
          handle_write_raw(s, state_handle(s, ((size_t)24ull) + i0 * ((size_t)5ull), ((size_t)2ull)), y + 1);
        } else if (v - 1 == (raw_value_t)y) {
          handle_write_raw(s, state_handle(s, ((size_t)24ull) + i0 * ((size_t)5ull), ((size_t)2ull)), x + 1);
        }
      }
    }
  }
  for (size_t i0 = 0; i0 < ((size_t)2ull); i0++) {
    if (x != y) {
      raw_value_t v = handle_read_raw(s, state_handle(s, ((size_t)34ull) + i0 * ((size_t)5ull), ((size_t)2ull)));
      if (v != 0) {
        if (v - 1 == (raw_value_t)x) {
          handle_write_raw(s, state_handle(s, ((size_t)34ull) + i0 * ((size_t)5ull), ((size_t)2ull)), y + 1);
        } else if (v - 1 == (raw_value_t)y) {
          handle_write_raw(s, state_handle(s, ((size_t)34ull) + i0 * ((size_t)5ull), ((size_t)2ull)), x + 1);
        }
      }
    }
  }
  for (size_t i0 = 0; i0 < ((size_t)2ull); i0++) {
    if (x != y) {
      raw_value_t v = handle_read_raw(s, state_handle(s, ((size_t)44ull) + i0 * ((size_t)5ull), ((size_t)2ull)));
      if (v != 0) {
        if (v - 1 == (raw_value_t)x) {
          handle_write_raw(s, state_handle(s, ((size_t)44ull) + i0 * ((size_t)5ull), ((size_t)2ull)), y + 1);
        } else if (v - 1 == (raw_value_t)y) {
          handle_write_raw(s, state_handle(s, ((size_t)44ull) + i0 * ((size_t)5ull), ((size_t)2ull)), x + 1);
        }
      }
    }
  }
  for (size_t i0 = 0; i0 < ((size_t)2ull); i0++) {
  }
  for (size_t i0 = 0; i0 < ((size_t)2ull); i0++) {
  }
  if (x != y) {
    raw_value_t v = handle_read_raw(s, state_handle(s, ((size_t)20ull), ((size_t)2ull)));
    if (v != 0) {
      if (v - 1 == (raw_value_t)x) {
        handle_write_raw(s, state_handle(s, ((size_t)20ull), ((size_t)2ull)), y + 1);
      } else if (v - 1 == (raw_value_t)y) {
        handle_write_raw(s, state_handle(s, ((size_t)20ull), ((size_t)2ull)), x + 1);
      }
    }
  }
  if (x != y) {
    raw_value_t v = handle_read_raw(s, state_handle(s, ((size_t)22ull), ((size_t)2ull)));
    if (v != 0) {
      if (v - 1 == (raw_value_t)x) {
        handle_write_raw(s, state_handle(s, ((size_t)22ull), ((size_t)2ull)), y + 1);
      } else if (v - 1 == (raw_value_t)y) {
        handle_write_raw(s, state_handle(s, ((size_t)22ull), ((size_t)2ull)), x + 1);
      }
    }
  }
}

static size_t schedule_read_NODE(const struct state *NONNULL s) {
  assert(s != NULL);
  return state_schedule_get(s, 0ul, 1ul);
}
static void schedule_write_NODE(struct state *NONNULL s, size_t schedule_index) {
  assert(s != NULL);
  state_schedule_set(s, 0ul, 1ul, schedule_index);
}
static size_t schedule_read_DATA(const struct state *NONNULL s) {
  assert(s != NULL);
  return state_schedule_get(s, 1ul, 1ul);
}
static void schedule_write_DATA(struct state *NONNULL s, size_t schedule_index) {
  assert(s != NULL);
  state_schedule_set(s, 1ul, 1ul, schedule_index);
}
static void state_canonicalise_exhaustive(struct state *s __attribute__((unused))) {

  assert(s != NULL && "attempt to canonicalise NULL state");

  /* A state to store the current permutation we are considering. */
  static _Thread_local struct state candidate;
  memcpy(&candidate, s, sizeof(candidate));

  if (state_cmp(&candidate, s) < 0) {
    /* Found a more canonical representation. */
    memcpy(s, &candidate, sizeof(*s));
  }

  {
    size_t stack_DATA[((size_t)2ull)] = { 0 };

    size_t schedule_DATA[((size_t)2ull)] = { 0 };
    if (USE_SCALARSET_SCHEDULES) {
      size_t stack[((size_t)2ull)];
      size_t index = schedule_read_DATA(&candidate);
      index_to_permutation(index, schedule_DATA, stack, ((size_t)2ull));
    }

    for (size_t i1 = 0; i1 < ((size_t)2ull); ) {
      if (stack_DATA[i1] < i1) {
        if (i1 % 2 == 0) {
          swap_DATA(&candidate, 0, i1);
          size_t tmp = schedule_DATA[0];
          schedule_DATA[0] = schedule_DATA[i1];
          schedule_DATA[i1] = tmp;
        } else {
          swap_DATA(&candidate, stack_DATA[i1], i1);
          size_t tmp = schedule_DATA[stack_DATA[i1]];
          schedule_DATA[stack_DATA[i1]] = schedule_DATA[i1];
          schedule_DATA[i1] = tmp;
        }
        /* save selected schedule to map this back for later more
         * comprehensible counterexample traces
         */
        if (USE_SCALARSET_SCHEDULES) {
          size_t stack[((size_t)2ull)];
          size_t working[((size_t)2ull)];
          size_t index = permutation_to_index(schedule_DATA, stack, working, ((size_t)2ull));
          schedule_write_DATA(&candidate, index);
        }
        if (state_cmp(&candidate, s) < 0) {
          /* Found a more canonical representation. */
          memcpy(s, &candidate, sizeof(*s));
        }

        stack_DATA[i1]++;
        i1 = 0;
      } else {
        stack_DATA[i1] = 0;
        i1++;
      }
    }
  }
  if (state_cmp(&candidate, s) < 0) {
    /* Found a more canonical representation. */
    memcpy(s, &candidate, sizeof(*s));
  }

  {
    size_t stack_NODE[((size_t)2ull)] = { 0 };

    size_t schedule_NODE[((size_t)2ull)] = { 0 };
    if (USE_SCALARSET_SCHEDULES) {
      size_t stack[((size_t)2ull)];
      size_t index = schedule_read_NODE(&candidate);
      index_to_permutation(index, schedule_NODE, stack, ((size_t)2ull));
    }

    for (size_t i0 = 0; i0 < ((size_t)2ull); ) {
      if (stack_NODE[i0] < i0) {
        if (i0 % 2 == 0) {
          swap_NODE(&candidate, 0, i0);
          size_t tmp = schedule_NODE[0];
          schedule_NODE[0] = schedule_NODE[i0];
          schedule_NODE[i0] = tmp;
        } else {
          swap_NODE(&candidate, stack_NODE[i0], i0);
          size_t tmp = schedule_NODE[stack_NODE[i0]];
          schedule_NODE[stack_NODE[i0]] = schedule_NODE[i0];
          schedule_NODE[i0] = tmp;
        }
        /* save selected schedule to map this back for later more
         * comprehensible counterexample traces
         */
        if (USE_SCALARSET_SCHEDULES) {
          size_t stack[((size_t)2ull)];
          size_t working[((size_t)2ull)];
          size_t index = permutation_to_index(schedule_NODE, stack, working, ((size_t)2ull));
          schedule_write_NODE(&candidate, index);
        }
        if (state_cmp(&candidate, s) < 0) {
          /* Found a more canonical representation. */
          memcpy(s, &candidate, sizeof(*s));
        }

        {
          size_t stack_DATA[((size_t)2ull)] = { 0 };

          size_t schedule_DATA[((size_t)2ull)] = { 0 };
          if (USE_SCALARSET_SCHEDULES) {
            size_t stack[((size_t)2ull)];
            size_t index = schedule_read_DATA(&candidate);
            index_to_permutation(index, schedule_DATA, stack, ((size_t)2ull));
          }

          for (size_t i1 = 0; i1 < ((size_t)2ull); ) {
            if (stack_DATA[i1] < i1) {
              if (i1 % 2 == 0) {
                swap_DATA(&candidate, 0, i1);
                size_t tmp = schedule_DATA[0];
                schedule_DATA[0] = schedule_DATA[i1];
                schedule_DATA[i1] = tmp;
              } else {
                swap_DATA(&candidate, stack_DATA[i1], i1);
                size_t tmp = schedule_DATA[stack_DATA[i1]];
                schedule_DATA[stack_DATA[i1]] = schedule_DATA[i1];
                schedule_DATA[i1] = tmp;
              }
              /* save selected schedule to map this back for later more
               * comprehensible counterexample traces
               */
              if (USE_SCALARSET_SCHEDULES) {
                size_t stack[((size_t)2ull)];
                size_t working[((size_t)2ull)];
                size_t index = permutation_to_index(schedule_DATA, stack, working, ((size_t)2ull));
                schedule_write_DATA(&candidate, index);
              }
              if (state_cmp(&candidate, s) < 0) {
                /* Found a more canonical representation. */
                memcpy(s, &candidate, sizeof(*s));
              }

              stack_DATA[i1]++;
              i1 = 0;
            } else {
              stack_DATA[i1] = 0;
              i1++;
            }
          }
        }
        stack_NODE[i0]++;
        i0 = 0;
      } else {
        stack_NODE[i0] = 0;
        i0++;
      }
    }
  }
}

static int compare_NODE(const struct state *s, size_t x, size_t y) {

  if (x == y) {
    return 0;
  }

  if (((size_t)0ull) + x * ((size_t)4ull) != ((size_t)0ull) + y * ((size_t)4ull)) {
    raw_value_t a = handle_read_raw(s, state_handle(s, ((size_t)0ull) + x * ((size_t)4ull), 2ull));
    raw_value_t b = handle_read_raw(s, state_handle(s, ((size_t)0ull) + y * ((size_t)4ull), 2ull));
    if (a < b) {
      return -1;
    } else if (a > b) {
      return 1;
    }
  }
  if (((size_t)0ull) + x * ((size_t)4ull) + ((size_t)2ull) != ((size_t)0ull) + y * ((size_t)4ull) + ((size_t)2ull)) {
    raw_value_t a = handle_read_raw(s, state_handle(s, ((size_t)0ull) + x * ((size_t)4ull) + ((size_t)2ull), 2ull));
    raw_value_t b = handle_read_raw(s, state_handle(s, ((size_t)0ull) + y * ((size_t)4ull) + ((size_t)2ull), 2ull));
    if (a < b) {
      return -1;
    } else if (a > b) {
      return 1;
    }
  }
  for (size_t i0 = 0; i0 < ((size_t)2ull); i0++) {
  }
  if (((size_t)24ull) + x * ((size_t)5ull) != ((size_t)24ull) + y * ((size_t)5ull)) {
    raw_value_t a = handle_read_raw(s, state_handle(s, ((size_t)24ull) + x * ((size_t)5ull), 2ull));
    raw_value_t b = handle_read_raw(s, state_handle(s, ((size_t)24ull) + y * ((size_t)5ull), 2ull));
    if (a < b) {
      return -1;
    } else if (a > b) {
      return 1;
    }
  }
  if (((size_t)24ull) + x * ((size_t)5ull) + ((size_t)2ull) != ((size_t)24ull) + y * ((size_t)5ull) + ((size_t)2ull)) {
    raw_value_t a = handle_read_raw(s, state_handle(s, ((size_t)24ull) + x * ((size_t)5ull) + ((size_t)2ull), 3ull));
    raw_value_t b = handle_read_raw(s, state_handle(s, ((size_t)24ull) + y * ((size_t)5ull) + ((size_t)2ull), 3ull));
    if (a < b) {
      return -1;
    } else if (a > b) {
      return 1;
    }
  }
  for (size_t i0 = 0; i0 < ((size_t)2ull); i0++) {
  }
  if (((size_t)34ull) + x * ((size_t)5ull) != ((size_t)34ull) + y * ((size_t)5ull)) {
    raw_value_t a = handle_read_raw(s, state_handle(s, ((size_t)34ull) + x * ((size_t)5ull), 2ull));
    raw_value_t b = handle_read_raw(s, state_handle(s, ((size_t)34ull) + y * ((size_t)5ull), 2ull));
    if (a < b) {
      return -1;
    } else if (a > b) {
      return 1;
    }
  }
  if (((size_t)34ull) + x * ((size_t)5ull) + ((size_t)2ull) != ((size_t)34ull) + y * ((size_t)5ull) + ((size_t)2ull)) {
    raw_value_t a = handle_read_raw(s, state_handle(s, ((size_t)34ull) + x * ((size_t)5ull) + ((size_t)2ull), 3ull));
    raw_value_t b = handle_read_raw(s, state_handle(s, ((size_t)34ull) + y * ((size_t)5ull) + ((size_t)2ull), 3ull));
    if (a < b) {
      return -1;
    } else if (a > b) {
      return 1;
    }
  }
  for (size_t i0 = 0; i0 < ((size_t)2ull); i0++) {
  }
  if (((size_t)44ull) + x * ((size_t)5ull) != ((size_t)44ull) + y * ((size_t)5ull)) {
    raw_value_t a = handle_read_raw(s, state_handle(s, ((size_t)44ull) + x * ((size_t)5ull), 2ull));
    raw_value_t b = handle_read_raw(s, state_handle(s, ((size_t)44ull) + y * ((size_t)5ull), 2ull));
    if (a < b) {
      return -1;
    } else if (a > b) {
      return 1;
    }
  }
  if (((size_t)44ull) + x * ((size_t)5ull) + ((size_t)2ull) != ((size_t)44ull) + y * ((size_t)5ull) + ((size_t)2ull)) {
    raw_value_t a = handle_read_raw(s, state_handle(s, ((size_t)44ull) + x * ((size_t)5ull) + ((size_t)2ull), 3ull));
    raw_value_t b = handle_read_raw(s, state_handle(s, ((size_t)44ull) + y * ((size_t)5ull) + ((size_t)2ull), 3ull));
    if (a < b) {
      return -1;
    } else if (a > b) {
      return 1;
    }
  }
  for (size_t i0 = 0; i0 < ((size_t)2ull); i0++) {
  }
  if (((size_t)8ull) + x * ((size_t)2ull) != ((size_t)8ull) + y * ((size_t)2ull)) {
    raw_value_t a = handle_read_raw(s, state_handle(s, ((size_t)8ull) + x * ((size_t)2ull), 2ull));
    raw_value_t b = handle_read_raw(s, state_handle(s, ((size_t)8ull) + y * ((size_t)2ull), 2ull));
    if (a < b) {
      return -1;
    } else if (a > b) {
      return 1;
    }
  }
  for (size_t i0 = 0; i0 < ((size_t)2ull); i0++) {
  }
  if (((size_t)12ull) + x * ((size_t)2ull) != ((size_t)12ull) + y * ((size_t)2ull)) {
    raw_value_t a = handle_read_raw(s, state_handle(s, ((size_t)12ull) + x * ((size_t)2ull), 2ull));
    raw_value_t b = handle_read_raw(s, state_handle(s, ((size_t)12ull) + y * ((size_t)2ull), 2ull));
    if (a < b) {
      return -1;
    } else if (a > b) {
      return 1;
    }
  }
  for (size_t i0 = 0; i0 < ((size_t)2ull); i0++) {
  }
  {
    raw_value_t v = handle_read_raw(s, state_handle(s, ((size_t)18ull), ((size_t)2ull)));
    if (v != 0) { /* ignored 'undefined' */
      if (v - 1 == (raw_value_t)x) {
        return -1;
      } else if (v - 1 == (raw_value_t)y) {
        return 1;
      }
    }
  }
  return 0;
}
static void sort_NODE(struct state *s, size_t *schedule, size_t lower, size_t upper) {

  /* If we have nothing to sort, bail out. */
  if (lower >= upper) {
    return;
  }

  /* Use Hoare's partitioning algorithm to apply quicksort. */
  size_t pivot = lower;
  size_t i = lower - 1;
  size_t j = upper + 1;

  for (;;) {

    do {
      i++;
      assert(i >= lower && i <= upper && "out of bounds access in sort_NODE()");
    } while (compare_NODE(s, i, pivot) < 0);

    do {
      j--;
      assert(j >= lower && j <= upper && "out of bounds access in sort_NODE()");
    } while (compare_NODE(s, j, pivot) > 0);

    if (i >= j) {
      break;
    }

    /* Swap elements i and j. */
    swap_NODE(s, i, j);
    {
      size_t tmp = schedule[i];
      schedule[i] = schedule[j]; 
      schedule[j] = tmp;
    }
    if (i == pivot) {
      pivot = j;
    } else if (j == pivot) {
      pivot = i;
    }
  }

  sort_NODE(s, schedule, lower, j);
  sort_NODE(s, schedule, j + 1, upper);
}
static int compare_DATA(const struct state *s, size_t x, size_t y) {

  if (x == y) {
    return 0;
  }

  for (size_t i0 = 0; i0 < ((size_t)2ull); i0++) {
    {
      raw_value_t v = handle_read_raw(s, state_handle(s, ((size_t)0ull) + i0 * ((size_t)4ull) + ((size_t)2ull), ((size_t)2ull)));
      if (v != 0) { /* ignored 'undefined' */
        if (v - 1 == (raw_value_t)x) {
          return -1;
        } else if (v - 1 == (raw_value_t)y) {
          return 1;
        }
      }
    }
  }
  for (size_t i0 = 0; i0 < ((size_t)2ull); i0++) {
    {
      raw_value_t v = handle_read_raw(s, state_handle(s, ((size_t)24ull) + i0 * ((size_t)5ull), ((size_t)2ull)));
      if (v != 0) { /* ignored 'undefined' */
        if (v - 1 == (raw_value_t)x) {
          return -1;
        } else if (v - 1 == (raw_value_t)y) {
          return 1;
        }
      }
    }
  }
  for (size_t i0 = 0; i0 < ((size_t)2ull); i0++) {
    {
      raw_value_t v = handle_read_raw(s, state_handle(s, ((size_t)34ull) + i0 * ((size_t)5ull), ((size_t)2ull)));
      if (v != 0) { /* ignored 'undefined' */
        if (v - 1 == (raw_value_t)x) {
          return -1;
        } else if (v - 1 == (raw_value_t)y) {
          return 1;
        }
      }
    }
  }
  for (size_t i0 = 0; i0 < ((size_t)2ull); i0++) {
    {
      raw_value_t v = handle_read_raw(s, state_handle(s, ((size_t)44ull) + i0 * ((size_t)5ull), ((size_t)2ull)));
      if (v != 0) { /* ignored 'undefined' */
        if (v - 1 == (raw_value_t)x) {
          return -1;
        } else if (v - 1 == (raw_value_t)y) {
          return 1;
        }
      }
    }
  }
  for (size_t i0 = 0; i0 < ((size_t)2ull); i0++) {
  }
  for (size_t i0 = 0; i0 < ((size_t)2ull); i0++) {
  }
  {
    raw_value_t v = handle_read_raw(s, state_handle(s, ((size_t)20ull), ((size_t)2ull)));
    if (v != 0) { /* ignored 'undefined' */
      if (v - 1 == (raw_value_t)x) {
        return -1;
      } else if (v - 1 == (raw_value_t)y) {
        return 1;
      }
    }
  }
  {
    raw_value_t v = handle_read_raw(s, state_handle(s, ((size_t)22ull), ((size_t)2ull)));
    if (v != 0) { /* ignored 'undefined' */
      if (v - 1 == (raw_value_t)x) {
        return -1;
      } else if (v - 1 == (raw_value_t)y) {
        return 1;
      }
    }
  }
  return 0;
}
static void sort_DATA(struct state *s, size_t *schedule, size_t lower, size_t upper) {

  /* If we have nothing to sort, bail out. */
  if (lower >= upper) {
    return;
  }

  /* Use Hoare's partitioning algorithm to apply quicksort. */
  size_t pivot = lower;
  size_t i = lower - 1;
  size_t j = upper + 1;

  for (;;) {

    do {
      i++;
      assert(i >= lower && i <= upper && "out of bounds access in sort_DATA()");
    } while (compare_DATA(s, i, pivot) < 0);

    do {
      j--;
      assert(j >= lower && j <= upper && "out of bounds access in sort_DATA()");
    } while (compare_DATA(s, j, pivot) > 0);

    if (i >= j) {
      break;
    }

    /* Swap elements i and j. */
    swap_DATA(s, i, j);
    {
      size_t tmp = schedule[i];
      schedule[i] = schedule[j]; 
      schedule[j] = tmp;
    }
    if (i == pivot) {
      pivot = j;
    } else if (j == pivot) {
      pivot = i;
    }
  }

  sort_DATA(s, schedule, lower, j);
  sort_DATA(s, schedule, j + 1, upper);
}
static void state_canonicalise_heuristic(struct state *s __attribute__((unused))) {

  assert(s != NULL && "attempt to canonicalise NULL state");

  {
    size_t schedule[(size_t)2ull];
    for (size_t i = 0; i < sizeof(schedule) / sizeof(schedule[0]); ++i) {
      schedule[i] = i;
    }
    if (USE_SCALARSET_SCHEDULES) {
      size_t index = schedule_read_NODE(s);
      size_t stack[(size_t)2ull];
      index_to_permutation(index, schedule, stack, 2ull);
    }
    sort_NODE(s, schedule, 0, ((size_t)2ull) - 1);
    /* save selected schedule to map this back for later more
     * comprehensible counterexample traces
     */
    if (USE_SCALARSET_SCHEDULES) {
      size_t stack[(size_t)2ull];
      size_t working[(size_t)2ull];
      size_t index = permutation_to_index(schedule, stack, working, 2ull);
      schedule_write_NODE(s, index);
    }
  }
  {
    size_t schedule[(size_t)2ull];
    for (size_t i = 0; i < sizeof(schedule) / sizeof(schedule[0]); ++i) {
      schedule[i] = i;
    }
    if (USE_SCALARSET_SCHEDULES) {
      size_t index = schedule_read_DATA(s);
      size_t stack[(size_t)2ull];
      index_to_permutation(index, schedule, stack, 2ull);
    }
    sort_DATA(s, schedule, 0, ((size_t)2ull) - 1);
    /* save selected schedule to map this back for later more
     * comprehensible counterexample traces
     */
    if (USE_SCALARSET_SCHEDULES) {
      size_t stack[(size_t)2ull];
      size_t working[(size_t)2ull];
      size_t index = permutation_to_index(schedule, stack, working, 2ull);
      schedule_write_DATA(s, index);
    }
  }
}



static const value_t ru_NODE_NUM __attribute__((unused)) = VALUE_C(2);

static const value_t ru_DATA_NUM __attribute__((unused)) = VALUE_C(2);

static bool startstate0(struct state *NONNULL s, struct handle ru_d) {
  static const char *rule_name __attribute__((unused)) = "startstate \"init\"";
  if (JMP_BUF_NEEDED) {
    if (sigsetjmp(checkpoint, 0)) {
      /* error triggered during this startstate */
      return false;
    }
  }
  const struct handle ru_Cache __attribute__((unused)) = state_handle(s, 0ull, 8ull);
  const struct handle ru_Chan1 __attribute__((unused)) = state_handle(s, 24ull, 10ull);
  const struct handle ru_Chan2 __attribute__((unused)) = state_handle(s, 34ull, 10ull);
  const struct handle ru_Chan3 __attribute__((unused)) = state_handle(s, 44ull, 10ull);
  const struct handle ru_InvSet __attribute__((unused)) = state_handle(s, 8ull, 4ull);
  const struct handle ru_ShrSet __attribute__((unused)) = state_handle(s, 12ull, 4ull);
  const struct handle ru_ExGntd __attribute__((unused)) = state_handle(s, 16ull, 2ull);
  const struct handle ru_CurCmd __attribute__((unused)) = state_handle(s, 54ull, 3ull);
  const struct handle ru_CurPtr __attribute__((unused)) = state_handle(s, 18ull, 2ull);
  const struct handle ru_MemData __attribute__((unused)) = state_handle(s, 20ull, 2ull);
  const struct handle ru_AuxData __attribute__((unused)) = state_handle(s, 22ull, 2ull);
  {
    {
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
  handle_write("german.m:41.5-25: ", rule_name, "Chan1[i].Cmd", s, VALUE_C(0), VALUE_C(6), handle_narrow(handle_index("german.m:41.5-12: ", rule_name, "Chan1[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan1, handle_read("german.m:41.11: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 3), VALUE_C(0));
  handle_write("german.m:41.28-48: ", rule_name, "Chan2[i].Cmd", s, VALUE_C(0), VALUE_C(6), handle_narrow(handle_index("german.m:41.28-35: ", rule_name, "Chan2[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan2, handle_read("german.m:41.34: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 3), VALUE_C(0));
  handle_write("german.m:41.51-71: ", rule_name, "Chan3[i].Cmd", s, VALUE_C(0), VALUE_C(6), handle_narrow(handle_index("german.m:41.51-58: ", rule_name, "Chan3[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan3, handle_read("german.m:41.57: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 3), VALUE_C(0));
  handle_write("german.m:42.5-23: ", rule_name, "Cache[i].State", s, VALUE_C(0), VALUE_C(2), handle_narrow(handle_index("german.m:42.5-12: ", rule_name, "Cache[i]", s, 4ull, VALUE_C(0), VALUE_C(1), ru_Cache, handle_read("german.m:42.11: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 0, 2), VALUE_C(0));
  handle_write("german.m:42.26-43: ", rule_name, "InvSet[i]", s, VALUE_C(0), VALUE_C(1), handle_index("german.m:42.26-34: ", rule_name, "InvSet[i]", s, 2ull, VALUE_C(0), VALUE_C(1), ru_InvSet, handle_read("german.m:42.33: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), VALUE_C(0));
  handle_write("german.m:42.46-63: ", rule_name, "ShrSet[i]", s, VALUE_C(0), VALUE_C(1), handle_index("german.m:42.46-54: ", rule_name, "ShrSet[i]", s, 2ull, VALUE_C(0), VALUE_C(1), ru_ShrSet, handle_read("german.m:42.53: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), VALUE_C(0));
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
;
    handle_write("german.m:44.3-17: ", rule_name, "ExGntd", s, VALUE_C(0), VALUE_C(1), ru_ExGntd, VALUE_C(0));
    handle_write("german.m:44.20-34: ", rule_name, "CurCmd", s, VALUE_C(0), VALUE_C(6), ru_CurCmd, VALUE_C(0));
    handle_write("german.m:44.37-48: ", rule_name, "MemData", s, VALUE_C(0), VALUE_C(1), ru_MemData, handle_read("german.m:44.48: ", rule_name, "d", s, VALUE_C(0), VALUE_C(1), ru_d));
    handle_write("german.m:44.51-62: ", rule_name, "AuxData", s, VALUE_C(0), VALUE_C(1), ru_AuxData, handle_read("german.m:44.62: ", rule_name, "d", s, VALUE_C(0), VALUE_C(1), ru_d));
  }

  return true;
}

static int guard0(const struct state *NONNULL s __attribute__((unused)), struct handle ru_i __attribute__((unused))) {
  static const char *rule_name __attribute__((unused)) = "guard of rule \"SendReqS\"";
  if (JMP_BUF_NEEDED) {
    if (sigsetjmp(checkpoint, 0)) {
      /* this guard triggered an error */
      return -1;
    }
  }
  const struct handle ru_Cache __attribute__((unused)) = state_handle(s, 0ull, 8ull);
  const struct handle ru_Chan1 __attribute__((unused)) = state_handle(s, 24ull, 10ull);
  const struct handle ru_Chan2 __attribute__((unused)) = state_handle(s, 34ull, 10ull);
  const struct handle ru_Chan3 __attribute__((unused)) = state_handle(s, 44ull, 10ull);
  const struct handle ru_InvSet __attribute__((unused)) = state_handle(s, 8ull, 4ull);
  const struct handle ru_ShrSet __attribute__((unused)) = state_handle(s, 12ull, 4ull);
  const struct handle ru_ExGntd __attribute__((unused)) = state_handle(s, 16ull, 2ull);
  const struct handle ru_CurCmd __attribute__((unused)) = state_handle(s, 54ull, 3ull);
  const struct handle ru_CurPtr __attribute__((unused)) = state_handle(s, 18ull, 2ull);
  const struct handle ru_MemData __attribute__((unused)) = state_handle(s, 20ull, 2ull);
  const struct handle ru_AuxData __attribute__((unused)) = state_handle(s, 22ull, 2ull);
  return ((handle_read("german.m:54.3-14: ", rule_name, "Chan1[i].Cmd", s, VALUE_C(0), VALUE_C(6), handle_narrow(handle_index("german.m:54.3-10: ", rule_name, "Chan1[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan1, handle_read("german.m:54.9: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 3)) == VALUE_C(0)) && (handle_read("german.m:54.26-39: ", rule_name, "Cache[i].State", s, VALUE_C(0), VALUE_C(2), handle_narrow(handle_index("german.m:54.26-33: ", rule_name, "Cache[i]", s, 4ull, VALUE_C(0), VALUE_C(1), ru_Cache, handle_read("german.m:54.32: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 0, 2)) == VALUE_C(0))) ? 1 : 0;

}

static bool rule0(struct state *NONNULL s, struct handle ru_i) {
  static const char *rule_name __attribute__((unused)) = "rule \"SendReqS\"";
  if (JMP_BUF_NEEDED) {
    if (sigsetjmp(checkpoint, 0)) {
      /* an error was triggered during this rule */
      return false;
    }
  }
  const struct handle ru_Cache __attribute__((unused)) = state_handle(s, 0ull, 8ull);
  const struct handle ru_Chan1 __attribute__((unused)) = state_handle(s, 24ull, 10ull);
  const struct handle ru_Chan2 __attribute__((unused)) = state_handle(s, 34ull, 10ull);
  const struct handle ru_Chan3 __attribute__((unused)) = state_handle(s, 44ull, 10ull);
  const struct handle ru_InvSet __attribute__((unused)) = state_handle(s, 8ull, 4ull);
  const struct handle ru_ShrSet __attribute__((unused)) = state_handle(s, 12ull, 4ull);
  const struct handle ru_ExGntd __attribute__((unused)) = state_handle(s, 16ull, 2ull);
  const struct handle ru_CurCmd __attribute__((unused)) = state_handle(s, 54ull, 3ull);
  const struct handle ru_CurPtr __attribute__((unused)) = state_handle(s, 18ull, 2ull);
  const struct handle ru_MemData __attribute__((unused)) = state_handle(s, 20ull, 2ull);
  const struct handle ru_AuxData __attribute__((unused)) = state_handle(s, 22ull, 2ull);
  {
  handle_write("german.m:56.3-22: ", rule_name, "Chan1[i].Cmd", s, VALUE_C(0), VALUE_C(6), handle_narrow(handle_index("german.m:56.3-10: ", rule_name, "Chan1[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan1, handle_read("german.m:56.9: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 3), VALUE_C(1));
  }

  return true;
}

static int guard1(const struct state *NONNULL s __attribute__((unused)), struct handle ru_i __attribute__((unused))) {
  static const char *rule_name __attribute__((unused)) = "guard of rule \"SendReqE\"";
  if (JMP_BUF_NEEDED) {
    if (sigsetjmp(checkpoint, 0)) {
      /* this guard triggered an error */
      return -1;
    }
  }
  const struct handle ru_Cache __attribute__((unused)) = state_handle(s, 0ull, 8ull);
  const struct handle ru_Chan1 __attribute__((unused)) = state_handle(s, 24ull, 10ull);
  const struct handle ru_Chan2 __attribute__((unused)) = state_handle(s, 34ull, 10ull);
  const struct handle ru_Chan3 __attribute__((unused)) = state_handle(s, 44ull, 10ull);
  const struct handle ru_InvSet __attribute__((unused)) = state_handle(s, 8ull, 4ull);
  const struct handle ru_ShrSet __attribute__((unused)) = state_handle(s, 12ull, 4ull);
  const struct handle ru_ExGntd __attribute__((unused)) = state_handle(s, 16ull, 2ull);
  const struct handle ru_CurCmd __attribute__((unused)) = state_handle(s, 54ull, 3ull);
  const struct handle ru_CurPtr __attribute__((unused)) = state_handle(s, 18ull, 2ull);
  const struct handle ru_MemData __attribute__((unused)) = state_handle(s, 20ull, 2ull);
  const struct handle ru_AuxData __attribute__((unused)) = state_handle(s, 22ull, 2ull);
  return ((handle_read("german.m:63.3-14: ", rule_name, "Chan1[i].Cmd", s, VALUE_C(0), VALUE_C(6), handle_narrow(handle_index("german.m:63.3-10: ", rule_name, "Chan1[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan1, handle_read("german.m:63.9: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 3)) == VALUE_C(0)) && ((handle_read("german.m:63.27-40: ", rule_name, "Cache[i].State", s, VALUE_C(0), VALUE_C(2), handle_narrow(handle_index("german.m:63.27-34: ", rule_name, "Cache[i]", s, 4ull, VALUE_C(0), VALUE_C(1), ru_Cache, handle_read("german.m:63.33: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 0, 2)) == VALUE_C(0)) || (handle_read("german.m:63.48-61: ", rule_name, "Cache[i].State", s, VALUE_C(0), VALUE_C(2), handle_narrow(handle_index("german.m:63.48-55: ", rule_name, "Cache[i]", s, 4ull, VALUE_C(0), VALUE_C(1), ru_Cache, handle_read("german.m:63.54: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 0, 2)) == VALUE_C(1)))) ? 1 : 0;

}

static bool rule1(struct state *NONNULL s, struct handle ru_i) {
  static const char *rule_name __attribute__((unused)) = "rule \"SendReqE\"";
  if (JMP_BUF_NEEDED) {
    if (sigsetjmp(checkpoint, 0)) {
      /* an error was triggered during this rule */
      return false;
    }
  }
  const struct handle ru_Cache __attribute__((unused)) = state_handle(s, 0ull, 8ull);
  const struct handle ru_Chan1 __attribute__((unused)) = state_handle(s, 24ull, 10ull);
  const struct handle ru_Chan2 __attribute__((unused)) = state_handle(s, 34ull, 10ull);
  const struct handle ru_Chan3 __attribute__((unused)) = state_handle(s, 44ull, 10ull);
  const struct handle ru_InvSet __attribute__((unused)) = state_handle(s, 8ull, 4ull);
  const struct handle ru_ShrSet __attribute__((unused)) = state_handle(s, 12ull, 4ull);
  const struct handle ru_ExGntd __attribute__((unused)) = state_handle(s, 16ull, 2ull);
  const struct handle ru_CurCmd __attribute__((unused)) = state_handle(s, 54ull, 3ull);
  const struct handle ru_CurPtr __attribute__((unused)) = state_handle(s, 18ull, 2ull);
  const struct handle ru_MemData __attribute__((unused)) = state_handle(s, 20ull, 2ull);
  const struct handle ru_AuxData __attribute__((unused)) = state_handle(s, 22ull, 2ull);
  {
  handle_write("german.m:65.3-22: ", rule_name, "Chan1[i].Cmd", s, VALUE_C(0), VALUE_C(6), handle_narrow(handle_index("german.m:65.3-10: ", rule_name, "Chan1[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan1, handle_read("german.m:65.9: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 3), VALUE_C(2));
  }

  return true;
}

static int guard2(const struct state *NONNULL s __attribute__((unused)), struct handle ru_i __attribute__((unused))) {
  static const char *rule_name __attribute__((unused)) = "guard of rule \"RecvReqS\"";
  if (JMP_BUF_NEEDED) {
    if (sigsetjmp(checkpoint, 0)) {
      /* this guard triggered an error */
      return -1;
    }
  }
  const struct handle ru_Cache __attribute__((unused)) = state_handle(s, 0ull, 8ull);
  const struct handle ru_Chan1 __attribute__((unused)) = state_handle(s, 24ull, 10ull);
  const struct handle ru_Chan2 __attribute__((unused)) = state_handle(s, 34ull, 10ull);
  const struct handle ru_Chan3 __attribute__((unused)) = state_handle(s, 44ull, 10ull);
  const struct handle ru_InvSet __attribute__((unused)) = state_handle(s, 8ull, 4ull);
  const struct handle ru_ShrSet __attribute__((unused)) = state_handle(s, 12ull, 4ull);
  const struct handle ru_ExGntd __attribute__((unused)) = state_handle(s, 16ull, 2ull);
  const struct handle ru_CurCmd __attribute__((unused)) = state_handle(s, 54ull, 3ull);
  const struct handle ru_CurPtr __attribute__((unused)) = state_handle(s, 18ull, 2ull);
  const struct handle ru_MemData __attribute__((unused)) = state_handle(s, 20ull, 2ull);
  const struct handle ru_AuxData __attribute__((unused)) = state_handle(s, 22ull, 2ull);
  return ((handle_read("german.m:76.3-8: ", rule_name, "CurCmd", s, VALUE_C(0), VALUE_C(6), ru_CurCmd) == VALUE_C(0)) && (handle_read("german.m:76.20-31: ", rule_name, "Chan1[i].Cmd", s, VALUE_C(0), VALUE_C(6), handle_narrow(handle_index("german.m:76.20-27: ", rule_name, "Chan1[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan1, handle_read("german.m:76.26: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 3)) == VALUE_C(1))) ? 1 : 0;

}

static bool rule2(struct state *NONNULL s, struct handle ru_i) {
  static const char *rule_name __attribute__((unused)) = "rule \"RecvReqS\"";
  if (JMP_BUF_NEEDED) {
    if (sigsetjmp(checkpoint, 0)) {
      /* an error was triggered during this rule */
      return false;
    }
  }
  const struct handle ru_Cache __attribute__((unused)) = state_handle(s, 0ull, 8ull);
  const struct handle ru_Chan1 __attribute__((unused)) = state_handle(s, 24ull, 10ull);
  const struct handle ru_Chan2 __attribute__((unused)) = state_handle(s, 34ull, 10ull);
  const struct handle ru_Chan3 __attribute__((unused)) = state_handle(s, 44ull, 10ull);
  const struct handle ru_InvSet __attribute__((unused)) = state_handle(s, 8ull, 4ull);
  const struct handle ru_ShrSet __attribute__((unused)) = state_handle(s, 12ull, 4ull);
  const struct handle ru_ExGntd __attribute__((unused)) = state_handle(s, 16ull, 2ull);
  const struct handle ru_CurCmd __attribute__((unused)) = state_handle(s, 54ull, 3ull);
  const struct handle ru_CurPtr __attribute__((unused)) = state_handle(s, 18ull, 2ull);
  const struct handle ru_MemData __attribute__((unused)) = state_handle(s, 20ull, 2ull);
  const struct handle ru_AuxData __attribute__((unused)) = state_handle(s, 22ull, 2ull);
  {
  handle_write("german.m:78.3-16: ", rule_name, "CurCmd", s, VALUE_C(0), VALUE_C(6), ru_CurCmd, VALUE_C(1));
  handle_write("german.m:78.19-29: ", rule_name, "CurPtr", s, VALUE_C(0), VALUE_C(1), ru_CurPtr, handle_read("german.m:78.29: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i));
  handle_write("german.m:79.3-23: ", rule_name, "Chan1[i].Cmd", s, VALUE_C(0), VALUE_C(6), handle_narrow(handle_index("german.m:79.3-10: ", rule_name, "Chan1[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan1, handle_read("german.m:79.9: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 3), VALUE_C(0));
  {
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_j = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_j == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_j - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_j - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_j += step) {
    uint8_t _ru2_j[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_j = { .base = _ru2_j, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_j, _ru1_j);
  handle_write("german.m:80.19-40: ", rule_name, "InvSet[j]", s, VALUE_C(0), VALUE_C(1), handle_index("german.m:80.19-27: ", rule_name, "InvSet[j]", s, 2ull, VALUE_C(0), VALUE_C(1), ru_InvSet, handle_read("german.m:80.26: ", rule_name, "j", s, VALUE_C(0), VALUE_C(1), ru_j)), handle_read("german.m:80.32-40: ", rule_name, "ShrSet[j]", s, VALUE_C(0), VALUE_C(1), handle_index("german.m:80.32-40: ", rule_name, "ShrSet[j]", s, 2ull, VALUE_C(0), VALUE_C(1), ru_ShrSet, handle_read("german.m:80.39: ", rule_name, "j", s, VALUE_C(0), VALUE_C(1), ru_j))));
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_j > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_j - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
;
  }

  return true;
}

static int guard3(const struct state *NONNULL s __attribute__((unused)), struct handle ru_i __attribute__((unused))) {
  static const char *rule_name __attribute__((unused)) = "guard of rule \"RecvReqE\"";
  if (JMP_BUF_NEEDED) {
    if (sigsetjmp(checkpoint, 0)) {
      /* this guard triggered an error */
      return -1;
    }
  }
  const struct handle ru_Cache __attribute__((unused)) = state_handle(s, 0ull, 8ull);
  const struct handle ru_Chan1 __attribute__((unused)) = state_handle(s, 24ull, 10ull);
  const struct handle ru_Chan2 __attribute__((unused)) = state_handle(s, 34ull, 10ull);
  const struct handle ru_Chan3 __attribute__((unused)) = state_handle(s, 44ull, 10ull);
  const struct handle ru_InvSet __attribute__((unused)) = state_handle(s, 8ull, 4ull);
  const struct handle ru_ShrSet __attribute__((unused)) = state_handle(s, 12ull, 4ull);
  const struct handle ru_ExGntd __attribute__((unused)) = state_handle(s, 16ull, 2ull);
  const struct handle ru_CurCmd __attribute__((unused)) = state_handle(s, 54ull, 3ull);
  const struct handle ru_CurPtr __attribute__((unused)) = state_handle(s, 18ull, 2ull);
  const struct handle ru_MemData __attribute__((unused)) = state_handle(s, 20ull, 2ull);
  const struct handle ru_AuxData __attribute__((unused)) = state_handle(s, 22ull, 2ull);
  return ((handle_read("german.m:90.3-14: ", rule_name, "Chan1[i].Cmd", s, VALUE_C(0), VALUE_C(6), handle_narrow(handle_index("german.m:90.3-10: ", rule_name, "Chan1[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan1, handle_read("german.m:90.9: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 3)) == VALUE_C(0)) && (handle_read("german.m:90.26-37: ", rule_name, "Chan1[i].Cmd", s, VALUE_C(0), VALUE_C(6), handle_narrow(handle_index("german.m:90.26-33: ", rule_name, "Chan1[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan1, handle_read("german.m:90.32: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 3)) == VALUE_C(2))) ? 1 : 0;

}

static bool rule3(struct state *NONNULL s, struct handle ru_i) {
  static const char *rule_name __attribute__((unused)) = "rule \"RecvReqE\"";
  if (JMP_BUF_NEEDED) {
    if (sigsetjmp(checkpoint, 0)) {
      /* an error was triggered during this rule */
      return false;
    }
  }
  const struct handle ru_Cache __attribute__((unused)) = state_handle(s, 0ull, 8ull);
  const struct handle ru_Chan1 __attribute__((unused)) = state_handle(s, 24ull, 10ull);
  const struct handle ru_Chan2 __attribute__((unused)) = state_handle(s, 34ull, 10ull);
  const struct handle ru_Chan3 __attribute__((unused)) = state_handle(s, 44ull, 10ull);
  const struct handle ru_InvSet __attribute__((unused)) = state_handle(s, 8ull, 4ull);
  const struct handle ru_ShrSet __attribute__((unused)) = state_handle(s, 12ull, 4ull);
  const struct handle ru_ExGntd __attribute__((unused)) = state_handle(s, 16ull, 2ull);
  const struct handle ru_CurCmd __attribute__((unused)) = state_handle(s, 54ull, 3ull);
  const struct handle ru_CurPtr __attribute__((unused)) = state_handle(s, 18ull, 2ull);
  const struct handle ru_MemData __attribute__((unused)) = state_handle(s, 20ull, 2ull);
  const struct handle ru_AuxData __attribute__((unused)) = state_handle(s, 22ull, 2ull);
  {
  handle_write("german.m:92.3-16: ", rule_name, "CurCmd", s, VALUE_C(0), VALUE_C(6), ru_CurCmd, VALUE_C(2));
  handle_write("german.m:92.19-29: ", rule_name, "CurPtr", s, VALUE_C(0), VALUE_C(1), ru_CurPtr, handle_read("german.m:92.29: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i));
  handle_write("german.m:93.3-23: ", rule_name, "Chan1[i].Cmd", s, VALUE_C(0), VALUE_C(6), handle_narrow(handle_index("german.m:93.3-10: ", rule_name, "Chan1[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan1, handle_read("german.m:93.9: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 3), VALUE_C(0));
  {
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_j = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_j == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_j - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_j - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_j += step) {
    uint8_t _ru2_j[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_j = { .base = _ru2_j, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_j, _ru1_j);
  handle_write("german.m:94.19-40: ", rule_name, "InvSet[j]", s, VALUE_C(0), VALUE_C(1), handle_index("german.m:94.19-27: ", rule_name, "InvSet[j]", s, 2ull, VALUE_C(0), VALUE_C(1), ru_InvSet, handle_read("german.m:94.26: ", rule_name, "j", s, VALUE_C(0), VALUE_C(1), ru_j)), handle_read("german.m:94.32-40: ", rule_name, "ShrSet[j]", s, VALUE_C(0), VALUE_C(1), handle_index("german.m:94.32-40: ", rule_name, "ShrSet[j]", s, 2ull, VALUE_C(0), VALUE_C(1), ru_ShrSet, handle_read("german.m:94.39: ", rule_name, "j", s, VALUE_C(0), VALUE_C(1), ru_j))));
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_j > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_j - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
;
  }

  return true;
}

static int guard4(const struct state *NONNULL s __attribute__((unused)), struct handle ru_i __attribute__((unused))) {
  static const char *rule_name __attribute__((unused)) = "guard of rule \"SendInv\"";
  if (JMP_BUF_NEEDED) {
    if (sigsetjmp(checkpoint, 0)) {
      /* this guard triggered an error */
      return -1;
    }
  }
  const struct handle ru_Cache __attribute__((unused)) = state_handle(s, 0ull, 8ull);
  const struct handle ru_Chan1 __attribute__((unused)) = state_handle(s, 24ull, 10ull);
  const struct handle ru_Chan2 __attribute__((unused)) = state_handle(s, 34ull, 10ull);
  const struct handle ru_Chan3 __attribute__((unused)) = state_handle(s, 44ull, 10ull);
  const struct handle ru_InvSet __attribute__((unused)) = state_handle(s, 8ull, 4ull);
  const struct handle ru_ShrSet __attribute__((unused)) = state_handle(s, 12ull, 4ull);
  const struct handle ru_ExGntd __attribute__((unused)) = state_handle(s, 16ull, 2ull);
  const struct handle ru_CurCmd __attribute__((unused)) = state_handle(s, 54ull, 3ull);
  const struct handle ru_CurPtr __attribute__((unused)) = state_handle(s, 18ull, 2ull);
  const struct handle ru_MemData __attribute__((unused)) = state_handle(s, 20ull, 2ull);
  const struct handle ru_AuxData __attribute__((unused)) = state_handle(s, 22ull, 2ull);
  return (((handle_read("german.m:104.3-14: ", rule_name, "Chan2[i].Cmd", s, VALUE_C(0), VALUE_C(6), handle_narrow(handle_index("german.m:104.3-10: ", rule_name, "Chan2[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan2, handle_read("german.m:104.9: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 3)) == VALUE_C(0)) && (handle_read("german.m:104.26-34: ", rule_name, "InvSet[i]", s, VALUE_C(0), VALUE_C(1), handle_index("german.m:104.26-34: ", rule_name, "InvSet[i]", s, 2ull, VALUE_C(0), VALUE_C(1), ru_InvSet, handle_read("german.m:104.33: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i))) == VALUE_C(1))) && ((handle_read("german.m:105.5-10: ", rule_name, "CurCmd", s, VALUE_C(0), VALUE_C(6), ru_CurCmd) == VALUE_C(2)) || ((handle_read("german.m:106.5-10: ", rule_name, "CurCmd", s, VALUE_C(0), VALUE_C(6), ru_CurCmd) == VALUE_C(1)) && (handle_read("german.m:106.21-26: ", rule_name, "ExGntd", s, VALUE_C(0), VALUE_C(1), ru_ExGntd) == VALUE_C(1))))) ? 1 : 0;

}

static bool rule4(struct state *NONNULL s, struct handle ru_i) {
  static const char *rule_name __attribute__((unused)) = "rule \"SendInv\"";
  if (JMP_BUF_NEEDED) {
    if (sigsetjmp(checkpoint, 0)) {
      /* an error was triggered during this rule */
      return false;
    }
  }
  const struct handle ru_Cache __attribute__((unused)) = state_handle(s, 0ull, 8ull);
  const struct handle ru_Chan1 __attribute__((unused)) = state_handle(s, 24ull, 10ull);
  const struct handle ru_Chan2 __attribute__((unused)) = state_handle(s, 34ull, 10ull);
  const struct handle ru_Chan3 __attribute__((unused)) = state_handle(s, 44ull, 10ull);
  const struct handle ru_InvSet __attribute__((unused)) = state_handle(s, 8ull, 4ull);
  const struct handle ru_ShrSet __attribute__((unused)) = state_handle(s, 12ull, 4ull);
  const struct handle ru_ExGntd __attribute__((unused)) = state_handle(s, 16ull, 2ull);
  const struct handle ru_CurCmd __attribute__((unused)) = state_handle(s, 54ull, 3ull);
  const struct handle ru_CurPtr __attribute__((unused)) = state_handle(s, 18ull, 2ull);
  const struct handle ru_MemData __attribute__((unused)) = state_handle(s, 20ull, 2ull);
  const struct handle ru_AuxData __attribute__((unused)) = state_handle(s, 22ull, 2ull);
  {
  handle_write("german.m:108.3-21: ", rule_name, "Chan2[i].Cmd", s, VALUE_C(0), VALUE_C(6), handle_narrow(handle_index("german.m:108.3-10: ", rule_name, "Chan2[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan2, handle_read("german.m:108.9: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 3), VALUE_C(3));
  handle_write("german.m:108.24-41: ", rule_name, "InvSet[i]", s, VALUE_C(0), VALUE_C(1), handle_index("german.m:108.24-32: ", rule_name, "InvSet[i]", s, 2ull, VALUE_C(0), VALUE_C(1), ru_InvSet, handle_read("german.m:108.31: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), VALUE_C(0));
  }

  return true;
}

static int guard5(const struct state *NONNULL s __attribute__((unused)), struct handle ru_i __attribute__((unused))) {
  static const char *rule_name __attribute__((unused)) = "guard of rule \"SendInvAck\"";
  if (JMP_BUF_NEEDED) {
    if (sigsetjmp(checkpoint, 0)) {
      /* this guard triggered an error */
      return -1;
    }
  }
  const struct handle ru_Cache __attribute__((unused)) = state_handle(s, 0ull, 8ull);
  const struct handle ru_Chan1 __attribute__((unused)) = state_handle(s, 24ull, 10ull);
  const struct handle ru_Chan2 __attribute__((unused)) = state_handle(s, 34ull, 10ull);
  const struct handle ru_Chan3 __attribute__((unused)) = state_handle(s, 44ull, 10ull);
  const struct handle ru_InvSet __attribute__((unused)) = state_handle(s, 8ull, 4ull);
  const struct handle ru_ShrSet __attribute__((unused)) = state_handle(s, 12ull, 4ull);
  const struct handle ru_ExGntd __attribute__((unused)) = state_handle(s, 16ull, 2ull);
  const struct handle ru_CurCmd __attribute__((unused)) = state_handle(s, 54ull, 3ull);
  const struct handle ru_CurPtr __attribute__((unused)) = state_handle(s, 18ull, 2ull);
  const struct handle ru_MemData __attribute__((unused)) = state_handle(s, 20ull, 2ull);
  const struct handle ru_AuxData __attribute__((unused)) = state_handle(s, 22ull, 2ull);
  return ((handle_read("german.m:112.3-14: ", rule_name, "Chan2[i].Cmd", s, VALUE_C(0), VALUE_C(6), handle_narrow(handle_index("german.m:112.3-10: ", rule_name, "Chan2[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan2, handle_read("german.m:112.9: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 3)) == VALUE_C(3)) && (handle_read("german.m:112.24-35: ", rule_name, "Chan3[i].Cmd", s, VALUE_C(0), VALUE_C(6), handle_narrow(handle_index("german.m:112.24-31: ", rule_name, "Chan3[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan3, handle_read("german.m:112.30: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 3)) == VALUE_C(0))) ? 1 : 0;

}

static bool rule5(struct state *NONNULL s, struct handle ru_i) {
  static const char *rule_name __attribute__((unused)) = "rule \"SendInvAck\"";
  if (JMP_BUF_NEEDED) {
    if (sigsetjmp(checkpoint, 0)) {
      /* an error was triggered during this rule */
      return false;
    }
  }
  const struct handle ru_Cache __attribute__((unused)) = state_handle(s, 0ull, 8ull);
  const struct handle ru_Chan1 __attribute__((unused)) = state_handle(s, 24ull, 10ull);
  const struct handle ru_Chan2 __attribute__((unused)) = state_handle(s, 34ull, 10ull);
  const struct handle ru_Chan3 __attribute__((unused)) = state_handle(s, 44ull, 10ull);
  const struct handle ru_InvSet __attribute__((unused)) = state_handle(s, 8ull, 4ull);
  const struct handle ru_ShrSet __attribute__((unused)) = state_handle(s, 12ull, 4ull);
  const struct handle ru_ExGntd __attribute__((unused)) = state_handle(s, 16ull, 2ull);
  const struct handle ru_CurCmd __attribute__((unused)) = state_handle(s, 54ull, 3ull);
  const struct handle ru_CurPtr __attribute__((unused)) = state_handle(s, 18ull, 2ull);
  const struct handle ru_MemData __attribute__((unused)) = state_handle(s, 20ull, 2ull);
  const struct handle ru_AuxData __attribute__((unused)) = state_handle(s, 22ull, 2ull);
  {
  handle_write("german.m:119.3-23: ", rule_name, "Chan2[i].Cmd", s, VALUE_C(0), VALUE_C(6), handle_narrow(handle_index("german.m:119.3-10: ", rule_name, "Chan2[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan2, handle_read("german.m:119.9: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 3), VALUE_C(0));
  handle_write("german.m:119.26-47: ", rule_name, "Chan3[i].Cmd", s, VALUE_C(0), VALUE_C(6), handle_narrow(handle_index("german.m:119.26-33: ", rule_name, "Chan3[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan3, handle_read("german.m:119.32: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 3), VALUE_C(4));
  if (handle_read("german.m:120.7-20: ", rule_name, "Cache[i].State", s, VALUE_C(0), VALUE_C(2), handle_narrow(handle_index("german.m:120.7-14: ", rule_name, "Cache[i]", s, 4ull, VALUE_C(0), VALUE_C(1), ru_Cache, handle_read("german.m:120.13: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 0, 2)) == VALUE_C(2)) {
handle_write("german.m:120.32-61: ", rule_name, "Chan3[i].Data", s, VALUE_C(0), VALUE_C(1), handle_narrow(handle_index("german.m:120.32-39: ", rule_name, "Chan3[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan3, handle_read("german.m:120.38: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 0, 2), handle_read("german.m:120.49-61: ", rule_name, "Cache[i].Data", s, VALUE_C(0), VALUE_C(1), handle_narrow(handle_index("german.m:120.49-56: ", rule_name, "Cache[i]", s, 4ull, VALUE_C(0), VALUE_C(1), ru_Cache, handle_read("german.m:120.55: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 2)));
}
;
  handle_write("german.m:121.3-21: ", rule_name, "Cache[i].State", s, VALUE_C(0), VALUE_C(2), handle_narrow(handle_index("german.m:121.3-10: ", rule_name, "Cache[i]", s, 4ull, VALUE_C(0), VALUE_C(1), ru_Cache, handle_read("german.m:121.9: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 0, 2), VALUE_C(0));
  handle_zero(handle_narrow(handle_index("german.m:121.33-40: ", rule_name, "Cache[i]", s, 4ull, VALUE_C(0), VALUE_C(1), ru_Cache, handle_read("german.m:121.39: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 2));
  }

  return true;
}

static int guard6(const struct state *NONNULL s __attribute__((unused)), struct handle ru_i __attribute__((unused))) {
  static const char *rule_name __attribute__((unused)) = "guard of rule \"RecvInvAck\"";
  if (JMP_BUF_NEEDED) {
    if (sigsetjmp(checkpoint, 0)) {
      /* this guard triggered an error */
      return -1;
    }
  }
  const struct handle ru_Cache __attribute__((unused)) = state_handle(s, 0ull, 8ull);
  const struct handle ru_Chan1 __attribute__((unused)) = state_handle(s, 24ull, 10ull);
  const struct handle ru_Chan2 __attribute__((unused)) = state_handle(s, 34ull, 10ull);
  const struct handle ru_Chan3 __attribute__((unused)) = state_handle(s, 44ull, 10ull);
  const struct handle ru_InvSet __attribute__((unused)) = state_handle(s, 8ull, 4ull);
  const struct handle ru_ShrSet __attribute__((unused)) = state_handle(s, 12ull, 4ull);
  const struct handle ru_ExGntd __attribute__((unused)) = state_handle(s, 16ull, 2ull);
  const struct handle ru_CurCmd __attribute__((unused)) = state_handle(s, 54ull, 3ull);
  const struct handle ru_CurPtr __attribute__((unused)) = state_handle(s, 18ull, 2ull);
  const struct handle ru_MemData __attribute__((unused)) = state_handle(s, 20ull, 2ull);
  const struct handle ru_AuxData __attribute__((unused)) = state_handle(s, 22ull, 2ull);
  return ((handle_read("german.m:125.3-14: ", rule_name, "Chan3[i].Cmd", s, VALUE_C(0), VALUE_C(6), handle_narrow(handle_index("german.m:125.3-10: ", rule_name, "Chan3[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan3, handle_read("german.m:125.9: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 3)) == VALUE_C(4)) && (handle_read("german.m:125.27-32: ", rule_name, "CurCmd", s, VALUE_C(0), VALUE_C(6), ru_CurCmd) != VALUE_C(0))) ? 1 : 0;

}

static bool rule6(struct state *NONNULL s, struct handle ru_i) {
  static const char *rule_name __attribute__((unused)) = "rule \"RecvInvAck\"";
  if (JMP_BUF_NEEDED) {
    if (sigsetjmp(checkpoint, 0)) {
      /* an error was triggered during this rule */
      return false;
    }
  }
  const struct handle ru_Cache __attribute__((unused)) = state_handle(s, 0ull, 8ull);
  const struct handle ru_Chan1 __attribute__((unused)) = state_handle(s, 24ull, 10ull);
  const struct handle ru_Chan2 __attribute__((unused)) = state_handle(s, 34ull, 10ull);
  const struct handle ru_Chan3 __attribute__((unused)) = state_handle(s, 44ull, 10ull);
  const struct handle ru_InvSet __attribute__((unused)) = state_handle(s, 8ull, 4ull);
  const struct handle ru_ShrSet __attribute__((unused)) = state_handle(s, 12ull, 4ull);
  const struct handle ru_ExGntd __attribute__((unused)) = state_handle(s, 16ull, 2ull);
  const struct handle ru_CurCmd __attribute__((unused)) = state_handle(s, 54ull, 3ull);
  const struct handle ru_CurPtr __attribute__((unused)) = state_handle(s, 18ull, 2ull);
  const struct handle ru_MemData __attribute__((unused)) = state_handle(s, 20ull, 2ull);
  const struct handle ru_AuxData __attribute__((unused)) = state_handle(s, 22ull, 2ull);
  {
  handle_write("german.m:127.3-23: ", rule_name, "Chan3[i].Cmd", s, VALUE_C(0), VALUE_C(6), handle_narrow(handle_index("german.m:127.3-10: ", rule_name, "Chan3[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan3, handle_read("german.m:127.9: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 3), VALUE_C(0));
  handle_write("german.m:127.26-43: ", rule_name, "ShrSet[i]", s, VALUE_C(0), VALUE_C(1), handle_index("german.m:127.26-34: ", rule_name, "ShrSet[i]", s, 2ull, VALUE_C(0), VALUE_C(1), ru_ShrSet, handle_read("german.m:127.33: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), VALUE_C(0));
  if (handle_read("german.m:128.7-12: ", rule_name, "ExGntd", s, VALUE_C(0), VALUE_C(1), ru_ExGntd) == VALUE_C(1)) {
handle_write("german.m:128.27-41: ", rule_name, "ExGntd", s, VALUE_C(0), VALUE_C(1), ru_ExGntd, VALUE_C(0));
handle_write("german.m:128.44-67: ", rule_name, "MemData", s, VALUE_C(0), VALUE_C(1), ru_MemData, handle_read("german.m:128.55-67: ", rule_name, "Chan3[i].Data", s, VALUE_C(0), VALUE_C(1), handle_narrow(handle_index("german.m:128.55-62: ", rule_name, "Chan3[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan3, handle_read("german.m:128.61: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 0, 2)));
handle_zero(handle_narrow(handle_index("german.m:129.12-19: ", rule_name, "Chan3[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan3, handle_read("german.m:129.18: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 0, 2));
}
;
  }

  return true;
}

static int guard7(const struct state *NONNULL s __attribute__((unused)), struct handle ru_i __attribute__((unused))) {
  static const char *rule_name __attribute__((unused)) = "guard of rule \"SendGntS\"";
  if (JMP_BUF_NEEDED) {
    if (sigsetjmp(checkpoint, 0)) {
      /* this guard triggered an error */
      return -1;
    }
  }
  const struct handle ru_Cache __attribute__((unused)) = state_handle(s, 0ull, 8ull);
  const struct handle ru_Chan1 __attribute__((unused)) = state_handle(s, 24ull, 10ull);
  const struct handle ru_Chan2 __attribute__((unused)) = state_handle(s, 34ull, 10ull);
  const struct handle ru_Chan3 __attribute__((unused)) = state_handle(s, 44ull, 10ull);
  const struct handle ru_InvSet __attribute__((unused)) = state_handle(s, 8ull, 4ull);
  const struct handle ru_ShrSet __attribute__((unused)) = state_handle(s, 12ull, 4ull);
  const struct handle ru_ExGntd __attribute__((unused)) = state_handle(s, 16ull, 2ull);
  const struct handle ru_CurCmd __attribute__((unused)) = state_handle(s, 54ull, 3ull);
  const struct handle ru_CurPtr __attribute__((unused)) = state_handle(s, 18ull, 2ull);
  const struct handle ru_MemData __attribute__((unused)) = state_handle(s, 20ull, 2ull);
  const struct handle ru_AuxData __attribute__((unused)) = state_handle(s, 22ull, 2ull);
  return ((((handle_read("german.m:133.3-8: ", rule_name, "CurCmd", s, VALUE_C(0), VALUE_C(6), ru_CurCmd) == VALUE_C(1)) && (handle_read("german.m:133.19-24: ", rule_name, "CurPtr", s, VALUE_C(0), VALUE_C(1), ru_CurPtr) == handle_read("german.m:133.28: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i))) && (handle_read("german.m:133.32-43: ", rule_name, "Chan2[i].Cmd", s, VALUE_C(0), VALUE_C(6), handle_narrow(handle_index("german.m:133.32-39: ", rule_name, "Chan2[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan2, handle_read("german.m:133.38: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 3)) == VALUE_C(0))) && (handle_read("german.m:133.55-60: ", rule_name, "ExGntd", s, VALUE_C(0), VALUE_C(1), ru_ExGntd) == VALUE_C(0))) ? 1 : 0;

}

static bool rule7(struct state *NONNULL s, struct handle ru_i) {
  static const char *rule_name __attribute__((unused)) = "rule \"SendGntS\"";
  if (JMP_BUF_NEEDED) {
    if (sigsetjmp(checkpoint, 0)) {
      /* an error was triggered during this rule */
      return false;
    }
  }
  const struct handle ru_Cache __attribute__((unused)) = state_handle(s, 0ull, 8ull);
  const struct handle ru_Chan1 __attribute__((unused)) = state_handle(s, 24ull, 10ull);
  const struct handle ru_Chan2 __attribute__((unused)) = state_handle(s, 34ull, 10ull);
  const struct handle ru_Chan3 __attribute__((unused)) = state_handle(s, 44ull, 10ull);
  const struct handle ru_InvSet __attribute__((unused)) = state_handle(s, 8ull, 4ull);
  const struct handle ru_ShrSet __attribute__((unused)) = state_handle(s, 12ull, 4ull);
  const struct handle ru_ExGntd __attribute__((unused)) = state_handle(s, 16ull, 2ull);
  const struct handle ru_CurCmd __attribute__((unused)) = state_handle(s, 54ull, 3ull);
  const struct handle ru_CurPtr __attribute__((unused)) = state_handle(s, 18ull, 2ull);
  const struct handle ru_MemData __attribute__((unused)) = state_handle(s, 20ull, 2ull);
  const struct handle ru_AuxData __attribute__((unused)) = state_handle(s, 22ull, 2ull);
  {
  handle_write("german.m:135.3-22: ", rule_name, "Chan2[i].Cmd", s, VALUE_C(0), VALUE_C(6), handle_narrow(handle_index("german.m:135.3-10: ", rule_name, "Chan2[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan2, handle_read("german.m:135.9: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 3), VALUE_C(5));
  handle_write("german.m:135.25-48: ", rule_name, "Chan2[i].Data", s, VALUE_C(0), VALUE_C(1), handle_narrow(handle_index("german.m:135.25-32: ", rule_name, "Chan2[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan2, handle_read("german.m:135.31: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 0, 2), handle_read("german.m:135.42-48: ", rule_name, "MemData", s, VALUE_C(0), VALUE_C(1), ru_MemData));
  handle_write("german.m:135.51-67: ", rule_name, "ShrSet[i]", s, VALUE_C(0), VALUE_C(1), handle_index("german.m:135.51-59: ", rule_name, "ShrSet[i]", s, 2ull, VALUE_C(0), VALUE_C(1), ru_ShrSet, handle_read("german.m:135.58: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), VALUE_C(1));
  handle_write("german.m:135.70-84: ", rule_name, "CurCmd", s, VALUE_C(0), VALUE_C(6), ru_CurCmd, VALUE_C(0));
  handle_zero(ru_CurPtr);
  }

  return true;
}

static int guard8(const struct state *NONNULL s __attribute__((unused)), struct handle ru_i __attribute__((unused))) {
  static const char *rule_name __attribute__((unused)) = "guard of rule \"SendGntE\"";
  if (JMP_BUF_NEEDED) {
    if (sigsetjmp(checkpoint, 0)) {
      /* this guard triggered an error */
      return -1;
    }
  }
  const struct handle ru_Cache __attribute__((unused)) = state_handle(s, 0ull, 8ull);
  const struct handle ru_Chan1 __attribute__((unused)) = state_handle(s, 24ull, 10ull);
  const struct handle ru_Chan2 __attribute__((unused)) = state_handle(s, 34ull, 10ull);
  const struct handle ru_Chan3 __attribute__((unused)) = state_handle(s, 44ull, 10ull);
  const struct handle ru_InvSet __attribute__((unused)) = state_handle(s, 8ull, 4ull);
  const struct handle ru_ShrSet __attribute__((unused)) = state_handle(s, 12ull, 4ull);
  const struct handle ru_ExGntd __attribute__((unused)) = state_handle(s, 16ull, 2ull);
  const struct handle ru_CurCmd __attribute__((unused)) = state_handle(s, 54ull, 3ull);
  const struct handle ru_CurPtr __attribute__((unused)) = state_handle(s, 18ull, 2ull);
  const struct handle ru_MemData __attribute__((unused)) = state_handle(s, 20ull, 2ull);
  const struct handle ru_AuxData __attribute__((unused)) = state_handle(s, 22ull, 2ull);
  return (((((handle_read("german.m:141.3-8: ", rule_name, "CurCmd", s, VALUE_C(0), VALUE_C(6), ru_CurCmd) == VALUE_C(2)) && (handle_read("german.m:141.19-24: ", rule_name, "CurPtr", s, VALUE_C(0), VALUE_C(1), ru_CurPtr) == handle_read("german.m:141.28: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i))) && (handle_read("german.m:141.32-43: ", rule_name, "Chan2[i].Cmd", s, VALUE_C(0), VALUE_C(6), handle_narrow(handle_index("german.m:141.32-39: ", rule_name, "Chan2[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan2, handle_read("german.m:141.38: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 3)) == VALUE_C(0))) && (handle_read("german.m:141.55-60: ", rule_name, "ExGntd", s, VALUE_C(0), VALUE_C(1), ru_ExGntd) == VALUE_C(0))) && ({ bool result = true; {
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_j = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_j == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_j - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_j - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_j += step) {
    uint8_t _ru2_j[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_j = { .base = _ru2_j, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_j, _ru1_j);
if (!(handle_read("german.m:142.22-30: ", rule_name, "ShrSet[j]", s, VALUE_C(0), VALUE_C(1), handle_index("german.m:142.22-30: ", rule_name, "ShrSet[j]", s, 2ull, VALUE_C(0), VALUE_C(1), ru_ShrSet, handle_read("german.m:142.29: ", rule_name, "j", s, VALUE_C(0), VALUE_C(1), ru_j))) == VALUE_C(0))) { result = false; break; }    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_j > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_j - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
 result; })) ? 1 : 0;

}

static bool rule8(struct state *NONNULL s, struct handle ru_i) {
  static const char *rule_name __attribute__((unused)) = "rule \"SendGntE\"";
  if (JMP_BUF_NEEDED) {
    if (sigsetjmp(checkpoint, 0)) {
      /* an error was triggered during this rule */
      return false;
    }
  }
  const struct handle ru_Cache __attribute__((unused)) = state_handle(s, 0ull, 8ull);
  const struct handle ru_Chan1 __attribute__((unused)) = state_handle(s, 24ull, 10ull);
  const struct handle ru_Chan2 __attribute__((unused)) = state_handle(s, 34ull, 10ull);
  const struct handle ru_Chan3 __attribute__((unused)) = state_handle(s, 44ull, 10ull);
  const struct handle ru_InvSet __attribute__((unused)) = state_handle(s, 8ull, 4ull);
  const struct handle ru_ShrSet __attribute__((unused)) = state_handle(s, 12ull, 4ull);
  const struct handle ru_ExGntd __attribute__((unused)) = state_handle(s, 16ull, 2ull);
  const struct handle ru_CurCmd __attribute__((unused)) = state_handle(s, 54ull, 3ull);
  const struct handle ru_CurPtr __attribute__((unused)) = state_handle(s, 18ull, 2ull);
  const struct handle ru_MemData __attribute__((unused)) = state_handle(s, 20ull, 2ull);
  const struct handle ru_AuxData __attribute__((unused)) = state_handle(s, 22ull, 2ull);
  {
  handle_write("german.m:144.3-22: ", rule_name, "Chan2[i].Cmd", s, VALUE_C(0), VALUE_C(6), handle_narrow(handle_index("german.m:144.3-10: ", rule_name, "Chan2[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan2, handle_read("german.m:144.9: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 3), VALUE_C(6));
  handle_write("german.m:144.25-48: ", rule_name, "Chan2[i].Data", s, VALUE_C(0), VALUE_C(1), handle_narrow(handle_index("german.m:144.25-32: ", rule_name, "Chan2[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan2, handle_read("german.m:144.31: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 0, 2), handle_read("german.m:144.42-48: ", rule_name, "MemData", s, VALUE_C(0), VALUE_C(1), ru_MemData));
  handle_write("german.m:144.51-67: ", rule_name, "ShrSet[i]", s, VALUE_C(0), VALUE_C(1), handle_index("german.m:144.51-59: ", rule_name, "ShrSet[i]", s, 2ull, VALUE_C(0), VALUE_C(1), ru_ShrSet, handle_read("german.m:144.58: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), VALUE_C(1));
  handle_write("german.m:145.3-16: ", rule_name, "ExGntd", s, VALUE_C(0), VALUE_C(1), ru_ExGntd, VALUE_C(1));
  handle_write("german.m:145.19-33: ", rule_name, "CurCmd", s, VALUE_C(0), VALUE_C(6), ru_CurCmd, VALUE_C(0));
  handle_zero(ru_CurPtr);
  }

  return true;
}

static int guard9(const struct state *NONNULL s __attribute__((unused)), struct handle ru_i __attribute__((unused))) {
  static const char *rule_name __attribute__((unused)) = "guard of rule \"RecvGntS\"";
  if (JMP_BUF_NEEDED) {
    if (sigsetjmp(checkpoint, 0)) {
      /* this guard triggered an error */
      return -1;
    }
  }
  const struct handle ru_Cache __attribute__((unused)) = state_handle(s, 0ull, 8ull);
  const struct handle ru_Chan1 __attribute__((unused)) = state_handle(s, 24ull, 10ull);
  const struct handle ru_Chan2 __attribute__((unused)) = state_handle(s, 34ull, 10ull);
  const struct handle ru_Chan3 __attribute__((unused)) = state_handle(s, 44ull, 10ull);
  const struct handle ru_InvSet __attribute__((unused)) = state_handle(s, 8ull, 4ull);
  const struct handle ru_ShrSet __attribute__((unused)) = state_handle(s, 12ull, 4ull);
  const struct handle ru_ExGntd __attribute__((unused)) = state_handle(s, 16ull, 2ull);
  const struct handle ru_CurCmd __attribute__((unused)) = state_handle(s, 54ull, 3ull);
  const struct handle ru_CurPtr __attribute__((unused)) = state_handle(s, 18ull, 2ull);
  const struct handle ru_MemData __attribute__((unused)) = state_handle(s, 20ull, 2ull);
  const struct handle ru_AuxData __attribute__((unused)) = state_handle(s, 22ull, 2ull);
  return (handle_read("german.m:150.3-14: ", rule_name, "Chan2[i].Cmd", s, VALUE_C(0), VALUE_C(6), handle_narrow(handle_index("german.m:150.3-10: ", rule_name, "Chan2[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan2, handle_read("german.m:150.9: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 3)) == VALUE_C(5)) ? 1 : 0;

}

static bool rule9(struct state *NONNULL s, struct handle ru_i) {
  static const char *rule_name __attribute__((unused)) = "rule \"RecvGntS\"";
  if (JMP_BUF_NEEDED) {
    if (sigsetjmp(checkpoint, 0)) {
      /* an error was triggered during this rule */
      return false;
    }
  }
  const struct handle ru_Cache __attribute__((unused)) = state_handle(s, 0ull, 8ull);
  const struct handle ru_Chan1 __attribute__((unused)) = state_handle(s, 24ull, 10ull);
  const struct handle ru_Chan2 __attribute__((unused)) = state_handle(s, 34ull, 10ull);
  const struct handle ru_Chan3 __attribute__((unused)) = state_handle(s, 44ull, 10ull);
  const struct handle ru_InvSet __attribute__((unused)) = state_handle(s, 8ull, 4ull);
  const struct handle ru_ShrSet __attribute__((unused)) = state_handle(s, 12ull, 4ull);
  const struct handle ru_ExGntd __attribute__((unused)) = state_handle(s, 16ull, 2ull);
  const struct handle ru_CurCmd __attribute__((unused)) = state_handle(s, 54ull, 3ull);
  const struct handle ru_CurPtr __attribute__((unused)) = state_handle(s, 18ull, 2ull);
  const struct handle ru_MemData __attribute__((unused)) = state_handle(s, 20ull, 2ull);
  const struct handle ru_AuxData __attribute__((unused)) = state_handle(s, 22ull, 2ull);
  {
  handle_write("german.m:152.3-21: ", rule_name, "Cache[i].State", s, VALUE_C(0), VALUE_C(2), handle_narrow(handle_index("german.m:152.3-10: ", rule_name, "Cache[i]", s, 4ull, VALUE_C(0), VALUE_C(1), ru_Cache, handle_read("german.m:152.9: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 0, 2), VALUE_C(1));
  handle_write("german.m:152.24-53: ", rule_name, "Cache[i].Data", s, VALUE_C(0), VALUE_C(1), handle_narrow(handle_index("german.m:152.24-31: ", rule_name, "Cache[i]", s, 4ull, VALUE_C(0), VALUE_C(1), ru_Cache, handle_read("german.m:152.30: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 2), handle_read("german.m:152.41-53: ", rule_name, "Chan2[i].Data", s, VALUE_C(0), VALUE_C(1), handle_narrow(handle_index("german.m:152.41-48: ", rule_name, "Chan2[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan2, handle_read("german.m:152.47: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 0, 2)));
  handle_write("german.m:153.3-23: ", rule_name, "Chan2[i].Cmd", s, VALUE_C(0), VALUE_C(6), handle_narrow(handle_index("german.m:153.3-10: ", rule_name, "Chan2[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan2, handle_read("german.m:153.9: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 3), VALUE_C(0));
  handle_zero(handle_narrow(handle_index("german.m:153.35-42: ", rule_name, "Chan2[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan2, handle_read("german.m:153.41: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 0, 2));
  }

  return true;
}

static int guard10(const struct state *NONNULL s __attribute__((unused)), struct handle ru_i __attribute__((unused))) {
  static const char *rule_name __attribute__((unused)) = "guard of rule \"RecvGntE\"";
  if (JMP_BUF_NEEDED) {
    if (sigsetjmp(checkpoint, 0)) {
      /* this guard triggered an error */
      return -1;
    }
  }
  const struct handle ru_Cache __attribute__((unused)) = state_handle(s, 0ull, 8ull);
  const struct handle ru_Chan1 __attribute__((unused)) = state_handle(s, 24ull, 10ull);
  const struct handle ru_Chan2 __attribute__((unused)) = state_handle(s, 34ull, 10ull);
  const struct handle ru_Chan3 __attribute__((unused)) = state_handle(s, 44ull, 10ull);
  const struct handle ru_InvSet __attribute__((unused)) = state_handle(s, 8ull, 4ull);
  const struct handle ru_ShrSet __attribute__((unused)) = state_handle(s, 12ull, 4ull);
  const struct handle ru_ExGntd __attribute__((unused)) = state_handle(s, 16ull, 2ull);
  const struct handle ru_CurCmd __attribute__((unused)) = state_handle(s, 54ull, 3ull);
  const struct handle ru_CurPtr __attribute__((unused)) = state_handle(s, 18ull, 2ull);
  const struct handle ru_MemData __attribute__((unused)) = state_handle(s, 20ull, 2ull);
  const struct handle ru_AuxData __attribute__((unused)) = state_handle(s, 22ull, 2ull);
  return (handle_read("german.m:158.3-14: ", rule_name, "Chan2[i].Cmd", s, VALUE_C(0), VALUE_C(6), handle_narrow(handle_index("german.m:158.3-10: ", rule_name, "Chan2[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan2, handle_read("german.m:158.9: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 3)) == VALUE_C(6)) ? 1 : 0;

}

static bool rule10(struct state *NONNULL s, struct handle ru_i) {
  static const char *rule_name __attribute__((unused)) = "rule \"RecvGntE\"";
  if (JMP_BUF_NEEDED) {
    if (sigsetjmp(checkpoint, 0)) {
      /* an error was triggered during this rule */
      return false;
    }
  }
  const struct handle ru_Cache __attribute__((unused)) = state_handle(s, 0ull, 8ull);
  const struct handle ru_Chan1 __attribute__((unused)) = state_handle(s, 24ull, 10ull);
  const struct handle ru_Chan2 __attribute__((unused)) = state_handle(s, 34ull, 10ull);
  const struct handle ru_Chan3 __attribute__((unused)) = state_handle(s, 44ull, 10ull);
  const struct handle ru_InvSet __attribute__((unused)) = state_handle(s, 8ull, 4ull);
  const struct handle ru_ShrSet __attribute__((unused)) = state_handle(s, 12ull, 4ull);
  const struct handle ru_ExGntd __attribute__((unused)) = state_handle(s, 16ull, 2ull);
  const struct handle ru_CurCmd __attribute__((unused)) = state_handle(s, 54ull, 3ull);
  const struct handle ru_CurPtr __attribute__((unused)) = state_handle(s, 18ull, 2ull);
  const struct handle ru_MemData __attribute__((unused)) = state_handle(s, 20ull, 2ull);
  const struct handle ru_AuxData __attribute__((unused)) = state_handle(s, 22ull, 2ull);
  {
  handle_write("german.m:160.3-21: ", rule_name, "Cache[i].State", s, VALUE_C(0), VALUE_C(2), handle_narrow(handle_index("german.m:160.3-10: ", rule_name, "Cache[i]", s, 4ull, VALUE_C(0), VALUE_C(1), ru_Cache, handle_read("german.m:160.9: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 0, 2), VALUE_C(2));
  handle_write("german.m:160.24-53: ", rule_name, "Cache[i].Data", s, VALUE_C(0), VALUE_C(1), handle_narrow(handle_index("german.m:160.24-31: ", rule_name, "Cache[i]", s, 4ull, VALUE_C(0), VALUE_C(1), ru_Cache, handle_read("german.m:160.30: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 2), handle_read("german.m:160.41-53: ", rule_name, "Chan2[i].Data", s, VALUE_C(0), VALUE_C(1), handle_narrow(handle_index("german.m:160.41-48: ", rule_name, "Chan2[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan2, handle_read("german.m:160.47: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 0, 2)));
  handle_write("german.m:161.3-23: ", rule_name, "Chan2[i].Cmd", s, VALUE_C(0), VALUE_C(6), handle_narrow(handle_index("german.m:161.3-10: ", rule_name, "Chan2[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan2, handle_read("german.m:161.9: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 3), VALUE_C(0));
  handle_zero(handle_narrow(handle_index("german.m:161.35-42: ", rule_name, "Chan2[i]", s, 5ull, VALUE_C(0), VALUE_C(1), ru_Chan2, handle_read("german.m:161.41: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 0, 2));
  }

  return true;
}

static int guard11(const struct state *NONNULL s __attribute__((unused)), struct handle ru_i __attribute__((unused)), struct handle ru_d __attribute__((unused))) {
  static const char *rule_name __attribute__((unused)) = "guard of rule \"Store\"";
  if (JMP_BUF_NEEDED) {
    if (sigsetjmp(checkpoint, 0)) {
      /* this guard triggered an error */
      return -1;
    }
  }
  const struct handle ru_Cache __attribute__((unused)) = state_handle(s, 0ull, 8ull);
  const struct handle ru_Chan1 __attribute__((unused)) = state_handle(s, 24ull, 10ull);
  const struct handle ru_Chan2 __attribute__((unused)) = state_handle(s, 34ull, 10ull);
  const struct handle ru_Chan3 __attribute__((unused)) = state_handle(s, 44ull, 10ull);
  const struct handle ru_InvSet __attribute__((unused)) = state_handle(s, 8ull, 4ull);
  const struct handle ru_ShrSet __attribute__((unused)) = state_handle(s, 12ull, 4ull);
  const struct handle ru_ExGntd __attribute__((unused)) = state_handle(s, 16ull, 2ull);
  const struct handle ru_CurCmd __attribute__((unused)) = state_handle(s, 54ull, 3ull);
  const struct handle ru_CurPtr __attribute__((unused)) = state_handle(s, 18ull, 2ull);
  const struct handle ru_MemData __attribute__((unused)) = state_handle(s, 20ull, 2ull);
  const struct handle ru_AuxData __attribute__((unused)) = state_handle(s, 22ull, 2ull);
  return (handle_read("german.m:166.3-16: ", rule_name, "Cache[i].State", s, VALUE_C(0), VALUE_C(2), handle_narrow(handle_index("german.m:166.3-10: ", rule_name, "Cache[i]", s, 4ull, VALUE_C(0), VALUE_C(1), ru_Cache, handle_read("german.m:166.9: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 0, 2)) == VALUE_C(2)) ? 1 : 0;

}

static bool rule11(struct state *NONNULL s, struct handle ru_i, struct handle ru_d) {
  static const char *rule_name __attribute__((unused)) = "rule \"Store\"";
  if (JMP_BUF_NEEDED) {
    if (sigsetjmp(checkpoint, 0)) {
      /* an error was triggered during this rule */
      return false;
    }
  }
  const struct handle ru_Cache __attribute__((unused)) = state_handle(s, 0ull, 8ull);
  const struct handle ru_Chan1 __attribute__((unused)) = state_handle(s, 24ull, 10ull);
  const struct handle ru_Chan2 __attribute__((unused)) = state_handle(s, 34ull, 10ull);
  const struct handle ru_Chan3 __attribute__((unused)) = state_handle(s, 44ull, 10ull);
  const struct handle ru_InvSet __attribute__((unused)) = state_handle(s, 8ull, 4ull);
  const struct handle ru_ShrSet __attribute__((unused)) = state_handle(s, 12ull, 4ull);
  const struct handle ru_ExGntd __attribute__((unused)) = state_handle(s, 16ull, 2ull);
  const struct handle ru_CurCmd __attribute__((unused)) = state_handle(s, 54ull, 3ull);
  const struct handle ru_CurPtr __attribute__((unused)) = state_handle(s, 18ull, 2ull);
  const struct handle ru_MemData __attribute__((unused)) = state_handle(s, 20ull, 2ull);
  const struct handle ru_AuxData __attribute__((unused)) = state_handle(s, 22ull, 2ull);
  {
  handle_write("german.m:168.3-20: ", rule_name, "Cache[i].Data", s, VALUE_C(0), VALUE_C(1), handle_narrow(handle_index("german.m:168.3-10: ", rule_name, "Cache[i]", s, 4ull, VALUE_C(0), VALUE_C(1), ru_Cache, handle_read("german.m:168.9: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 2), handle_read("german.m:168.20: ", rule_name, "d", s, VALUE_C(0), VALUE_C(1), ru_d));
  handle_write("german.m:168.23-34: ", rule_name, "AuxData", s, VALUE_C(0), VALUE_C(1), ru_AuxData, handle_read("german.m:168.34: ", rule_name, "d", s, VALUE_C(0), VALUE_C(1), ru_d));
  }

  return true;
}

static __attribute__((unused)) bool property0(const struct state *NONNULL s) {
  static const char *rule_name __attribute__((unused)) = "property \"CtrlProp\"";
  const struct handle ru_Cache __attribute__((unused)) = state_handle(s, 0ull, 8ull);
  const struct handle ru_Chan1 __attribute__((unused)) = state_handle(s, 24ull, 10ull);
  const struct handle ru_Chan2 __attribute__((unused)) = state_handle(s, 34ull, 10ull);
  const struct handle ru_Chan3 __attribute__((unused)) = state_handle(s, 44ull, 10ull);
  const struct handle ru_InvSet __attribute__((unused)) = state_handle(s, 8ull, 4ull);
  const struct handle ru_ShrSet __attribute__((unused)) = state_handle(s, 12ull, 4ull);
  const struct handle ru_ExGntd __attribute__((unused)) = state_handle(s, 16ull, 2ull);
  const struct handle ru_CurCmd __attribute__((unused)) = state_handle(s, 54ull, 3ull);
  const struct handle ru_CurPtr __attribute__((unused)) = state_handle(s, 18ull, 2ull);
  const struct handle ru_MemData __attribute__((unused)) = state_handle(s, 20ull, 2ull);
  const struct handle ru_AuxData __attribute__((unused)) = state_handle(s, 22ull, 2ull);
  return ({ bool result = true; {
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
if (!({ bool result = true; {
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_j = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_j == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_j - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_j - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_j += step) {
    uint8_t _ru2_j[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_j = { .base = _ru2_j, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_j, _ru1_j);
if (!(!(handle_read("german.m:177.2: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i) != handle_read("german.m:177.5: ", rule_name, "j", s, VALUE_C(0), VALUE_C(1), ru_j)) || ((!(handle_read("german.m:178.4-17: ", rule_name, "Cache[i].State", s, VALUE_C(0), VALUE_C(2), handle_narrow(handle_index("german.m:178.4-11: ", rule_name, "Cache[i]", s, 4ull, VALUE_C(0), VALUE_C(1), ru_Cache, handle_read("german.m:178.10: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 0, 2)) == VALUE_C(2)) || (handle_read("german.m:178.26-39: ", rule_name, "Cache[j].State", s, VALUE_C(0), VALUE_C(2), handle_narrow(handle_index("german.m:178.26-33: ", rule_name, "Cache[j]", s, 4ull, VALUE_C(0), VALUE_C(1), ru_Cache, handle_read("german.m:178.32: ", rule_name, "j", s, VALUE_C(0), VALUE_C(1), ru_j)), 0, 2)) == VALUE_C(0))) && (!(handle_read("german.m:179.4-17: ", rule_name, "Cache[i].State", s, VALUE_C(0), VALUE_C(2), handle_narrow(handle_index("german.m:179.4-11: ", rule_name, "Cache[i]", s, 4ull, VALUE_C(0), VALUE_C(1), ru_Cache, handle_read("german.m:179.10: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 0, 2)) == VALUE_C(1)) || ((handle_read("german.m:179.26-39: ", rule_name, "Cache[j].State", s, VALUE_C(0), VALUE_C(2), handle_narrow(handle_index("german.m:179.26-33: ", rule_name, "Cache[j]", s, 4ull, VALUE_C(0), VALUE_C(1), ru_Cache, handle_read("german.m:179.32: ", rule_name, "j", s, VALUE_C(0), VALUE_C(1), ru_j)), 0, 2)) == VALUE_C(0)) || (handle_read("german.m:179.47-60: ", rule_name, "Cache[j].State", s, VALUE_C(0), VALUE_C(2), handle_narrow(handle_index("german.m:179.47-54: ", rule_name, "Cache[j]", s, 4ull, VALUE_C(0), VALUE_C(1), ru_Cache, handle_read("german.m:179.53: ", rule_name, "j", s, VALUE_C(0), VALUE_C(1), ru_j)), 0, 2)) == VALUE_C(1))))))) { result = false; break; }    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_j > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_j - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
 result; })) { result = false; break; }    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
 result; });

}

static __attribute__((unused)) bool property1(const struct state *NONNULL s) {
  static const char *rule_name __attribute__((unused)) = "property \"DataProp\"";
  const struct handle ru_Cache __attribute__((unused)) = state_handle(s, 0ull, 8ull);
  const struct handle ru_Chan1 __attribute__((unused)) = state_handle(s, 24ull, 10ull);
  const struct handle ru_Chan2 __attribute__((unused)) = state_handle(s, 34ull, 10ull);
  const struct handle ru_Chan3 __attribute__((unused)) = state_handle(s, 44ull, 10ull);
  const struct handle ru_InvSet __attribute__((unused)) = state_handle(s, 8ull, 4ull);
  const struct handle ru_ShrSet __attribute__((unused)) = state_handle(s, 12ull, 4ull);
  const struct handle ru_ExGntd __attribute__((unused)) = state_handle(s, 16ull, 2ull);
  const struct handle ru_CurCmd __attribute__((unused)) = state_handle(s, 54ull, 3ull);
  const struct handle ru_CurPtr __attribute__((unused)) = state_handle(s, 18ull, 2ull);
  const struct handle ru_MemData __attribute__((unused)) = state_handle(s, 20ull, 2ull);
  const struct handle ru_AuxData __attribute__((unused)) = state_handle(s, 22ull, 2ull);
  return ((!(handle_read("german.m:183.3-8: ", rule_name, "ExGntd", s, VALUE_C(0), VALUE_C(1), ru_ExGntd) == VALUE_C(0)) || (handle_read("german.m:183.21-27: ", rule_name, "MemData", s, VALUE_C(0), VALUE_C(1), ru_MemData) == handle_read("german.m:183.31-37: ", rule_name, "AuxData", s, VALUE_C(0), VALUE_C(1), ru_AuxData))) && ({ bool result = true; {
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
if (!(!(handle_read("german.m:185.5-18: ", rule_name, "Cache[i].State", s, VALUE_C(0), VALUE_C(2), handle_narrow(handle_index("german.m:185.5-12: ", rule_name, "Cache[i]", s, 4ull, VALUE_C(0), VALUE_C(1), ru_Cache, handle_read("german.m:185.11: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 0, 2)) != VALUE_C(0)) || (handle_read("german.m:186.5-17: ", rule_name, "Cache[i].Data", s, VALUE_C(0), VALUE_C(1), handle_narrow(handle_index("german.m:186.5-12: ", rule_name, "Cache[i]", s, 4ull, VALUE_C(0), VALUE_C(1), ru_Cache, handle_read("german.m:186.11: ", rule_name, "i", s, VALUE_C(0), VALUE_C(1), ru_i)), 2, 2)) == handle_read("german.m:186.21-27: ", rule_name, "AuxData", s, VALUE_C(0), VALUE_C(1), ru_AuxData)))) { result = false; break; }    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
 result; }));

}

static bool check_invariants(const struct state *NONNULL s __attribute__((unused))) {
  static const char *rule_name __attribute__((unused)) = NULL;
  if (JMP_BUF_NEEDED) {
    if (sigsetjmp(checkpoint, 0)) {
      /* invariant violated */
      return false;
    }
  }
  {
    if (!property0(s)) {
      error(s, "invariant %s failed", "\"CtrlProp\"");
    }
  }
  {
    if (!property1(s)) {
      error(s, "invariant %s failed", "\"DataProp\"");
    }
  }
  return true;
}

static bool check_assumptions(const struct state *NONNULL s __attribute__((unused))) {
  static const char *rule_name __attribute__((unused)) = NULL;
  if (JMP_BUF_NEEDED) {
    if (sigsetjmp(checkpoint, 0)) {
      /* one of the properties triggered an error */
      return false;
    }
  }
  return true;
}

static bool check_covers(const struct state *NONNULL s __attribute__((unused))) {
  static const char *rule_name __attribute__((unused)) = NULL;
  if (JMP_BUF_NEEDED) {
    if (sigsetjmp(checkpoint, 0)) {
      /* one of the properties triggered an error */
      return false;
    }
  }
  return true;
}

#if LIVENESS_COUNT > 0
static bool check_liveness(struct state *NONNULL s __attribute__((unused))) {
  static const char *rule_name __attribute__((unused)) = NULL;
  if (JMP_BUF_NEEDED) {
    if (sigsetjmp(checkpoint, 0)) {
      /* one of the liveness properties triggered an error */
      return false;
    }
  }
  size_t liveness_index __attribute__((unused)) = 0;
  return true;
}

static void check_liveness_final(void) {

  static const char *rule_name __attribute__((unused)) = NULL;

  if (!MACHINE_READABLE_OUTPUT) {
    put("trying to prove remaining liveness constraints...\n");
  }

  /* find how many liveness bits are unknown */
  unsigned long remaining = 0;
  unsigned long long last_update = 0;
  unsigned long learned_since_last = 0;
  if (!MACHINE_READABLE_OUTPUT) {
    for (size_t i = 0; i < set_size(local_seen); i++) {

      slot_t slot = __atomic_load_n(&local_seen->bucket[i], __ATOMIC_SEQ_CST);

      ASSERT(!slot_is_tombstone(slot)
        && "seen set being migrated during final liveness check");

      if (slot_is_empty(slot)) {
        /* skip empty entries in the hash table */
        continue;
      }

      struct state *s = slot_to_state(slot);
      ASSERT(s != NULL && "null pointer stored in state set");

      remaining += unknown_liveness(s);
    }
    put("\t ");
    put_uint(remaining);
    put(" constraints remaining\n");
    last_update = gettime();
  }

  bool progress = true;
  while (progress) {
    progress = false;

    /* Run through all seen states trying to learn new liveness information. */
    for (size_t i = 0; i < set_size(local_seen); i++) {

      slot_t slot = __atomic_load_n(&local_seen->bucket[i], __ATOMIC_SEQ_CST);

      ASSERT(!slot_is_tombstone(slot)
        && "seen set being migrated during final liveness check");

      if (slot_is_empty(slot)) {
        /* skip empty entries in the hash table */
        continue;
      }

      struct state *s = slot_to_state(slot);
      ASSERT(s != NULL && "null pointer stored in state set");

      if (unknown_liveness(s) == 0) {
        /* skip entries where liveness is fully satisfied already */
        continue;
      }

#if BOUND > 0
      /* If we're doing bounded checking and this state is at the bound limit,
       * it's not valid to expand beyond this.
       */
      ASSERT(state_bound_get(s) <= BOUND && "a state that exceeded the bound depth was explored");
      if (state_bound_get(s) == BOUND) {
        continue;
      }
#endif

      {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
        do {
          struct state *n = state_dup(s);

          int g = guard0(n, ru_i);
          if (g == -1) {
            /* guard triggered an error */
            state_free(n);
            break;
          } else if (g == 1) {
            if (!rule0(n, ru_i)) {
              /* this rule triggered an error */
              state_free(n);
              break;
            }
            state_canonicalise(n);
            if (!check_assumptions(n)) {
              /* assumption violated */
              state_free(n);
              break;
            }

            /* note that we can skip an invariant check because we already know it
             * passed from prior expansion of this state.
             */

            /* We should be able to find this state in the seen set. */
            const struct state *t = set_find(n);
            ASSERT(t != NULL && "state encountered during final liveness wrap up "
              "that was not previously seen");

            /* See if this successor state learned a liveness property it never
             * passed back to us. This can occur if the state our exploration
             * encountered (`n`) was not the first of its kind seen and thus was
             * de-duped and never made it into the seen set with a back pointer
             * to `s`.
             */
            unsigned long learned = learn_liveness(s, t);
            if (learned > 0) {
              if (!MACHINE_READABLE_OUTPUT) {
                learned_since_last += learned;
                remaining -= learned;
                unsigned long long t = gettime();
                if (t > last_update) {
                  put("\t ");
                  put_uint(learned_since_last);
                  put(" further liveness constraints proved in ");
                  put_uint(t - last_update);
                  put("s, with ");
                  put(green()); put_uint(remaining); put(reset());
                  put(" remaining\n");
                  learned_since_last = 0;
                  last_update = t;
                }
              }
              progress = true;
            }
          }
          /* we don't need this state anymore. */
          state_free(n);
        } while (0);
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
}
      {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
        do {
          struct state *n = state_dup(s);

          int g = guard1(n, ru_i);
          if (g == -1) {
            /* guard triggered an error */
            state_free(n);
            break;
          } else if (g == 1) {
            if (!rule1(n, ru_i)) {
              /* this rule triggered an error */
              state_free(n);
              break;
            }
            state_canonicalise(n);
            if (!check_assumptions(n)) {
              /* assumption violated */
              state_free(n);
              break;
            }

            /* note that we can skip an invariant check because we already know it
             * passed from prior expansion of this state.
             */

            /* We should be able to find this state in the seen set. */
            const struct state *t = set_find(n);
            ASSERT(t != NULL && "state encountered during final liveness wrap up "
              "that was not previously seen");

            /* See if this successor state learned a liveness property it never
             * passed back to us. This can occur if the state our exploration
             * encountered (`n`) was not the first of its kind seen and thus was
             * de-duped and never made it into the seen set with a back pointer
             * to `s`.
             */
            unsigned long learned = learn_liveness(s, t);
            if (learned > 0) {
              if (!MACHINE_READABLE_OUTPUT) {
                learned_since_last += learned;
                remaining -= learned;
                unsigned long long t = gettime();
                if (t > last_update) {
                  put("\t ");
                  put_uint(learned_since_last);
                  put(" further liveness constraints proved in ");
                  put_uint(t - last_update);
                  put("s, with ");
                  put(green()); put_uint(remaining); put(reset());
                  put(" remaining\n");
                  learned_since_last = 0;
                  last_update = t;
                }
              }
              progress = true;
            }
          }
          /* we don't need this state anymore. */
          state_free(n);
        } while (0);
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
}
      {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
        do {
          struct state *n = state_dup(s);

          int g = guard2(n, ru_i);
          if (g == -1) {
            /* guard triggered an error */
            state_free(n);
            break;
          } else if (g == 1) {
            if (!rule2(n, ru_i)) {
              /* this rule triggered an error */
              state_free(n);
              break;
            }
            state_canonicalise(n);
            if (!check_assumptions(n)) {
              /* assumption violated */
              state_free(n);
              break;
            }

            /* note that we can skip an invariant check because we already know it
             * passed from prior expansion of this state.
             */

            /* We should be able to find this state in the seen set. */
            const struct state *t = set_find(n);
            ASSERT(t != NULL && "state encountered during final liveness wrap up "
              "that was not previously seen");

            /* See if this successor state learned a liveness property it never
             * passed back to us. This can occur if the state our exploration
             * encountered (`n`) was not the first of its kind seen and thus was
             * de-duped and never made it into the seen set with a back pointer
             * to `s`.
             */
            unsigned long learned = learn_liveness(s, t);
            if (learned > 0) {
              if (!MACHINE_READABLE_OUTPUT) {
                learned_since_last += learned;
                remaining -= learned;
                unsigned long long t = gettime();
                if (t > last_update) {
                  put("\t ");
                  put_uint(learned_since_last);
                  put(" further liveness constraints proved in ");
                  put_uint(t - last_update);
                  put("s, with ");
                  put(green()); put_uint(remaining); put(reset());
                  put(" remaining\n");
                  learned_since_last = 0;
                  last_update = t;
                }
              }
              progress = true;
            }
          }
          /* we don't need this state anymore. */
          state_free(n);
        } while (0);
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
}
      {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
        do {
          struct state *n = state_dup(s);

          int g = guard3(n, ru_i);
          if (g == -1) {
            /* guard triggered an error */
            state_free(n);
            break;
          } else if (g == 1) {
            if (!rule3(n, ru_i)) {
              /* this rule triggered an error */
              state_free(n);
              break;
            }
            state_canonicalise(n);
            if (!check_assumptions(n)) {
              /* assumption violated */
              state_free(n);
              break;
            }

            /* note that we can skip an invariant check because we already know it
             * passed from prior expansion of this state.
             */

            /* We should be able to find this state in the seen set. */
            const struct state *t = set_find(n);
            ASSERT(t != NULL && "state encountered during final liveness wrap up "
              "that was not previously seen");

            /* See if this successor state learned a liveness property it never
             * passed back to us. This can occur if the state our exploration
             * encountered (`n`) was not the first of its kind seen and thus was
             * de-duped and never made it into the seen set with a back pointer
             * to `s`.
             */
            unsigned long learned = learn_liveness(s, t);
            if (learned > 0) {
              if (!MACHINE_READABLE_OUTPUT) {
                learned_since_last += learned;
                remaining -= learned;
                unsigned long long t = gettime();
                if (t > last_update) {
                  put("\t ");
                  put_uint(learned_since_last);
                  put(" further liveness constraints proved in ");
                  put_uint(t - last_update);
                  put("s, with ");
                  put(green()); put_uint(remaining); put(reset());
                  put(" remaining\n");
                  learned_since_last = 0;
                  last_update = t;
                }
              }
              progress = true;
            }
          }
          /* we don't need this state anymore. */
          state_free(n);
        } while (0);
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
}
      {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
        do {
          struct state *n = state_dup(s);

          int g = guard4(n, ru_i);
          if (g == -1) {
            /* guard triggered an error */
            state_free(n);
            break;
          } else if (g == 1) {
            if (!rule4(n, ru_i)) {
              /* this rule triggered an error */
              state_free(n);
              break;
            }
            state_canonicalise(n);
            if (!check_assumptions(n)) {
              /* assumption violated */
              state_free(n);
              break;
            }

            /* note that we can skip an invariant check because we already know it
             * passed from prior expansion of this state.
             */

            /* We should be able to find this state in the seen set. */
            const struct state *t = set_find(n);
            ASSERT(t != NULL && "state encountered during final liveness wrap up "
              "that was not previously seen");

            /* See if this successor state learned a liveness property it never
             * passed back to us. This can occur if the state our exploration
             * encountered (`n`) was not the first of its kind seen and thus was
             * de-duped and never made it into the seen set with a back pointer
             * to `s`.
             */
            unsigned long learned = learn_liveness(s, t);
            if (learned > 0) {
              if (!MACHINE_READABLE_OUTPUT) {
                learned_since_last += learned;
                remaining -= learned;
                unsigned long long t = gettime();
                if (t > last_update) {
                  put("\t ");
                  put_uint(learned_since_last);
                  put(" further liveness constraints proved in ");
                  put_uint(t - last_update);
                  put("s, with ");
                  put(green()); put_uint(remaining); put(reset());
                  put(" remaining\n");
                  learned_since_last = 0;
                  last_update = t;
                }
              }
              progress = true;
            }
          }
          /* we don't need this state anymore. */
          state_free(n);
        } while (0);
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
}
      {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
        do {
          struct state *n = state_dup(s);

          int g = guard5(n, ru_i);
          if (g == -1) {
            /* guard triggered an error */
            state_free(n);
            break;
          } else if (g == 1) {
            if (!rule5(n, ru_i)) {
              /* this rule triggered an error */
              state_free(n);
              break;
            }
            state_canonicalise(n);
            if (!check_assumptions(n)) {
              /* assumption violated */
              state_free(n);
              break;
            }

            /* note that we can skip an invariant check because we already know it
             * passed from prior expansion of this state.
             */

            /* We should be able to find this state in the seen set. */
            const struct state *t = set_find(n);
            ASSERT(t != NULL && "state encountered during final liveness wrap up "
              "that was not previously seen");

            /* See if this successor state learned a liveness property it never
             * passed back to us. This can occur if the state our exploration
             * encountered (`n`) was not the first of its kind seen and thus was
             * de-duped and never made it into the seen set with a back pointer
             * to `s`.
             */
            unsigned long learned = learn_liveness(s, t);
            if (learned > 0) {
              if (!MACHINE_READABLE_OUTPUT) {
                learned_since_last += learned;
                remaining -= learned;
                unsigned long long t = gettime();
                if (t > last_update) {
                  put("\t ");
                  put_uint(learned_since_last);
                  put(" further liveness constraints proved in ");
                  put_uint(t - last_update);
                  put("s, with ");
                  put(green()); put_uint(remaining); put(reset());
                  put(" remaining\n");
                  learned_since_last = 0;
                  last_update = t;
                }
              }
              progress = true;
            }
          }
          /* we don't need this state anymore. */
          state_free(n);
        } while (0);
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
}
      {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
        do {
          struct state *n = state_dup(s);

          int g = guard6(n, ru_i);
          if (g == -1) {
            /* guard triggered an error */
            state_free(n);
            break;
          } else if (g == 1) {
            if (!rule6(n, ru_i)) {
              /* this rule triggered an error */
              state_free(n);
              break;
            }
            state_canonicalise(n);
            if (!check_assumptions(n)) {
              /* assumption violated */
              state_free(n);
              break;
            }

            /* note that we can skip an invariant check because we already know it
             * passed from prior expansion of this state.
             */

            /* We should be able to find this state in the seen set. */
            const struct state *t = set_find(n);
            ASSERT(t != NULL && "state encountered during final liveness wrap up "
              "that was not previously seen");

            /* See if this successor state learned a liveness property it never
             * passed back to us. This can occur if the state our exploration
             * encountered (`n`) was not the first of its kind seen and thus was
             * de-duped and never made it into the seen set with a back pointer
             * to `s`.
             */
            unsigned long learned = learn_liveness(s, t);
            if (learned > 0) {
              if (!MACHINE_READABLE_OUTPUT) {
                learned_since_last += learned;
                remaining -= learned;
                unsigned long long t = gettime();
                if (t > last_update) {
                  put("\t ");
                  put_uint(learned_since_last);
                  put(" further liveness constraints proved in ");
                  put_uint(t - last_update);
                  put("s, with ");
                  put(green()); put_uint(remaining); put(reset());
                  put(" remaining\n");
                  learned_since_last = 0;
                  last_update = t;
                }
              }
              progress = true;
            }
          }
          /* we don't need this state anymore. */
          state_free(n);
        } while (0);
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
}
      {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
        do {
          struct state *n = state_dup(s);

          int g = guard7(n, ru_i);
          if (g == -1) {
            /* guard triggered an error */
            state_free(n);
            break;
          } else if (g == 1) {
            if (!rule7(n, ru_i)) {
              /* this rule triggered an error */
              state_free(n);
              break;
            }
            state_canonicalise(n);
            if (!check_assumptions(n)) {
              /* assumption violated */
              state_free(n);
              break;
            }

            /* note that we can skip an invariant check because we already know it
             * passed from prior expansion of this state.
             */

            /* We should be able to find this state in the seen set. */
            const struct state *t = set_find(n);
            ASSERT(t != NULL && "state encountered during final liveness wrap up "
              "that was not previously seen");

            /* See if this successor state learned a liveness property it never
             * passed back to us. This can occur if the state our exploration
             * encountered (`n`) was not the first of its kind seen and thus was
             * de-duped and never made it into the seen set with a back pointer
             * to `s`.
             */
            unsigned long learned = learn_liveness(s, t);
            if (learned > 0) {
              if (!MACHINE_READABLE_OUTPUT) {
                learned_since_last += learned;
                remaining -= learned;
                unsigned long long t = gettime();
                if (t > last_update) {
                  put("\t ");
                  put_uint(learned_since_last);
                  put(" further liveness constraints proved in ");
                  put_uint(t - last_update);
                  put("s, with ");
                  put(green()); put_uint(remaining); put(reset());
                  put(" remaining\n");
                  learned_since_last = 0;
                  last_update = t;
                }
              }
              progress = true;
            }
          }
          /* we don't need this state anymore. */
          state_free(n);
        } while (0);
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
}
      {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
        do {
          struct state *n = state_dup(s);

          int g = guard8(n, ru_i);
          if (g == -1) {
            /* guard triggered an error */
            state_free(n);
            break;
          } else if (g == 1) {
            if (!rule8(n, ru_i)) {
              /* this rule triggered an error */
              state_free(n);
              break;
            }
            state_canonicalise(n);
            if (!check_assumptions(n)) {
              /* assumption violated */
              state_free(n);
              break;
            }

            /* note that we can skip an invariant check because we already know it
             * passed from prior expansion of this state.
             */

            /* We should be able to find this state in the seen set. */
            const struct state *t = set_find(n);
            ASSERT(t != NULL && "state encountered during final liveness wrap up "
              "that was not previously seen");

            /* See if this successor state learned a liveness property it never
             * passed back to us. This can occur if the state our exploration
             * encountered (`n`) was not the first of its kind seen and thus was
             * de-duped and never made it into the seen set with a back pointer
             * to `s`.
             */
            unsigned long learned = learn_liveness(s, t);
            if (learned > 0) {
              if (!MACHINE_READABLE_OUTPUT) {
                learned_since_last += learned;
                remaining -= learned;
                unsigned long long t = gettime();
                if (t > last_update) {
                  put("\t ");
                  put_uint(learned_since_last);
                  put(" further liveness constraints proved in ");
                  put_uint(t - last_update);
                  put("s, with ");
                  put(green()); put_uint(remaining); put(reset());
                  put(" remaining\n");
                  learned_since_last = 0;
                  last_update = t;
                }
              }
              progress = true;
            }
          }
          /* we don't need this state anymore. */
          state_free(n);
        } while (0);
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
}
      {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
        do {
          struct state *n = state_dup(s);

          int g = guard9(n, ru_i);
          if (g == -1) {
            /* guard triggered an error */
            state_free(n);
            break;
          } else if (g == 1) {
            if (!rule9(n, ru_i)) {
              /* this rule triggered an error */
              state_free(n);
              break;
            }
            state_canonicalise(n);
            if (!check_assumptions(n)) {
              /* assumption violated */
              state_free(n);
              break;
            }

            /* note that we can skip an invariant check because we already know it
             * passed from prior expansion of this state.
             */

            /* We should be able to find this state in the seen set. */
            const struct state *t = set_find(n);
            ASSERT(t != NULL && "state encountered during final liveness wrap up "
              "that was not previously seen");

            /* See if this successor state learned a liveness property it never
             * passed back to us. This can occur if the state our exploration
             * encountered (`n`) was not the first of its kind seen and thus was
             * de-duped and never made it into the seen set with a back pointer
             * to `s`.
             */
            unsigned long learned = learn_liveness(s, t);
            if (learned > 0) {
              if (!MACHINE_READABLE_OUTPUT) {
                learned_since_last += learned;
                remaining -= learned;
                unsigned long long t = gettime();
                if (t > last_update) {
                  put("\t ");
                  put_uint(learned_since_last);
                  put(" further liveness constraints proved in ");
                  put_uint(t - last_update);
                  put("s, with ");
                  put(green()); put_uint(remaining); put(reset());
                  put(" remaining\n");
                  learned_since_last = 0;
                  last_update = t;
                }
              }
              progress = true;
            }
          }
          /* we don't need this state anymore. */
          state_free(n);
        } while (0);
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
}
      {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
        do {
          struct state *n = state_dup(s);

          int g = guard10(n, ru_i);
          if (g == -1) {
            /* guard triggered an error */
            state_free(n);
            break;
          } else if (g == 1) {
            if (!rule10(n, ru_i)) {
              /* this rule triggered an error */
              state_free(n);
              break;
            }
            state_canonicalise(n);
            if (!check_assumptions(n)) {
              /* assumption violated */
              state_free(n);
              break;
            }

            /* note that we can skip an invariant check because we already know it
             * passed from prior expansion of this state.
             */

            /* We should be able to find this state in the seen set. */
            const struct state *t = set_find(n);
            ASSERT(t != NULL && "state encountered during final liveness wrap up "
              "that was not previously seen");

            /* See if this successor state learned a liveness property it never
             * passed back to us. This can occur if the state our exploration
             * encountered (`n`) was not the first of its kind seen and thus was
             * de-duped and never made it into the seen set with a back pointer
             * to `s`.
             */
            unsigned long learned = learn_liveness(s, t);
            if (learned > 0) {
              if (!MACHINE_READABLE_OUTPUT) {
                learned_since_last += learned;
                remaining -= learned;
                unsigned long long t = gettime();
                if (t > last_update) {
                  put("\t ");
                  put_uint(learned_since_last);
                  put(" further liveness constraints proved in ");
                  put_uint(t - last_update);
                  put("s, with ");
                  put(green()); put_uint(remaining); put(reset());
                  put(" remaining\n");
                  learned_since_last = 0;
                  last_update = t;
                }
              }
              progress = true;
            }
          }
          /* we don't need this state anymore. */
          state_free(n);
        } while (0);
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
}
      {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_d = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_d == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_d - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_d - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_d += step) {
    uint8_t _ru2_d[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_d = { .base = _ru2_d, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_d, _ru1_d);
        do {
          struct state *n = state_dup(s);

          int g = guard11(n, ru_i, ru_d);
          if (g == -1) {
            /* guard triggered an error */
            state_free(n);
            break;
          } else if (g == 1) {
            if (!rule11(n, ru_i, ru_d)) {
              /* this rule triggered an error */
              state_free(n);
              break;
            }
            state_canonicalise(n);
            if (!check_assumptions(n)) {
              /* assumption violated */
              state_free(n);
              break;
            }

            /* note that we can skip an invariant check because we already know it
             * passed from prior expansion of this state.
             */

            /* We should be able to find this state in the seen set. */
            const struct state *t = set_find(n);
            ASSERT(t != NULL && "state encountered during final liveness wrap up "
              "that was not previously seen");

            /* See if this successor state learned a liveness property it never
             * passed back to us. This can occur if the state our exploration
             * encountered (`n`) was not the first of its kind seen and thus was
             * de-duped and never made it into the seen set with a back pointer
             * to `s`.
             */
            unsigned long learned = learn_liveness(s, t);
            if (learned > 0) {
              if (!MACHINE_READABLE_OUTPUT) {
                learned_since_last += learned;
                remaining -= learned;
                unsigned long long t = gettime();
                if (t > last_update) {
                  put("\t ");
                  put_uint(learned_since_last);
                  put(" further liveness constraints proved in ");
                  put_uint(t - last_update);
                  put("s, with ");
                  put(green()); put_uint(remaining); put(reset());
                  put(" remaining\n");
                  learned_since_last = 0;
                  last_update = t;
                }
              }
              progress = true;
            }
          }
          /* we don't need this state anymore. */
          state_free(n);
        } while (0);
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_d > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_d - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
}
    }
  }
}


static unsigned long check_liveness_summarise(void) {

  /* We can now finally check whether all liveness properties were hit. */
  bool missed[LIVENESS_COUNT];
  memset(missed, 0, sizeof(missed));
  for (size_t i = 0; i < set_size(local_seen); i++) {

    slot_t slot = __atomic_load_n(&local_seen->bucket[i], __ATOMIC_SEQ_CST);

    ASSERT(!slot_is_tombstone(slot)
      && "seen set being migrated during final liveness check");

    if (slot_is_empty(slot)) {
      /* skip empty entries in the hash table */
      continue;
    }

    const struct state *s = slot_to_state(slot);
    ASSERT(s != NULL && "null pointer stored in state set");

    size_t index __attribute__((unused)) = 0;
  }

  /* total up how many misses we saw */
  unsigned long total = 0;
  for (size_t i = 0; i < sizeof(missed) / sizeof(missed[0]); i++) {
    if (missed[i]) {
      total++;
    }
  }

  return total;
}
#endif

static void init(void) {
  static const char *rule_name __attribute__((unused)) = NULL;
  size_t queue_id = 0;
  uint64_t rule_taken = 1;
  {
    struct state *s = NULL;
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_d = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_d == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_d - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_d - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_d += step) {
    uint8_t _ru2_d[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_d = { .base = _ru2_d, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_d, _ru1_d);
    do {
      s = state_new();
      memset(s, 0, sizeof(*s));
#if COUNTEREXAMPLE_TRACE != CEX_OFF
      state_rule_taken_set(s, rule_taken);
#endif
      if (!startstate0(s, ru_d)) {
        /* startstate triggered an error */
        state_free(s);
        break;
      }
      state_canonicalise(s);
      if (!check_assumptions(s)) {
        /* assumption violated */
        state_free(s);
        break;
      }
      if (!check_invariants(s)) {
        /* invariant violated */
        state_free(s);
        break;
      }
      size_t size;
      if (set_insert(s, &size)) {
        if (!check_covers(s)) {
          /* one of the cover properties triggered an error */
          break;
        }
#if LIVENESS_COUNT > 0
        if (!check_liveness(s)) {
          /* one of the liveness properties triggered an error */
          break;
        }
#endif
        (void)queue_enqueue(s, queue_id);
        queue_id = (queue_id + 1) % (sizeof(q) / sizeof(q[0]));
      } else {
        state_free(s);
      }
    } while (0);
    rule_taken++;
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_d > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_d - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
  }
}

static void explore(void) {

  /* Used when writing to quantifier variables. */
  static const char *rule_name __attribute__((unused)) = NULL;

  size_t last_queue_size = 0;

  /* Identifier of the last queue we interacted with. */
  size_t queue_id = thread_id;

  for (;;) {

    if (THREADS > 1 && __atomic_load_n(&error_count,
        __ATOMIC_SEQ_CST) >= MAX_ERRORS) {
      /* Another thread found an error. */
      break;
    }

    const struct state *s = queue_dequeue(&queue_id);
    if (s == NULL) {
      break;
    }

    bool possible_deadlock = true;
    uint64_t rule_taken = 1;
    {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
      do {
        struct state *n = state_dup(s);
#if COUNTEREXAMPLE_TRACE != CEX_OFF
        state_rule_taken_set(n, rule_taken);
#endif
        int g = guard0(n, ru_i);
        if (g == -1) {
          /* error() was called */
          state_free(n);
          break;
        } else if (g == 1) {
          if (!rule0(n, ru_i)) {
            /* this rule triggered an error */
            state_free(n);
            break;
          }
          rules_fired_local++;
          if (DEADLOCK_DETECTION != DEADLOCK_DETECTION_STUTTERING || !state_eq(s, n)) {
            possible_deadlock = false;
          }
          state_canonicalise(n);
          if (!check_assumptions(n)) {
            /* assumption violated */
            state_free(n);
            break;
          }
          if (!check_invariants(n)) {
            /* invariant violated */
            state_free(n);
            break;
          }
          size_t size;
          if (set_insert(n, &size)) {

            if (!check_covers(n)) {
              /* one of the cover properties triggered an error */
              break;
            }
#if LIVENESS_COUNT > 0
            if (!check_liveness(n)) {
              /* one of the liveness properties triggered an error */
              break;
            }
#endif

#if BOUND > 0
            if (state_bound_get(n) < BOUND) {
#endif
            size_t queue_size = queue_enqueue(n, thread_id);
            queue_id = thread_id;

            if (size % 10000 == 0 && ftrylockfile(stdout) == 0) {
              if (MACHINE_READABLE_OUTPUT) {
                put("<progress states=\"");
                put_uint(size);
                put("\" duration_seconds=\"");
                put_uint(gettime());
                put("\" rules_fired=\"");
                put_uint(rules_fired_local);
                put("\" queue_size=\"");
                put_uint(queue_size);
                put("\" thread_id=\"");
                put_uint(thread_id);
                put("\"/>\n");
              } else {
                put("\t ");
                if (THREADS > 1) {
                  put("thread ");
                  put_uint(thread_id);
                  put(": ");
                }
                put_uint(size);
                put(" states explored in ");
                put_uint(gettime());
                put("s, with ");
                put_uint(rules_fired_local);
                put(" rules fired and ");
                put(queue_size > last_queue_size ? yellow() : green());
                put_uint(queue_size);
                put(reset());
                put(" states in the queue.\n");
              }
              funlockfile(stdout);
              last_queue_size = queue_size;
            }

            if (THREADS > 1 && thread_id == 0 && phase == WARMUP && queue_size > 20) {
              start_secondary_threads();
              phase = RUN;
            }

#if BOUND > 0
            }
#endif
          } else {
            state_free(n);
          }
        } else {
          state_free(n);
        }
      } while (0);
      rule_taken++;
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
}
    {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
      do {
        struct state *n = state_dup(s);
#if COUNTEREXAMPLE_TRACE != CEX_OFF
        state_rule_taken_set(n, rule_taken);
#endif
        int g = guard1(n, ru_i);
        if (g == -1) {
          /* error() was called */
          state_free(n);
          break;
        } else if (g == 1) {
          if (!rule1(n, ru_i)) {
            /* this rule triggered an error */
            state_free(n);
            break;
          }
          rules_fired_local++;
          if (DEADLOCK_DETECTION != DEADLOCK_DETECTION_STUTTERING || !state_eq(s, n)) {
            possible_deadlock = false;
          }
          state_canonicalise(n);
          if (!check_assumptions(n)) {
            /* assumption violated */
            state_free(n);
            break;
          }
          if (!check_invariants(n)) {
            /* invariant violated */
            state_free(n);
            break;
          }
          size_t size;
          if (set_insert(n, &size)) {

            if (!check_covers(n)) {
              /* one of the cover properties triggered an error */
              break;
            }
#if LIVENESS_COUNT > 0
            if (!check_liveness(n)) {
              /* one of the liveness properties triggered an error */
              break;
            }
#endif

#if BOUND > 0
            if (state_bound_get(n) < BOUND) {
#endif
            size_t queue_size = queue_enqueue(n, thread_id);
            queue_id = thread_id;

            if (size % 10000 == 0 && ftrylockfile(stdout) == 0) {
              if (MACHINE_READABLE_OUTPUT) {
                put("<progress states=\"");
                put_uint(size);
                put("\" duration_seconds=\"");
                put_uint(gettime());
                put("\" rules_fired=\"");
                put_uint(rules_fired_local);
                put("\" queue_size=\"");
                put_uint(queue_size);
                put("\" thread_id=\"");
                put_uint(thread_id);
                put("\"/>\n");
              } else {
                put("\t ");
                if (THREADS > 1) {
                  put("thread ");
                  put_uint(thread_id);
                  put(": ");
                }
                put_uint(size);
                put(" states explored in ");
                put_uint(gettime());
                put("s, with ");
                put_uint(rules_fired_local);
                put(" rules fired and ");
                put(queue_size > last_queue_size ? yellow() : green());
                put_uint(queue_size);
                put(reset());
                put(" states in the queue.\n");
              }
              funlockfile(stdout);
              last_queue_size = queue_size;
            }

            if (THREADS > 1 && thread_id == 0 && phase == WARMUP && queue_size > 20) {
              start_secondary_threads();
              phase = RUN;
            }

#if BOUND > 0
            }
#endif
          } else {
            state_free(n);
          }
        } else {
          state_free(n);
        }
      } while (0);
      rule_taken++;
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
}
    {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
      do {
        struct state *n = state_dup(s);
#if COUNTEREXAMPLE_TRACE != CEX_OFF
        state_rule_taken_set(n, rule_taken);
#endif
        int g = guard2(n, ru_i);
        if (g == -1) {
          /* error() was called */
          state_free(n);
          break;
        } else if (g == 1) {
          if (!rule2(n, ru_i)) {
            /* this rule triggered an error */
            state_free(n);
            break;
          }
          rules_fired_local++;
          if (DEADLOCK_DETECTION != DEADLOCK_DETECTION_STUTTERING || !state_eq(s, n)) {
            possible_deadlock = false;
          }
          state_canonicalise(n);
          if (!check_assumptions(n)) {
            /* assumption violated */
            state_free(n);
            break;
          }
          if (!check_invariants(n)) {
            /* invariant violated */
            state_free(n);
            break;
          }
          size_t size;
          if (set_insert(n, &size)) {

            if (!check_covers(n)) {
              /* one of the cover properties triggered an error */
              break;
            }
#if LIVENESS_COUNT > 0
            if (!check_liveness(n)) {
              /* one of the liveness properties triggered an error */
              break;
            }
#endif

#if BOUND > 0
            if (state_bound_get(n) < BOUND) {
#endif
            size_t queue_size = queue_enqueue(n, thread_id);
            queue_id = thread_id;

            if (size % 10000 == 0 && ftrylockfile(stdout) == 0) {
              if (MACHINE_READABLE_OUTPUT) {
                put("<progress states=\"");
                put_uint(size);
                put("\" duration_seconds=\"");
                put_uint(gettime());
                put("\" rules_fired=\"");
                put_uint(rules_fired_local);
                put("\" queue_size=\"");
                put_uint(queue_size);
                put("\" thread_id=\"");
                put_uint(thread_id);
                put("\"/>\n");
              } else {
                put("\t ");
                if (THREADS > 1) {
                  put("thread ");
                  put_uint(thread_id);
                  put(": ");
                }
                put_uint(size);
                put(" states explored in ");
                put_uint(gettime());
                put("s, with ");
                put_uint(rules_fired_local);
                put(" rules fired and ");
                put(queue_size > last_queue_size ? yellow() : green());
                put_uint(queue_size);
                put(reset());
                put(" states in the queue.\n");
              }
              funlockfile(stdout);
              last_queue_size = queue_size;
            }

            if (THREADS > 1 && thread_id == 0 && phase == WARMUP && queue_size > 20) {
              start_secondary_threads();
              phase = RUN;
            }

#if BOUND > 0
            }
#endif
          } else {
            state_free(n);
          }
        } else {
          state_free(n);
        }
      } while (0);
      rule_taken++;
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
}
    {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
      do {
        struct state *n = state_dup(s);
#if COUNTEREXAMPLE_TRACE != CEX_OFF
        state_rule_taken_set(n, rule_taken);
#endif
        int g = guard3(n, ru_i);
        if (g == -1) {
          /* error() was called */
          state_free(n);
          break;
        } else if (g == 1) {
          if (!rule3(n, ru_i)) {
            /* this rule triggered an error */
            state_free(n);
            break;
          }
          rules_fired_local++;
          if (DEADLOCK_DETECTION != DEADLOCK_DETECTION_STUTTERING || !state_eq(s, n)) {
            possible_deadlock = false;
          }
          state_canonicalise(n);
          if (!check_assumptions(n)) {
            /* assumption violated */
            state_free(n);
            break;
          }
          if (!check_invariants(n)) {
            /* invariant violated */
            state_free(n);
            break;
          }
          size_t size;
          if (set_insert(n, &size)) {

            if (!check_covers(n)) {
              /* one of the cover properties triggered an error */
              break;
            }
#if LIVENESS_COUNT > 0
            if (!check_liveness(n)) {
              /* one of the liveness properties triggered an error */
              break;
            }
#endif

#if BOUND > 0
            if (state_bound_get(n) < BOUND) {
#endif
            size_t queue_size = queue_enqueue(n, thread_id);
            queue_id = thread_id;

            if (size % 10000 == 0 && ftrylockfile(stdout) == 0) {
              if (MACHINE_READABLE_OUTPUT) {
                put("<progress states=\"");
                put_uint(size);
                put("\" duration_seconds=\"");
                put_uint(gettime());
                put("\" rules_fired=\"");
                put_uint(rules_fired_local);
                put("\" queue_size=\"");
                put_uint(queue_size);
                put("\" thread_id=\"");
                put_uint(thread_id);
                put("\"/>\n");
              } else {
                put("\t ");
                if (THREADS > 1) {
                  put("thread ");
                  put_uint(thread_id);
                  put(": ");
                }
                put_uint(size);
                put(" states explored in ");
                put_uint(gettime());
                put("s, with ");
                put_uint(rules_fired_local);
                put(" rules fired and ");
                put(queue_size > last_queue_size ? yellow() : green());
                put_uint(queue_size);
                put(reset());
                put(" states in the queue.\n");
              }
              funlockfile(stdout);
              last_queue_size = queue_size;
            }

            if (THREADS > 1 && thread_id == 0 && phase == WARMUP && queue_size > 20) {
              start_secondary_threads();
              phase = RUN;
            }

#if BOUND > 0
            }
#endif
          } else {
            state_free(n);
          }
        } else {
          state_free(n);
        }
      } while (0);
      rule_taken++;
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
}
    {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
      do {
        struct state *n = state_dup(s);
#if COUNTEREXAMPLE_TRACE != CEX_OFF
        state_rule_taken_set(n, rule_taken);
#endif
        int g = guard4(n, ru_i);
        if (g == -1) {
          /* error() was called */
          state_free(n);
          break;
        } else if (g == 1) {
          if (!rule4(n, ru_i)) {
            /* this rule triggered an error */
            state_free(n);
            break;
          }
          rules_fired_local++;
          if (DEADLOCK_DETECTION != DEADLOCK_DETECTION_STUTTERING || !state_eq(s, n)) {
            possible_deadlock = false;
          }
          state_canonicalise(n);
          if (!check_assumptions(n)) {
            /* assumption violated */
            state_free(n);
            break;
          }
          if (!check_invariants(n)) {
            /* invariant violated */
            state_free(n);
            break;
          }
          size_t size;
          if (set_insert(n, &size)) {

            if (!check_covers(n)) {
              /* one of the cover properties triggered an error */
              break;
            }
#if LIVENESS_COUNT > 0
            if (!check_liveness(n)) {
              /* one of the liveness properties triggered an error */
              break;
            }
#endif

#if BOUND > 0
            if (state_bound_get(n) < BOUND) {
#endif
            size_t queue_size = queue_enqueue(n, thread_id);
            queue_id = thread_id;

            if (size % 10000 == 0 && ftrylockfile(stdout) == 0) {
              if (MACHINE_READABLE_OUTPUT) {
                put("<progress states=\"");
                put_uint(size);
                put("\" duration_seconds=\"");
                put_uint(gettime());
                put("\" rules_fired=\"");
                put_uint(rules_fired_local);
                put("\" queue_size=\"");
                put_uint(queue_size);
                put("\" thread_id=\"");
                put_uint(thread_id);
                put("\"/>\n");
              } else {
                put("\t ");
                if (THREADS > 1) {
                  put("thread ");
                  put_uint(thread_id);
                  put(": ");
                }
                put_uint(size);
                put(" states explored in ");
                put_uint(gettime());
                put("s, with ");
                put_uint(rules_fired_local);
                put(" rules fired and ");
                put(queue_size > last_queue_size ? yellow() : green());
                put_uint(queue_size);
                put(reset());
                put(" states in the queue.\n");
              }
              funlockfile(stdout);
              last_queue_size = queue_size;
            }

            if (THREADS > 1 && thread_id == 0 && phase == WARMUP && queue_size > 20) {
              start_secondary_threads();
              phase = RUN;
            }

#if BOUND > 0
            }
#endif
          } else {
            state_free(n);
          }
        } else {
          state_free(n);
        }
      } while (0);
      rule_taken++;
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
}
    {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
      do {
        struct state *n = state_dup(s);
#if COUNTEREXAMPLE_TRACE != CEX_OFF
        state_rule_taken_set(n, rule_taken);
#endif
        int g = guard5(n, ru_i);
        if (g == -1) {
          /* error() was called */
          state_free(n);
          break;
        } else if (g == 1) {
          if (!rule5(n, ru_i)) {
            /* this rule triggered an error */
            state_free(n);
            break;
          }
          rules_fired_local++;
          if (DEADLOCK_DETECTION != DEADLOCK_DETECTION_STUTTERING || !state_eq(s, n)) {
            possible_deadlock = false;
          }
          state_canonicalise(n);
          if (!check_assumptions(n)) {
            /* assumption violated */
            state_free(n);
            break;
          }
          if (!check_invariants(n)) {
            /* invariant violated */
            state_free(n);
            break;
          }
          size_t size;
          if (set_insert(n, &size)) {

            if (!check_covers(n)) {
              /* one of the cover properties triggered an error */
              break;
            }
#if LIVENESS_COUNT > 0
            if (!check_liveness(n)) {
              /* one of the liveness properties triggered an error */
              break;
            }
#endif

#if BOUND > 0
            if (state_bound_get(n) < BOUND) {
#endif
            size_t queue_size = queue_enqueue(n, thread_id);
            queue_id = thread_id;

            if (size % 10000 == 0 && ftrylockfile(stdout) == 0) {
              if (MACHINE_READABLE_OUTPUT) {
                put("<progress states=\"");
                put_uint(size);
                put("\" duration_seconds=\"");
                put_uint(gettime());
                put("\" rules_fired=\"");
                put_uint(rules_fired_local);
                put("\" queue_size=\"");
                put_uint(queue_size);
                put("\" thread_id=\"");
                put_uint(thread_id);
                put("\"/>\n");
              } else {
                put("\t ");
                if (THREADS > 1) {
                  put("thread ");
                  put_uint(thread_id);
                  put(": ");
                }
                put_uint(size);
                put(" states explored in ");
                put_uint(gettime());
                put("s, with ");
                put_uint(rules_fired_local);
                put(" rules fired and ");
                put(queue_size > last_queue_size ? yellow() : green());
                put_uint(queue_size);
                put(reset());
                put(" states in the queue.\n");
              }
              funlockfile(stdout);
              last_queue_size = queue_size;
            }

            if (THREADS > 1 && thread_id == 0 && phase == WARMUP && queue_size > 20) {
              start_secondary_threads();
              phase = RUN;
            }

#if BOUND > 0
            }
#endif
          } else {
            state_free(n);
          }
        } else {
          state_free(n);
        }
      } while (0);
      rule_taken++;
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
}
    {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
      do {
        struct state *n = state_dup(s);
#if COUNTEREXAMPLE_TRACE != CEX_OFF
        state_rule_taken_set(n, rule_taken);
#endif
        int g = guard6(n, ru_i);
        if (g == -1) {
          /* error() was called */
          state_free(n);
          break;
        } else if (g == 1) {
          if (!rule6(n, ru_i)) {
            /* this rule triggered an error */
            state_free(n);
            break;
          }
          rules_fired_local++;
          if (DEADLOCK_DETECTION != DEADLOCK_DETECTION_STUTTERING || !state_eq(s, n)) {
            possible_deadlock = false;
          }
          state_canonicalise(n);
          if (!check_assumptions(n)) {
            /* assumption violated */
            state_free(n);
            break;
          }
          if (!check_invariants(n)) {
            /* invariant violated */
            state_free(n);
            break;
          }
          size_t size;
          if (set_insert(n, &size)) {

            if (!check_covers(n)) {
              /* one of the cover properties triggered an error */
              break;
            }
#if LIVENESS_COUNT > 0
            if (!check_liveness(n)) {
              /* one of the liveness properties triggered an error */
              break;
            }
#endif

#if BOUND > 0
            if (state_bound_get(n) < BOUND) {
#endif
            size_t queue_size = queue_enqueue(n, thread_id);
            queue_id = thread_id;

            if (size % 10000 == 0 && ftrylockfile(stdout) == 0) {
              if (MACHINE_READABLE_OUTPUT) {
                put("<progress states=\"");
                put_uint(size);
                put("\" duration_seconds=\"");
                put_uint(gettime());
                put("\" rules_fired=\"");
                put_uint(rules_fired_local);
                put("\" queue_size=\"");
                put_uint(queue_size);
                put("\" thread_id=\"");
                put_uint(thread_id);
                put("\"/>\n");
              } else {
                put("\t ");
                if (THREADS > 1) {
                  put("thread ");
                  put_uint(thread_id);
                  put(": ");
                }
                put_uint(size);
                put(" states explored in ");
                put_uint(gettime());
                put("s, with ");
                put_uint(rules_fired_local);
                put(" rules fired and ");
                put(queue_size > last_queue_size ? yellow() : green());
                put_uint(queue_size);
                put(reset());
                put(" states in the queue.\n");
              }
              funlockfile(stdout);
              last_queue_size = queue_size;
            }

            if (THREADS > 1 && thread_id == 0 && phase == WARMUP && queue_size > 20) {
              start_secondary_threads();
              phase = RUN;
            }

#if BOUND > 0
            }
#endif
          } else {
            state_free(n);
          }
        } else {
          state_free(n);
        }
      } while (0);
      rule_taken++;
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
}
    {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
      do {
        struct state *n = state_dup(s);
#if COUNTEREXAMPLE_TRACE != CEX_OFF
        state_rule_taken_set(n, rule_taken);
#endif
        int g = guard7(n, ru_i);
        if (g == -1) {
          /* error() was called */
          state_free(n);
          break;
        } else if (g == 1) {
          if (!rule7(n, ru_i)) {
            /* this rule triggered an error */
            state_free(n);
            break;
          }
          rules_fired_local++;
          if (DEADLOCK_DETECTION != DEADLOCK_DETECTION_STUTTERING || !state_eq(s, n)) {
            possible_deadlock = false;
          }
          state_canonicalise(n);
          if (!check_assumptions(n)) {
            /* assumption violated */
            state_free(n);
            break;
          }
          if (!check_invariants(n)) {
            /* invariant violated */
            state_free(n);
            break;
          }
          size_t size;
          if (set_insert(n, &size)) {

            if (!check_covers(n)) {
              /* one of the cover properties triggered an error */
              break;
            }
#if LIVENESS_COUNT > 0
            if (!check_liveness(n)) {
              /* one of the liveness properties triggered an error */
              break;
            }
#endif

#if BOUND > 0
            if (state_bound_get(n) < BOUND) {
#endif
            size_t queue_size = queue_enqueue(n, thread_id);
            queue_id = thread_id;

            if (size % 10000 == 0 && ftrylockfile(stdout) == 0) {
              if (MACHINE_READABLE_OUTPUT) {
                put("<progress states=\"");
                put_uint(size);
                put("\" duration_seconds=\"");
                put_uint(gettime());
                put("\" rules_fired=\"");
                put_uint(rules_fired_local);
                put("\" queue_size=\"");
                put_uint(queue_size);
                put("\" thread_id=\"");
                put_uint(thread_id);
                put("\"/>\n");
              } else {
                put("\t ");
                if (THREADS > 1) {
                  put("thread ");
                  put_uint(thread_id);
                  put(": ");
                }
                put_uint(size);
                put(" states explored in ");
                put_uint(gettime());
                put("s, with ");
                put_uint(rules_fired_local);
                put(" rules fired and ");
                put(queue_size > last_queue_size ? yellow() : green());
                put_uint(queue_size);
                put(reset());
                put(" states in the queue.\n");
              }
              funlockfile(stdout);
              last_queue_size = queue_size;
            }

            if (THREADS > 1 && thread_id == 0 && phase == WARMUP && queue_size > 20) {
              start_secondary_threads();
              phase = RUN;
            }

#if BOUND > 0
            }
#endif
          } else {
            state_free(n);
          }
        } else {
          state_free(n);
        }
      } while (0);
      rule_taken++;
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
}
    {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
      do {
        struct state *n = state_dup(s);
#if COUNTEREXAMPLE_TRACE != CEX_OFF
        state_rule_taken_set(n, rule_taken);
#endif
        int g = guard8(n, ru_i);
        if (g == -1) {
          /* error() was called */
          state_free(n);
          break;
        } else if (g == 1) {
          if (!rule8(n, ru_i)) {
            /* this rule triggered an error */
            state_free(n);
            break;
          }
          rules_fired_local++;
          if (DEADLOCK_DETECTION != DEADLOCK_DETECTION_STUTTERING || !state_eq(s, n)) {
            possible_deadlock = false;
          }
          state_canonicalise(n);
          if (!check_assumptions(n)) {
            /* assumption violated */
            state_free(n);
            break;
          }
          if (!check_invariants(n)) {
            /* invariant violated */
            state_free(n);
            break;
          }
          size_t size;
          if (set_insert(n, &size)) {

            if (!check_covers(n)) {
              /* one of the cover properties triggered an error */
              break;
            }
#if LIVENESS_COUNT > 0
            if (!check_liveness(n)) {
              /* one of the liveness properties triggered an error */
              break;
            }
#endif

#if BOUND > 0
            if (state_bound_get(n) < BOUND) {
#endif
            size_t queue_size = queue_enqueue(n, thread_id);
            queue_id = thread_id;

            if (size % 10000 == 0 && ftrylockfile(stdout) == 0) {
              if (MACHINE_READABLE_OUTPUT) {
                put("<progress states=\"");
                put_uint(size);
                put("\" duration_seconds=\"");
                put_uint(gettime());
                put("\" rules_fired=\"");
                put_uint(rules_fired_local);
                put("\" queue_size=\"");
                put_uint(queue_size);
                put("\" thread_id=\"");
                put_uint(thread_id);
                put("\"/>\n");
              } else {
                put("\t ");
                if (THREADS > 1) {
                  put("thread ");
                  put_uint(thread_id);
                  put(": ");
                }
                put_uint(size);
                put(" states explored in ");
                put_uint(gettime());
                put("s, with ");
                put_uint(rules_fired_local);
                put(" rules fired and ");
                put(queue_size > last_queue_size ? yellow() : green());
                put_uint(queue_size);
                put(reset());
                put(" states in the queue.\n");
              }
              funlockfile(stdout);
              last_queue_size = queue_size;
            }

            if (THREADS > 1 && thread_id == 0 && phase == WARMUP && queue_size > 20) {
              start_secondary_threads();
              phase = RUN;
            }

#if BOUND > 0
            }
#endif
          } else {
            state_free(n);
          }
        } else {
          state_free(n);
        }
      } while (0);
      rule_taken++;
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
}
    {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
      do {
        struct state *n = state_dup(s);
#if COUNTEREXAMPLE_TRACE != CEX_OFF
        state_rule_taken_set(n, rule_taken);
#endif
        int g = guard9(n, ru_i);
        if (g == -1) {
          /* error() was called */
          state_free(n);
          break;
        } else if (g == 1) {
          if (!rule9(n, ru_i)) {
            /* this rule triggered an error */
            state_free(n);
            break;
          }
          rules_fired_local++;
          if (DEADLOCK_DETECTION != DEADLOCK_DETECTION_STUTTERING || !state_eq(s, n)) {
            possible_deadlock = false;
          }
          state_canonicalise(n);
          if (!check_assumptions(n)) {
            /* assumption violated */
            state_free(n);
            break;
          }
          if (!check_invariants(n)) {
            /* invariant violated */
            state_free(n);
            break;
          }
          size_t size;
          if (set_insert(n, &size)) {

            if (!check_covers(n)) {
              /* one of the cover properties triggered an error */
              break;
            }
#if LIVENESS_COUNT > 0
            if (!check_liveness(n)) {
              /* one of the liveness properties triggered an error */
              break;
            }
#endif

#if BOUND > 0
            if (state_bound_get(n) < BOUND) {
#endif
            size_t queue_size = queue_enqueue(n, thread_id);
            queue_id = thread_id;

            if (size % 10000 == 0 && ftrylockfile(stdout) == 0) {
              if (MACHINE_READABLE_OUTPUT) {
                put("<progress states=\"");
                put_uint(size);
                put("\" duration_seconds=\"");
                put_uint(gettime());
                put("\" rules_fired=\"");
                put_uint(rules_fired_local);
                put("\" queue_size=\"");
                put_uint(queue_size);
                put("\" thread_id=\"");
                put_uint(thread_id);
                put("\"/>\n");
              } else {
                put("\t ");
                if (THREADS > 1) {
                  put("thread ");
                  put_uint(thread_id);
                  put(": ");
                }
                put_uint(size);
                put(" states explored in ");
                put_uint(gettime());
                put("s, with ");
                put_uint(rules_fired_local);
                put(" rules fired and ");
                put(queue_size > last_queue_size ? yellow() : green());
                put_uint(queue_size);
                put(reset());
                put(" states in the queue.\n");
              }
              funlockfile(stdout);
              last_queue_size = queue_size;
            }

            if (THREADS > 1 && thread_id == 0 && phase == WARMUP && queue_size > 20) {
              start_secondary_threads();
              phase = RUN;
            }

#if BOUND > 0
            }
#endif
          } else {
            state_free(n);
          }
        } else {
          state_free(n);
        }
      } while (0);
      rule_taken++;
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
}
    {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
      do {
        struct state *n = state_dup(s);
#if COUNTEREXAMPLE_TRACE != CEX_OFF
        state_rule_taken_set(n, rule_taken);
#endif
        int g = guard10(n, ru_i);
        if (g == -1) {
          /* error() was called */
          state_free(n);
          break;
        } else if (g == 1) {
          if (!rule10(n, ru_i)) {
            /* this rule triggered an error */
            state_free(n);
            break;
          }
          rules_fired_local++;
          if (DEADLOCK_DETECTION != DEADLOCK_DETECTION_STUTTERING || !state_eq(s, n)) {
            possible_deadlock = false;
          }
          state_canonicalise(n);
          if (!check_assumptions(n)) {
            /* assumption violated */
            state_free(n);
            break;
          }
          if (!check_invariants(n)) {
            /* invariant violated */
            state_free(n);
            break;
          }
          size_t size;
          if (set_insert(n, &size)) {

            if (!check_covers(n)) {
              /* one of the cover properties triggered an error */
              break;
            }
#if LIVENESS_COUNT > 0
            if (!check_liveness(n)) {
              /* one of the liveness properties triggered an error */
              break;
            }
#endif

#if BOUND > 0
            if (state_bound_get(n) < BOUND) {
#endif
            size_t queue_size = queue_enqueue(n, thread_id);
            queue_id = thread_id;

            if (size % 10000 == 0 && ftrylockfile(stdout) == 0) {
              if (MACHINE_READABLE_OUTPUT) {
                put("<progress states=\"");
                put_uint(size);
                put("\" duration_seconds=\"");
                put_uint(gettime());
                put("\" rules_fired=\"");
                put_uint(rules_fired_local);
                put("\" queue_size=\"");
                put_uint(queue_size);
                put("\" thread_id=\"");
                put_uint(thread_id);
                put("\"/>\n");
              } else {
                put("\t ");
                if (THREADS > 1) {
                  put("thread ");
                  put_uint(thread_id);
                  put(": ");
                }
                put_uint(size);
                put(" states explored in ");
                put_uint(gettime());
                put("s, with ");
                put_uint(rules_fired_local);
                put(" rules fired and ");
                put(queue_size > last_queue_size ? yellow() : green());
                put_uint(queue_size);
                put(reset());
                put(" states in the queue.\n");
              }
              funlockfile(stdout);
              last_queue_size = queue_size;
            }

            if (THREADS > 1 && thread_id == 0 && phase == WARMUP && queue_size > 20) {
              start_secondary_threads();
              phase = RUN;
            }

#if BOUND > 0
            }
#endif
          } else {
            state_free(n);
          }
        } else {
          state_free(n);
        }
      } while (0);
      rule_taken++;
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
}
    {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_d = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_d == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_d - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_d - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_d += step) {
    uint8_t _ru2_d[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_d = { .base = _ru2_d, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_d, _ru1_d);
      do {
        struct state *n = state_dup(s);
#if COUNTEREXAMPLE_TRACE != CEX_OFF
        state_rule_taken_set(n, rule_taken);
#endif
        int g = guard11(n, ru_i, ru_d);
        if (g == -1) {
          /* error() was called */
          state_free(n);
          break;
        } else if (g == 1) {
          if (!rule11(n, ru_i, ru_d)) {
            /* this rule triggered an error */
            state_free(n);
            break;
          }
          rules_fired_local++;
          if (DEADLOCK_DETECTION != DEADLOCK_DETECTION_STUTTERING || !state_eq(s, n)) {
            possible_deadlock = false;
          }
          state_canonicalise(n);
          if (!check_assumptions(n)) {
            /* assumption violated */
            state_free(n);
            break;
          }
          if (!check_invariants(n)) {
            /* invariant violated */
            state_free(n);
            break;
          }
          size_t size;
          if (set_insert(n, &size)) {

            if (!check_covers(n)) {
              /* one of the cover properties triggered an error */
              break;
            }
#if LIVENESS_COUNT > 0
            if (!check_liveness(n)) {
              /* one of the liveness properties triggered an error */
              break;
            }
#endif

#if BOUND > 0
            if (state_bound_get(n) < BOUND) {
#endif
            size_t queue_size = queue_enqueue(n, thread_id);
            queue_id = thread_id;

            if (size % 10000 == 0 && ftrylockfile(stdout) == 0) {
              if (MACHINE_READABLE_OUTPUT) {
                put("<progress states=\"");
                put_uint(size);
                put("\" duration_seconds=\"");
                put_uint(gettime());
                put("\" rules_fired=\"");
                put_uint(rules_fired_local);
                put("\" queue_size=\"");
                put_uint(queue_size);
                put("\" thread_id=\"");
                put_uint(thread_id);
                put("\"/>\n");
              } else {
                put("\t ");
                if (THREADS > 1) {
                  put("thread ");
                  put_uint(thread_id);
                  put(": ");
                }
                put_uint(size);
                put(" states explored in ");
                put_uint(gettime());
                put("s, with ");
                put_uint(rules_fired_local);
                put(" rules fired and ");
                put(queue_size > last_queue_size ? yellow() : green());
                put_uint(queue_size);
                put(reset());
                put(" states in the queue.\n");
              }
              funlockfile(stdout);
              last_queue_size = queue_size;
            }

            if (THREADS > 1 && thread_id == 0 && phase == WARMUP && queue_size > 20) {
              start_secondary_threads();
              phase = RUN;
            }

#if BOUND > 0
            }
#endif
          } else {
            state_free(n);
          }
        } else {
          state_free(n);
        }
      } while (0);
      rule_taken++;
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_d > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_d - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
}
    /* If we did not toggle 'possible_deadlock' off by this point, we
     * have a deadlock.
     */
    if (DEADLOCK_DETECTION != DEADLOCK_DETECTION_OFF && possible_deadlock) {
      deadlock(s);
    }

  }
  exit_with(EXIT_SUCCESS);
}

static void state_print(const struct state *previous, const struct state *NONNULL s) {
  const struct handle ru_Cache __attribute__((unused)) = state_handle(s, 0ull, 8ull);
  const struct handle ru_Chan1 __attribute__((unused)) = state_handle(s, 24ull, 10ull);
  const struct handle ru_Chan2 __attribute__((unused)) = state_handle(s, 34ull, 10ull);
  const struct handle ru_Chan3 __attribute__((unused)) = state_handle(s, 44ull, 10ull);
  const struct handle ru_InvSet __attribute__((unused)) = state_handle(s, 8ull, 4ull);
  const struct handle ru_ShrSet __attribute__((unused)) = state_handle(s, 12ull, 4ull);
  const struct handle ru_ExGntd __attribute__((unused)) = state_handle(s, 16ull, 2ull);
  const struct handle ru_CurCmd __attribute__((unused)) = state_handle(s, 54ull, 3ull);
  const struct handle ru_CurPtr __attribute__((unused)) = state_handle(s, 18ull, 2ull);
  const struct handle ru_MemData __attribute__((unused)) = state_handle(s, 20ull, 2ull);
  const struct handle ru_AuxData __attribute__((unused)) = state_handle(s, 22ull, 2ull);
{
  size_t schedule1[2ull];
  /* setup a default identity mapping for when scalarset
   * schedules are not in use
   */
  for (size_t i = 0; i < 2ull; ++i) {
    schedule1[i] = i;
  }
  if (USE_SCALARSET_SCHEDULES && previous != NULL) {
    size_t index = schedule_read_NODE(previous);
    size_t stack[2ull];
    index_to_permutation(index, schedule1, stack, (size_t)2ull);
  }
  size_t schedule0[2ull];
  /* setup a default identity mapping for when scalarset
   * schedules are not in use
   */
  for (size_t i = 0; i < 2ull; ++i) {
    schedule0[i] = i;
  }
  if (USE_SCALARSET_SCHEDULES) {
    size_t index = schedule_read_NODE(s);
    size_t stack[2ull];
    index_to_permutation(index, schedule0, stack, (size_t)2ull);
  }
{
  for (size_t i2 = 0; i2 < 2ull; ++i2) {
    size_t j3;
    for (j3 = 0; j3 < 2ull; ++j3) {
      if (schedule0[j3] == i2) {
        break;
      }
    }
    assert(j3 < 2ull &&
      "failed to find permuted scalarset index");
    size_t k4;
    for (k4 = 0; k4 < 2ull; ++k4) {
      if (schedule1[k4] == i2) {
        break;
      }
    }
    assert(k4 < 2ull &&
      "failed to find permuted scalarset index");
{
  raw_value_t v = handle_read_raw(s, ((struct handle){ .base = ((struct handle){ .base = ru_Cache.base + (ru_Cache.offset + (j3 * ((size_t)4ull))) / CHAR_BIT, .offset = (ru_Cache.offset + (j3 * ((size_t)4ull))) % CHAR_BIT, .width = 4ull }).base + (((struct handle){ .base = ru_Cache.base + (ru_Cache.offset + (j3 * ((size_t)4ull))) / CHAR_BIT, .offset = (ru_Cache.offset + (j3 * ((size_t)4ull))) % CHAR_BIT, .width = 4ull }).offset + ((size_t)0)) / CHAR_BIT, .offset = (((struct handle){ .base = ru_Cache.base + (ru_Cache.offset + (j3 * ((size_t)4ull))) / CHAR_BIT, .offset = (ru_Cache.offset + (j3 * ((size_t)4ull))) % CHAR_BIT, .width = 4ull }).offset + ((size_t)0)) % CHAR_BIT, .width = 2ull }));
  raw_value_t v_previous = 0;
  if (previous != NULL) {
    v_previous = handle_read_raw(previous, ((struct handle){ .base = ((struct handle){ .base = ((struct handle){ .base = (uint8_t*)previous->data + (ru_Cache.base - (const uint8_t*)s->data), .offset = ru_Cache.offset, .width = ru_Cache.width }).base + (((struct handle){ .base = (uint8_t*)previous->data + (ru_Cache.base - (const uint8_t*)s->data), .offset = ru_Cache.offset, .width = ru_Cache.width }).offset + (k4 * ((size_t)4ull))) / CHAR_BIT, .offset = (((struct handle){ .base = (uint8_t*)previous->data + (ru_Cache.base - (const uint8_t*)s->data), .offset = ru_Cache.offset, .width = ru_Cache.width }).offset + (k4 * ((size_t)4ull))) % CHAR_BIT, .width = 4ull }).base + (((struct handle){ .base = ((struct handle){ .base = (uint8_t*)previous->data + (ru_Cache.base - (const uint8_t*)s->data), .offset = ru_Cache.offset, .width = ru_Cache.width }).base + (((struct handle){ .base = (uint8_t*)previous->data + (ru_Cache.base - (const uint8_t*)s->data), .offset = ru_Cache.offset, .width = ru_Cache.width }).offset + (k4 * ((size_t)4ull))) / CHAR_BIT, .offset = (((struct handle){ .base = (uint8_t*)previous->data + (ru_Cache.base - (const uint8_t*)s->data), .offset = ru_Cache.offset, .width = ru_Cache.width }).offset + (k4 * ((size_t)4ull))) % CHAR_BIT, .width = 4ull }).offset + ((size_t)0)) / CHAR_BIT, .offset = (((struct handle){ .base = ((struct handle){ .base = (uint8_t*)previous->data + (ru_Cache.base - (const uint8_t*)s->data), .offset = ru_Cache.offset, .width = ru_Cache.width }).base + (((struct handle){ .base = (uint8_t*)previous->data + (ru_Cache.base - (const uint8_t*)s->data), .offset = ru_Cache.offset, .width = ru_Cache.width }).offset + (k4 * ((size_t)4ull))) / CHAR_BIT, .offset = (((struct handle){ .base = (uint8_t*)previous->data + (ru_Cache.base - (const uint8_t*)s->data), .offset = ru_Cache.offset, .width = ru_Cache.width }).offset + (k4 * ((size_t)4ull))) % CHAR_BIT, .width = 4ull }).offset + ((size_t)0)) % CHAR_BIT, .width = 2ull }));
  }
  if (previous == NULL || v != v_previous) {
    if (1 && MACHINE_READABLE_OUTPUT) {
      put("<state_component name=\"");
      do { put("Cache"); put("["); put("NODE"); put("_"); put_val((value_t)(i2)); put("]"); put("."); put("State"); } while (0);
      put("\" value=\"");
    } else {
      do { put("Cache"); put("["); put("NODE"); put("_"); put_val((value_t)(i2)); put("]"); put("."); put("State"); } while (0);
      put(":");
    }
    if (v == 0) {
      put("Undefined");
    } else if (v == VALUE_C(1)) {
      put("I");
    } else if (v == VALUE_C(2)) {
      put("S");
    } else if (v == VALUE_C(3)) {
      put("E");
    } else {
      assert(!"illegal value for enum");
    }
    if (1 && MACHINE_READABLE_OUTPUT) {
      put("\"/>");
    }
    put("\n");
  }
}
{
  raw_value_t v = handle_read_raw(s, ((struct handle){ .base = ((struct handle){ .base = ru_Cache.base + (ru_Cache.offset + (j3 * ((size_t)4ull))) / CHAR_BIT, .offset = (ru_Cache.offset + (j3 * ((size_t)4ull))) % CHAR_BIT, .width = 4ull }).base + (((struct handle){ .base = ru_Cache.base + (ru_Cache.offset + (j3 * ((size_t)4ull))) / CHAR_BIT, .offset = (ru_Cache.offset + (j3 * ((size_t)4ull))) % CHAR_BIT, .width = 4ull }).offset + ((size_t)2)) / CHAR_BIT, .offset = (((struct handle){ .base = ru_Cache.base + (ru_Cache.offset + (j3 * ((size_t)4ull))) / CHAR_BIT, .offset = (ru_Cache.offset + (j3 * ((size_t)4ull))) % CHAR_BIT, .width = 4ull }).offset + ((size_t)2)) % CHAR_BIT, .width = 2ull }));
  raw_value_t v_previous = 0;
  if (previous != NULL) {
    v_previous = handle_read_raw(previous, ((struct handle){ .base = ((struct handle){ .base = ((struct handle){ .base = (uint8_t*)previous->data + (ru_Cache.base - (const uint8_t*)s->data), .offset = ru_Cache.offset, .width = ru_Cache.width }).base + (((struct handle){ .base = (uint8_t*)previous->data + (ru_Cache.base - (const uint8_t*)s->data), .offset = ru_Cache.offset, .width = ru_Cache.width }).offset + (k4 * ((size_t)4ull))) / CHAR_BIT, .offset = (((struct handle){ .base = (uint8_t*)previous->data + (ru_Cache.base - (const uint8_t*)s->data), .offset = ru_Cache.offset, .width = ru_Cache.width }).offset + (k4 * ((size_t)4ull))) % CHAR_BIT, .width = 4ull }).base + (((struct handle){ .base = ((struct handle){ .base = (uint8_t*)previous->data + (ru_Cache.base - (const uint8_t*)s->data), .offset = ru_Cache.offset, .width = ru_Cache.width }).base + (((struct handle){ .base = (uint8_t*)previous->data + (ru_Cache.base - (const uint8_t*)s->data), .offset = ru_Cache.offset, .width = ru_Cache.width }).offset + (k4 * ((size_t)4ull))) / CHAR_BIT, .offset = (((struct handle){ .base = (uint8_t*)previous->data + (ru_Cache.base - (const uint8_t*)s->data), .offset = ru_Cache.offset, .width = ru_Cache.width }).offset + (k4 * ((size_t)4ull))) % CHAR_BIT, .width = 4ull }).offset + ((size_t)2)) / CHAR_BIT, .offset = (((struct handle){ .base = ((struct handle){ .base = (uint8_t*)previous->data + (ru_Cache.base - (const uint8_t*)s->data), .offset = ru_Cache.offset, .width = ru_Cache.width }).base + (((struct handle){ .base = (uint8_t*)previous->data + (ru_Cache.base - (const uint8_t*)s->data), .offset = ru_Cache.offset, .width = ru_Cache.width }).offset + (k4 * ((size_t)4ull))) / CHAR_BIT, .offset = (((struct handle){ .base = (uint8_t*)previous->data + (ru_Cache.base - (const uint8_t*)s->data), .offset = ru_Cache.offset, .width = ru_Cache.width }).offset + (k4 * ((size_t)4ull))) % CHAR_BIT, .width = 4ull }).offset + ((size_t)2)) % CHAR_BIT, .width = 2ull }));
    if (USE_SCALARSET_SCHEDULES && v_previous != 0) {
      if (COUNTEREXAMPLE_TRACE == CEX_OFF) {
        assert(PRINTS_SCALARSETS && "accessing a scalarset "
          "schedule which was unanticipated; bug in"
          "prints_scalarsets()?");
      }
      /* we can use the saved schedule to map this value back to a
       * more intuitive string for the user
       */
      size_t index = schedule_read_DATA(previous);
      size_t schedule[((size_t)2ull)];
      size_t stack[((size_t)2ull)];
      index_to_permutation(index, schedule, stack, ((size_t)2ull));
      ASSERT((size_t)v_previous - 1 < ((size_t)2ull) && "illegal scalarset "
        "value found during printing");
      v_previous = (raw_value_t)schedule[(size_t)v_previous - 1];
    }
  }
  if (previous == NULL || v != v_previous) {
    if (1 && MACHINE_READABLE_OUTPUT) {
      put("<state_component name=\"");
      do { put("Cache"); put("["); put("NODE"); put("_"); put_val((value_t)(i2)); put("]"); put("."); put("Data"); } while (0);
      put("\" value=\"");
    } else {
      do { put("Cache"); put("["); put("NODE"); put("_"); put_val((value_t)(i2)); put("]"); put("."); put("Data"); } while (0);
      put(":");
    }
    if (v == 0) {
      put("Undefined");
    } else if (USE_SCALARSET_SCHEDULES) {
      if (COUNTEREXAMPLE_TRACE == CEX_OFF) {
        assert(PRINTS_SCALARSETS && "accessing a scalarset "
          "schedule which was unanticipated; bug in"
          "prints_scalarsets()?");
      }
      /* we can use the saved schedule to map this value back to a
       * more intuitive string for the user
       */
      size_t index = schedule_read_DATA(s);
      size_t schedule[((size_t)2ull)];
      size_t stack[((size_t)2ull)];
      index_to_permutation(index, schedule, stack, ((size_t)2ull));
      if (1 && MACHINE_READABLE_OUTPUT) {
        xml_printf("DATA");
      } else {
        put("DATA");
      }
      ASSERT((size_t)v - 1 < ((size_t)2ull) && "illegal scalarset "
        "value found during printing");
      put("_");
      put_uint(schedule[(size_t)v - 1]);
    } else {
      put_val(v - 1);
    }
    if (1 && MACHINE_READABLE_OUTPUT) {
      put("\"/>");
    }
    put("\n");
  }
}
  }
}
}
{
  size_t schedule1[2ull];
  /* setup a default identity mapping for when scalarset
   * schedules are not in use
   */
  for (size_t i = 0; i < 2ull; ++i) {
    schedule1[i] = i;
  }
  if (USE_SCALARSET_SCHEDULES && previous != NULL) {
    size_t index = schedule_read_NODE(previous);
    size_t stack[2ull];
    index_to_permutation(index, schedule1, stack, (size_t)2ull);
  }
  size_t schedule0[2ull];
  /* setup a default identity mapping for when scalarset
   * schedules are not in use
   */
  for (size_t i = 0; i < 2ull; ++i) {
    schedule0[i] = i;
  }
  if (USE_SCALARSET_SCHEDULES) {
    size_t index = schedule_read_NODE(s);
    size_t stack[2ull];
    index_to_permutation(index, schedule0, stack, (size_t)2ull);
  }
{
  for (size_t i2 = 0; i2 < 2ull; ++i2) {
    size_t j3;
    for (j3 = 0; j3 < 2ull; ++j3) {
      if (schedule0[j3] == i2) {
        break;
      }
    }
    assert(j3 < 2ull &&
      "failed to find permuted scalarset index");
    size_t k4;
    for (k4 = 0; k4 < 2ull; ++k4) {
      if (schedule1[k4] == i2) {
        break;
      }
    }
    assert(k4 < 2ull &&
      "failed to find permuted scalarset index");
{
  raw_value_t v = handle_read_raw(s, ((struct handle){ .base = ((struct handle){ .base = ru_Chan1.base + (ru_Chan1.offset + (j3 * ((size_t)5ull))) / CHAR_BIT, .offset = (ru_Chan1.offset + (j3 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).base + (((struct handle){ .base = ru_Chan1.base + (ru_Chan1.offset + (j3 * ((size_t)5ull))) / CHAR_BIT, .offset = (ru_Chan1.offset + (j3 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).offset + ((size_t)0)) / CHAR_BIT, .offset = (((struct handle){ .base = ru_Chan1.base + (ru_Chan1.offset + (j3 * ((size_t)5ull))) / CHAR_BIT, .offset = (ru_Chan1.offset + (j3 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).offset + ((size_t)0)) % CHAR_BIT, .width = 2ull }));
  raw_value_t v_previous = 0;
  if (previous != NULL) {
    v_previous = handle_read_raw(previous, ((struct handle){ .base = ((struct handle){ .base = ((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan1.base - (const uint8_t*)s->data), .offset = ru_Chan1.offset, .width = ru_Chan1.width }).base + (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan1.base - (const uint8_t*)s->data), .offset = ru_Chan1.offset, .width = ru_Chan1.width }).offset + (k4 * ((size_t)5ull))) / CHAR_BIT, .offset = (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan1.base - (const uint8_t*)s->data), .offset = ru_Chan1.offset, .width = ru_Chan1.width }).offset + (k4 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).base + (((struct handle){ .base = ((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan1.base - (const uint8_t*)s->data), .offset = ru_Chan1.offset, .width = ru_Chan1.width }).base + (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan1.base - (const uint8_t*)s->data), .offset = ru_Chan1.offset, .width = ru_Chan1.width }).offset + (k4 * ((size_t)5ull))) / CHAR_BIT, .offset = (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan1.base - (const uint8_t*)s->data), .offset = ru_Chan1.offset, .width = ru_Chan1.width }).offset + (k4 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).offset + ((size_t)0)) / CHAR_BIT, .offset = (((struct handle){ .base = ((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan1.base - (const uint8_t*)s->data), .offset = ru_Chan1.offset, .width = ru_Chan1.width }).base + (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan1.base - (const uint8_t*)s->data), .offset = ru_Chan1.offset, .width = ru_Chan1.width }).offset + (k4 * ((size_t)5ull))) / CHAR_BIT, .offset = (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan1.base - (const uint8_t*)s->data), .offset = ru_Chan1.offset, .width = ru_Chan1.width }).offset + (k4 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).offset + ((size_t)0)) % CHAR_BIT, .width = 2ull }));
    if (USE_SCALARSET_SCHEDULES && v_previous != 0) {
      if (COUNTEREXAMPLE_TRACE == CEX_OFF) {
        assert(PRINTS_SCALARSETS && "accessing a scalarset "
          "schedule which was unanticipated; bug in"
          "prints_scalarsets()?");
      }
      /* we can use the saved schedule to map this value back to a
       * more intuitive string for the user
       */
      size_t index = schedule_read_DATA(previous);
      size_t schedule[((size_t)2ull)];
      size_t stack[((size_t)2ull)];
      index_to_permutation(index, schedule, stack, ((size_t)2ull));
      ASSERT((size_t)v_previous - 1 < ((size_t)2ull) && "illegal scalarset "
        "value found during printing");
      v_previous = (raw_value_t)schedule[(size_t)v_previous - 1];
    }
  }
  if (previous == NULL || v != v_previous) {
    if (1 && MACHINE_READABLE_OUTPUT) {
      put("<state_component name=\"");
      do { put("Chan1"); put("["); put("NODE"); put("_"); put_val((value_t)(i2)); put("]"); put("."); put("Data"); } while (0);
      put("\" value=\"");
    } else {
      do { put("Chan1"); put("["); put("NODE"); put("_"); put_val((value_t)(i2)); put("]"); put("."); put("Data"); } while (0);
      put(":");
    }
    if (v == 0) {
      put("Undefined");
    } else if (USE_SCALARSET_SCHEDULES) {
      if (COUNTEREXAMPLE_TRACE == CEX_OFF) {
        assert(PRINTS_SCALARSETS && "accessing a scalarset "
          "schedule which was unanticipated; bug in"
          "prints_scalarsets()?");
      }
      /* we can use the saved schedule to map this value back to a
       * more intuitive string for the user
       */
      size_t index = schedule_read_DATA(s);
      size_t schedule[((size_t)2ull)];
      size_t stack[((size_t)2ull)];
      index_to_permutation(index, schedule, stack, ((size_t)2ull));
      if (1 && MACHINE_READABLE_OUTPUT) {
        xml_printf("DATA");
      } else {
        put("DATA");
      }
      ASSERT((size_t)v - 1 < ((size_t)2ull) && "illegal scalarset "
        "value found during printing");
      put("_");
      put_uint(schedule[(size_t)v - 1]);
    } else {
      put_val(v - 1);
    }
    if (1 && MACHINE_READABLE_OUTPUT) {
      put("\"/>");
    }
    put("\n");
  }
}
{
  raw_value_t v = handle_read_raw(s, ((struct handle){ .base = ((struct handle){ .base = ru_Chan1.base + (ru_Chan1.offset + (j3 * ((size_t)5ull))) / CHAR_BIT, .offset = (ru_Chan1.offset + (j3 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).base + (((struct handle){ .base = ru_Chan1.base + (ru_Chan1.offset + (j3 * ((size_t)5ull))) / CHAR_BIT, .offset = (ru_Chan1.offset + (j3 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).offset + ((size_t)2)) / CHAR_BIT, .offset = (((struct handle){ .base = ru_Chan1.base + (ru_Chan1.offset + (j3 * ((size_t)5ull))) / CHAR_BIT, .offset = (ru_Chan1.offset + (j3 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).offset + ((size_t)2)) % CHAR_BIT, .width = 3ull }));
  raw_value_t v_previous = 0;
  if (previous != NULL) {
    v_previous = handle_read_raw(previous, ((struct handle){ .base = ((struct handle){ .base = ((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan1.base - (const uint8_t*)s->data), .offset = ru_Chan1.offset, .width = ru_Chan1.width }).base + (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan1.base - (const uint8_t*)s->data), .offset = ru_Chan1.offset, .width = ru_Chan1.width }).offset + (k4 * ((size_t)5ull))) / CHAR_BIT, .offset = (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan1.base - (const uint8_t*)s->data), .offset = ru_Chan1.offset, .width = ru_Chan1.width }).offset + (k4 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).base + (((struct handle){ .base = ((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan1.base - (const uint8_t*)s->data), .offset = ru_Chan1.offset, .width = ru_Chan1.width }).base + (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan1.base - (const uint8_t*)s->data), .offset = ru_Chan1.offset, .width = ru_Chan1.width }).offset + (k4 * ((size_t)5ull))) / CHAR_BIT, .offset = (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan1.base - (const uint8_t*)s->data), .offset = ru_Chan1.offset, .width = ru_Chan1.width }).offset + (k4 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).offset + ((size_t)2)) / CHAR_BIT, .offset = (((struct handle){ .base = ((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan1.base - (const uint8_t*)s->data), .offset = ru_Chan1.offset, .width = ru_Chan1.width }).base + (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan1.base - (const uint8_t*)s->data), .offset = ru_Chan1.offset, .width = ru_Chan1.width }).offset + (k4 * ((size_t)5ull))) / CHAR_BIT, .offset = (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan1.base - (const uint8_t*)s->data), .offset = ru_Chan1.offset, .width = ru_Chan1.width }).offset + (k4 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).offset + ((size_t)2)) % CHAR_BIT, .width = 3ull }));
  }
  if (previous == NULL || v != v_previous) {
    if (1 && MACHINE_READABLE_OUTPUT) {
      put("<state_component name=\"");
      do { put("Chan1"); put("["); put("NODE"); put("_"); put_val((value_t)(i2)); put("]"); put("."); put("Cmd"); } while (0);
      put("\" value=\"");
    } else {
      do { put("Chan1"); put("["); put("NODE"); put("_"); put_val((value_t)(i2)); put("]"); put("."); put("Cmd"); } while (0);
      put(":");
    }
    if (v == 0) {
      put("Undefined");
    } else if (v == VALUE_C(1)) {
      put("Empty");
    } else if (v == VALUE_C(2)) {
      put("ReqS");
    } else if (v == VALUE_C(3)) {
      put("ReqE");
    } else if (v == VALUE_C(4)) {
      put("Inv");
    } else if (v == VALUE_C(5)) {
      put("InvAck");
    } else if (v == VALUE_C(6)) {
      put("GntS");
    } else if (v == VALUE_C(7)) {
      put("GntE");
    } else {
      assert(!"illegal value for enum");
    }
    if (1 && MACHINE_READABLE_OUTPUT) {
      put("\"/>");
    }
    put("\n");
  }
}
  }
}
}
{
  size_t schedule1[2ull];
  /* setup a default identity mapping for when scalarset
   * schedules are not in use
   */
  for (size_t i = 0; i < 2ull; ++i) {
    schedule1[i] = i;
  }
  if (USE_SCALARSET_SCHEDULES && previous != NULL) {
    size_t index = schedule_read_NODE(previous);
    size_t stack[2ull];
    index_to_permutation(index, schedule1, stack, (size_t)2ull);
  }
  size_t schedule0[2ull];
  /* setup a default identity mapping for when scalarset
   * schedules are not in use
   */
  for (size_t i = 0; i < 2ull; ++i) {
    schedule0[i] = i;
  }
  if (USE_SCALARSET_SCHEDULES) {
    size_t index = schedule_read_NODE(s);
    size_t stack[2ull];
    index_to_permutation(index, schedule0, stack, (size_t)2ull);
  }
{
  for (size_t i2 = 0; i2 < 2ull; ++i2) {
    size_t j3;
    for (j3 = 0; j3 < 2ull; ++j3) {
      if (schedule0[j3] == i2) {
        break;
      }
    }
    assert(j3 < 2ull &&
      "failed to find permuted scalarset index");
    size_t k4;
    for (k4 = 0; k4 < 2ull; ++k4) {
      if (schedule1[k4] == i2) {
        break;
      }
    }
    assert(k4 < 2ull &&
      "failed to find permuted scalarset index");
{
  raw_value_t v = handle_read_raw(s, ((struct handle){ .base = ((struct handle){ .base = ru_Chan2.base + (ru_Chan2.offset + (j3 * ((size_t)5ull))) / CHAR_BIT, .offset = (ru_Chan2.offset + (j3 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).base + (((struct handle){ .base = ru_Chan2.base + (ru_Chan2.offset + (j3 * ((size_t)5ull))) / CHAR_BIT, .offset = (ru_Chan2.offset + (j3 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).offset + ((size_t)0)) / CHAR_BIT, .offset = (((struct handle){ .base = ru_Chan2.base + (ru_Chan2.offset + (j3 * ((size_t)5ull))) / CHAR_BIT, .offset = (ru_Chan2.offset + (j3 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).offset + ((size_t)0)) % CHAR_BIT, .width = 2ull }));
  raw_value_t v_previous = 0;
  if (previous != NULL) {
    v_previous = handle_read_raw(previous, ((struct handle){ .base = ((struct handle){ .base = ((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan2.base - (const uint8_t*)s->data), .offset = ru_Chan2.offset, .width = ru_Chan2.width }).base + (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan2.base - (const uint8_t*)s->data), .offset = ru_Chan2.offset, .width = ru_Chan2.width }).offset + (k4 * ((size_t)5ull))) / CHAR_BIT, .offset = (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan2.base - (const uint8_t*)s->data), .offset = ru_Chan2.offset, .width = ru_Chan2.width }).offset + (k4 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).base + (((struct handle){ .base = ((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan2.base - (const uint8_t*)s->data), .offset = ru_Chan2.offset, .width = ru_Chan2.width }).base + (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan2.base - (const uint8_t*)s->data), .offset = ru_Chan2.offset, .width = ru_Chan2.width }).offset + (k4 * ((size_t)5ull))) / CHAR_BIT, .offset = (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan2.base - (const uint8_t*)s->data), .offset = ru_Chan2.offset, .width = ru_Chan2.width }).offset + (k4 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).offset + ((size_t)0)) / CHAR_BIT, .offset = (((struct handle){ .base = ((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan2.base - (const uint8_t*)s->data), .offset = ru_Chan2.offset, .width = ru_Chan2.width }).base + (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan2.base - (const uint8_t*)s->data), .offset = ru_Chan2.offset, .width = ru_Chan2.width }).offset + (k4 * ((size_t)5ull))) / CHAR_BIT, .offset = (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan2.base - (const uint8_t*)s->data), .offset = ru_Chan2.offset, .width = ru_Chan2.width }).offset + (k4 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).offset + ((size_t)0)) % CHAR_BIT, .width = 2ull }));
    if (USE_SCALARSET_SCHEDULES && v_previous != 0) {
      if (COUNTEREXAMPLE_TRACE == CEX_OFF) {
        assert(PRINTS_SCALARSETS && "accessing a scalarset "
          "schedule which was unanticipated; bug in"
          "prints_scalarsets()?");
      }
      /* we can use the saved schedule to map this value back to a
       * more intuitive string for the user
       */
      size_t index = schedule_read_DATA(previous);
      size_t schedule[((size_t)2ull)];
      size_t stack[((size_t)2ull)];
      index_to_permutation(index, schedule, stack, ((size_t)2ull));
      ASSERT((size_t)v_previous - 1 < ((size_t)2ull) && "illegal scalarset "
        "value found during printing");
      v_previous = (raw_value_t)schedule[(size_t)v_previous - 1];
    }
  }
  if (previous == NULL || v != v_previous) {
    if (1 && MACHINE_READABLE_OUTPUT) {
      put("<state_component name=\"");
      do { put("Chan2"); put("["); put("NODE"); put("_"); put_val((value_t)(i2)); put("]"); put("."); put("Data"); } while (0);
      put("\" value=\"");
    } else {
      do { put("Chan2"); put("["); put("NODE"); put("_"); put_val((value_t)(i2)); put("]"); put("."); put("Data"); } while (0);
      put(":");
    }
    if (v == 0) {
      put("Undefined");
    } else if (USE_SCALARSET_SCHEDULES) {
      if (COUNTEREXAMPLE_TRACE == CEX_OFF) {
        assert(PRINTS_SCALARSETS && "accessing a scalarset "
          "schedule which was unanticipated; bug in"
          "prints_scalarsets()?");
      }
      /* we can use the saved schedule to map this value back to a
       * more intuitive string for the user
       */
      size_t index = schedule_read_DATA(s);
      size_t schedule[((size_t)2ull)];
      size_t stack[((size_t)2ull)];
      index_to_permutation(index, schedule, stack, ((size_t)2ull));
      if (1 && MACHINE_READABLE_OUTPUT) {
        xml_printf("DATA");
      } else {
        put("DATA");
      }
      ASSERT((size_t)v - 1 < ((size_t)2ull) && "illegal scalarset "
        "value found during printing");
      put("_");
      put_uint(schedule[(size_t)v - 1]);
    } else {
      put_val(v - 1);
    }
    if (1 && MACHINE_READABLE_OUTPUT) {
      put("\"/>");
    }
    put("\n");
  }
}
{
  raw_value_t v = handle_read_raw(s, ((struct handle){ .base = ((struct handle){ .base = ru_Chan2.base + (ru_Chan2.offset + (j3 * ((size_t)5ull))) / CHAR_BIT, .offset = (ru_Chan2.offset + (j3 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).base + (((struct handle){ .base = ru_Chan2.base + (ru_Chan2.offset + (j3 * ((size_t)5ull))) / CHAR_BIT, .offset = (ru_Chan2.offset + (j3 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).offset + ((size_t)2)) / CHAR_BIT, .offset = (((struct handle){ .base = ru_Chan2.base + (ru_Chan2.offset + (j3 * ((size_t)5ull))) / CHAR_BIT, .offset = (ru_Chan2.offset + (j3 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).offset + ((size_t)2)) % CHAR_BIT, .width = 3ull }));
  raw_value_t v_previous = 0;
  if (previous != NULL) {
    v_previous = handle_read_raw(previous, ((struct handle){ .base = ((struct handle){ .base = ((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan2.base - (const uint8_t*)s->data), .offset = ru_Chan2.offset, .width = ru_Chan2.width }).base + (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan2.base - (const uint8_t*)s->data), .offset = ru_Chan2.offset, .width = ru_Chan2.width }).offset + (k4 * ((size_t)5ull))) / CHAR_BIT, .offset = (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan2.base - (const uint8_t*)s->data), .offset = ru_Chan2.offset, .width = ru_Chan2.width }).offset + (k4 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).base + (((struct handle){ .base = ((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan2.base - (const uint8_t*)s->data), .offset = ru_Chan2.offset, .width = ru_Chan2.width }).base + (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan2.base - (const uint8_t*)s->data), .offset = ru_Chan2.offset, .width = ru_Chan2.width }).offset + (k4 * ((size_t)5ull))) / CHAR_BIT, .offset = (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan2.base - (const uint8_t*)s->data), .offset = ru_Chan2.offset, .width = ru_Chan2.width }).offset + (k4 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).offset + ((size_t)2)) / CHAR_BIT, .offset = (((struct handle){ .base = ((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan2.base - (const uint8_t*)s->data), .offset = ru_Chan2.offset, .width = ru_Chan2.width }).base + (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan2.base - (const uint8_t*)s->data), .offset = ru_Chan2.offset, .width = ru_Chan2.width }).offset + (k4 * ((size_t)5ull))) / CHAR_BIT, .offset = (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan2.base - (const uint8_t*)s->data), .offset = ru_Chan2.offset, .width = ru_Chan2.width }).offset + (k4 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).offset + ((size_t)2)) % CHAR_BIT, .width = 3ull }));
  }
  if (previous == NULL || v != v_previous) {
    if (1 && MACHINE_READABLE_OUTPUT) {
      put("<state_component name=\"");
      do { put("Chan2"); put("["); put("NODE"); put("_"); put_val((value_t)(i2)); put("]"); put("."); put("Cmd"); } while (0);
      put("\" value=\"");
    } else {
      do { put("Chan2"); put("["); put("NODE"); put("_"); put_val((value_t)(i2)); put("]"); put("."); put("Cmd"); } while (0);
      put(":");
    }
    if (v == 0) {
      put("Undefined");
    } else if (v == VALUE_C(1)) {
      put("Empty");
    } else if (v == VALUE_C(2)) {
      put("ReqS");
    } else if (v == VALUE_C(3)) {
      put("ReqE");
    } else if (v == VALUE_C(4)) {
      put("Inv");
    } else if (v == VALUE_C(5)) {
      put("InvAck");
    } else if (v == VALUE_C(6)) {
      put("GntS");
    } else if (v == VALUE_C(7)) {
      put("GntE");
    } else {
      assert(!"illegal value for enum");
    }
    if (1 && MACHINE_READABLE_OUTPUT) {
      put("\"/>");
    }
    put("\n");
  }
}
  }
}
}
{
  size_t schedule1[2ull];
  /* setup a default identity mapping for when scalarset
   * schedules are not in use
   */
  for (size_t i = 0; i < 2ull; ++i) {
    schedule1[i] = i;
  }
  if (USE_SCALARSET_SCHEDULES && previous != NULL) {
    size_t index = schedule_read_NODE(previous);
    size_t stack[2ull];
    index_to_permutation(index, schedule1, stack, (size_t)2ull);
  }
  size_t schedule0[2ull];
  /* setup a default identity mapping for when scalarset
   * schedules are not in use
   */
  for (size_t i = 0; i < 2ull; ++i) {
    schedule0[i] = i;
  }
  if (USE_SCALARSET_SCHEDULES) {
    size_t index = schedule_read_NODE(s);
    size_t stack[2ull];
    index_to_permutation(index, schedule0, stack, (size_t)2ull);
  }
{
  for (size_t i2 = 0; i2 < 2ull; ++i2) {
    size_t j3;
    for (j3 = 0; j3 < 2ull; ++j3) {
      if (schedule0[j3] == i2) {
        break;
      }
    }
    assert(j3 < 2ull &&
      "failed to find permuted scalarset index");
    size_t k4;
    for (k4 = 0; k4 < 2ull; ++k4) {
      if (schedule1[k4] == i2) {
        break;
      }
    }
    assert(k4 < 2ull &&
      "failed to find permuted scalarset index");
{
  raw_value_t v = handle_read_raw(s, ((struct handle){ .base = ((struct handle){ .base = ru_Chan3.base + (ru_Chan3.offset + (j3 * ((size_t)5ull))) / CHAR_BIT, .offset = (ru_Chan3.offset + (j3 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).base + (((struct handle){ .base = ru_Chan3.base + (ru_Chan3.offset + (j3 * ((size_t)5ull))) / CHAR_BIT, .offset = (ru_Chan3.offset + (j3 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).offset + ((size_t)0)) / CHAR_BIT, .offset = (((struct handle){ .base = ru_Chan3.base + (ru_Chan3.offset + (j3 * ((size_t)5ull))) / CHAR_BIT, .offset = (ru_Chan3.offset + (j3 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).offset + ((size_t)0)) % CHAR_BIT, .width = 2ull }));
  raw_value_t v_previous = 0;
  if (previous != NULL) {
    v_previous = handle_read_raw(previous, ((struct handle){ .base = ((struct handle){ .base = ((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan3.base - (const uint8_t*)s->data), .offset = ru_Chan3.offset, .width = ru_Chan3.width }).base + (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan3.base - (const uint8_t*)s->data), .offset = ru_Chan3.offset, .width = ru_Chan3.width }).offset + (k4 * ((size_t)5ull))) / CHAR_BIT, .offset = (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan3.base - (const uint8_t*)s->data), .offset = ru_Chan3.offset, .width = ru_Chan3.width }).offset + (k4 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).base + (((struct handle){ .base = ((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan3.base - (const uint8_t*)s->data), .offset = ru_Chan3.offset, .width = ru_Chan3.width }).base + (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan3.base - (const uint8_t*)s->data), .offset = ru_Chan3.offset, .width = ru_Chan3.width }).offset + (k4 * ((size_t)5ull))) / CHAR_BIT, .offset = (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan3.base - (const uint8_t*)s->data), .offset = ru_Chan3.offset, .width = ru_Chan3.width }).offset + (k4 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).offset + ((size_t)0)) / CHAR_BIT, .offset = (((struct handle){ .base = ((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan3.base - (const uint8_t*)s->data), .offset = ru_Chan3.offset, .width = ru_Chan3.width }).base + (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan3.base - (const uint8_t*)s->data), .offset = ru_Chan3.offset, .width = ru_Chan3.width }).offset + (k4 * ((size_t)5ull))) / CHAR_BIT, .offset = (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan3.base - (const uint8_t*)s->data), .offset = ru_Chan3.offset, .width = ru_Chan3.width }).offset + (k4 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).offset + ((size_t)0)) % CHAR_BIT, .width = 2ull }));
    if (USE_SCALARSET_SCHEDULES && v_previous != 0) {
      if (COUNTEREXAMPLE_TRACE == CEX_OFF) {
        assert(PRINTS_SCALARSETS && "accessing a scalarset "
          "schedule which was unanticipated; bug in"
          "prints_scalarsets()?");
      }
      /* we can use the saved schedule to map this value back to a
       * more intuitive string for the user
       */
      size_t index = schedule_read_DATA(previous);
      size_t schedule[((size_t)2ull)];
      size_t stack[((size_t)2ull)];
      index_to_permutation(index, schedule, stack, ((size_t)2ull));
      ASSERT((size_t)v_previous - 1 < ((size_t)2ull) && "illegal scalarset "
        "value found during printing");
      v_previous = (raw_value_t)schedule[(size_t)v_previous - 1];
    }
  }
  if (previous == NULL || v != v_previous) {
    if (1 && MACHINE_READABLE_OUTPUT) {
      put("<state_component name=\"");
      do { put("Chan3"); put("["); put("NODE"); put("_"); put_val((value_t)(i2)); put("]"); put("."); put("Data"); } while (0);
      put("\" value=\"");
    } else {
      do { put("Chan3"); put("["); put("NODE"); put("_"); put_val((value_t)(i2)); put("]"); put("."); put("Data"); } while (0);
      put(":");
    }
    if (v == 0) {
      put("Undefined");
    } else if (USE_SCALARSET_SCHEDULES) {
      if (COUNTEREXAMPLE_TRACE == CEX_OFF) {
        assert(PRINTS_SCALARSETS && "accessing a scalarset "
          "schedule which was unanticipated; bug in"
          "prints_scalarsets()?");
      }
      /* we can use the saved schedule to map this value back to a
       * more intuitive string for the user
       */
      size_t index = schedule_read_DATA(s);
      size_t schedule[((size_t)2ull)];
      size_t stack[((size_t)2ull)];
      index_to_permutation(index, schedule, stack, ((size_t)2ull));
      if (1 && MACHINE_READABLE_OUTPUT) {
        xml_printf("DATA");
      } else {
        put("DATA");
      }
      ASSERT((size_t)v - 1 < ((size_t)2ull) && "illegal scalarset "
        "value found during printing");
      put("_");
      put_uint(schedule[(size_t)v - 1]);
    } else {
      put_val(v - 1);
    }
    if (1 && MACHINE_READABLE_OUTPUT) {
      put("\"/>");
    }
    put("\n");
  }
}
{
  raw_value_t v = handle_read_raw(s, ((struct handle){ .base = ((struct handle){ .base = ru_Chan3.base + (ru_Chan3.offset + (j3 * ((size_t)5ull))) / CHAR_BIT, .offset = (ru_Chan3.offset + (j3 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).base + (((struct handle){ .base = ru_Chan3.base + (ru_Chan3.offset + (j3 * ((size_t)5ull))) / CHAR_BIT, .offset = (ru_Chan3.offset + (j3 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).offset + ((size_t)2)) / CHAR_BIT, .offset = (((struct handle){ .base = ru_Chan3.base + (ru_Chan3.offset + (j3 * ((size_t)5ull))) / CHAR_BIT, .offset = (ru_Chan3.offset + (j3 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).offset + ((size_t)2)) % CHAR_BIT, .width = 3ull }));
  raw_value_t v_previous = 0;
  if (previous != NULL) {
    v_previous = handle_read_raw(previous, ((struct handle){ .base = ((struct handle){ .base = ((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan3.base - (const uint8_t*)s->data), .offset = ru_Chan3.offset, .width = ru_Chan3.width }).base + (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan3.base - (const uint8_t*)s->data), .offset = ru_Chan3.offset, .width = ru_Chan3.width }).offset + (k4 * ((size_t)5ull))) / CHAR_BIT, .offset = (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan3.base - (const uint8_t*)s->data), .offset = ru_Chan3.offset, .width = ru_Chan3.width }).offset + (k4 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).base + (((struct handle){ .base = ((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan3.base - (const uint8_t*)s->data), .offset = ru_Chan3.offset, .width = ru_Chan3.width }).base + (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan3.base - (const uint8_t*)s->data), .offset = ru_Chan3.offset, .width = ru_Chan3.width }).offset + (k4 * ((size_t)5ull))) / CHAR_BIT, .offset = (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan3.base - (const uint8_t*)s->data), .offset = ru_Chan3.offset, .width = ru_Chan3.width }).offset + (k4 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).offset + ((size_t)2)) / CHAR_BIT, .offset = (((struct handle){ .base = ((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan3.base - (const uint8_t*)s->data), .offset = ru_Chan3.offset, .width = ru_Chan3.width }).base + (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan3.base - (const uint8_t*)s->data), .offset = ru_Chan3.offset, .width = ru_Chan3.width }).offset + (k4 * ((size_t)5ull))) / CHAR_BIT, .offset = (((struct handle){ .base = (uint8_t*)previous->data + (ru_Chan3.base - (const uint8_t*)s->data), .offset = ru_Chan3.offset, .width = ru_Chan3.width }).offset + (k4 * ((size_t)5ull))) % CHAR_BIT, .width = 5ull }).offset + ((size_t)2)) % CHAR_BIT, .width = 3ull }));
  }
  if (previous == NULL || v != v_previous) {
    if (1 && MACHINE_READABLE_OUTPUT) {
      put("<state_component name=\"");
      do { put("Chan3"); put("["); put("NODE"); put("_"); put_val((value_t)(i2)); put("]"); put("."); put("Cmd"); } while (0);
      put("\" value=\"");
    } else {
      do { put("Chan3"); put("["); put("NODE"); put("_"); put_val((value_t)(i2)); put("]"); put("."); put("Cmd"); } while (0);
      put(":");
    }
    if (v == 0) {
      put("Undefined");
    } else if (v == VALUE_C(1)) {
      put("Empty");
    } else if (v == VALUE_C(2)) {
      put("ReqS");
    } else if (v == VALUE_C(3)) {
      put("ReqE");
    } else if (v == VALUE_C(4)) {
      put("Inv");
    } else if (v == VALUE_C(5)) {
      put("InvAck");
    } else if (v == VALUE_C(6)) {
      put("GntS");
    } else if (v == VALUE_C(7)) {
      put("GntE");
    } else {
      assert(!"illegal value for enum");
    }
    if (1 && MACHINE_READABLE_OUTPUT) {
      put("\"/>");
    }
    put("\n");
  }
}
  }
}
}
{
  size_t schedule1[2ull];
  /* setup a default identity mapping for when scalarset
   * schedules are not in use
   */
  for (size_t i = 0; i < 2ull; ++i) {
    schedule1[i] = i;
  }
  if (USE_SCALARSET_SCHEDULES && previous != NULL) {
    size_t index = schedule_read_NODE(previous);
    size_t stack[2ull];
    index_to_permutation(index, schedule1, stack, (size_t)2ull);
  }
  size_t schedule0[2ull];
  /* setup a default identity mapping for when scalarset
   * schedules are not in use
   */
  for (size_t i = 0; i < 2ull; ++i) {
    schedule0[i] = i;
  }
  if (USE_SCALARSET_SCHEDULES) {
    size_t index = schedule_read_NODE(s);
    size_t stack[2ull];
    index_to_permutation(index, schedule0, stack, (size_t)2ull);
  }
{
  for (size_t i2 = 0; i2 < 2ull; ++i2) {
    size_t j3;
    for (j3 = 0; j3 < 2ull; ++j3) {
      if (schedule0[j3] == i2) {
        break;
      }
    }
    assert(j3 < 2ull &&
      "failed to find permuted scalarset index");
    size_t k4;
    for (k4 = 0; k4 < 2ull; ++k4) {
      if (schedule1[k4] == i2) {
        break;
      }
    }
    assert(k4 < 2ull &&
      "failed to find permuted scalarset index");
{
  raw_value_t v = handle_read_raw(s, ((struct handle){ .base = ru_InvSet.base + (ru_InvSet.offset + (j3 * ((size_t)2ull))) / CHAR_BIT, .offset = (ru_InvSet.offset + (j3 * ((size_t)2ull))) % CHAR_BIT, .width = 2ull }));
  raw_value_t v_previous = 0;
  if (previous != NULL) {
    v_previous = handle_read_raw(previous, ((struct handle){ .base = ((struct handle){ .base = (uint8_t*)previous->data + (ru_InvSet.base - (const uint8_t*)s->data), .offset = ru_InvSet.offset, .width = ru_InvSet.width }).base + (((struct handle){ .base = (uint8_t*)previous->data + (ru_InvSet.base - (const uint8_t*)s->data), .offset = ru_InvSet.offset, .width = ru_InvSet.width }).offset + (k4 * ((size_t)2ull))) / CHAR_BIT, .offset = (((struct handle){ .base = (uint8_t*)previous->data + (ru_InvSet.base - (const uint8_t*)s->data), .offset = ru_InvSet.offset, .width = ru_InvSet.width }).offset + (k4 * ((size_t)2ull))) % CHAR_BIT, .width = 2ull }));
  }
  if (previous == NULL || v != v_previous) {
    if (1 && MACHINE_READABLE_OUTPUT) {
      put("<state_component name=\"");
      do { put("InvSet"); put("["); put("NODE"); put("_"); put_val((value_t)(i2)); put("]"); } while (0);
      put("\" value=\"");
    } else {
      do { put("InvSet"); put("["); put("NODE"); put("_"); put_val((value_t)(i2)); put("]"); } while (0);
      put(":");
    }
    if (v == 0) {
      put("Undefined");
    } else if (v == VALUE_C(1)) {
      put("false");
    } else if (v == VALUE_C(2)) {
      put("true");
    } else {
      assert(!"illegal value for enum");
    }
    if (1 && MACHINE_READABLE_OUTPUT) {
      put("\"/>");
    }
    put("\n");
  }
}
  }
}
}
{
  size_t schedule1[2ull];
  /* setup a default identity mapping for when scalarset
   * schedules are not in use
   */
  for (size_t i = 0; i < 2ull; ++i) {
    schedule1[i] = i;
  }
  if (USE_SCALARSET_SCHEDULES && previous != NULL) {
    size_t index = schedule_read_NODE(previous);
    size_t stack[2ull];
    index_to_permutation(index, schedule1, stack, (size_t)2ull);
  }
  size_t schedule0[2ull];
  /* setup a default identity mapping for when scalarset
   * schedules are not in use
   */
  for (size_t i = 0; i < 2ull; ++i) {
    schedule0[i] = i;
  }
  if (USE_SCALARSET_SCHEDULES) {
    size_t index = schedule_read_NODE(s);
    size_t stack[2ull];
    index_to_permutation(index, schedule0, stack, (size_t)2ull);
  }
{
  for (size_t i2 = 0; i2 < 2ull; ++i2) {
    size_t j3;
    for (j3 = 0; j3 < 2ull; ++j3) {
      if (schedule0[j3] == i2) {
        break;
      }
    }
    assert(j3 < 2ull &&
      "failed to find permuted scalarset index");
    size_t k4;
    for (k4 = 0; k4 < 2ull; ++k4) {
      if (schedule1[k4] == i2) {
        break;
      }
    }
    assert(k4 < 2ull &&
      "failed to find permuted scalarset index");
{
  raw_value_t v = handle_read_raw(s, ((struct handle){ .base = ru_ShrSet.base + (ru_ShrSet.offset + (j3 * ((size_t)2ull))) / CHAR_BIT, .offset = (ru_ShrSet.offset + (j3 * ((size_t)2ull))) % CHAR_BIT, .width = 2ull }));
  raw_value_t v_previous = 0;
  if (previous != NULL) {
    v_previous = handle_read_raw(previous, ((struct handle){ .base = ((struct handle){ .base = (uint8_t*)previous->data + (ru_ShrSet.base - (const uint8_t*)s->data), .offset = ru_ShrSet.offset, .width = ru_ShrSet.width }).base + (((struct handle){ .base = (uint8_t*)previous->data + (ru_ShrSet.base - (const uint8_t*)s->data), .offset = ru_ShrSet.offset, .width = ru_ShrSet.width }).offset + (k4 * ((size_t)2ull))) / CHAR_BIT, .offset = (((struct handle){ .base = (uint8_t*)previous->data + (ru_ShrSet.base - (const uint8_t*)s->data), .offset = ru_ShrSet.offset, .width = ru_ShrSet.width }).offset + (k4 * ((size_t)2ull))) % CHAR_BIT, .width = 2ull }));
  }
  if (previous == NULL || v != v_previous) {
    if (1 && MACHINE_READABLE_OUTPUT) {
      put("<state_component name=\"");
      do { put("ShrSet"); put("["); put("NODE"); put("_"); put_val((value_t)(i2)); put("]"); } while (0);
      put("\" value=\"");
    } else {
      do { put("ShrSet"); put("["); put("NODE"); put("_"); put_val((value_t)(i2)); put("]"); } while (0);
      put(":");
    }
    if (v == 0) {
      put("Undefined");
    } else if (v == VALUE_C(1)) {
      put("false");
    } else if (v == VALUE_C(2)) {
      put("true");
    } else {
      assert(!"illegal value for enum");
    }
    if (1 && MACHINE_READABLE_OUTPUT) {
      put("\"/>");
    }
    put("\n");
  }
}
  }
}
}
{
  raw_value_t v = handle_read_raw(s, ru_ExGntd);
  raw_value_t v_previous = 0;
  if (previous != NULL) {
    v_previous = handle_read_raw(previous, ((struct handle){ .base = (uint8_t*)previous->data + (ru_ExGntd.base - (const uint8_t*)s->data), .offset = ru_ExGntd.offset, .width = ru_ExGntd.width }));
  }
  if (previous == NULL || v != v_previous) {
    if (1 && MACHINE_READABLE_OUTPUT) {
      put("<state_component name=\"");
      do { put("ExGntd"); } while (0);
      put("\" value=\"");
    } else {
      do { put("ExGntd"); } while (0);
      put(":");
    }
    if (v == 0) {
      put("Undefined");
    } else if (v == VALUE_C(1)) {
      put("false");
    } else if (v == VALUE_C(2)) {
      put("true");
    } else {
      assert(!"illegal value for enum");
    }
    if (1 && MACHINE_READABLE_OUTPUT) {
      put("\"/>");
    }
    put("\n");
  }
}
{
  raw_value_t v = handle_read_raw(s, ru_CurCmd);
  raw_value_t v_previous = 0;
  if (previous != NULL) {
    v_previous = handle_read_raw(previous, ((struct handle){ .base = (uint8_t*)previous->data + (ru_CurCmd.base - (const uint8_t*)s->data), .offset = ru_CurCmd.offset, .width = ru_CurCmd.width }));
  }
  if (previous == NULL || v != v_previous) {
    if (1 && MACHINE_READABLE_OUTPUT) {
      put("<state_component name=\"");
      do { put("CurCmd"); } while (0);
      put("\" value=\"");
    } else {
      do { put("CurCmd"); } while (0);
      put(":");
    }
    if (v == 0) {
      put("Undefined");
    } else if (v == VALUE_C(1)) {
      put("Empty");
    } else if (v == VALUE_C(2)) {
      put("ReqS");
    } else if (v == VALUE_C(3)) {
      put("ReqE");
    } else if (v == VALUE_C(4)) {
      put("Inv");
    } else if (v == VALUE_C(5)) {
      put("InvAck");
    } else if (v == VALUE_C(6)) {
      put("GntS");
    } else if (v == VALUE_C(7)) {
      put("GntE");
    } else {
      assert(!"illegal value for enum");
    }
    if (1 && MACHINE_READABLE_OUTPUT) {
      put("\"/>");
    }
    put("\n");
  }
}
{
  raw_value_t v = handle_read_raw(s, ru_CurPtr);
  raw_value_t v_previous = 0;
  if (previous != NULL) {
    v_previous = handle_read_raw(previous, ((struct handle){ .base = (uint8_t*)previous->data + (ru_CurPtr.base - (const uint8_t*)s->data), .offset = ru_CurPtr.offset, .width = ru_CurPtr.width }));
    if (USE_SCALARSET_SCHEDULES && v_previous != 0) {
      if (COUNTEREXAMPLE_TRACE == CEX_OFF) {
        assert(PRINTS_SCALARSETS && "accessing a scalarset "
          "schedule which was unanticipated; bug in"
          "prints_scalarsets()?");
      }
      /* we can use the saved schedule to map this value back to a
       * more intuitive string for the user
       */
      size_t index = schedule_read_NODE(previous);
      size_t schedule[((size_t)2ull)];
      size_t stack[((size_t)2ull)];
      index_to_permutation(index, schedule, stack, ((size_t)2ull));
      ASSERT((size_t)v_previous - 1 < ((size_t)2ull) && "illegal scalarset "
        "value found during printing");
      v_previous = (raw_value_t)schedule[(size_t)v_previous - 1];
    }
  }
  if (previous == NULL || v != v_previous) {
    if (1 && MACHINE_READABLE_OUTPUT) {
      put("<state_component name=\"");
      do { put("CurPtr"); } while (0);
      put("\" value=\"");
    } else {
      do { put("CurPtr"); } while (0);
      put(":");
    }
    if (v == 0) {
      put("Undefined");
    } else if (USE_SCALARSET_SCHEDULES) {
      if (COUNTEREXAMPLE_TRACE == CEX_OFF) {
        assert(PRINTS_SCALARSETS && "accessing a scalarset "
          "schedule which was unanticipated; bug in"
          "prints_scalarsets()?");
      }
      /* we can use the saved schedule to map this value back to a
       * more intuitive string for the user
       */
      size_t index = schedule_read_NODE(s);
      size_t schedule[((size_t)2ull)];
      size_t stack[((size_t)2ull)];
      index_to_permutation(index, schedule, stack, ((size_t)2ull));
      if (1 && MACHINE_READABLE_OUTPUT) {
        xml_printf("NODE");
      } else {
        put("NODE");
      }
      ASSERT((size_t)v - 1 < ((size_t)2ull) && "illegal scalarset "
        "value found during printing");
      put("_");
      put_uint(schedule[(size_t)v - 1]);
    } else {
      put_val(v - 1);
    }
    if (1 && MACHINE_READABLE_OUTPUT) {
      put("\"/>");
    }
    put("\n");
  }
}
{
  raw_value_t v = handle_read_raw(s, ru_MemData);
  raw_value_t v_previous = 0;
  if (previous != NULL) {
    v_previous = handle_read_raw(previous, ((struct handle){ .base = (uint8_t*)previous->data + (ru_MemData.base - (const uint8_t*)s->data), .offset = ru_MemData.offset, .width = ru_MemData.width }));
    if (USE_SCALARSET_SCHEDULES && v_previous != 0) {
      if (COUNTEREXAMPLE_TRACE == CEX_OFF) {
        assert(PRINTS_SCALARSETS && "accessing a scalarset "
          "schedule which was unanticipated; bug in"
          "prints_scalarsets()?");
      }
      /* we can use the saved schedule to map this value back to a
       * more intuitive string for the user
       */
      size_t index = schedule_read_DATA(previous);
      size_t schedule[((size_t)2ull)];
      size_t stack[((size_t)2ull)];
      index_to_permutation(index, schedule, stack, ((size_t)2ull));
      ASSERT((size_t)v_previous - 1 < ((size_t)2ull) && "illegal scalarset "
        "value found during printing");
      v_previous = (raw_value_t)schedule[(size_t)v_previous - 1];
    }
  }
  if (previous == NULL || v != v_previous) {
    if (1 && MACHINE_READABLE_OUTPUT) {
      put("<state_component name=\"");
      do { put("MemData"); } while (0);
      put("\" value=\"");
    } else {
      do { put("MemData"); } while (0);
      put(":");
    }
    if (v == 0) {
      put("Undefined");
    } else if (USE_SCALARSET_SCHEDULES) {
      if (COUNTEREXAMPLE_TRACE == CEX_OFF) {
        assert(PRINTS_SCALARSETS && "accessing a scalarset "
          "schedule which was unanticipated; bug in"
          "prints_scalarsets()?");
      }
      /* we can use the saved schedule to map this value back to a
       * more intuitive string for the user
       */
      size_t index = schedule_read_DATA(s);
      size_t schedule[((size_t)2ull)];
      size_t stack[((size_t)2ull)];
      index_to_permutation(index, schedule, stack, ((size_t)2ull));
      if (1 && MACHINE_READABLE_OUTPUT) {
        xml_printf("DATA");
      } else {
        put("DATA");
      }
      ASSERT((size_t)v - 1 < ((size_t)2ull) && "illegal scalarset "
        "value found during printing");
      put("_");
      put_uint(schedule[(size_t)v - 1]);
    } else {
      put_val(v - 1);
    }
    if (1 && MACHINE_READABLE_OUTPUT) {
      put("\"/>");
    }
    put("\n");
  }
}
{
  raw_value_t v = handle_read_raw(s, ru_AuxData);
  raw_value_t v_previous = 0;
  if (previous != NULL) {
    v_previous = handle_read_raw(previous, ((struct handle){ .base = (uint8_t*)previous->data + (ru_AuxData.base - (const uint8_t*)s->data), .offset = ru_AuxData.offset, .width = ru_AuxData.width }));
    if (USE_SCALARSET_SCHEDULES && v_previous != 0) {
      if (COUNTEREXAMPLE_TRACE == CEX_OFF) {
        assert(PRINTS_SCALARSETS && "accessing a scalarset "
          "schedule which was unanticipated; bug in"
          "prints_scalarsets()?");
      }
      /* we can use the saved schedule to map this value back to a
       * more intuitive string for the user
       */
      size_t index = schedule_read_DATA(previous);
      size_t schedule[((size_t)2ull)];
      size_t stack[((size_t)2ull)];
      index_to_permutation(index, schedule, stack, ((size_t)2ull));
      ASSERT((size_t)v_previous - 1 < ((size_t)2ull) && "illegal scalarset "
        "value found during printing");
      v_previous = (raw_value_t)schedule[(size_t)v_previous - 1];
    }
  }
  if (previous == NULL || v != v_previous) {
    if (1 && MACHINE_READABLE_OUTPUT) {
      put("<state_component name=\"");
      do { put("AuxData"); } while (0);
      put("\" value=\"");
    } else {
      do { put("AuxData"); } while (0);
      put(":");
    }
    if (v == 0) {
      put("Undefined");
    } else if (USE_SCALARSET_SCHEDULES) {
      if (COUNTEREXAMPLE_TRACE == CEX_OFF) {
        assert(PRINTS_SCALARSETS && "accessing a scalarset "
          "schedule which was unanticipated; bug in"
          "prints_scalarsets()?");
      }
      /* we can use the saved schedule to map this value back to a
       * more intuitive string for the user
       */
      size_t index = schedule_read_DATA(s);
      size_t schedule[((size_t)2ull)];
      size_t stack[((size_t)2ull)];
      index_to_permutation(index, schedule, stack, ((size_t)2ull));
      if (1 && MACHINE_READABLE_OUTPUT) {
        xml_printf("DATA");
      } else {
        put("DATA");
      }
      ASSERT((size_t)v - 1 < ((size_t)2ull) && "illegal scalarset "
        "value found during printing");
      put("_");
      put_uint(schedule[(size_t)v - 1]);
    } else {
      put_val(v - 1);
    }
    if (1 && MACHINE_READABLE_OUTPUT) {
      put("\"/>");
    }
    put("\n");
  }
}
}

static void print_transition(const struct state *NONNULL s __attribute__((unused))) {
  ASSERT(s != NULL);
  static const char *rule_name __attribute__((unused)) = NULL;
#if COUNTEREXAMPLE_TRACE != CEX_OFF

  ASSERT(state_rule_taken_get(s) != 0 && "unknown state transition");

  if (state_previous_get(s) == NULL) {
    uint64_t rule_taken = 1;
    {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_d = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_d == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_d - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_d - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_d += step) {
    uint8_t _ru2_d[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_d = { .base = _ru2_d, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_d, _ru1_d);
  if (state_rule_taken_get(s) == rule_taken) {
    if (MACHINE_READABLE_OUTPUT) {
      put("<transition>");
      xml_printf("Startstate \"init\"");
    } else {
      put("Startstate \"init\"");
    }
    {
      value_t v = (value_t)((rule_taken - 1) / (1) % 2) + VALUE_C(0);
      if (MACHINE_READABLE_OUTPUT) {
        put("<parameter name=\"");
        xml_printf("d");
        put("\">");
      } else {
        put(", d: ");
      }
        if (USE_SCALARSET_SCHEDULES) {
          put("DATA_");
          put_val(v);
        } else {
          put_val(v);
        }
      if (MACHINE_READABLE_OUTPUT) {
        put("</parameter>");
      }
    }
    if (MACHINE_READABLE_OUTPUT) {
      put("</transition>\n");
    } else {
      put(" fired.\n");
    }
    return;
  }
      rule_taken++;
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_d > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_d - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
    }
  } else {
    uint64_t rule_taken = 1;
    {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
  if (state_rule_taken_get(s) == rule_taken) {
    if (MACHINE_READABLE_OUTPUT) {
      put("<transition>");
      xml_printf("Rule \"SendReqS\"");
    } else {
      put("Rule \"SendReqS\"");
    }
    {
      value_t v = (value_t)((rule_taken - 1) / (1) % 2) + VALUE_C(0);
      if (MACHINE_READABLE_OUTPUT) {
        put("<parameter name=\"");
        xml_printf("i");
        put("\">");
      } else {
        put(", i: ");
      }
      {
        size_t schedule[((size_t)2ull)];
        /* setup a default identity mapping for when
         * symmetry reduction is off
         */
        for (size_t i = 0; i < ((size_t)2ull); ++i) {
          schedule[i] = i;
        }
        if (USE_SCALARSET_SCHEDULES) {
          size_t index = schedule_read_NODE(state_previous_get(s));
          size_t stack[((size_t)2ull)];
          index_to_permutation(index, schedule, stack, ((size_t)2ull));
        }
        assert((size_t)v < ((size_t)2ull) && "illegal scalarset  parameter recorded");
        v = (value_t)schedule[(size_t)v];
        if (USE_SCALARSET_SCHEDULES) {
          put("NODE_");
          put_val(v);
        } else {
          put_val(v);
        }
}
      if (MACHINE_READABLE_OUTPUT) {
        put("</parameter>");
      }
    }
    if (MACHINE_READABLE_OUTPUT) {
      put("</transition>\n");
    } else {
      put(" fired.\n");
    }
    return;
  }
      rule_taken++;
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
    }
    {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
  if (state_rule_taken_get(s) == rule_taken) {
    if (MACHINE_READABLE_OUTPUT) {
      put("<transition>");
      xml_printf("Rule \"SendReqE\"");
    } else {
      put("Rule \"SendReqE\"");
    }
    {
      value_t v = (value_t)((rule_taken - 3) / (1) % 2) + VALUE_C(0);
      if (MACHINE_READABLE_OUTPUT) {
        put("<parameter name=\"");
        xml_printf("i");
        put("\">");
      } else {
        put(", i: ");
      }
      {
        size_t schedule[((size_t)2ull)];
        /* setup a default identity mapping for when
         * symmetry reduction is off
         */
        for (size_t i = 0; i < ((size_t)2ull); ++i) {
          schedule[i] = i;
        }
        if (USE_SCALARSET_SCHEDULES) {
          size_t index = schedule_read_NODE(state_previous_get(s));
          size_t stack[((size_t)2ull)];
          index_to_permutation(index, schedule, stack, ((size_t)2ull));
        }
        assert((size_t)v < ((size_t)2ull) && "illegal scalarset  parameter recorded");
        v = (value_t)schedule[(size_t)v];
        if (USE_SCALARSET_SCHEDULES) {
          put("NODE_");
          put_val(v);
        } else {
          put_val(v);
        }
}
      if (MACHINE_READABLE_OUTPUT) {
        put("</parameter>");
      }
    }
    if (MACHINE_READABLE_OUTPUT) {
      put("</transition>\n");
    } else {
      put(" fired.\n");
    }
    return;
  }
      rule_taken++;
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
    }
    {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
  if (state_rule_taken_get(s) == rule_taken) {
    if (MACHINE_READABLE_OUTPUT) {
      put("<transition>");
      xml_printf("Rule \"RecvReqS\"");
    } else {
      put("Rule \"RecvReqS\"");
    }
    {
      value_t v = (value_t)((rule_taken - 5) / (1) % 2) + VALUE_C(0);
      if (MACHINE_READABLE_OUTPUT) {
        put("<parameter name=\"");
        xml_printf("i");
        put("\">");
      } else {
        put(", i: ");
      }
      {
        size_t schedule[((size_t)2ull)];
        /* setup a default identity mapping for when
         * symmetry reduction is off
         */
        for (size_t i = 0; i < ((size_t)2ull); ++i) {
          schedule[i] = i;
        }
        if (USE_SCALARSET_SCHEDULES) {
          size_t index = schedule_read_NODE(state_previous_get(s));
          size_t stack[((size_t)2ull)];
          index_to_permutation(index, schedule, stack, ((size_t)2ull));
        }
        assert((size_t)v < ((size_t)2ull) && "illegal scalarset  parameter recorded");
        v = (value_t)schedule[(size_t)v];
        if (USE_SCALARSET_SCHEDULES) {
          put("NODE_");
          put_val(v);
        } else {
          put_val(v);
        }
}
      if (MACHINE_READABLE_OUTPUT) {
        put("</parameter>");
      }
    }
    if (MACHINE_READABLE_OUTPUT) {
      put("</transition>\n");
    } else {
      put(" fired.\n");
    }
    return;
  }
      rule_taken++;
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
    }
    {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
  if (state_rule_taken_get(s) == rule_taken) {
    if (MACHINE_READABLE_OUTPUT) {
      put("<transition>");
      xml_printf("Rule \"RecvReqE\"");
    } else {
      put("Rule \"RecvReqE\"");
    }
    {
      value_t v = (value_t)((rule_taken - 7) / (1) % 2) + VALUE_C(0);
      if (MACHINE_READABLE_OUTPUT) {
        put("<parameter name=\"");
        xml_printf("i");
        put("\">");
      } else {
        put(", i: ");
      }
      {
        size_t schedule[((size_t)2ull)];
        /* setup a default identity mapping for when
         * symmetry reduction is off
         */
        for (size_t i = 0; i < ((size_t)2ull); ++i) {
          schedule[i] = i;
        }
        if (USE_SCALARSET_SCHEDULES) {
          size_t index = schedule_read_NODE(state_previous_get(s));
          size_t stack[((size_t)2ull)];
          index_to_permutation(index, schedule, stack, ((size_t)2ull));
        }
        assert((size_t)v < ((size_t)2ull) && "illegal scalarset  parameter recorded");
        v = (value_t)schedule[(size_t)v];
        if (USE_SCALARSET_SCHEDULES) {
          put("NODE_");
          put_val(v);
        } else {
          put_val(v);
        }
}
      if (MACHINE_READABLE_OUTPUT) {
        put("</parameter>");
      }
    }
    if (MACHINE_READABLE_OUTPUT) {
      put("</transition>\n");
    } else {
      put(" fired.\n");
    }
    return;
  }
      rule_taken++;
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
    }
    {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
  if (state_rule_taken_get(s) == rule_taken) {
    if (MACHINE_READABLE_OUTPUT) {
      put("<transition>");
      xml_printf("Rule \"SendInv\"");
    } else {
      put("Rule \"SendInv\"");
    }
    {
      value_t v = (value_t)((rule_taken - 9) / (1) % 2) + VALUE_C(0);
      if (MACHINE_READABLE_OUTPUT) {
        put("<parameter name=\"");
        xml_printf("i");
        put("\">");
      } else {
        put(", i: ");
      }
      {
        size_t schedule[((size_t)2ull)];
        /* setup a default identity mapping for when
         * symmetry reduction is off
         */
        for (size_t i = 0; i < ((size_t)2ull); ++i) {
          schedule[i] = i;
        }
        if (USE_SCALARSET_SCHEDULES) {
          size_t index = schedule_read_NODE(state_previous_get(s));
          size_t stack[((size_t)2ull)];
          index_to_permutation(index, schedule, stack, ((size_t)2ull));
        }
        assert((size_t)v < ((size_t)2ull) && "illegal scalarset  parameter recorded");
        v = (value_t)schedule[(size_t)v];
        if (USE_SCALARSET_SCHEDULES) {
          put("NODE_");
          put_val(v);
        } else {
          put_val(v);
        }
}
      if (MACHINE_READABLE_OUTPUT) {
        put("</parameter>");
      }
    }
    if (MACHINE_READABLE_OUTPUT) {
      put("</transition>\n");
    } else {
      put(" fired.\n");
    }
    return;
  }
      rule_taken++;
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
    }
    {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
  if (state_rule_taken_get(s) == rule_taken) {
    if (MACHINE_READABLE_OUTPUT) {
      put("<transition>");
      xml_printf("Rule \"SendInvAck\"");
    } else {
      put("Rule \"SendInvAck\"");
    }
    {
      value_t v = (value_t)((rule_taken - 11) / (1) % 2) + VALUE_C(0);
      if (MACHINE_READABLE_OUTPUT) {
        put("<parameter name=\"");
        xml_printf("i");
        put("\">");
      } else {
        put(", i: ");
      }
      {
        size_t schedule[((size_t)2ull)];
        /* setup a default identity mapping for when
         * symmetry reduction is off
         */
        for (size_t i = 0; i < ((size_t)2ull); ++i) {
          schedule[i] = i;
        }
        if (USE_SCALARSET_SCHEDULES) {
          size_t index = schedule_read_NODE(state_previous_get(s));
          size_t stack[((size_t)2ull)];
          index_to_permutation(index, schedule, stack, ((size_t)2ull));
        }
        assert((size_t)v < ((size_t)2ull) && "illegal scalarset  parameter recorded");
        v = (value_t)schedule[(size_t)v];
        if (USE_SCALARSET_SCHEDULES) {
          put("NODE_");
          put_val(v);
        } else {
          put_val(v);
        }
}
      if (MACHINE_READABLE_OUTPUT) {
        put("</parameter>");
      }
    }
    if (MACHINE_READABLE_OUTPUT) {
      put("</transition>\n");
    } else {
      put(" fired.\n");
    }
    return;
  }
      rule_taken++;
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
    }
    {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
  if (state_rule_taken_get(s) == rule_taken) {
    if (MACHINE_READABLE_OUTPUT) {
      put("<transition>");
      xml_printf("Rule \"RecvInvAck\"");
    } else {
      put("Rule \"RecvInvAck\"");
    }
    {
      value_t v = (value_t)((rule_taken - 13) / (1) % 2) + VALUE_C(0);
      if (MACHINE_READABLE_OUTPUT) {
        put("<parameter name=\"");
        xml_printf("i");
        put("\">");
      } else {
        put(", i: ");
      }
      {
        size_t schedule[((size_t)2ull)];
        /* setup a default identity mapping for when
         * symmetry reduction is off
         */
        for (size_t i = 0; i < ((size_t)2ull); ++i) {
          schedule[i] = i;
        }
        if (USE_SCALARSET_SCHEDULES) {
          size_t index = schedule_read_NODE(state_previous_get(s));
          size_t stack[((size_t)2ull)];
          index_to_permutation(index, schedule, stack, ((size_t)2ull));
        }
        assert((size_t)v < ((size_t)2ull) && "illegal scalarset  parameter recorded");
        v = (value_t)schedule[(size_t)v];
        if (USE_SCALARSET_SCHEDULES) {
          put("NODE_");
          put_val(v);
        } else {
          put_val(v);
        }
}
      if (MACHINE_READABLE_OUTPUT) {
        put("</parameter>");
      }
    }
    if (MACHINE_READABLE_OUTPUT) {
      put("</transition>\n");
    } else {
      put(" fired.\n");
    }
    return;
  }
      rule_taken++;
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
    }
    {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
  if (state_rule_taken_get(s) == rule_taken) {
    if (MACHINE_READABLE_OUTPUT) {
      put("<transition>");
      xml_printf("Rule \"SendGntS\"");
    } else {
      put("Rule \"SendGntS\"");
    }
    {
      value_t v = (value_t)((rule_taken - 15) / (1) % 2) + VALUE_C(0);
      if (MACHINE_READABLE_OUTPUT) {
        put("<parameter name=\"");
        xml_printf("i");
        put("\">");
      } else {
        put(", i: ");
      }
      {
        size_t schedule[((size_t)2ull)];
        /* setup a default identity mapping for when
         * symmetry reduction is off
         */
        for (size_t i = 0; i < ((size_t)2ull); ++i) {
          schedule[i] = i;
        }
        if (USE_SCALARSET_SCHEDULES) {
          size_t index = schedule_read_NODE(state_previous_get(s));
          size_t stack[((size_t)2ull)];
          index_to_permutation(index, schedule, stack, ((size_t)2ull));
        }
        assert((size_t)v < ((size_t)2ull) && "illegal scalarset  parameter recorded");
        v = (value_t)schedule[(size_t)v];
        if (USE_SCALARSET_SCHEDULES) {
          put("NODE_");
          put_val(v);
        } else {
          put_val(v);
        }
}
      if (MACHINE_READABLE_OUTPUT) {
        put("</parameter>");
      }
    }
    if (MACHINE_READABLE_OUTPUT) {
      put("</transition>\n");
    } else {
      put(" fired.\n");
    }
    return;
  }
      rule_taken++;
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
    }
    {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
  if (state_rule_taken_get(s) == rule_taken) {
    if (MACHINE_READABLE_OUTPUT) {
      put("<transition>");
      xml_printf("Rule \"SendGntE\"");
    } else {
      put("Rule \"SendGntE\"");
    }
    {
      value_t v = (value_t)((rule_taken - 17) / (1) % 2) + VALUE_C(0);
      if (MACHINE_READABLE_OUTPUT) {
        put("<parameter name=\"");
        xml_printf("i");
        put("\">");
      } else {
        put(", i: ");
      }
      {
        size_t schedule[((size_t)2ull)];
        /* setup a default identity mapping for when
         * symmetry reduction is off
         */
        for (size_t i = 0; i < ((size_t)2ull); ++i) {
          schedule[i] = i;
        }
        if (USE_SCALARSET_SCHEDULES) {
          size_t index = schedule_read_NODE(state_previous_get(s));
          size_t stack[((size_t)2ull)];
          index_to_permutation(index, schedule, stack, ((size_t)2ull));
        }
        assert((size_t)v < ((size_t)2ull) && "illegal scalarset  parameter recorded");
        v = (value_t)schedule[(size_t)v];
        if (USE_SCALARSET_SCHEDULES) {
          put("NODE_");
          put_val(v);
        } else {
          put_val(v);
        }
}
      if (MACHINE_READABLE_OUTPUT) {
        put("</parameter>");
      }
    }
    if (MACHINE_READABLE_OUTPUT) {
      put("</transition>\n");
    } else {
      put(" fired.\n");
    }
    return;
  }
      rule_taken++;
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
    }
    {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
  if (state_rule_taken_get(s) == rule_taken) {
    if (MACHINE_READABLE_OUTPUT) {
      put("<transition>");
      xml_printf("Rule \"RecvGntS\"");
    } else {
      put("Rule \"RecvGntS\"");
    }
    {
      value_t v = (value_t)((rule_taken - 19) / (1) % 2) + VALUE_C(0);
      if (MACHINE_READABLE_OUTPUT) {
        put("<parameter name=\"");
        xml_printf("i");
        put("\">");
      } else {
        put(", i: ");
      }
      {
        size_t schedule[((size_t)2ull)];
        /* setup a default identity mapping for when
         * symmetry reduction is off
         */
        for (size_t i = 0; i < ((size_t)2ull); ++i) {
          schedule[i] = i;
        }
        if (USE_SCALARSET_SCHEDULES) {
          size_t index = schedule_read_NODE(state_previous_get(s));
          size_t stack[((size_t)2ull)];
          index_to_permutation(index, schedule, stack, ((size_t)2ull));
        }
        assert((size_t)v < ((size_t)2ull) && "illegal scalarset  parameter recorded");
        v = (value_t)schedule[(size_t)v];
        if (USE_SCALARSET_SCHEDULES) {
          put("NODE_");
          put_val(v);
        } else {
          put_val(v);
        }
}
      if (MACHINE_READABLE_OUTPUT) {
        put("</parameter>");
      }
    }
    if (MACHINE_READABLE_OUTPUT) {
      put("</transition>\n");
    } else {
      put(" fired.\n");
    }
    return;
  }
      rule_taken++;
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
    }
    {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
  if (state_rule_taken_get(s) == rule_taken) {
    if (MACHINE_READABLE_OUTPUT) {
      put("<transition>");
      xml_printf("Rule \"RecvGntE\"");
    } else {
      put("Rule \"RecvGntE\"");
    }
    {
      value_t v = (value_t)((rule_taken - 21) / (1) % 2) + VALUE_C(0);
      if (MACHINE_READABLE_OUTPUT) {
        put("<parameter name=\"");
        xml_printf("i");
        put("\">");
      } else {
        put(", i: ");
      }
      {
        size_t schedule[((size_t)2ull)];
        /* setup a default identity mapping for when
         * symmetry reduction is off
         */
        for (size_t i = 0; i < ((size_t)2ull); ++i) {
          schedule[i] = i;
        }
        if (USE_SCALARSET_SCHEDULES) {
          size_t index = schedule_read_NODE(state_previous_get(s));
          size_t stack[((size_t)2ull)];
          index_to_permutation(index, schedule, stack, ((size_t)2ull));
        }
        assert((size_t)v < ((size_t)2ull) && "illegal scalarset  parameter recorded");
        v = (value_t)schedule[(size_t)v];
        if (USE_SCALARSET_SCHEDULES) {
          put("NODE_");
          put_val(v);
        } else {
          put_val(v);
        }
}
      if (MACHINE_READABLE_OUTPUT) {
        put("</parameter>");
      }
    }
    if (MACHINE_READABLE_OUTPUT) {
      put("</transition>\n");
    } else {
      put(" fired.\n");
    }
    return;
  }
      rule_taken++;
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
    }
    {
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_i = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_i == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_i += step) {
    uint8_t _ru2_i[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_i = { .base = _ru2_i, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_i, _ru1_i);
{
  const value_t lb = VALUE_C(0);
  const value_t ub = VALUE_C(1);
  const raw_value_t step = (raw_value_t)(ub >= lb ? 1 : -1);
  if (step == 0) {
    error(s, "infinite loop due to step being 0");
  }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  if ((ub > lb && (value_t)step < 0) ||
      (ub < lb && (value_t)step > 0)) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
    error(s, "infinite loop due to step being in the wrong direction");
  }
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
#endif
  ASSERT(lb >= VALUE_C(0) && lb <= VALUE_C(1) && "iteration lower bound exceeds type limits");
  ASSERT(ub >= VALUE_C(0) && ub <= VALUE_C(1) && "iteration upper bound exceeds type limits");
#if !defined(__clang__) && defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
  for (raw_value_t _ru1_d = ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))); _ru1_d == ((raw_value_t)((raw_value_t)(((raw_value_t)lb) + (raw_value_t)1) - (raw_value_t)(VALUE_C(0)))) || (lb < ub && ((value_t)(_ru1_d - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) <= ub) || (lb > ub && ((value_t)(_ru1_d - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) >= ub); _ru1_d += step) {
    uint8_t _ru2_d[BITS_TO_BYTES(((size_t)2ull))] = { 0 };
    struct handle ru_d = { .base = _ru2_d, .offset = 0, .width = ((size_t)2ull) };
    handle_write_raw(s, ru_d, _ru1_d);
  if (state_rule_taken_get(s) == rule_taken) {
    if (MACHINE_READABLE_OUTPUT) {
      put("<transition>");
      xml_printf("Rule \"Store\"");
    } else {
      put("Rule \"Store\"");
    }
    {
      value_t v = (value_t)((rule_taken - 23) / (1 * 2) % 2) + VALUE_C(0);
      if (MACHINE_READABLE_OUTPUT) {
        put("<parameter name=\"");
        xml_printf("i");
        put("\">");
      } else {
        put(", i: ");
      }
      {
        size_t schedule[((size_t)2ull)];
        /* setup a default identity mapping for when
         * symmetry reduction is off
         */
        for (size_t i = 0; i < ((size_t)2ull); ++i) {
          schedule[i] = i;
        }
        if (USE_SCALARSET_SCHEDULES) {
          size_t index = schedule_read_NODE(state_previous_get(s));
          size_t stack[((size_t)2ull)];
          index_to_permutation(index, schedule, stack, ((size_t)2ull));
        }
        assert((size_t)v < ((size_t)2ull) && "illegal scalarset  parameter recorded");
        v = (value_t)schedule[(size_t)v];
        if (USE_SCALARSET_SCHEDULES) {
          put("NODE_");
          put_val(v);
        } else {
          put_val(v);
        }
}
      if (MACHINE_READABLE_OUTPUT) {
        put("</parameter>");
      }
    }
    {
      value_t v = (value_t)((rule_taken - 23) / (1) % 2) + VALUE_C(0);
      if (MACHINE_READABLE_OUTPUT) {
        put("<parameter name=\"");
        xml_printf("d");
        put("\">");
      } else {
        put(", d: ");
      }
      {
        size_t schedule[((size_t)2ull)];
        /* setup a default identity mapping for when
         * symmetry reduction is off
         */
        for (size_t i = 0; i < ((size_t)2ull); ++i) {
          schedule[i] = i;
        }
        if (USE_SCALARSET_SCHEDULES) {
          size_t index = schedule_read_DATA(state_previous_get(s));
          size_t stack[((size_t)2ull)];
          index_to_permutation(index, schedule, stack, ((size_t)2ull));
        }
        assert((size_t)v < ((size_t)2ull) && "illegal scalarset  parameter recorded");
        v = (value_t)schedule[(size_t)v];
        if (USE_SCALARSET_SCHEDULES) {
          put("DATA_");
          put_val(v);
        } else {
          put_val(v);
        }
}
      if (MACHINE_READABLE_OUTPUT) {
        put("</parameter>");
      }
    }
    if (MACHINE_READABLE_OUTPUT) {
      put("</transition>\n");
    } else {
      put(" fired.\n");
    }
    return;
  }
      rule_taken++;
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_d > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_d - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
    /* If this iteration runs right up to the type limits, the last
     * increment will overflow and fail to terminate, so we guard
     * against that here.
     */
    {
      /* GCC issues spurious -Wsign-compare warnings when doing
       * subtraction on raw_value_t literals, so we suppress these
       * by indirecting via this local constant. For more information:
       * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=38341
       */
      const raw_value_t max_ = RAW_VALUE_MAX;
      if (ub >= lb && (value_t)step > 0 && ((value_t)(max_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ub && _ru1_i > RAW_VALUE_MAX - step) {
        break;
      }
    }
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-unsigned-zero-compare"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  #pragma GCC diagnostic ignored "-Wsign-compare"
#endif
    {
      /* see above explanation of why we use an extra constant here */
      const raw_value_t min_ = RAW_VALUE_MIN;
      if (ub <= lb && (value_t)step < 0 && ((value_t)(min_ - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) > ub && ((value_t)(_ru1_i - (raw_value_t)1 + (raw_value_t)(VALUE_C(0)))) < ((value_t)(RAW_VALUE_MIN - step - (raw_value_t)1 + (raw_value_t)(VALUE_C(0))))) {
#ifdef __clang__
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
        break;
      }
    }
  }
}
    }
  }

  /* give some helpful output for debugging problems with this function. */
  fprintf(stderr, "no rule found to link to state at depth %zu\n", state_depth(s));
  ASSERT(!"unreachable");
#endif
}

static void state_print_field_offsets(void) {
  put("	* state struct is ");
  put_uint(__alignof__(struct state));
  put("-byte aligned\n");
  put("	* field Cache is located at state offset 0 bits\n");
  put("	* field Chan1 is located at state offset 24 bits\n");
  put("	* field Chan2 is located at state offset 34 bits\n");
  put("	* field Chan3 is located at state offset 44 bits\n");
  put("	* field InvSet is located at state offset 8 bits\n");
  put("	* field ShrSet is located at state offset 12 bits\n");
  put("	* field ExGntd is located at state offset 16 bits\n");
  put("	* field CurCmd is located at state offset 54 bits\n");
  put("	* field CurPtr is located at state offset 18 bits\n");
  put("	* field MemData is located at state offset 20 bits\n");
  put("	* field AuxData is located at state offset 22 bits\n");
  put("\n");
}

