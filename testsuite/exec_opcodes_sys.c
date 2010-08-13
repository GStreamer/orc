
#include "config.h"

#include <stdio.h>

#include <orc/orc.h>
#include <orc-test/orctest.h>


int error = FALSE;
int verbose = FALSE;

void test_opcode_src (OrcStaticOpcode *opcode);
void test_opcode_const (OrcStaticOpcode *opcode);
void test_opcode_param (OrcStaticOpcode *opcode);
void test_opcode_inplace (OrcStaticOpcode *opcode);
void test_opcode_src_2d (OrcStaticOpcode *opcode);
void test_opcode_src_const_n (OrcStaticOpcode *opcode);
void test_opcode_src_const_n_2d (OrcStaticOpcode *opcode);

int
main (int argc, char *argv[])
{
  int i;
  OrcOpcodeSet *opcode_set;

  orc_test_init();
  orc_init();

  opcode_set = orc_opcode_set_get ("sys");

  for(i=0;i<opcode_set->n_opcodes;i++){
    if (verbose) printf("%s src %d,%d,%d,%d\n",
        opcode_set->opcodes[i].name,
        opcode_set->opcodes[i].dest_size[0],
        opcode_set->opcodes[i].dest_size[1],
        opcode_set->opcodes[i].src_size[0],
        opcode_set->opcodes[i].src_size[1]);
    test_opcode_src (opcode_set->opcodes + i);
  }
  for(i=0;i<opcode_set->n_opcodes;i++){
    if (verbose) printf("%s const %d,%d,%d\n",
        opcode_set->opcodes[i].name,
        opcode_set->opcodes[i].dest_size[0],
        opcode_set->opcodes[i].src_size[0],
        opcode_set->opcodes[i].src_size[1]);
    test_opcode_const (opcode_set->opcodes + i);
  }
  for(i=0;i<opcode_set->n_opcodes;i++){
    if (verbose) printf("%s param %d,%d,%d\n",
        opcode_set->opcodes[i].name,
        opcode_set->opcodes[i].dest_size[0],
        opcode_set->opcodes[i].src_size[0],
        opcode_set->opcodes[i].src_size[1]);
    test_opcode_param (opcode_set->opcodes + i);
  }
  for(i=0;i<opcode_set->n_opcodes;i++){
    if (verbose) printf("%s inplace %d,%d,%d\n",
        opcode_set->opcodes[i].name,
        opcode_set->opcodes[i].dest_size[0],
        opcode_set->opcodes[i].src_size[0],
        opcode_set->opcodes[i].src_size[1]);
    test_opcode_inplace (opcode_set->opcodes + i);
  }
  for(i=0;i<opcode_set->n_opcodes;i++){
    if (verbose) printf("%s src 2d %d,%d,%d\n",
        opcode_set->opcodes[i].name,
        opcode_set->opcodes[i].dest_size[0],
        opcode_set->opcodes[i].src_size[0],
        opcode_set->opcodes[i].src_size[1]);
    test_opcode_src_2d (opcode_set->opcodes + i);
  }
  for(i=0;i<opcode_set->n_opcodes;i++){
    if (verbose) printf("%s src const n %d,%d,%d\n",
        opcode_set->opcodes[i].name,
        opcode_set->opcodes[i].dest_size[0],
        opcode_set->opcodes[i].src_size[0],
        opcode_set->opcodes[i].src_size[1]);
    test_opcode_src_const_n (opcode_set->opcodes + i);
  }
  for(i=0;i<opcode_set->n_opcodes;i++){
    if (verbose) printf("%s src const n 2d %d,%d,%d\n",
        opcode_set->opcodes[i].name,
        opcode_set->opcodes[i].dest_size[0],
        opcode_set->opcodes[i].src_size[0],
        opcode_set->opcodes[i].src_size[1]);
    test_opcode_src_const_n_2d (opcode_set->opcodes + i);
  }

  if (error) return 1;
  return 0;
}

