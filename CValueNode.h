/*
 *  CValueNode.h
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 10.05.07.
 *  Copyright 2007 M. Uli Kusterer. All rights reserved.
 *
 */

#pragma once

// -----------------------------------------------------------------------------
//	Headers:
// -----------------------------------------------------------------------------

#include "CNode.h"
#include <math.h>
#include <stdexcept>


namespace Carlson
{


// -----------------------------------------------------------------------------
//	Classes:
// -----------------------------------------------------------------------------

class CValueNode : public CNode
{
public:
	CValueNode() {};
	virtual ~CValueNode() {};
	
	virtual size_t	GetLineNum()		{ return 0; };
	
	virtual void	Simplify()			{};
	
	virtual bool	IsConstant()		{ return false; };
	
	virtual CValueNode*	Copy()			{ return NULL; };
	
	// If IsConstant() gives TRUE, you can call the following to try and get at your values:
	virtual int			GetAsInt()			{ throw std::runtime_error( "ERROR: Can't make value into integer." ); return 0; };
	virtual std::string	GetAsString()		{ throw std::runtime_error( "ERROR: Can't make value into string." ); return std::string(); };
	virtual bool		GetAsBool()			{ throw std::runtime_error( "ERROR: Can't make value into boolean." ); return false; };
	virtual float		GetAsFloat()		{ throw std::runtime_error( "ERROR: Can't make value into float." ); return 0.0; };
};

class CIntValueNode : public CValueNode
{
public:
	CIntValueNode( int n ) : CValueNode(), mIntValue(n) {};
	
	virtual bool			IsConstant()	{ return true; };

	virtual CIntValueNode*	Copy()			{ return new CIntValueNode( mIntValue ); };

	virtual void			DebugPrint( std::ostream& destStream, size_t indentLevel )
	{
		INDENT_PREPARE(indentLevel);
		
		destStream << indentChars << "int( " << mIntValue << " )" << std::endl;
	};

	virtual int				GetAsInt()		{ return mIntValue; };
	virtual float			GetAsFloat()	{ return mIntValue; };
	virtual std::string		GetAsString()	{ char	numStr[256]; snprintf(numStr, 256, "%d", mIntValue); return std::string( numStr ); };

protected:
	int		mIntValue;
};


class CFloatValueNode : public CValueNode
{
public:
	CFloatValueNode( float n ) : CValueNode(), mFloatValue(n) {};

	virtual bool				IsConstant()		{ return true; };

	virtual CFloatValueNode*	Copy()		{ return new CFloatValueNode( mFloatValue ); };

	virtual void				DebugPrint( std::ostream& destStream, size_t indentLevel )
	{
		INDENT_PREPARE(indentLevel);
		
		destStream << indentChars << "float( " << mFloatValue << " )" << std::endl;
	};
	
	virtual float				GetAsFloat()	{ return mFloatValue; };
	virtual int					GetAsInt()		{ if( mFloatValue == truncf(mFloatValue) ) return mFloatValue; else throw std::runtime_error( "ERROR: Can't make floating point number into integer." ); return 0.0; };
	virtual std::string			GetAsString()	{ char	numStr[256]; snprintf(numStr, 256, "%f", mFloatValue); return std::string( numStr ); };

protected:
	float		mFloatValue;
};


class CBoolValueNode : public CValueNode
{
public:
	CBoolValueNode( bool n ) : CValueNode(), mBoolValue(n) {};
	
	virtual bool				IsConstant()		{ return true; };

	virtual CBoolValueNode*		Copy()		{ return new CBoolValueNode( mBoolValue ); };

	virtual void				DebugPrint( std::ostream& destStream, size_t indentLevel )
	{
		INDENT_PREPARE(indentLevel);
		
		destStream << indentChars << (mBoolValue ? "true" : "false") << std::endl;
	};
	
	virtual bool				GetAsBool()		{ return mBoolValue; };
	virtual std::string			GetAsString()	{ return std::string( mBoolValue ? "true" : "false" ); };

protected:
	bool		mBoolValue;
};


class CStringValueNode : public CValueNode
{
public:
	CStringValueNode( const std::string& n ) : CValueNode(), mStringValue(n) {};
	
	virtual bool				IsConstant()		{ return true; };

	virtual CStringValueNode*	Copy()									{ return new CStringValueNode( mStringValue ); };

	virtual void				DebugPrint( std::ostream& destStream, size_t indentLevel )
	{
		INDENT_PREPARE(indentLevel);
		
		destStream << indentChars << "\"" << mStringValue.c_str() << "\"" << std::endl;
	};
	
	virtual std::string			GetAsString()	{ return mStringValue; };
	virtual bool				GetAsBool()		{ if( mStringValue.compare("true") == 0 ) return true; else if( mStringValue.compare("false") != 0 ) throw std::runtime_error( "ERROR: Can't make string into boolean." ); return false; };

protected:
	std::string		mStringValue;
};


class CLocalVariableRefValueNode : public CValueNode
{
public:
	CLocalVariableRefValueNode( const std::string& inVarName ) : CValueNode(), mVarName(inVarName) {};
	
	virtual CLocalVariableRefValueNode*	Copy()							{ return new CLocalVariableRefValueNode( mVarName ); };
	
	virtual void				DebugPrint( std::ostream& destStream, size_t indentLevel )
	{
		INDENT_PREPARE(indentLevel);
		
		destStream << indentChars << "localVar( " << mVarName.c_str() << " )" << std::endl;
	};

protected:
	std::string			mVarName;
};

}