/*
 *  CVariableEntry.h
 *  HyperCompiler
 *
 *  Created by Uli on 6/7/07.
 *  Copyright 2007 M. Uli Kusterer. All rights reserved.
 *
 */

#pragma once

#include <string>

namespace Carlson
{


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
typedef enum TChunkType	// Keep this same as LEOChunkType:
{
	TChunkTypeInvalid = 0,
	TChunkTypeByte,
	TChunkTypeCharacter,
	TChunkTypeItem,
	TChunkTypeLine,
	TChunkTypeWord
} TChunkType;

class CVariableEntry
{
public:
	bool			mInitWithName;		// Variables are inited with an empty string if this is FALSE, with their name as a string if this is TRUE. This is handy for cases like unquoted one-word string constants or variables used before anything was put in them.
	bool			mIsParameter;		// Is this a parameter to this function?
	bool			mIsGlobal;			// Is this a global variable pulled into this function's scope?
	bool			mDontDispose;		// Don't dispose this variable at the end of its handler (currently only used for result).
	std::string		mRealName;			// Real name as the user sees it. User-defined variables internally get a prefix "var_" to avoid collisions with built-in system vars.
	TVariantType	mVariableType;		// Type for this variable.
	long			mBPRelativeOffset;	// Backpointer-relative offset of this variable, so we can find it.
	static int		mTempCounterSeed;
	
public:
	CVariableEntry( const std::string& realName, TVariantType theType, bool initWithName = false, bool isParam = false, bool isGlobal = false, bool dontDispose = false )
		: mInitWithName( initWithName ), mIsParameter( isParam ), mIsGlobal( isGlobal ), mRealName( realName ), mDontDispose(dontDispose), mBPRelativeOffset(LONG_MAX) {};
	CVariableEntry()
		: mInitWithName( false ), mIsParameter( false ), mIsGlobal( false ), mDontDispose( false ), mRealName(), mBPRelativeOffset(LONG_MAX) {};

	static const std::string GetNewTempName();
};
	
}