void
test_opcode_src (OrcStaticOpcode *opcode)
{
  OrcProgram *p;
  char s[40];
  int ret;
  int flags = 0;

  if (opcode->flags & ORC_STATIC_OPCODE_SCALAR) {
    return;
  }

  p = orc_program_new ();
  if (opcode->flags & ORC_STATIC_OPCODE_ACCUMULATOR) {
    orc_program_add_accumulator (p, opcode->dest_size[0], "d1");
  } else {
    orc_program_add_destination (p, opcode->dest_size[0], "d1");
  }
  if (opcode->dest_size[1] != 0) {
    orc_program_add_destination (p, opcode->dest_size[1], "d2");
  }
  orc_program_add_source (p, opcode->src_size[0], "s1");
  if (opcode->src_size[1] != 0) {
    orc_program_add_source (p, opcode->src_size[1], "s2");
  }

  if ((opcode->flags & ORC_STATIC_OPCODE_FLOAT_SRC) ||
      (opcode->flags & ORC_STATIC_OPCODE_FLOAT_DEST)) {
    flags = ORC_TEST_FLAGS_FLOAT;
  }

  sprintf(s, "test_s_%s", opcode->name);
  orc_program_set_name (p, s);

  if (opcode->dest_size[1] != 0) {
    orc_program_append_dds_str (p, opcode->name, "d1", "d2", "s1");
  } else {
    orc_program_append_str (p, opcode->name, "d1", "s1", "s2");
  }

  ret = orc_test_compare_output_full (p, flags);
  if (!ret) {
    printf("test failed\n");
    error = TRUE;
  }

  orc_program_free (p);
}

void
test_opcode_const (OrcStaticOpcode *opcode)
{
  OrcProgram *p;
  char s[40];
  int ret;
  int flags = 0;
  int args[4] = { -1, -1, -1, -1 };
  int n_args = 0;

  if (opcode->src_size[1] == 0) {
    return;
  }
  p = orc_program_new ();
  if (opcode->flags & ORC_STATIC_OPCODE_ACCUMULATOR) {
    args[n_args++] =
      orc_program_add_accumulator (p, opcode->dest_size[0], "d1");
  } else {
    args[n_args++] =
      orc_program_add_destination (p, opcode->dest_size[0], "d1");
  }
  if (opcode->dest_size[1] != 0) {
    args[n_args++] =
      orc_program_add_destination (p, opcode->dest_size[1], "d2");
  }
  args[n_args++] =
    orc_program_add_source (p, opcode->src_size[0], "s1");
  args[n_args++] =
    orc_program_add_constant (p, opcode->src_size[1], 1, "c1");
  if (opcode->src_size[2]) {
    args[n_args++] =
      orc_program_add_constant (p, opcode->src_size[2], 1, "c2");
  }

  if ((opcode->flags & ORC_STATIC_OPCODE_FLOAT_SRC) ||
      (opcode->flags & ORC_STATIC_OPCODE_FLOAT_DEST)) {
    flags = ORC_TEST_FLAGS_FLOAT;
  }

  sprintf(s, "test_const_%s", opcode->name);
  orc_program_set_name (p, s);

  orc_program_append_2 (p, opcode->name, 0, args[0], args[1],
      args[2], args[3]);

  ret = orc_test_compare_output_full (p, flags);
  if (!ret) {
    printf("test failed\n");
    error = TRUE;
  }

  orc_program_free (p);
}

