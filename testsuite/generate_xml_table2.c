
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <orc/orc.h>
#include <orc-test/orctest.h>


int error = FALSE;

char * get_desc (OrcStaticOpcode *opcode);
char * get_code (OrcStaticOpcode *opcode);

int
main (int argc, char *argv[])
{
  int i;
  OrcOpcodeSet *opcode_set;

  orc_init();
  orc_test_init();
  
  printf(
"<table frame=\"all\" id=\"table-basictypes\" xreflabel=\"Table of Opcodes\">\n"
"<title>Table of Opcodes</title>\n"
"<tgroup cols=\"3\" align=\"left\" colsep=\"1\" rowsep=\"1\">\n"
"<thead>\n"
"<row>\n"
"<entry>opcode</entry>\n"
"<entry>destination size</entry>\n"
"<entry>source 1 size</entry>\n"
"<entry>source 2 size</entry>\n"
"<entry>description</entry>\n"
"<entry>pseudo code</entry>\n"
"</row>\n"
"</thead>\n"
"<tbody valign=\"top\">\n");

  opcode_set = orc_opcode_set_get ("sys");

  for(i=0;i<opcode_set->n_opcodes;i++){
    printf("<row>\n");
    printf("<entry>%s</entry>\n", opcode_set->opcodes[i].name);
    printf("<entry>%d</entry>\n", opcode_set->opcodes[i].dest_size[0]);
    printf("<entry>%d</entry>\n", opcode_set->opcodes[i].src_size[0]);
    if (opcode_set->opcodes[i].src_size[1]) {
      printf("<entry>%d%s</entry>\n", opcode_set->opcodes[i].src_size[1],
          (opcode_set->opcodes[i].flags & ORC_STATIC_OPCODE_SCALAR) ? "S" : "");
    } else {
      printf("<entry></entry>\n");
    }
    printf("<entry>%s</entry>\n", get_desc(&opcode_set->opcodes[i]));
    printf("<entry>%s</entry>\n", get_code(&opcode_set->opcodes[i]));
    printf("</row>\n");
  }
  printf(
"</tbody>\n"
"</tgroup>\n"
"</table>\n");

  return 0;
}

struct a {
  char *name;
  char *code;
  char *desc;
};

struct a ops[] = {
  { "absb", "(a &lt; 0) ? -a : a", "absolute value" },
  { "addb", "a + b", "add" },
  { "addssb", "clamp(a + b)", "add with signed saturate" },
  { "addusb", "clamp(a + b)", "add with unsigned saturate" },
  { "andb", "a &amp; b", "bitwise AND" },
  { "andnb", "a &amp; (~b)", "bitwise AND NOT" },
  { "avgsb", "(a + b + 1)&gt;&gt;1", "signed average" },
  { "avgub", "(a + b + 1)&gt;&gt;1", "unsigned average" },
  { "cmpeqb", "(a == b) ? (~0) : 0", "compare equal" },
  { "cmpgtsb", "(a &gt; b) ? (~0) : 0", "compare greater than" },
  { "copyb", "a", "copy" },
  { "maxsb", "(a &gt; b) ? a : b", "signed maximum" },
  { "maxub", "(a &gt; b) ? a : b", "unsigned maximum" },
  { "minsb", "(a &lt; b) ? a : b", "signed minimum" },
  { "minub", "(a &lt; b) ? a : b", "unsigned minimum" },
  { "mullb", "a * b", "low bits of multiply" },
  { "mulhsb", "(a * b) &gt;&gt; 8", "high bits of signed multiply" },
  { "mulhub", "(a * b) &gt;&gt; 8", "high bits of unsigned multiply" },
  { "orb", "a | b", "bitwise or" },
  { "shlb", "a &lt;&lt; b", "shift left" },
  { "shrsb", "a &gt;&gt; b", "signed shift right" },
  { "shrub", "a &gt;&gt; b", "unsigned shift right" },
  { "signb", "sign(a)", "sign" },
  { "subb", "a - b", "subtract" },
  { "subssb", "clamp(a - b)", "subtract with signed saturate" },
  { "subusb", "clamp(a - b)", "subtract with unsigned saturate" },
  { "xorb", "a ^ b", "bitwise XOR" },

