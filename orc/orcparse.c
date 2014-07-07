
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


#define ORC_ERROR_LENGTH 256
#define ORC_LINE_MAX_TOKENS 16

typedef struct _OrcParser OrcParser;
struct _OrcParser {
  const char *code;
  int code_length;

  const char *p;

  int line_number;
  char *line;
  int line_length;
  int creg_index;

  OrcOpcodeSet *opcode_set;
  OrcProgram *program;
  OrcProgram *error_program;

  OrcVector programs;
  OrcVector errors;
  int enable_errors;

  char *init_function;
};

typedef struct _OrcLine OrcLine;
struct _OrcLine {
    char *p;
    char *end;
    const char *tokens[ORC_LINE_MAX_TOKENS];
    int n_tokens;
};

typedef struct _OrcDirective OrcDirective;
struct _OrcDirective {
  const char *name;
  int (* handler) (OrcParser *parser, const OrcLine *line);
};


static void orc_line_init (OrcLine *line, OrcParser *parser);
static int orc_line_has_data (const OrcLine *line);
static int orc_line_is_comment (const OrcLine *line);
static int orc_line_is_blank (const OrcLine *line);
static void orc_line_skip_blanks (OrcLine *line);
static int orc_line_is_separator (const OrcLine *line);
static int orc_line_has_tokens (const OrcLine *line);
#ifdef ORC_PARSE_DEBUG
static void orc_line_dump_tokens (const OrcLine *line);
#endif
static void orc_line_parse_tokens (OrcLine *line);
static void orc_line_advance (OrcLine *line);
static void orc_line_add_token (OrcLine *line);
static int orc_line_has_tokens (const OrcLine *line);
static int orc_line_is_directive (const OrcLine *line);
static int orc_line_match_directive (const OrcLine *line, const char *dirname);

static void orc_parse_add_error_valist (OrcParser *parser, const char *format, va_list args);
static void orc_parse_add_error (OrcParser *parser, const char *format, ...);
static OrcParseError * orc_parse_error_new (const char *source, int line_number, int where, const char *text);
static void orc_parse_error_free (OrcParseError *error);
static void orc_parse_splat_error (OrcParseError **errors, int n_errors, char **log);

static void orc_parse_init (OrcParser *parser, const char *code, int enable_errors);
static int orc_parse_has_data (OrcParser *parser);
static void orc_parse_get_line (OrcParser *parser);
static void orc_parse_put_line (OrcParser *parser);
static void orc_parse_copy_line (OrcParser *parser);
static void orc_parse_find_line_length (OrcParser *parser);
static void orc_parse_advance (OrcParser *parser);
static void orc_parse_sanity_check (OrcParser *parser, OrcProgram *program);

static int orc_parse_handle_legacy (OrcParser *parser, const OrcLine *line);
static int orc_parse_handle_function (OrcParser *parser, const OrcLine *line);
static int orc_parse_handle_directive (OrcParser *parser, const OrcLine *line);

static int orc_parse_handle_opcode (OrcParser *parser, const OrcLine *line);

static OrcStaticOpcode * orc_parse_find_opcode (OrcParser *parser, const char *opcode);
static int opcode_n_args (OrcStaticOpcode *opcode);
static int opcode_arg_size (OrcStaticOpcode *opcode, int arg);


const char *
orc_parse_get_init_function (OrcProgram *program)
{
  return program->init_function;
}

int
orc_parse (const char *code, OrcProgram ***programs)
{
  int n_programs = 0;
  orc_parse_code (code, programs, &n_programs, NULL, NULL);
  return n_programs;
}

int
orc_parse_full (const char *code, OrcProgram ***programs, char **log)
{
  int n_programs = 0;

  if (*log) {
    int n_errors = 0;
    OrcParseError **errors;

    orc_parse_code (code, programs, &n_programs, &errors, &n_errors);

    orc_parse_splat_error (errors, n_errors, log);
  } else {
    orc_parse_code (code, programs, &n_programs, NULL, NULL);
  }

  return n_programs;
}

