/*
 *  TVariantConstants.h
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 12.05.07.
 *  Copyright 2007 M. Uli Kusterer. All rights reserved.
 *
 */

#pragma once

#include <stddef.h>

/* -----------------------------------------------------------------------------
	Constants & Data types:
   -------------------------------------------------------------------------- */

// Types of data a variant may contain:
typedef enum TVariantType
{
	TVariantTypeEmptyString,			// Special, light-weight type for initialising variables.
	TVariantTypeString,					// char* in mStringValue.
	TVariantTypeInt,					// Integer in mIntValue.
	TVariantTypeBool,					// Boolean in mBoolValue.
	TVariantTypeFloat,					// Decimal (fractional) number in mFloatValue.
	TVariantTypeList,					// An array of CVariants.
	TVariantTypeNotSet,					// An empty string that can be queried whether it has been set and returns FALSE. Used to find out whether a parameter or array item exists.
	TVariantTypeNativeObject,			// A reference to a native object (CNativeObjectWrapper).
	TVariantTypeChunk,					// A chunk expression that can be assigned to.
	TVariantTypeReference,				// A reference to another value, i.e. a poor man's pointer.
	TVariantTypeFile,					// A file where mString is the path.
	TVariantType_INVALID = 0xDEADBEEF	// Value set as type before a variant is freed
} TVariantType;


// The four kinds of "put" commands we currently support:
typedef enum TPutOperationType
{
	TPutOperationTypeAssign,		// put x into y
	TPutOperationTypeAppend,		// put x after y
	TPutOperationTypePrepend,		// put x before y
	TPutOperationTypePrint			// put x
} TPutOperationType;


// The chunk types we currently support:
typedef enum TChunkType
{
	TChunkTypeInvalid = 0,
	TChunkTypeCharacter,
	TChunkTypeItem,
	TChunkTypeLine,
	TChunkTypeWord
} TChunkType;


// Special values for mPointerSize:
//	These must all be <= 0, which means they're invalid sizes.
enum TSpecialPointerSizes
{
	TPointerSizeNULL	= 0,	// A NULL pointer.
	TPointerSizeUnknown	= -1,	// We don't know how large it is, but we own it now.
	TPointerSizeDontOwn = -2	// Someone else owns this pointer, we just wrap it.
};


// A range of characters used for handing over the result of a chunk expression:
struct TCharacterRange
{
	size_t		mStartOffs;
	size_t		mLength;
};


// Describe a chunk:
//struct TChunkDescriptor
//{
//	CVariant*		mValue;		// The value to get the chunk from.
//	TChunkType		mType;		// Type of chunk.
//	size_t			mStart;		// Start number (e.g. item <mStart> of <mValue>)
//	size_t			mEnd;		// End number (e.g. item <mStart> to <mEnd> of <mValue>). If this chunk isn't a range, mStart = mEnd.
//};
