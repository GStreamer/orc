
#ifndef _ORC_ORC_CPUINSN_H_
#define _ORC_ORC_CPUINSN_H_

#ifdef ORC_ENABLE_UNSTABLE_API

#include <orc/orcutils.h>

struct _OrcSysOpcode {
  // Mnemonic
  char name[16];
  // Type of instruction (source instruction set, operation type)
  int type;
  // (x86 only) Escape prefix (0F) 3A or 38
  // See ss. 3.1.1.2 Intel® 64 and IA-32 Architectures Software Developer’s
  // Manual
  int flags;
  // (x86 only) Instruction set prefix (EVEX, VEX, 2-byte SSE...)
  // See ss. 2.3.5 Intel® 64 and IA-32 Architectures Software Developer’s
  // Manual
  orc_uint8 prefix;
  // Opcode (may include the last byte of the prefix as an opcode extension)
  // For x86, see ss. 2.1.2 Intel® 64 and IA-32 Architectures Software
  // Developer’s Manual
  orc_uint32 code;
  // Additional opcode (if any)
  int code2;
};

typedef struct _OrcSysOpcode OrcSysOpcode;

#define ORC_SYS_OPCODE_FLAG_FIXED (1<<0)

#endif

#endif

