#ifndef rvemu_RVEMU_H
#define rvemu_RVEMU_H

#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "elfdef.h"
#include "reg.h"
#include "types.h"

#define fatalf(fmt, ...)                                                       \
  (fprintf(stderr, "fatal: %s:%d " fmt "\n", __FILE__, __LINE__, __VA_ARGS__), \
   exit(1))
#define fatal(msg) \
  (fprintf(stderr, "fatal: %s:%d %s\n", __FILE__, __LINE__, msg), exit(1))
#define unreachable() (fatal("unreachable"), __builtin_unreachable())

#define ROUNDDOWN(x, k) ((x) & -(k))
#define ROUNDUP(x, k) (((x) + (k)-1) & -(k))
#define MIN(x, y) ((y) > (x) ? (x) : (y))
#define MAX(x, y) ((y) < (x) ? (x) : (y))

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define GUEST_MEMORY_OFFSET 0x088800000000ULL

#define TO_HOST(addr) (addr + GUEST_MEMORY_OFFSET)
#define TO_GUEST(addr) (addr - GUEST_MEMORY_OFFSET)

enum insn_type_t {
  insn_lb,
  insn_lh,
  insn_lw,
  insn_ld,
  insn_lbu,
  insn_lhu,
  insn_lwu,
  insn_fence,
  insn_fence_i,
  insn_addi,
  insn_slli,
  insn_slti,
  insn_sltiu,
  insn_xori,
  insn_srli,
  insn_srai,
  insn_ori,
  insn_andi,
  insn_auipc,
  insn_addiw,
  insn_slliw,
  insn_srliw,
  insn_sraiw,
  insn_sb,
  insn_sh,
  insn_sw,
  insn_sd,
  insn_add,
  insn_sll,
  insn_slt,
  insn_sltu,
  insn_xor,
  insn_srl,
  insn_or,
  insn_and,
  insn_mul,
  insn_mulh,
  insn_mulhsu,
  insn_mulhu,
  insn_div,
  insn_divu,
  insn_rem,
  insn_remu,
  insn_sub,
  insn_sra,
  insn_lui,
  insn_addw,
  insn_sllw,
  insn_srlw,
  insn_mulw,
  insn_divw,
  insn_divuw,
  insn_remw,
  insn_remuw,
  insn_subw,
  insn_sraw,
  insn_beq,
  insn_bne,
  insn_blt,
  insn_bge,
  insn_bltu,
  insn_bgeu,
  insn_jalr,
  insn_jal,
  insn_ecall,
  insn_csrrc,
  insn_csrrci,
  insn_csrrs,
  insn_csrrsi,
  insn_csrrw,
  insn_csrrwi,
  insn_flw,
  insn_fsw,
  insn_fmadd_s,
  insn_fmsub_s,
  insn_fnmsub_s,
  insn_fnmadd_s,
  insn_fadd_s,
  insn_fsub_s,
  insn_fmul_s,
  insn_fdiv_s,
  insn_fsqrt_s,
  insn_fsgnj_s,
  insn_fsgnjn_s,
  insn_fsgnjx_s,
  insn_fmin_s,
  insn_fmax_s,
  insn_fcvt_w_s,
  insn_fcvt_wu_s,
  insn_fmv_x_w,
  insn_feq_s,
  insn_flt_s,
  insn_fle_s,
  insn_fclass_s,
  insn_fcvt_s_w,
  insn_fcvt_s_wu,
  insn_fmv_w_x,
  insn_fcvt_l_s,
  insn_fcvt_lu_s,
  insn_fcvt_s_l,
  insn_fcvt_s_lu,
  insn_fld,
  insn_fsd,
  insn_fmadd_d,
  insn_fmsub_d,
  insn_fnmsub_d,
  insn_fnmadd_d,
  insn_fadd_d,
  insn_fsub_d,
  insn_fmul_d,
  insn_fdiv_d,
  insn_fsqrt_d,
  insn_fsgnj_d,
  insn_fsgnjn_d,
  insn_fsgnjx_d,
  insn_fmin_d,
  insn_fmax_d,
  insn_fcvt_s_d,
  insn_fcvt_d_s,
  insn_feq_d,
  insn_flt_d,
  insn_fle_d,
  insn_fclass_d,
  insn_fcvt_w_d,
  insn_fcvt_wu_d,
  insn_fcvt_d_w,
  insn_fcvt_d_wu,
  insn_fcvt_l_d,
  insn_fcvt_lu_d,
  insn_fmv_x_d,
  insn_fcvt_d_l,
  insn_fcvt_d_lu,
  insn_fmv_d_x,
  num_insns,
};