int
orc_parse_code (const char *code, OrcProgram ***programs, int *n_programs,
                OrcParseError ***errors, int *n_errors)
{
  OrcParser _parser;
  OrcParser *parser = &_parser;
  int enable_errors = (errors && n_errors);

  orc_parse_init (parser, code, enable_errors);

  while (orc_parse_has_data (parser)) {
    OrcLine _line;
    OrcLine *line = &_line;

    orc_parse_get_line (parser);
    if (parser->program) {
      orc_program_set_line (parser->program, parser->line_number);
    }

    orc_line_init (line, parser);
    orc_line_skip_blanks (line);

    if (!orc_line_has_data (line) || orc_line_is_comment (line)) {
      continue;
    }

    orc_line_parse_tokens (line);

#ifdef ORC_PARSE_DEBUG
    orc_line_dump_tokens (line);
#endif

    if (!orc_line_has_tokens (line)) {
      continue;
    }

    if (orc_line_is_directive (line)) {
      orc_parse_handle_directive (parser, line);
    } else {
      orc_parse_handle_opcode (parser, line);
    }

    orc_parse_put_line (parser);
  }

  if (parser->program) {
    orc_parse_sanity_check (parser, parser->program);
  }

  if (enable_errors) {
    *errors = ORC_VECTOR_AS_TYPE (&parser->errors, OrcParseError);
    *n_errors = orc_vector_length (&parser->errors);
  }

  if (orc_vector_has_data (&parser->programs)) {
    OrcProgram *prog = ORC_VECTOR_GET_ITEM (&parser->programs, 0, OrcProgram *);
    prog->init_function = parser->init_function;
  } else {
    free (parser->init_function);
  }

  *programs = ORC_VECTOR_AS_TYPE (&parser->programs, OrcProgram);
  if (n_programs) {
    *n_programs = orc_vector_length (&parser->programs);
  }
  return orc_vector_has_data (&parser->errors) ?-1 :0;
}

static void
orc_line_init (OrcLine *line, OrcParser *parser)
{
  memset (line, 0, sizeof(*line));
  line->p = parser->line;
  line->end = line->p + parser->line_length;
}

static int
orc_line_has_data (const OrcLine *line)
{
  return line->p[0] != 0;
}

static int
orc_line_is_comment (const OrcLine *line)
{
  return line->p[0] == '#';
}

static int
orc_line_is_blank (const OrcLine *line)
{
  return (line->p[0] == ' ' || line->p[0] == '\t');
}

static void
orc_line_skip_blanks (OrcLine *line)
{
  while (orc_line_has_data (line) && orc_line_is_blank (line)) {
    line->p++;
  }
}

static int
orc_line_is_separator (const OrcLine *line)
{
  return line->p[0] == ',';
}

static void
orc_line_advance (OrcLine *line)
{
  while (orc_line_has_data (line) &&
        !orc_line_is_blank (line) &&
        !orc_line_is_separator (line)) {
    line->p++;
  }
}

static void
orc_line_add_token (OrcLine *line)
{
  line->tokens[line->n_tokens] = line->p;
  orc_line_advance (line);
  line->n_tokens++;

  line->p[0] = 0;
  line->p++;
}

static int
orc_line_has_tokens (const OrcLine *line)
{
  return line->n_tokens > 0;
}

#ifdef ORC_PARSE_DEBUG
static void
orc_line_dump_tokens (const OrcLine *line)
{
  int i = 0;

  for(i=0;i<line->n_tokens;i++) {
    fprintf(stderr, "'%s' ", line->tokens[i]);
  }
  fprintf(stderr, "\n");
}
#endif

static void
orc_line_parse_tokens (OrcLine *line)
{
  while (line->p < line->end) {
    orc_line_skip_blanks (line);
    if (!orc_line_has_data (line) || orc_line_is_comment (line)) {
      break;
    }
    orc_line_add_token (line);
  }
}

static int
orc_line_is_directive (const OrcLine *line)
{
  return line->tokens[0][0] == '.';
}

static int
orc_line_match_directive (const OrcLine *line, const char *dirname)
{
  return strcmp (line->tokens[0], dirname) == 0;
}


static void
orc_parse_init (OrcParser *parser, const char *code, int enable_errors)
{
  memset (parser, 0, sizeof(*parser));

  parser->code = code;
  parser->code_length = strlen (code);
  parser->line_number = 0;
  parser->p = code;
  parser->opcode_set = orc_opcode_set_get ("sys");
  parser->enable_errors = enable_errors;
}

