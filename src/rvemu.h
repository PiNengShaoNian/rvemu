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

/**
 * mmu.c
 */
typedef struct {
  u64 entry;
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