  { "absw", "(a &lt; 0) ? -a : a", "absolute value" },
  { "addw", "a + b", "add" },
  { "addssw", "clamp(a + b)", "add with signed saturate" },
  { "addusw", "clamp(a + b)", "add with unsigned saturate" },
  { "andw", "a &amp; b", "bitwise AND" },
  { "andnw", "a &amp; (~b)", "bitwise AND NOT" },
  { "avgsw", "(a + b + 1)&gt;&gt;1", "signed average" },
  { "avguw", "(a + b + 1)&gt;&gt;1", "unsigned average" },
  { "cmpeqw", "(a == b) ? (~0) : 0", "compare equal" },
  { "cmpgtsw", "(a &gt; b) ? (~0) : 0", "compare greater than" },
  { "copyw", "a", "copy" },
  { "maxsw", "(a &gt; b) ? a : b", "signed maximum" },
  { "maxuw", "(a &gt; b) ? a : b", "unsigned maximum" },
  { "minsw", "(a &lt; b) ? a : b", "signed minimum" },
  { "minuw", "(a &lt; b) ? a : b", "unsigned minimum" },
  { "mullw", "a * b", "low bits of multiply" },
  { "mulhsw", "(a * b) &gt;&gt; 8", "high bits of signed multiply" },
  { "mulhuw", "(a * b) &gt;&gt; 8", "high bits of unsigned multiply" },
  { "orw", "a | b", "bitwise or" },
  { "shlw", "a &lt;&lt; b", "shift left" },
  { "shrsw", "a &gt;&gt; b", "signed shift right" },
  { "shruw", "a &gt;&gt; b", "unsigned shift right" },
  { "signw", "sign(a)", "sign" },
  { "subw", "a - b", "subtract" },
  { "subssw", "clamp(a - b)", "subtract with signed saturate" },
  { "subusw", "clamp(a - b)", "subtract with unsigned saturate" },
  { "xorw", "a ^ b", "bitwise XOR" },

  { "absl", "(a &lt; 0) ? -a : a", "absolute value" },
  { "addl", "a + b", "add" },
  { "addssl", "clamp(a + b)", "add with signed saturate" },
  { "addusl", "clamp(a + b)", "add with unsigned saturate" },
  { "andl", "a &amp; b", "bitwise AND" },
  { "andnl", "a &amp; (~b)", "bitwise AND NOT" },
  { "avgsl", "(a + b + 1)&gt;&gt;1", "signed average" },
  { "avgul", "(a + b + 1)&gt;&gt;1", "unsigned average" },
  { "cmpeql", "(a == b) ? (~0) : 0", "compare equal" },
  { "cmpgtsl", "(a &gt; b) ? (~0) : 0", "compare greater than" },
  { "copyl", "a", "copy" },
  { "maxsl", "(a &gt; b) ? a : b", "signed maximum" },
  { "maxul", "(a &gt; b) ? a : b", "unsigned maximum" },
  { "minsl", "(a &lt; b) ? a : b", "signed minimum" },
  { "minul", "(a &lt; b) ? a : b", "unsigned minimum" },
  { "mulll", "a * b", "low bits of multiply" },
  { "mulhsl", "(a * b) &gt;&gt; 16", "high bits of signed multiply" },
  { "mulhul", "(a * b) &gt;&gt; 16", "high bits of unsigned multiply" },
  { "orl", "a | b", "bitwise or" },
  { "shll", "a &lt;&lt; b", "shift left" },
  { "shrsl", "a &gt;&gt; b", "signed shift right" },
  { "shrul", "a &gt;&gt; b", "unsigned shift right" },
  { "signl", "sign(a)", "sign" },
  { "subl", "a - b", "subtract" },
  { "subssl", "clamp(a - b)", "subtract with signed saturate" },
  { "subusl", "clamp(a - b)", "subtract with unsigned saturate" },
  { "xorl", "a ^ b", "bitwise XOR" },

  { "convsbw", "a", "convert signed" },
  { "convubw", "a", "convert unsigned" },
  { "convswl", "a", "convert signed" },
  { "convuwl", "a", "convert unsigned" },
  { "convwb", "a", "convert" },
  { "convssswb", "clamp(a)", "convert signed to signed with saturation" },
  { "convsuswb", "clamp(a)", "convert signed to unsigned with saturation" },
  { "convusswb", "clamp(a)", "convert unsigned to signed with saturation" },
  { "convuuswb", "clamp(a)", "convert unsigned to unsigned with saturation" },
  { "convlw", "a", "convert" },
  { "convssslw", "clamp(a)", "convert signed to signed with saturation" },
  { "convsuslw", "clamp(a)", "convert signed to unsigned with saturation" },
  { "convusslw", "clamp(a)", "convert unsigned to signed with saturation" },
  { "convuuslw", "clamp(a)", "convert unsigned to unsigned with saturation" },
  { "mulsbw", "a * b", "multiply signed" },
  { "mulubw", "a * b", "multiply unsigned" },
  { "mulswl", "a * b", "multiply signed" },
  { "muluwl", "a * b", "multiply unsigned" },
  { "mergewl", "special", "merge halves" },
  { "mergebw", "special", "merge halves" },
  { "select0wb", "special", "select first half" },
  { "select1wb", "special", "select second half" },
  { "select0lw", "special", "select first half" },
  { "select1lw", "special", "select second half" },
  { "swapw", "special", "endianness swap" },
  { "swapl", "special", "endianness swap" },