static int
orc_parse_has_data (OrcParser *parser)
{
  return parser->p[0] != 0;
}

static void
orc_parse_splat_error (OrcParseError **errors, int n_errors, char **log)
{
  int i;
  int len = 0;
  int size = ORC_ERROR_LENGTH;
  char *_log = NULL;

  for(i=0;i<n_errors;i++){
    int n = strlen (errors[i]->text + sizeof ("error: \n"));
    if (len + n >= size) {
      size += ORC_ERROR_LENGTH;
      _log = realloc(_log, size);
    }
    len += sprintf (_log + len, "error: %s\n", errors[i]->text);
  }
  *log = _log;
}

void
orc_parse_error_freev(OrcParseError **errors)
{
  int i;

  if (errors == NULL)
    return;

  for (i=0;errors[i]!=NULL;i++) {
    orc_parse_error_free(errors[i]);
  }
  free(errors);
}

static OrcParseError *
orc_parse_error_new (const char *source, int line_number, int where, const char *text)
{
  OrcParseError *error = calloc (1, sizeof (*error));

  if (error == NULL)
    return NULL;

  error->source = source; /* soft reference */
  error->line_number = line_number;
  error->where = where;
  error->text = strdup (text);

  return error;
}

static void
orc_parse_error_free (OrcParseError *error)
{
  if (error == NULL)
    return;

  free ((void *)error->text);
  free (error);
}

static const char *
orc_parse_get_error_where (OrcParser *parser)
{
  if (parser->program == NULL) {
    return "<source>";
  }
  if (parser->program->name == NULL) {
    return "<program>";
  }
  return parser->program->name;
}

static void
orc_parse_add_error_valist (OrcParser *parser, const char *format, va_list args)
{
  char text[ORC_ERROR_LENGTH] = { '\0' };

  if (parser->error_program != parser->program) {
    parser->error_program = parser->program;
  }

  vsprintf (text, format, args);

  orc_vector_append (&parser->errors,
                     orc_parse_error_new (orc_parse_get_error_where (parser),
                                          parser->line_number, -1, text));
}

static void
orc_parse_add_error (OrcParser *parser, const char *format, ...)
{
  if (parser->enable_errors) {
    va_list var_args;

    va_start (var_args, format);
    orc_parse_add_error_valist (parser, format, var_args);
    va_end (var_args);
  }
}

