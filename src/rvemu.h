#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "elfdef.h"
#include "types.h"

#define fatal(msg) \
  (fprintf(stderr, "fatal: %s:%d %s\n", __FILE__, __LINE__, msg), exit(1))

#define ROUNDDOWN(x, k) ((x) & -(k))
#define ROUNDUP(x, k) (((x) + (k)-1) & -(k))
#define MIN(x, y) ((y) > (x) ? (x) : (y))
#define MAX(x, y) ((y) < (x) ? (x) : (y))

#define GUEST_MEMORY_OFFSET 0x088800000000ULL

#define TO_HOST(addr) (addr + GUEST_MEMORY_OFFSET)
#define TO_GUEST(addr) (addr - GUEST_MEMORY_OFFSET)

/**
 * mmu.c
 */
typedef struct {
  u64 entry;
  u64 host_alloc;
  u64 alloc;
  u64 base;
} mmu_t;

void mmu_load_elf(mmu_t *mmu, int fd);

/**
 * state.c
 */
typedef struct {
  u64 gp_regs[32];
  u64 pc;
} state_t;

/**
 * machine.c
 */
typedef struct {
  state_t state;
  mmu_t mmu;
} machine_t;

void machine_load_program(machine_t *m, char *prog);