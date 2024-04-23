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
#include "reg.h"
#include "types.h"

#define fatal(msg) \
  (fprintf(stderr, "fatal: %s:%d %s\n", __FILE__, __LINE__, msg), exit(1))
#define unreachable() (fatal("unreachable"), __builtin_unreachable())

#define ROUNDDOWN(x, k) ((x) & -(k))
#define ROUNDUP(x, k) (((x) + (k)-1) & -(k))
#define MIN(x, y) ((y) > (x) ? (x) : (y))
#define MAX(x, y) ((y) < (x) ? (x) : (y))

#define GUEST_MEMORY_OFFSET 0x088800000000ULL

#define TO_HOST(addr) (addr + GUEST_MEMORY_OFFSET)
#define TO_GUEST(addr) (addr - GUEST_MEMORY_OFFSET)

enum insn_type_t {
  insn_addi,
  num_insns,
};

typedef struct {
  i8 rd;
  i8 rs1;
  i8 rs2;
  i32 imm;
  enum insn_type_t type;
  bool rvc;
  bool cont;
} insn_t;

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
enum exit_reason_t {
  none,
  direct_branch,
  indirect_branch,
  ecall,
};

typedef struct {
  enum exit_reason_t exit_reason;
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

enum exit_reason_t machine_step(machine_t *m);

/**
 * interp.c
 */
void exec_block_interp(state_t *state);

/**
 * decode.c
 */
void insn_decode(insn_t *insn, u32 data);