
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <orc/orc.h>
#include <orc-test/orctest.h>
#include <orc/orcparse.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char * read_file (const char *filename);

void test_opcodes (void);

int error = FALSE;

int
main (int argc, char *argv[])
{
  int i;
  OrcProgram **programs;
  const char *filename = NULL;

  orc_init ();
  orc_test_init ();

  for(i=1;i<argc;i++){
    if (strcmp(argv[i], "--help") == 0) {
      printf("Usage:\n");
      printf("  orc-bugreport [file.orc]\n");
      printf("\n");
      printf("Options:\n");
      printf("  --help                    Show help options\n");
      printf("  --verbose                 Increase debugging messages\n");
      printf("\n");
      printf("Environment Variables:\n");
      printf("  ORC_DEBUG=<LEVEL>         Set debugging level\n");
      printf("  ORC_CODE=[KEYWORDS,...]   Modify code generation\n");
      printf("    General keywords:\n");
      printf("      backup     Always use backup function\n");
      printf("      debug      Generate debuggable code (useful for backtraces on i386)\n");
      printf("    SSE keywords:\n");
      printf("      -sse2      Disable SSE2\n");
      printf("      -sse3      Disable SSE3\n");
      printf("      -ssse3     Disable SSEE3\n");
      printf("      -sse41     Disable SSE4.1\n");
      printf("      -sse42     Disable SSE4.2\n");
      printf("      -sse4a     Disable SSE4a\n");
      printf("      -sse5      Disable SSE5\n");
      printf("\n");
      exit (0);
    }

    filename = argv[i];
  }

  printf("Orc " VERSION " - integrated testing tool\n");

  printf("Active backend: %s\n",
      orc_target_get_name(orc_target_get_default()));

  {
    int level1, level2, level3;
    orc_get_data_cache_sizes(&level1, &level2, &level3);
    printf("L1 cache: %d\n", level1);
    printf("L2 cache: %d\n", level2);
    printf("L3 cache: %d\n", level3);
  }

  {
    int family, model, stepping;
    orc_get_cpu_family_model_stepping (&family, &model, &stepping);
    printf("Family/Model/Stepping: %d/%d/%d\n", family, model, stepping);
    printf("CPU name: %s\n", orc_get_cpu_name ());
  }

  {
    int i;
    int flags = orc_target_get_default_flags (orc_target_get_default());

    printf("Compiler options: ");
    for(i=0;i<32;i++){
      if (flags & (1<<i)) {
        printf("%s ", orc_target_get_flag_name (orc_target_get_default(), i));
      }
    }
    printf("\n");
  }

  if (filename) {
    int n;
    int ret;
    char *code;

    code = read_file (filename);
    if (!code) {
      printf("orc-bugreport: could not read file %s\n", filename);
      exit(1);
    }

    printf("Parsing %s\n", filename);
    n = orc_parse (code, &programs);

    for(i=0;i<n;i++){
      ret = orc_test_compare_output_full (programs[i], 0);
      if (!ret) {
        printf("FAIL: %s\n", programs[i]->name);
        error = TRUE;
      }
    }
  } else {
    printf("Opcode test:\n");
    test_opcodes();
  }

  if (error) {
    printf("Errors detected.  Please send entire output to ds@schleef.org.\n");
    return 1;
  } else {
    printf("No errors detected.\n");
    return 0;
  }
}


static char *
read_file (const char *filename)
{
  FILE *file = NULL;
  char *contents = NULL;
  long size;
  int ret;

  file = fopen (filename, "r");
  if (file == NULL) return NULL;

  ret = fseek (file, 0, SEEK_END);
  if (ret < 0) goto bail;

  size = ftell (file);
  if (size < 0) goto bail;

  ret = fseek (file, 0, SEEK_SET);
  if (ret < 0) goto bail;

  contents = malloc (size + 1);
  if (contents == NULL) goto bail;

  ret = fread (contents, size, 1, file);
  if (ret < 0) goto bail;

  contents[size] = 0;

  return contents;
bail:
  /* something failed */
  if (file) fclose (file);
  if (contents) free (contents);

  return NULL;
}


void test_opcode_src (OrcStaticOpcode *opcode);
void test_opcode_const (OrcStaticOpcode *opcode);
void test_opcode_param (OrcStaticOpcode *opcode);
void test_opcode_inplace (OrcStaticOpcode *opcode);
void test_opcode_src_2d (OrcStaticOpcode *opcode);
void test_opcode_src_const_n (OrcStaticOpcode *opcode);
void test_opcode_src_const_n_2d (OrcStaticOpcode *opcode);

void
test_opcodes (void)
{
  int i;
  OrcOpcodeSet *opcode_set;

  opcode_set = orc_opcode_set_get ("sys");

  for(i=0;i<opcode_set->n_opcodes;i++){
    test_opcode_src (opcode_set->opcodes + i);
  }
  for(i=0;i<opcode_set->n_opcodes;i++){
    test_opcode_const (opcode_set->opcodes + i);
  }
  for(i=0;i<opcode_set->n_opcodes;i++){
    test_opcode_param (opcode_set->opcodes + i);
  }
  for(i=0;i<opcode_set->n_opcodes;i++){
    test_opcode_inplace (opcode_set->opcodes + i);
  }
  for(i=0;i<opcode_set->n_opcodes;i++){
    test_opcode_src_2d (opcode_set->opcodes + i);
  }
  for(i=0;i<opcode_set->n_opcodes;i++){
    test_opcode_src_const_n (opcode_set->opcodes + i);
  }
  for(i=0;i<opcode_set->n_opcodes;i++){
    test_opcode_src_const_n_2d (opcode_set->opcodes + i);
  }
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
    printf("FAIL: %s src\n", opcode->name);
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
    printf("FAIL: %s const\n", opcode->name);
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
    printf("FAIL: %s param\n", opcode->name);
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
    printf("FAIL: %s inplace\n", opcode->name);
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
    printf("FAIL: %s src_2d\n", opcode->name);
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
    printf("FAIL: %s src_const_n\n", opcode->name);
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
    printf("FAIL: %s src_const_n_2d\n", opcode->name);
    error = TRUE;
  }

  orc_program_free (p);
}

