/*
  genvector - Generalized Vector
  Pseudo-templated, Cpp-like dynamic linear array for custom data types.
  Copyright (c) 2017, Dmitry D. Chernov
*/

#ifndef ZZ_GENA_GENVECTOR_H_IG
#define ZZ_GENA_GENVECTOR_H_IG

#include "../internals/coredefs.h"

typedef void* gvec_h;
typedef void* gvec_ptr; /* because void** is not a generic pointer */

typedef struct {
  size_t count;
  size_t size;
  size_t entry_size;
} igvec_head_s, *igvec_head_p;

#define IGVEC_GET_BUFFER(handle) \
  ZGENA_VOIDP_SUB(handle, sizeof(igvec_head_s))

#define IGVEC_GET_HEADER(handle) \
  ((igvec_head_p)IGVEC_GET_BUFFER(handle))

/******************************************************************************/

extern gvec_h igvec_new( size_t min_count, size_t entry_size );
extern gvec_h gvec_set( gvec_ptr phandle, gvec_h source );
extern gvec_h gvec_copy( gvec_h handle );
extern void gvec_clear( gvec_h handle );
extern void gvec_free( gvec_h handle );

extern gena_error_e gvec_resize( gvec_ptr phandle, size_t new_count );
extern gena_error_e gvec_reserve( gvec_ptr phandle, size_t count );
extern gena_error_e gvec_shrink( gvec_ptr phandle );

extern gena_error_e igvec_insert( gvec_ptr phandle, size_t pos, size_t count );
extern void gvec_erase( gvec_h handle, size_t pos, size_t count );
extern gena_error_e igvec_push( gvec_ptr phandle );
extern void gvec_pop( gvec_h handle );

extern void* gvec_at( gvec_h handle, size_t pos );
extern void* gvec_front( gvec_h handle );
extern void* gvec_back( gvec_h handle );

extern size_t gvec_count( gvec_h handle );
extern size_t gvec_size( gvec_h handle );
extern gena_bool gvec_empty( gvec_h handle );

/******************************************************************************/

#include "gvec_template.inc"

#endif /* ZZ_GENA_GENVECTOR_H_IG */
