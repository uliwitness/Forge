/*
 *  vcy_lib.h
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 14.12.08.
 *  Copyright 2008 The Void Software. All rights reserved.
 *
 */

// -----------------------------------------------------------------------------
//	Headers:
// -----------------------------------------------------------------------------

#include <stdbool.h>
#include <string.h>


#if __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
//	Constants:
// -----------------------------------------------------------------------------

enum e_vcy_variant_type
{
	VCY_VARIANT_TYPE_INVALID,
	VCY_VARIANT_TYPE_CONST_STRING,	// value.string points to a constant string of length count.
	VCY_VARIANT_TYPE_STRING,		// value.string points to a malloc()ed buffer containing a string of length count.
	VCY_VARIANT_TYPE_NUMBER,		// value.number is the number.
	VCY_VARIANT_TYPE_BOOLEAN,		// value.boolean is the boolean.
	VCY_VARIANT_TYPE_LIST			// value.variant is a malloc()ed buffer to an array of count variants.
};
typedef int		vcy_variant_type;


// The chunk types we currently support:
typedef enum TChunkType
{
	TChunkTypeInvalid = 0,
	TChunkTypeCharacter,
	TChunkTypeItem,
	TChunkTypeLine,
	TChunkTypeWord
} TChunkType;


// -----------------------------------------------------------------------------
//	Data Types:
// -----------------------------------------------------------------------------

typedef struct vcy_variant_t
{
	vcy_variant_type	type;
	unsigned int		count;
	union
	{
		char*					string;		// String size is stored in count, but the buffer is actually zero-terminated, so count+1 in size!
		double					number;
		struct vcy_variant_t*	variant;	// Number of array elements is stored in count.
		bool					boolean;
	}	value;
} vcy_variant_t;


// -----------------------------------------------------------------------------
//	Prototypes:
// -----------------------------------------------------------------------------

void		vcy_alloc_empty( vcy_variant_t* theVal );
void		vcy_alloc_number( vcy_variant_t* theVal, double theNum );
void		vcy_alloc_boolean( vcy_variant_t* theVal, bool theBool );
void		vcy_alloc_const_string( vcy_variant_t* theVal, const char* theStr, size_t theLen );
void		vcy_alloc_const_cstring( vcy_variant_t* theVal, const char* theStr );
void		vcy_alloc_string( vcy_variant_t* theVal, const char* theStr, size_t theLen );
void		vcy_alloc_cstring( vcy_variant_t* theVal, const char* theStr );
void		vcy_kill( vcy_variant_t* theVal );
void		vcy_free( vcy_variant_t* theVal );
void		vcy_alloc_list( vcy_variant_t* theVal );
void		vcy_overwrite_list( vcy_variant_t* theVal );
void		vcy_overwrite( vcy_variant_t* srcVal, vcy_variant_t* dstVal );
vcy_variant_t*	vcy_add_item( vcy_variant_t* theVal );
void		vcy_alloc_value( vcy_variant_t* srcVal, vcy_variant_t* dstVal );
void		vcy_append( vcy_variant_t* theVal, vcy_variant_t* appendVal );
void		vcy_cat( vcy_variant_t* firstVal, vcy_variant_t* lastVal, vcy_variant_t* dstVal );
void		vcy_cat_space( vcy_variant_t* firstVal, vcy_variant_t* lastVal, char theDelim, vcy_variant_t* dstVal );
void		vcy_list_getref( vcy_variant_t* dstVal, vcy_variant_t* list, size_t oneBasedIndex );
vcy_variant_t	vcy_list_get_const( vcy_variant_t* list, size_t oneBasedIndex );
void		vcy_alloc_list_chunks( vcy_variant_t* destVal, vcy_variant_t* srcVal, int chunkType );
size_t		vcy_chunk_count( vcy_variant_t* srcVal, int chunkType );
void		vcy_clear_value( vcy_variant_t* theVal );
void		vcy_list_assign_items( vcy_variant_t* dstVal, size_t numItems, ... );
size_t		vcy_list_count( vcy_variant_t* theVal );
void		vcy_print_atdepth( vcy_variant_t* theVal, size_t theDepth );
void		vcy_print( vcy_variant_t* theVal );

double		vcy_as_number( vcy_variant_t* argA );
const char*	vcy_as_string( vcy_variant_t* argA, size_t *outLen );
bool		vcy_as_boolean( vcy_variant_t* argA );


// Operators:
vcy_variant_t	vcy_add( vcy_variant_t* argA, vcy_variant_t* argB );

// Built-in handlers and functions:
vcy_variant_t	vcy_fun_date( vcy_variant_t* paramList );

void			test_vcy_lib( void );

#if __cplusplus
}
#endif
