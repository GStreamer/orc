
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <orc/orc.h>
#include <orc/orcparse.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * SECTION:orcparse
 * @title: Parser
 * @short_description: Parse Orc source code
 */


typedef struct _OrcParser OrcParser;
struct _OrcParser {
  const char *code;
  int code_length;

  const char *p;

  int line_number;
  char *line;
  int creg_index;

  OrcOpcodeSet *opcode_set;
  OrcProgram *program;

  OrcProgram **programs;
  int n_programs;
  int n_programs_alloc;
};

static void orc_parse_get_line (OrcParser *parser);
static OrcStaticOpcode * get_opcode (OrcParser *parser, const char *opcode);


int
orc_parse (const char *code, OrcProgram ***programs)
{
  OrcParser _parser;
  OrcParser *parser = &_parser;

  memset (parser, 0, sizeof(*parser));

  parser->code = code;
  parser->code_length = strlen (code);
  parser->line_number = -1;
  parser->p = code;
  parser->opcode_set = orc_opcode_set_get ("sys");

  while (parser->p[0] != 0) {
    char *p;
    char *end;
    char *token[10];
    int n_tokens;

    orc_parse_get_line (parser);

    p = parser->line;
    end = p + strlen (p);
    //printf("%d: %s\n", parser->line_number, parser->line);

    while (p[0] == ' ' || p[0] == '\t') p++;

    if (p[0] == 0) {
      continue;
    }

    if (p[0] == '#') {
      //printf("comment: %s\n", p+1);
      continue;
    }

    n_tokens = 0;

    while (p < end) {
      if (p[0] == ' ' || p[0] == '\t' || p[0] == ',') p++;
      if (p[0] == 0 || p[0] == '#') break;

      token[n_tokens] = p;
      while (p[0] != 0 && p[0] != ' ' && p[0] != '\t' && p[0] != ',') p++;
      n_tokens++;

      p[0] = 0;
      p++;
    }

    if (n_tokens == 0) {
      continue;
    }

    {
      int i;
      for(i=0;i<n_tokens;i++){
        //printf("'%s' ", token[i]);
      }
      //printf("\n");
    }

    if (token[0][0] == '.') {
      if (strcmp (token[0], ".function") == 0) {
        parser->program = orc_program_new ();
        orc_program_set_name (parser->program, token[1]);
        if (parser->n_programs == parser->n_programs_alloc) {
          parser->n_programs_alloc += 32;
          parser->programs = realloc (parser->programs,
              sizeof(OrcProgram *)*parser->n_programs_alloc);
        }
        parser->programs[parser->n_programs] = parser->program;
        parser->n_programs++;
        parser->creg_index = 1;
      } else if (strcmp (token[0], ".flags") == 0) {
        int i;
        for(i=1;i<n_tokens;i++){
          if (!strcmp (token[i], "2d")) {
            orc_program_set_2d (parser->program);
          }
        }
      } else if (strcmp (token[0], ".n") == 0) {
        int size = strtol (token[1], NULL, 0);
        orc_program_set_constant_n (parser->program, size);
      } else if (strcmp (token[0], ".m") == 0) {
        int size = strtol (token[1], NULL, 0);
        orc_program_set_constant_m (parser->program, size);
      } else if (strcmp (token[0], ".source") == 0) {
        int size = strtol (token[1], NULL, 0);
        int var;
        var = orc_program_add_source (parser->program, size, token[2]);
        if (n_tokens > 3) {
          orc_program_set_type_name (parser->program, var, token[3]);
        }
      } else if (strcmp (token[0], ".dest") == 0) {
        int size = strtol (token[1], NULL, 0);
        int var;
        var = orc_program_add_destination (parser->program, size, token[2]);
        if (n_tokens > 3) {
          orc_program_set_type_name (parser->program, var, token[3]);
        }
      } else if (strcmp (token[0], ".accumulator") == 0) {
        int size = strtol (token[1], NULL, 0);
        int var;
        var = orc_program_add_accumulator (parser->program, size, token[2]);
        if (n_tokens > 3) {
          orc_program_set_type_name (parser->program, var, token[3]);
        }
      } else if (strcmp (token[0], ".temp") == 0) {
        int size = strtol (token[1], NULL, 0);
        orc_program_add_temporary (parser->program, size, token[2]);
      } else if (strcmp (token[0], ".param") == 0) {
        int size = strtol (token[1], NULL, 0);
        orc_program_add_parameter (parser->program, size, token[2]);
      } else if (strcmp (token[0], ".const") == 0) {
        int size = strtol (token[1], NULL, 0);
        int value = strtoul (token[3], NULL, 0);
        orc_program_add_constant (parser->program, size, value, token[2]);
      } else if (strcmp (token[0], ".floatparam") == 0) {
        int size = strtol (token[1], NULL, 0);
        orc_program_add_parameter_float (parser->program, size, token[2]);
      } else {
        ORC_ERROR("ERROR: unknown directive: %s", token[0]);
      }
    } else {
      OrcStaticOpcode *o;

      o = get_opcode (parser, token[0]);

      if (o) {
        if (n_tokens == 4) {
          char *end;
          int imm = strtol (token[3], &end, 0);
          if (end != token[3]) {
            char creg[10];
            sprintf(creg, "c%d", parser->creg_index);
            parser->creg_index++;
            orc_program_add_constant (parser->program, 2, imm, creg);
            orc_program_append_str (parser->program, token[0],
                token[1], token[2], creg);
          } else {
            orc_program_append_str (parser->program, token[0],
                token[1], token[2], token[3]);
          }
        } else {
          orc_program_append_ds_str (parser->program, token[0],
              token[1], token[2]);
        }
      } else {
        printf("ERROR: unknown token[0]: %s\n", token[0]);
      }
    }
  }


  if (parser->line) free (parser->line);

  *programs = parser->programs;
  return parser->n_programs;
}

static OrcStaticOpcode *
get_opcode (OrcParser *parser, const char *opcode)
{
  int i;

  for(i=0;i<parser->opcode_set->n_opcodes;i++){
    if (strcmp (opcode, parser->opcode_set->opcodes[i].name) == 0) {
      return parser->opcode_set->opcodes + i;
    }
  }

  return NULL;
}

static void
orc_parse_get_line (OrcParser *parser)
{
  const char *end;
  int n;

  if (parser->line) {
    free (parser->line);
    parser->line = NULL;
  }

  end = strchr (parser->p, '\n');
  if (end == NULL) {
    end = parser->code + parser->code_length;
  }

  n = end - parser->p;
  parser->line = malloc (n + 1);
  memcpy (parser->line, parser->p, n);
  parser->line[n] = 0;

  parser->p = end;
  if (parser->p[0] == '\n') {
    parser->p++;
  }
  parser->line_number++;
}


