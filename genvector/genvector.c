/*
  Copyright (c) 2017, Dmitry D. Chernov
*/

#include "genvector.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef GVEC_GROWTH_FACTOR
  #define GVEC_GROWTH_FACTOR 1.5
#endif

/******************************************************************************/

static GENA_INLINE size_t storage_calculate( size_t current_size,
  size_t min_growth )
{
  size_t size;
{
  size = (current_size > 0) ? current_size : 1;

  #ifdef GVEC_CALCULATE_SIZE_MATH

  size = (size_t)pow(
    GVEC_GROWTH_FACTOR,
    ceil( log( size+min_growth ) / log(GVEC_GROWTH_FACTOR) )
  );

  #else

  while (size < current_size+min_growth) {
    /* GROWTH_FACTOR == 1.5: size = (size << 1) - (size >> 1);
       GROWTH_FACTOR == 2.0: size <<= 1; */
    size = (size_t)ceil( size * GVEC_GROWTH_FACTOR );
  }

  #endif

  return size;
}}

static igvec_header_p storage_prepare( gvec_h* phandle, size_t size,
  size_t entry_size )
{
  void* buffer;
  igvec_header_p header;
{
  assert( phandle != NULL );
  assert( size > 0 );
  assert( entry_size > 0 );

  buffer = (*phandle == NULL) ? NULL : IGVEC_BUFFER(*phandle);

  if (buffer != NULL) {
    header = (igvec_header_p)buffer;
    if (size == header->size) { return header; }
  }

  buffer = realloc( buffer, sizeof(igvec_header_s) + size * entry_size );
  if (buffer == NULL) { return NULL; }

  *phandle = ZGENA_VOIDPTR_ADD( buffer, sizeof(igvec_header_s) );

  header = (igvec_header_p)buffer;
  header->size = size;
  header->entry_size = entry_size;

  return header;
}}

/******************************************************************************/

gvec_h igvec_new( size_t min_count, size_t entry_size, gena_tag_z tag ) {
  gvec_h handle = NULL;
  igvec_header_p header;
  size_t new_size;
{
  new_size = storage_calculate( 0, min_count );
  header = storage_prepare( &handle, new_size, entry_size );
  if (header == NULL) { return NULL; }

  header->count = 0;
  header->tag = tag;

  return handle;
}}

gvec_h igvec_resize( gvec_h handle, size_t new_count ) {
{
  assert( handle != NULL );

  handle = igvec_reserve( handle, new_count );
  if (handle == NULL) { return NULL; }

  IGVEC_HEADER(handle)->count = new_count;
  return handle;
}}

gvec_h igvec_reserve( gvec_h handle, size_t min_count ) {
  igvec_header_p header;
  size_t new_size;
{
  assert( handle != NULL );
  header = IGVEC_HEADER(handle);

  if (min_count <= header->size) { return handle; }

  new_size = storage_calculate( header->size, min_count - header->count );
  header = storage_prepare( &handle, new_size, header->entry_size );
  if (header == NULL) { return NULL; }

  return handle;
}}

gvec_h igvec_shrink( gvec_h handle ) {
  igvec_header_p header;
  size_t new_size;
{
  assert( handle != NULL );
  header = IGVEC_HEADER(handle);

  new_size = storage_calculate( 0, header->count );
  header = storage_prepare( &handle, new_size, header->entry_size );
  if (header == NULL) { return NULL; }

  return handle;
}}

gvec_h igvec_insert( gvec_h handle, size_t position, size_t count ) {
  gvec_h dest_gvec;
  igvec_header_p header;
  size_t entry_size, old_count, new_count;
{
  assert( handle != NULL );
  header = IGVEC_HEADER(handle);
  assert( position <= header->count );

  if (count == 0) { return handle; }

  entry_size = header->entry_size;
  old_count = header->count;
  new_count = old_count + count;

  #ifndef GVEC_INSERT_NO_REALLOC

  handle = igvec_reserve( handle, new_count );
  if (handle == NULL) { return NULL; }
  header = IGVEC_HEADER(handle);
  dest_gvec = handle;

  #else

  /* This one doesn't use memory relocation and better for many insertions at
  arbitrary positions, in theory. */
  if (new_count > header->size) {
    dest_gvec = igvec_new( new_count, entry_size, header->tag );
    if (dest_gvec == NULL) { return NULL; }
    header = IGVEC_HEADER(dest_gvec);
    memmove( dest_gvec, handle, position * entry_size );
  } else {
    dest_gvec = handle;
  }

  #endif

  memmove(
    ZGENA_VOIDPTR_ADD( dest_gvec, (position+count) * entry_size ),
    ZGENA_VOIDPTR_ADD( handle, position * entry_size ),
    (old_count-position) * entry_size
  );

  header->count = new_count;

  #ifdef GVEC_INSERT_NO_REALLOC
  if (handle != dest_gvec) { gvec_free(handle); }
  #endif

  return dest_gvec;
}}

