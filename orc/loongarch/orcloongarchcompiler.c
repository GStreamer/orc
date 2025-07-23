/*
  Copyright (c) 2025 Loongson Technology Corporation Limited

  Author: jinbo, jinbo@loongson.cn
  Author: hecai yuan, yuanhecai@loongson.cn

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#include <orc/orccompiler.h>
#include <orc/orcprogram.h>
#include <orc/orcinternal.h>
#include <orc/orcdebug.h>

typedef enum {
  ORC_LOONG_OFFS16,
  ORC_LOONG_OFFS21,
  ORC_LOONG_OFFS26
} OrcLoongBranchType;

void
orc_loongarch_compiler_add_label (OrcCompiler *c, int label)
{
  ORC_ASSERT (label < ORC_N_LABELS);

  ORC_ASM_CODE (c, ".L%s_%d:\n", c->program->name, label);
  c->labels[label] = c->codeptr;
}

void
orc_loongarch_compiler_add_fixup (OrcCompiler *c, int label, int type)
{
  ORC_ASSERT (c->n_fixups < ORC_N_FIXUPS);

  c->fixups[c->n_fixups].ptr = c->codeptr;
  c->fixups[c->n_fixups].label = label;
  c->fixups[c->n_fixups].type = type;
  c->n_fixups++;
}

void
orc_loongarch_compiler_do_fixups (OrcCompiler *c)
{
  int i;
  for(i=0;i<c->n_fixups;i++){
    unsigned char *label = c->labels[c->fixups[i].label];
    unsigned char *ptr = c->fixups[i].ptr;
    orc_uint32 code;
    int offset;

    offset = (label - ptr) >> 2;
    code = ORC_READ_UINT32_LE (ptr);

    switch (c->fixups[i].type) {
      case ORC_LOONG_OFFS16:
        code |= (offset & 0xffff) << 10;
        break;
      case ORC_LOONG_OFFS21:
        code |= (offset & 0xffff) << 10 | ((offset >> 16) & 0x1f);
        break;
      case ORC_LOONG_OFFS26:
        code |= (offset & 0xffff) << 10 | ((offset >> 16) & 0x3ff);
        break;
       default:
        ORC_ASSERT ("Invalid type"); /* unreachable */
        break;
    }
    ORC_WRITE_UINT32_LE (ptr, code);
  }
}
