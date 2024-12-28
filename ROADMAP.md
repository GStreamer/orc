Versions
========

0.6
---
- [ ] Migrate to GLib?
- [ ] Add support for external opcodes through (plugins)[https://gitlab.freedesktop.org/gstreamer/orc/-/issues/53]?

0.5
---
- [ ] Reduce the API to the minimum to run orcc and the generated code
- [ ] Remove orc-stdint.h.
- [ ] For `ORC_ENABLE_UNSTABLE_API`, create a private header and keep the symbol there
- [X] Make the `OrcX86OpcodeIdx` to not include the accepted type of data (mem, registers, etc)
- [X] The `OrcX86Insn` and `OrcX86Opcode` have redundant fields. The instruction should include the prefix stuff, not the opcode
- [ ] (Improve)[https://gitlab.freedesktop.org/gstreamer/orc/-/issues/55] the accumulator implementation to support not only addition but other (already existed) opcodes 

0.4.9999
--------
- [X] Make all .[ch] files to include only functions that belong to that particular component
- [ ] Have all code formatted accordingly
- [ ] Have a pre-commit hook to avoid new commits with wrong format
- [ ] Avoid using any direct access to a structure from the generated code, use getters/setters instead
- [ ] Avoid using any direct access to a structure from other components, use getters/setters instead
- [X] Put every tool under the tools folder
- [ ] Have a folder per target
- [X] Share all program code between mmx, sse and avx
- [X] Move target code into a target related file, not a program related file
- [ ] Properly set the first shift on x86 loops. A variable with alignment 16 on a 32 byte boundary (AVX) should start the loop at 4 shift, not 0
- [ ] Properly align compiler variables on x86 arch, once a known alignment is know, based on iterations mark the `is_aligned`, not only for one var
- [ ] Make `orc_compiler_append_code` and therefore `ORC_ASM_CODE` keep track of the current instruction, to put the comment in the correct place
- [X] Have an `ORC_TARGET` envvar to choose the target at runtime

Past ideas
==========
The following items come from the previous TODO and ROADMAP files which still need to be confirmed

- [ ] Memory management for executable sections of code.
- [ ] Parser
- [ ] APIs to handle the replacement for normal functions (schro)
- [ ] APIs to handle the construction of functions at runtime (pixman)
- [ ] Handle compilation failure to fallback to emulation
- [ ] Automatic testing of compiled code
- [ ] Automatic speed measurement
- [ ] Better MMX and SSE engine latency/throughput measurement
- [ ] SSE write alignment
- [ ] Handle SSE cache information and small n
- [ ] Contant n in NEON
- [ ] Aligned source checks
- [ ] New opcodes for sampling: video scaling, fir filtering, fir filtering with downsampling, upsampling/downsampling
- [ ] New opcodes for composite
- [ ] Float parameters
- [ ] Inline orc calls in the compiler
- [ ] Initialization functions in the compiler
- [ ] Instruction scheduler
- [ ] Improve emulation
