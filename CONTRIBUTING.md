Contributing
============

Commit message
--------------
It is preferable to prefix the commit message with the affected component. For
example, in case the modified component is the `orc` compiler: `orcc`, prefix
the commit with `orcc:`. Some of the components are:

* docs: Anything related to documentation
* [TARGET]: In case modifications of a particular target like sse or avx
* orcc: For a change in the compiler

How to add a new architecture / instruction set target?
-------------------------------------------------------

There are roughly three steps to add a new architecture or instruction set backend:

1. Add detection of the relevant instruction sets
2. Define a new `OrcTarget` for the architecture or instruction set
3. Implement the necessary codegen functions
3. Initialize and register the target in `orc.c`

### Adding detection of instruction sets

If you need to add a new instruction set to an existing architecture, you should
only need to add the required CPUID bits or defines to the existing machinery.
A good example is [the commit for detecting AVX](https://gitlab.freedesktop.org/gstreamer/orc/-/commit/bd7851494c33d9f79cd610a91d889b5e74dfb116).

Conversely, if you need to add it from scratch, a good idea might be to look at
[how Arm and NEON support is detected](https://gitlab.freedesktop.org/gstreamer/orc/blob/ba0b5f2a56dab846a4926a493d76ed0b24a60570/orc/orccpu-arm.c#L68).

Either way, you'll need to add to `orctarget.h` (or another header,
depending on your backend's structure) the flags you'll use to detect
the instruction sets.

### The new `OrcTarget`

The `OrcTarget` structure looks like this:

```c
struct _OrcTarget {
  const char *name;
  orc_bool executable;
  int data_register_offset;

  unsigned int (*get_default_flags)(void);
  void (*compiler_init)(OrcCompiler *compiler);
  void (*compile)(OrcCompiler *compiler);

  OrcRuleSet rule_sets[ORC_N_RULE_SETS];
  int n_rule_sets;

  const char * (*get_asm_preamble)(void);
  void (*load_constant)(OrcCompiler *compiler, int reg, int size, int value);
  const char * (*get_flag_name)(int shift);
  void (*flush_cache) (OrcCode *code);
  void (*load_constant_long)(OrcCompiler *compiler, int reg,
      OrcConstant *constant);

  void *_unused[5];
};
```

- `name`: a name for your target (usually the architecture or instruction set in lowercase)
- `executable`: set to `TRUE` on initialization if your CPUID returns the instruction set is available
- `data_register_offset`: this is an offset against the `OrcCompiler`'s table of available registers that will tell it where to start storing the available SIMD registers. Usually expands to `ORC_VEC_REG_BASE`.
- `get_default_flags`: call up your CPUID implementation here, and/or return any other relevant architecture flag defaults for the `OrcCompiler`.
- `compiler_init`: here you need to set up the table of available registers, which registers are callee saved (if applicable), and the relevant codegen rules for your architecture. There are also other flags you need to set, for which [this example](https://gitlab.freedesktop.org/gstreamer/orc/-/blob/1fb793ea5aabb6a5b16308465f63c5722437b20b/orc/orcprogram-avx.c#L117-239) may be a better description.
- `compile`: here you'll generate the machine and assembly code and store them in the `OrcCompiler`'s `program` member.
- `rule_sets`: initialize to `{{ 0 }}` as rules should be registered at compiler initialization time.
- `n_rule_sets`: initialize to `0` for the same reasons as above.
- `get_asm_preamble`: if your assembler (GNU AS currently) needs a fixed preamble for every function, it must be codegen'd here.
- `load_constant`: codegens a broadcast of `value` into the provided `reg` vector register.
- `get_flag_name`: maps the list of CPUID flags to a readable string (see the previous section).
- `flush_cache`: some architectures, like Arm, may need to flush the instruction cache for a given address range. If necessary, this must be implemented in this function.
- `load_constant_long`: same as `load_constant` but this is a multi-word constant (for the cases where the value doesn't fit into a 32-bit integer, like mask constants).

### Generating the machine and assembly code

This step is highly architecture and implementation dependent, but basically
you'll read a stream of `OrcInstruction`s provided by the `OrcCompiler`
and write:

- machine code, to the region provided in `compiler->codeptr` (don't forget to advance the pointer as you write bytes!)
- assembly code, with the `printf`-like function `orc_compiler_append_code`

### Registering the target

You need to call up your target registration function in `orc_init` inside the 
file `orc.c`. Make sure to guard it with `#ifdef ENABLE_TARGET_<YOUR_ARCH_HERE>`
so that it's only inserted if your new backend is being built.

The target registration function must create the `OrcTarget` structure described
earlier, register it with `orc_target_register (&target)`, and then register
the available rule sets with `orc_rule_set_new` and `orc_rule_register`.

How to add a new target instruction?
------------------------------------
Each target has a way to generate its own instructions. For example, in SSE,
you have the function `orc_sse_emit_mulps` which will add a `mulps`
instruction to the corresponding program. If you need a new instruction to be
available, as long as the instruction is used in `orc` as part of an
`OrcStaticOpcode`, it will be accepted; otherwise, it won't.

How to add a new `orc` opcode?
------------------------------
In order to add a new `OrcStaticOpcode` to `orc`, you first need to add the new
opcode to `orc/orcopcodes.c`at the end of the `opcodes` array. This is important
to not break bytecode ABI.

Once it is done, you need to declare the new opcode in `orc/opcodes.h`  and
generate the emulation code by calling
`$build_dir/tools/generate-emulation -o orc/orcemulateopcodes.c`.

This will generate the emulation code needed in case the target platform does not support the
corresponding opcode. You need at least one target implementation of the new opcode in order to get accepted

Finally, to update the documentation, you first need to add the corresponding opcode at
`tools/generate_xml_table2.c` and generate it by calling
`$build_dir/tools/generate_xml_table2 > doc/opcode_table.xml` and update the target support table
by generating the table with `$build_dir/tools/generate_xml_table > doc/table.xml` 
