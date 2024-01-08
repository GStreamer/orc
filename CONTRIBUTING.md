Contributing
============

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
`$build_dir/orc/generate-emulation -o orc/orcemulateopcodes.c`.

This will generate the emulation code needed in case the target platform does not support the
corresponding opcode. You need at least one target implementation of the new opcode in order to get accepted

Finally, to update the documentation, you first need to add the corresponding opcode at
`testsuite/generate_xml_table2.c` and generate it by calling
`$build_dir/orc/testsuite/generate_xml_table2 > doc/opcode_table.xml` and update the target support table
by generating the table with `$build_dir/orc/testsuite/generate_xml_table > doc/table.xml` 