  { "accw", "+= a", "accumulate" },
  { "accl", "+= a", "accumulate" },
  { "accsadubl", "+= abs(a - b)", "accumulate absolute difference" },

  { "splitlw" , "special", "split first/second words" },
  { "splitwb" , "special", "split first/second bytes" },
  { "addf", "a + b", "add" },
  { "subf", "a - b", "subtract" },
  { "mulf", "a * b", "multiply" },
  { "divf", "a / b", "divide" },
  { "sqrtf", "sqrt(a)", "square root" },
  { "maxf", "max(a,b)", "maximum" },
  { "minf", "min(a,b)", "minimum" },
  { "cmpeqf", "(a == b) ? (~0) : 0", "compare equal" },
  { "cmpltf", "(a == b) ? (~0) : 0", "compare less than" },
  { "cmplef", "(a == b) ? (~0) : 0", "compare less than or equal" },
  { "convfl", "a", "convert float point to integer" },
  { "convlf", "a", "convert integer to floating point" },

  { "loadb", "array[i]", "load from memory" },
  { "loadw", "array[i]", "load from memory" },
  { "loadl", "array[i]", "load from memory" },
  { "loadq", "array[i]", "load from memory" },
  { "storeb", "special", "store to memory" },
  { "storew", "special", "store to memory" },
  { "storel", "special", "store to memory" },
  { "storeq", "special", "store to memory" },
  { "loadoffb", "array[i+offset]", "load from memory with offset" },
  { "loadoffw", "array[i+offset]", "load from memory with offset" },
  { "loadoffl", "array[i+offset]", "load from memory with offset" },
  { "loadoffq", "array[i+offset]", "load from memory with offset" },
  { "loadpb", "scalar", "load parameter or constant" },
  { "loadpw", "scalar", "load parameter or constant" },
  { "loadpl", "scalar", "load parameter or constant" },
  { "loadpq", "scalar", "load parameter or constant" },
  { "loadupdb", "array[i>>1]", "load upsampled duplicate" },
  { "loadupib", "(array[i>>1] + array[(i+1)>>1] + 1)>>1",
    "load upsampled interpolate" },
  { "ldresnearb", "array[(b+c*i)>>8]", "load, nearest neighbor resampled" },
  { "ldreslinb", "special", "load, bilinear resampled" },
  { "ldresnearl", "array[(b+c*i)>>8]", "load, nearest neighbor resampled" },
  { "ldreslinl", "special", "load, bilinear resampled" },
  { "div255w", "a/255", "divide by 255" },
  { "divluw", "clamp(a/(b &amp; 255),0,255)", "saturated unsigned divide 16-bit by 8-bit" },
  { "splatw3q", "special", "duplicates high 16-bits to lower 48 bits" },
  { "splatbw", "special", "duplicates 8 bits to both halfs of 16 bits" },
  { "splatbl", "special", "duplicates 8 bits to all parts of 32 bits" },
  { "convql", "a", "convert" },
  { "convslq", "a", "signed convert" },
  { "convulq", "a", "unsigned convert" },
  { "convhwb", "a>>8", "shift and convert" },
  { "convhlw", "a>>16", "shift and convert" },
  { "convsssql", "clamp(a)", "convert signed to signed with saturation" },
  { "convsusql", "clamp(a)", "convert signed to unsigned with saturation" },
  { "convussql", "clamp(a)", "convert unsigned to signed with saturation" },
  { "convuusql", "clamp(a)", "convert unsigned to unsigned with saturation" },

};


char *
get_desc (OrcStaticOpcode *opcode)
{
  int i;
  for(i=0;i<sizeof(ops)/sizeof(ops[0]);i++){
    if (strcmp (opcode->name, ops[i].name) == 0) {
      return ops[i].desc;
    }
  }
  return "";
}

char *
get_code (OrcStaticOpcode *opcode)
{
  int i;
  for(i=0;i<sizeof(ops)/sizeof(ops[0]);i++){
    if (strcmp (opcode->name, ops[i].name) == 0) {
      return ops[i].code;
    }
  }
  return "";
}