static int
orc_parse_handle_legacy (OrcParser *parser, const OrcLine *line)
{
  const char **token = (const char **)(line->tokens);
  int n_tokens = line->n_tokens;

  if (strcmp (token[0], ".init") == 0) {
    free (parser->init_function);
    parser->init_function = NULL;
    if (n_tokens < 2) {
      orc_parse_add_error (parser, ".init without function name");
    } else {
      parser->init_function = strdup (token[1]);
    }
  } else if (strcmp (token[0], ".flags") == 0) {
    int i;
    for(i=1;i<n_tokens;i++){
      if (!strcmp (token[i], "2d")) {
        orc_program_set_2d (parser->program);
      }
    }
  } else if (strcmp (token[0], ".n") == 0) {
    int i;
    for(i=1;i<n_tokens;i++){
      if (strcmp (token[i], "mult") == 0) {
        if (i == n_tokens - 1) {
          orc_parse_add_error (parser, ".n mult requires multiple value");
        } else {
          orc_program_set_n_multiple (parser->program,
              strtol (token[1], NULL, 0));
          i++;
        }
      } else if (strcmp (token[i], "min") == 0) {
        if (i == n_tokens - 1) {
          orc_parse_add_error (parser, ".n min requires multiple value");
        } else {
          orc_program_set_n_minimum (parser->program,
              strtol (token[1], NULL, 0));
          i++;
        }
      } else if (strcmp (token[i], "max") == 0) {
        if (i == n_tokens - 1) {
          orc_parse_add_error (parser, ".n max requires multiple value");
        } else {
          orc_program_set_n_maximum (parser->program,
              strtol (token[1], NULL, 0));
          i++;
        }
      } else if (i == n_tokens - 1) {
        orc_program_set_constant_n (parser->program,
            strtol (token[1], NULL, 0));
      } else {
        orc_parse_add_error (parser, "unknown .n token '%s'", token[i]);
      }
    }
  } else if (strcmp (token[0], ".m") == 0) {
    if (n_tokens < 2) {
      orc_parse_add_error (parser, "line %d: .m without value\n",
          parser->line_number);
    } else {
      int size = strtol (token[1], NULL, 0);
      orc_program_set_constant_m (parser->program, size);
    }
  } else if (strcmp (token[0], ".source") == 0) {
    if (n_tokens < 3) {
      orc_parse_add_error (parser, "line %d: .source without size or identifier\n",
          parser->line_number);
    } else {
      int size = strtol (token[1], NULL, 0);
      int var;
      int i;
      var = orc_program_add_source (parser->program, size, token[2]);
      for(i=3;i<n_tokens;i++){
        if (strcmp (token[i], "align") == 0) {
          if (i == n_tokens - 1) {
            orc_parse_add_error (parser, "line %d: .source align requires alignment value\n",
                parser->line_number);
          } else {
            int alignment = strtol (token[i+1], NULL, 0);
            orc_program_set_var_alignment (parser->program, var, alignment);
            i++;
          }
        } else if (i == n_tokens - 1) {
          orc_program_set_type_name (parser->program, var, token[i]);
        } else {
          orc_parse_add_error (parser, "line %d: unknown .source token '%s'\n",
              parser->line_number, token[i]);
        }
      }
    }
  } else if (strcmp (token[0], ".dest") == 0) {
    if (n_tokens < 3) {
      orc_parse_add_error (parser, "line %d: .dest without size or identifier\n",
          parser->line_number);
    } else {
      int size = strtol (token[1], NULL, 0);
      int var;
      int i;
      var = orc_program_add_destination (parser->program, size, token[2]);
      for(i=3;i<n_tokens;i++){
        if (strcmp (token[i], "align") == 0) {
          if (i == n_tokens - 1) {
            orc_parse_add_error (parser, "line %d: .source align requires alignment value\n",
                parser->line_number);
          } else {
            int alignment = strtol (token[i+1], NULL, 0);
            orc_program_set_var_alignment (parser->program, var, alignment);
            i++;
          }
        } else if (i == n_tokens - 1) {
          orc_program_set_type_name (parser->program, var, token[i]);
        } else {
          orc_parse_add_error (parser, "line %d: unknown .dest token '%s'\n",
              parser->line_number, token[i]);
        }
      }
    }
  } else if (strcmp (token[0], ".accumulator") == 0) {
    if (n_tokens < 3) {
      orc_parse_add_error (parser, "line %d: .accumulator without size or name\n",
          parser->line_number);
    } else {
      int size = strtol (token[1], NULL, 0);
      int var;
      var = orc_program_add_accumulator (parser->program, size, token[2]);
      if (n_tokens > 3) {
        orc_program_set_type_name (parser->program, var, token[3]);
      }
    }
  } else if (strcmp (token[0], ".temp") == 0) {
    if (n_tokens < 3) {
      orc_parse_add_error (parser, "line %d: .temp without size or name\n",
          parser->line_number);
    } else {
      int size = strtol (token[1], NULL, 0);
      orc_program_add_temporary (parser->program, size, token[2]);
    }
  } else if (strcmp (token[0], ".param") == 0) {
    if (n_tokens < 3) {
      orc_parse_add_error (parser, "line %d: .param without size or name\n",
          parser->line_number);
    } else {
      int size = strtol (token[1], NULL, 0);
      orc_program_add_parameter (parser->program, size, token[2]);
    }
  } else if (strcmp (token[0], ".longparam") == 0) {
    if (n_tokens < 3) {
      orc_parse_add_error (parser, "line %d: .longparam without size or name\n",
          parser->line_number);
    } else {
      int size = strtol (token[1], NULL, 0);
      orc_program_add_parameter_int64 (parser->program, size, token[2]);
    }
  } else if (strcmp (token[0], ".const") == 0) {
    if (n_tokens < 4) {
      orc_parse_add_error (parser, "line %d: .const without size, name or value\n",
          parser->line_number);
    } else {
      int size = strtol (token[1], NULL, 0);

      orc_program_add_constant_str (parser->program, size, token[3], token[2]);
    }
  } else if (strcmp (token[0], ".floatparam") == 0) {
    if (n_tokens < 3) {
      orc_parse_add_error (parser, "line %d: .floatparam without size or name\n",
          parser->line_number);
    } else {
      int size = strtol (token[1], NULL, 0);
      orc_program_add_parameter_float (parser->program, size, token[2]);
    }
  } else if (strcmp (token[0], ".doubleparam") == 0) {
    if (n_tokens < 3) {
      orc_parse_add_error (parser, "line %d: .doubleparam without size or name\n",
          parser->line_number);
    } else {
      int size = strtol (token[1], NULL, 0);
      orc_program_add_parameter_double (parser->program, size, token[2]);
    }
  } else {
    orc_parse_add_error (parser, "unknown directive: %s\n", token[0]);
  }

  return 1;
}