gvec_h igvec_push( gvec_h handle ) {
{
  assert( handle != NULL );
  return igvec_resize( handle, gvec_count(handle)+1 );
}}

void* igvec_pop( gvec_h handle ) {
  igvec_header_p header;
{
  assert( handle != NULL );
  header = IGVEC_HEADER(handle);
  assert( header->count > 0 );
  return ZGENA_VOIDPTR_ADD( handle, --(header->count) * header->entry_size );
}}

void* igvec_at( gvec_h handle, size_t position ) {
  igvec_header_p header;
{
  assert( handle != NULL );
  header = IGVEC_HEADER(handle);
  return (position < header->count)
    ? ZGENA_VOIDPTR_ADD( handle, position * header->entry_size )
    : NULL;
}}

/******************************************************************************/

gvec_h gvec_assign( gvec_h handle, gvec_h source ) {
  igvec_header_p dest_hdr, src_hdr;
{
  assert( handle != NULL );
  assert( source != NULL );

  dest_hdr = IGVEC_HEADER(handle);
  src_hdr = IGVEC_HEADER(source);

  assert( dest_hdr->tag == src_hdr->tag );

  handle = igvec_resize( handle, src_hdr->count );
  if (handle == NULL) { return NULL; }

  return memcpy( handle, source, src_hdr->count * src_hdr->entry_size );
}}

gvec_h gvec_copy( gvec_h handle ) {
  gvec_h dest_handle = NULL;
  igvec_header_p src_hdr, dest_hdr;
{
  assert( handle != NULL );
  src_hdr = IGVEC_HEADER(handle);

  dest_hdr = storage_prepare( &dest_handle, src_hdr->size,
    src_hdr->entry_size );
  if (dest_hdr == NULL) { return NULL; }

  dest_hdr->count = src_hdr->count;
  return memcpy( dest_handle, handle, src_hdr->count * src_hdr->entry_size );
}}

void gvec_free( gvec_h handle ) {
{
  if (handle == NULL) { return; }
  free( IGVEC_BUFFER(handle) );
}}

/******************************************************************************/

void gvec_clear( gvec_h handle ) {
{
  assert( handle != NULL );
  IGVEC_HEADER(handle)->count = 0;
}}

void gvec_reduce( gvec_h handle, size_t new_count ) {
  igvec_header_p header;
{
  assert( handle != NULL );
  header = IGVEC_HEADER(handle);
  assert( new_count <= header->count );

  header->count = new_count;
}}

void gvec_remove( gvec_h handle, size_t position, size_t count ) {
  igvec_header_p header;
  size_t entry_size, tail_size;
{
  assert( handle != NULL );
  header = IGVEC_HEADER(handle);
  assert( position+count <= header->count );

  if (count == 0) { return; }

  entry_size = header->entry_size;
  tail_size = (header->count-(position+count)) * entry_size;

  memmove(
    ZGENA_VOIDPTR_ADD( handle, position * entry_size ),
    ZGENA_VOIDPTR_ADD( handle, (position+count) * entry_size ),
    tail_size
  );

  header->count -= count;
}}

void gvec_drop( gvec_h handle ) {
{
  igvec_pop( handle );
}}

/******************************************************************************/

size_t gvec_count( gvec_h handle ) {
{
  assert( handle != NULL );
  return IGVEC_HEADER(handle)->count;
}}

size_t gvec_size( gvec_h handle ) {
{
  assert( handle != NULL );
  return IGVEC_HEADER(handle)->size;
}}

gena_bool gvec_empty( gvec_h handle ) {
{
  assert( handle != NULL );
  return ( IGVEC_HEADER(handle)->count == 0 );
}}