void
test_opcode_param (OrcStaticOpcode *opcode)
{
  OrcProgram *p;
  char s[40];
  int ret;
  int flags = 0;
  int args[4] = { -1, -1, -1, -1 };
  int n_args = 0;

  if (opcode->src_size[1] == 0) {
    return;
  }
  p = orc_program_new ();
  if (opcode->flags & ORC_STATIC_OPCODE_ACCUMULATOR) {
    args[n_args++] =
      orc_program_add_accumulator (p, opcode->dest_size[0], "d1");
  } else {
    args[n_args++] =
      orc_program_add_destination (p, opcode->dest_size[0], "d1");
  }
  if (opcode->dest_size[1] != 0) {
    args[n_args++] =
      orc_program_add_destination (p, opcode->dest_size[1], "d2");
  }
  args[n_args++] =
    orc_program_add_source (p, opcode->src_size[0], "s1");
  args[n_args++] =
    orc_program_add_parameter (p, opcode->src_size[1], "p1");
  if (opcode->src_size[2]) {
    args[n_args++] =
      orc_program_add_parameter (p, opcode->src_size[2], "p2");
  }

  if ((opcode->flags & ORC_STATIC_OPCODE_FLOAT_SRC) ||
      (opcode->flags & ORC_STATIC_OPCODE_FLOAT_DEST)) {
    flags = ORC_TEST_FLAGS_FLOAT;
  }

  sprintf(s, "test_p_%s", opcode->name);
  orc_program_set_name (p, s);

  orc_program_append_2 (p, opcode->name, 0, args[0], args[1],
      args[2], args[3]);

  ret = orc_test_compare_output_full (p, flags);
  if (!ret) {
    printf("test failed\n");
    error = TRUE;
  }

  orc_program_free (p);
}

void
test_opcode_inplace (OrcStaticOpcode *opcode)
{
  OrcProgram *p;
  char s[40];
  int ret;
  int flags = 0;

  if (opcode->dest_size[0] != opcode->src_size[0]) return;

  if (opcode->flags & ORC_STATIC_OPCODE_SCALAR ||
      opcode->flags & ORC_STATIC_OPCODE_ACCUMULATOR) {
    return;
  }

  p = orc_program_new ();
  orc_program_add_destination (p, opcode->dest_size[0], "d1");
  if (opcode->dest_size[1] != 0) {
    orc_program_add_destination (p, opcode->dest_size[1], "d2");
  }
  if (opcode->src_size[1] != 0) {
    orc_program_add_source (p, opcode->src_size[0], "s2");
  }

  if ((opcode->flags & ORC_STATIC_OPCODE_FLOAT_SRC) ||
      (opcode->flags & ORC_STATIC_OPCODE_FLOAT_DEST)) {
    flags = ORC_TEST_FLAGS_FLOAT;
  }

  sprintf(s, "test_inplace_%s", opcode->name);
  orc_program_set_name (p, s);

  orc_program_append_str (p, opcode->name, "d1", "d1", "s2");

  ret = orc_test_compare_output_full (p, flags);
  if (!ret) {
    printf("test failed\n");
    error = TRUE;
  }

  orc_program_free (p);
}

void
test_opcode_src_2d (OrcStaticOpcode *opcode)
{
  OrcProgram *p;
  char s[40];
  int ret;
  int flags = 0;

  if (opcode->flags & ORC_STATIC_OPCODE_SCALAR) {
    return;
  }

  p = orc_program_new ();
  if (opcode->flags & ORC_STATIC_OPCODE_ACCUMULATOR) {
    orc_program_add_accumulator (p, opcode->dest_size[0], "d1");
  } else {
    orc_program_add_destination (p, opcode->dest_size[0], "d1");
  }
  if (opcode->dest_size[1] != 0) {
    orc_program_add_destination (p, opcode->dest_size[1], "d2");
  }
  orc_program_add_source (p, opcode->src_size[0], "s1");
  if (opcode->src_size[1] != 0) {
    orc_program_add_source (p, opcode->src_size[1], "s2");
  }

  if ((opcode->flags & ORC_STATIC_OPCODE_FLOAT_SRC) ||
      (opcode->flags & ORC_STATIC_OPCODE_FLOAT_DEST)) {
    flags = ORC_TEST_FLAGS_FLOAT;
  }

  sprintf(s, "test_s_%s", opcode->name);
  orc_program_set_name (p, s);
  orc_program_set_2d (p);

  if (opcode->dest_size[1] != 0) {
    orc_program_append_dds_str (p, opcode->name, "d1", "d2", "s1");
  } else {
    orc_program_append_str (p, opcode->name, "d1", "s1", "s2");
  }

  ret = orc_test_compare_output_full (p, flags);
  if (!ret) {
    printf("test failed\n");
    error = TRUE;
  }

  orc_program_free (p);
}