static int
orc_parse_handle_function (OrcParser *parser, const OrcLine *line)
{
  const char *name;

  if (line->n_tokens < 2) {
    orc_parse_add_error (parser, "line %d: .function without function name\n",
        parser->line_number);
    name = "unknown_function";
  } else {
    name = line->tokens[1];
  }

  if (parser->program) {
    orc_parse_sanity_check (parser, parser->program);
  }
  parser->program = orc_program_new ();
  orc_program_set_name (parser->program, name);

  orc_vector_append (&parser->programs, parser->program);
  parser->creg_index = 1;

  return 1;
}

static int
orc_parse_handle_backup (OrcParser *parser, const OrcLine *line)
{
  if (line->n_tokens < 2) {
    orc_parse_add_error (parser, "line %d: .backup without function name\n",
        parser->line_number);
    return 0;
  }

  orc_program_set_backup_name (parser->program, line->tokens[1]);

  return 1;
}

static int
orc_parse_handle_directive (OrcParser *parser, const OrcLine *line)
{
  static const OrcDirective dirs[] = {
    { ".function", orc_parse_handle_function },
    { ".backup", orc_parse_handle_backup },
    { NULL, NULL }
  };
  int i;
  for (i=0;dirs[i].name;i++) {
    if (orc_line_match_directive (line, dirs[i].name)) {
      dirs[i].handler (parser, line);
      return 1;
    }
  }
  return orc_parse_handle_legacy (parser, line);
}


static OrcStaticOpcode *
orc_parse_find_opcode (OrcParser *parser, const char *opcode)
{
  int i;

  for(i=0;i<parser->opcode_set->n_opcodes;i++){
    if (strcmp (opcode, parser->opcode_set->opcodes[i].name) == 0) {
      return parser->opcode_set->opcodes + i;
    }
  }

  return NULL;
}

static int
opcode_n_args (OrcStaticOpcode *opcode)
{
  int i;
  int n = 0;
  for(i=0;i<ORC_STATIC_OPCODE_N_DEST;i++){
    if (opcode->dest_size[i] != 0) n++;
  }
  for(i=0;i<ORC_STATIC_OPCODE_N_SRC;i++){
    if (opcode->src_size[i] != 0) n++;
  }
  return n;
}

static int
opcode_arg_size (OrcStaticOpcode *opcode, int arg)
{
  int i;
  for(i=0;i<ORC_STATIC_OPCODE_N_DEST;i++){
    if (opcode->dest_size[i] != 0 && arg-- == 0)
      return opcode->dest_size[i];
  }
  for(i=0;i<ORC_STATIC_OPCODE_N_SRC;i++){
    if (opcode->src_size[i] != 0 && arg-- == 0)
      return opcode->src_size[i];
  }
  return 0;
}