typedef struct {
  i8 rd;
  i8 rs1;
  i8 rs2;
  i8 rs3;
  i32 imm;
  i16 csr;
  enum insn_type_t type;
  bool rvc;
  bool cont;
} insn_t;

/**
 * stack.c
 */
#define STACK_CAP 256
typedef struct {
  i64 top;
  u64 elems[STACK_CAP];
} stack_t;

void stack_push(stack_t *stack, u64 elem);
bool stack_pop(stack_t *stack, u64 *elem);
void stack_reset(stack_t *stack);
void stack_print(stack_t *stack);

/**
 * str.c
 */
#define STR_MAX_PREALLOC (1024 * 1024)
#define STRHDR(s) ((strhdr_t *)((s) - (sizeof(strhdr_t))))

#define DECLARE_STATIC_STR(name) \
  static str_t name = NULL;      \
  if (name)                      \
    str_clear(name);             \
  else                           \
    name = str_new();

typedef char *str_t;

typedef struct {
  u64 len;
  u64 alloc;
  char buf[];
} strhdr_t;

inline str_t str_new() {
  strhdr_t *h = (strhdr_t *)calloc(1, sizeof(strhdr_t));
  return h->buf;
}

inline size_t str_len(const str_t str) { return STRHDR(str)->len; }

void str_clear(str_t str);

str_t str_append(str_t str, const char *t);

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
u64 mmu_alloc(mmu_t *mmu, i64 sz);
inline void mmu_write(u64 addr, u8 *data, size_t len) {
  memcpy((void *)TO_HOST(addr), (void *)data, len);
}

/**
 * cache.c
 */
#define CACHE_ENTRY_SIZE (64 * 1024)
#define CACHE_SIZE (64 * 1024 * 1024)

typedef struct {
  u64 pc;
  u64 hot;
  u64 offset;
} cache_item_t;

typedef struct {
  u8 *jitcode;
  u64 offset;
  cache_item_t table[CACHE_ENTRY_SIZE];
} cache_t;

cache_t *new_cache();
u8 *cache_lookup(cache_t *cache, u64 pc);
u8 *cache_add(cache_t *cache, u64 pc, u8 *code, size_t sz, u64 align);
bool cache_hot(cache_t *cache, u64 pc);

/**
 * state.c
 */
enum exit_reason_t {
  none,
  direct_branch,
  indirect_branch,
  ecall,
};

enum csr_t {
  fflags = 0x001,
  frm = 0x002,
  fcsr = 0x003,
};

typedef struct {
  enum exit_reason_t exit_reason;
  u64 reenter_pc;
  u64 gp_regs[num_gp_regs];
  fp_reg_t fp_regs[num_fp_regs];
  u64 pc;
} state_t;

/**
 * machine.c
 */
typedef struct {
  state_t state;
  mmu_t mmu;
} machine_t;

inline u64 machine_get_gp_reg(machine_t *m, i32 reg) {
  assert(reg >= 0 && reg <= num_gp_regs);
  return m->state.gp_regs[reg];
}

inline void machine_set_gp_reg(machine_t *m, i32 reg, u64 data) {
  assert(reg >= 0 && reg <= num_gp_regs);
  m->state.gp_regs[reg] = data;
}

void machine_setup(machine_t *m, int argc, char *argv[]);
void machine_load_program(machine_t *m, char *prog);
enum exit_reason_t machine_step(machine_t *m);

/**
 * interp.c
 */
void exec_block_interp(state_t *state);

/**
 * set.c
 */
#define SET_SIZE (32 * 1024)

typedef struct {
  u64 table[SET_SIZE];
} set_t;

bool set_hash(set_t *set, u64 elem);
bool set_add(set_t *set, u64 elem);
void set_reset(set_t *set);

/**
 * decode.c
 */
void insn_decode(insn_t *insn, u32 data);

/**
 * syscall.c
 */
u64 do_syscall(machine_t *m, u64 n);

#endif  // rvemu_RVEMU_H