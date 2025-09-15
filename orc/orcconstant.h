
#ifndef _ORC_CONSTANT_H_
#define _ORC_CONSTANT_H_

#include <orc/orcutils.h>
#include <orc/orclimits.h>

ORC_BEGIN_DECLS

/**
 * OrcConstantType:
 *
 * @ORC_CONST_ZERO      : A constant with value zero
 * @ORC_CONST_SPLAT_B   : The byte, 1 byte (8 bits), will be repeated in all the register
 * @ORC_CONST_SPLAT_W   : The word, 2 bytes (16 bits), will be repeated in all the register
 * @ORC_CONST_SPLAT_L   : The doubleword, 4 bytes (32 bits), will be repeated in all the register
 * @ORC_CONST_SPLAT_Q   : The quadword, 8 bytes (64 bits), will be repeated in all the register
 * @ORC_CONST_SPLAT_DQ  : The double quadword, 16 bytes (128 bits), will be repeated in all the register
 * @ORC_CONST_SPLAT_QQ  : The quadruple quadword, 32 bytes (512 bits), will be repeated in all the register
 * @ORC_CONSTANT_FULL   : The constant will be stored as is
 */

typedef enum _OrcConstantType {
  ORC_CONST_ZERO,
  ORC_CONST_SPLAT_B,
  ORC_CONST_SPLAT_W,
  ORC_CONST_SPLAT_L,
  ORC_CONST_FULL,
  ORC_CONST_SPLAT_Q,
  ORC_CONST_SPLAT_DQ,
  ORC_CONST_SPLAT_QQ,
} OrcConstantType;

/**
 * OrcConstant:
 *
 * The OrcConstant structure has no public members
 */
struct _OrcConstant {
  /*< private >*/
  OrcConstantType type;
  int alloc_reg; // This should be part of a new OrcCompilerConstant
  unsigned int value; // unused
  unsigned int full_value[4]; // unused
  int use_count;
  int is_long; // unused
  int label;
  orc_union64 v[8]; // A total of 8*8 bytes = 512 bits
};

#ifdef ORC_ENABLE_UNSTABLE_API
ORC_API orc_bool orc_constant_resolve (const OrcConstant *c, OrcConstant *r,
    int reg_size);
ORC_API orc_bool orc_constant_is_equal (const OrcConstant *c1,
    const OrcConstant *c2, int reg_size);

/* The following helper macros set 1, 2, 4, 8, 16, 32 or 64 bytes of data
 * in little-endian endianness
 */
#define ORC_CONSTANT_INIT_U8(u8) { .type = ORC_CONST_SPLAT_B, .v[0].x8[0] = u8 }
#define ORC_CONSTANT_INIT_U16(u16) { .type = ORC_CONST_SPLAT_W, .v[0].x4[0] = u16 }
#define ORC_CONSTANT_INIT_U32(u32) { .type = ORC_CONST_SPLAT_L, .v[0].x2[0] = u32 }
#define ORC_CONSTANT_INIT_U64(u64) { .type = ORC_CONST_SPLAT_Q, .v[0].i = u64 }
#define ORC_CONSTANT_INIT_U128(a, b) { .type = ORC_CONST_SPLAT_DQ, .v[0].i = a, .v[1].i = b }
#define ORC_CONSTANT_INIT_U256(a, b, c, d) { .type = ORC_CONST_SPLAT_QQ, .v[0].i = a, .v[1].i = b, .v[2].i = c, .v[3].i = d }
#define ORC_CONSTANT_INIT_U512(a, b, c, d, e, f, g, h) { .type = ORC_CONST_FULL, .v[0].i = a, .v[1].i = b, .v[2].i = c, .v[3].i = d, .v[4].i = e, .v[5].i = f,  .v[6].i = g, .v[7].i = h }

#endif

ORC_END_DECLS

#endif
