/*
 *  CVariableEntry.h
 *  HyperCompiler
 *
 *  Created by Uli on 6/7/07.
 *  Copyright 2007 M. Uli Kusterer. All rights reserved.
 *
 */

#include "TVariantConstants.h"
#include <string>

namespace Carlson
{

class CVariableEntry
{
public:
	bool			mInitWithName;		// Variables are inited with an empty string if this is FALSE, with their name as a string if this is TRUE. This is handy for cases like unquoted one-word string constants or variables used before anything was put in them.
	bool			mIsParameter;		// Is this a parameter to this function?
	bool			mIsGlobal;			// Is this a global variable pulled into this function's scope?
	bool			mDontDispose;		// Don't dispose this variable at the end of its handler (currently only used for result).
	std::string		mRealName;			// Real name as the user sees it. User-defined variables internally get a prefix "var_" to avoid collisions with built-in system vars.
//	std::string		mInitCode;			// Custom init code for this variable. If this is an empty string this is ignored.
//	bool			mInitDirectly;		// Generate a "CVariant nameXXX" statement (with XXX being mInitCode) instead of "CVariant name;\nXXX".
	TVariantType	mVariableType;		// Type for this variable.
	static int		mTempCounterSeed;
	
public:
	CVariableEntry( const std::string& realName, TVariantType theType, bool initWithName = false, bool isParam = false, bool isGlobal = false, bool dontDispose = false )
		: mInitWithName( initWithName ), mIsParameter( isParam ), mIsGlobal( isGlobal ), mRealName( realName ), mDontDispose(dontDispose) {};
	CVariableEntry( const std::string& realName, const std::string& initCode, bool dontDispose = false, bool initDirectly = false )
		: mInitWithName( false ), mIsParameter( false ), mIsGlobal( false ), mRealName( realName ), mDontDispose(dontDispose) {};
	CVariableEntry()
		: mInitWithName( false ), mIsParameter( false ), mIsGlobal( false ), mDontDispose( false ), mRealName() {};

	static const std::string GetNewTempName();
};
	
}