static int
orc_parse_handle_opcode (OrcParser *parser, const OrcLine *line)
{
  OrcStaticOpcode *o;
  unsigned int flags = 0;
  int offset = 0;
  int error = 0;

  if (strcmp (line->tokens[0], "x4") == 0) {
    flags |= ORC_INSTRUCTION_FLAG_X4;
    offset = 1;
  } else if (strcmp (line->tokens[0], "x2") == 0) {
    flags |= ORC_INSTRUCTION_FLAG_X2;
    offset = 1;
  }

  o = orc_parse_find_opcode (parser, line->tokens[offset]);

  if (o) {
    int n_args = opcode_n_args (o);
    int i, j;
    const char *args[6] = { NULL };

    if (line->n_tokens != 1 + offset + n_args) {
      orc_parse_add_error (parser, "too %s arguments for %s (expected %d)",
        (line->n_tokens < 1+offset+n_args) ? "few" : "many",
        line->tokens[offset], n_args);
    }

    for(i=offset+1,j=0;i<line->n_tokens;i++,j++){
      char *end;
      double unused ORC_GNUC_UNUSED;

      args[j] = line->tokens[i];

      unused = strtod (line->tokens[i], &end);
      if (end != line->tokens[i]) {
        char varname[80];
        int id;

        /* make a unique name based on value and size */
        snprintf (varname, sizeof (varname), "_%d.%s", opcode_arg_size(o, j), line->tokens[i]);
        id = orc_program_add_constant_str (parser->program, opcode_arg_size(o, j),
            line->tokens[i], varname);
        /* it's possible we reused an existing variable, get its name so
         * that we can refer to it in the opcode */
        args[j] = parser->program->vars[id].name;
      }
    }

    error = orc_program_append_str_n (parser->program, line->tokens[offset], flags,
                n_args, args);

    if (error > 0) {
      orc_parse_add_error (parser, "bad operand \"%s\" in position %d",
              line->tokens[offset + error], error);
    }
    return 1;
  }
  orc_parse_add_error (parser, "unknown opcode: %s", line->tokens[offset]);
  return 0;
}

static void
orc_parse_find_line_length (OrcParser *parser)
{
  const char *end = NULL;

  end = strchr (parser->p, '\n');
  if (end == NULL) {
    end = parser->code + parser->code_length;
  }
  /* windows text files might have \r\n as line ending */
  if (end > parser->p && *(end - 1) == '\r')
    end--;

  parser->line_length = end - parser->p;
}

static void
orc_parse_advance (OrcParser *parser)
{
  parser->p += parser->line_length;
  if (parser->p[0] == '\n' || parser->p[0] == '\r') {
    parser->p++;
  }
}

static void
orc_parse_copy_line (OrcParser *parser)
{
  parser->line = strndup (parser->p, parser->line_length);
  parser->line_number++;
}

static void
orc_parse_put_line (OrcParser *parser)
{
  if (parser->line) {
    free (parser->line);
    parser->line = NULL;
  }
}

static void
orc_parse_get_line (OrcParser *parser)
{
  orc_parse_find_line_length (parser);
  orc_parse_copy_line (parser);
  orc_parse_advance (parser);
}


static void
orc_parse_sanity_check (OrcParser *parser, OrcProgram *program)
{
  int i;
  int j;

  for(i=0;i<=ORC_VAR_T15;i++) {
    if (program->vars[i].size == 0) {
      continue;
    }
    for(j=i+1;j<=ORC_VAR_T15;j++) {
      if (program->vars[j].size == 0) {
        continue;
      }

      if (strcmp (program->vars[i].name, program->vars[j].name) == 0) {
        orc_parse_add_error (parser, "duplicate variable name: %s",
            program->vars[i].name);
      }
    }
  }

  for(i=0;i<program->n_insns;i++){
    OrcInstruction *insn = program->insns + i;
    OrcStaticOpcode *opcode = insn->opcode;

    for(j=0;j<ORC_STATIC_OPCODE_N_DEST;j++){
      if (opcode->dest_size[j] == 0) {
        continue;
      }
      if (program->vars[insn->dest_args[j]].used &&
          program->vars[insn->dest_args[j]].vartype == ORC_VAR_TYPE_DEST) {
        orc_parse_add_error (parser, "destination %d \"%s\" written multiple times",
            j+1, program->vars[insn->dest_args[j]].name);
      }
      program->vars[insn->dest_args[j]].used = TRUE;
    }

    for(j=0;j<ORC_STATIC_OPCODE_N_SRC;j++){
      if (opcode->src_size[j] == 0) {
        continue;
      }
      if (program->vars[insn->src_args[j]].used &&
          program->vars[insn->src_args[j]].vartype == ORC_VAR_TYPE_SRC) {
        orc_parse_add_error (parser, "source %d \"%s\" read multiple times",
            j+1, program->vars[insn->src_args[j]].name);
      }
      if (!program->vars[insn->src_args[j]].used &&
          program->vars[insn->src_args[j]].vartype == ORC_VAR_TYPE_TEMP) {
        orc_parse_add_error (parser, "variable %d \"%s\" used before being written",
            j+1, program->vars[insn->src_args[j]].name);
      }
    }
  }
}