void
test_opcode_src_const_n (OrcStaticOpcode *opcode)
{
  OrcProgram *p;
  char s[40];
  int ret;
  int flags = 0;

  if (opcode->flags & ORC_STATIC_OPCODE_SCALAR) {
    return;
  }

  p = orc_program_new ();
  if (opcode->flags & ORC_STATIC_OPCODE_ACCUMULATOR) {
    orc_program_add_accumulator (p, opcode->dest_size[0], "d1");
  } else {
    orc_program_add_destination (p, opcode->dest_size[0], "d1");
  }
  if (opcode->dest_size[1] != 0) {
    orc_program_add_destination (p, opcode->dest_size[1], "d2");
  }
  orc_program_add_source (p, opcode->src_size[0], "s1");
  if (opcode->src_size[1] != 0) {
    orc_program_add_source (p, opcode->src_size[1], "s2");
  }

  if ((opcode->flags & ORC_STATIC_OPCODE_FLOAT_SRC) ||
      (opcode->flags & ORC_STATIC_OPCODE_FLOAT_DEST)) {
    flags = ORC_TEST_FLAGS_FLOAT;
  }

  sprintf(s, "test_s_%s", opcode->name);
  orc_program_set_name (p, s);
  orc_program_set_constant_n (p, 8);

  if (opcode->dest_size[1] != 0) {
    orc_program_append_dds_str (p, opcode->name, "d1", "d2", "s1");
  } else {
    orc_program_append_str (p, opcode->name, "d1", "s1", "s2");
  }

  ret = orc_test_compare_output_full (p, flags);
  if (!ret) {
    printf("test failed\n");
    error = TRUE;
  }

  orc_program_free (p);
}

void
test_opcode_src_const_n_2d (OrcStaticOpcode *opcode)
{
  OrcProgram *p;
  char s[40];
  int ret;
  int flags = 0;

  if (opcode->flags & ORC_STATIC_OPCODE_SCALAR) {
    return;
  }

  p = orc_program_new ();
  if (opcode->flags & ORC_STATIC_OPCODE_ACCUMULATOR) {
    orc_program_add_accumulator (p, opcode->dest_size[0], "d1");
  } else {
    orc_program_add_destination (p, opcode->dest_size[0], "d1");
  }
  if (opcode->dest_size[1] != 0) {
    orc_program_add_destination (p, opcode->dest_size[1], "d2");
  }
  orc_program_add_source (p, opcode->src_size[0], "s1");
  if (opcode->src_size[1] != 0) {
    orc_program_add_source (p, opcode->src_size[1], "s2");
  }

  if ((opcode->flags & ORC_STATIC_OPCODE_FLOAT_SRC) ||
      (opcode->flags & ORC_STATIC_OPCODE_FLOAT_DEST)) {
    flags = ORC_TEST_FLAGS_FLOAT;
  }

  sprintf(s, "test_s_%s", opcode->name);
  orc_program_set_name (p, s);
  orc_program_set_2d (p);
  orc_program_set_constant_n (p, 8);

  if (opcode->dest_size[1] != 0) {
    orc_program_append_dds_str (p, opcode->name, "d1", "d2", "s1");
  } else {
    orc_program_append_str (p, opcode->name, "d1", "s1", "s2");
  }

  ret = orc_test_compare_output_full (p, flags);
  if (!ret) {
    printf("test failed\n");
    error = TRUE;
  }

  orc_program_free (p);
}

