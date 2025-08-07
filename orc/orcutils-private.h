/*
 * ORC - Library of Optimized Inner Loops
 * Copyright (c) 2007 David A. Schleef <ds@schleef.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _ORC_UTILS_PRIVATE_H_
#define _ORC_UTILS_PRIVATE_H_

ORC_BEGIN_DECLS

#if defined(__arm__) || defined(__aarch64__) || defined(__mips__) || defined(__riscv) || defined(__loongarch__)
char * get_proc_cpuinfo (void);
#endif

char * _strndup (const char *s, int n);
char ** strsplit (const char *s, char delimiter);
char * get_tag_value (char *s, const char *tag);

void * orc_malloc(size_t size);
void * orc_realloc(void * ptr, size_t size);

orc_int64 _strtoll (const char *nptr, char **endptr, int base);

#define ORC_VECTOR_ITEM_CHUNK 32

typedef struct _OrcVector OrcVector;
struct _OrcVector {
  void **items;
  int n_items;
  int n_items_alloc;
};

#define ORC_VECTOR_AS_TYPE(VECTOR, TYPE) \
    ((TYPE **)(((VECTOR)->items)))

#define ORC_VECTOR_GET_ITEM(VECTOR, INDEX, TYPEPTR) \
    ((TYPEPTR) ((VECTOR)->items[(INDEX)]))

void orc_vector_extend (OrcVector *vector);
void orc_vector_append (OrcVector *vector, void *item);
int orc_vector_length (OrcVector *vector);
int orc_vector_has_data (OrcVector *vector);

ORC_END_DECLS

#endif /* _ORC_UTILS_PRIVATE_H_ */
