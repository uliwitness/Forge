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
#include "CForgeExceptions.h"


namespace Carlson
{

class CCodeBlockNodeBase;

// -----------------------------------------------------------------------------
//	Classes:
// -----------------------------------------------------------------------------

class CValueNode : public CNode
{
public:
	explicit CValueNode( CParseTree* inTree, size_t inLineNum ) : CNode(inTree), mLineNum(inLineNum) {};
	virtual ~CValueNode() {};
	
	virtual size_t	GetLineNum()						{ return mLineNum; };
	virtual void	SetLineNum( size_t inLineNum )		{ mLineNum = inLineNum; };

	virtual void	GenerateCode( CCodeBlock* inCodeBlock );	// Generate the actual bytecode so it leaves the result on the stack.
		
	virtual bool	IsConstant()		{ return false; };
	
	virtual CValueNode*	Copy()			{ return NULL; };
	
	// If IsConstant() gives TRUE, you can call the following to try and get at your values:
	virtual int			GetAsInt()			{ throw CForgeParseError( "Can't make value into integer.", GetLineNum() ); return 0; };
	virtual std::string	GetAsString()		{ throw CForgeParseError( "Can't make value into string.", GetLineNum() ); return std::string(); };
	virtual bool		GetAsBool()			{ throw CForgeParseError( "Can't make value into boolean.", GetLineNum() ); return false; };
	virtual float		GetAsFloat()		{ throw CForgeParseError( "Can't make value into float.", GetLineNum() ); return 0.0; };

protected:
	size_t		mLineNum;
};

class CIntValueNode : public CValueNode
{
public:
	CIntValueNode( CParseTree* inTree, long n, size_t inLineNum ) : CValueNode(inTree,inLineNum), mIntValue(n) {};
	
	virtual void			GenerateCode( CCodeBlock* inCodeBlock );	// Generate the actual bytecode so it leaves the result on the stack.

	virtual bool			IsConstant()	{ return true; };

	virtual CIntValueNode*	Copy()			{ return new CIntValueNode( mParseTree, mIntValue, mLineNum ); };

	virtual void			DebugPrint( std::ostream& destStream, size_t indentLevel )
	{
		INDENT_PREPARE(indentLevel);
		
		destStream << indentChars << "int( " << mIntValue << " )" << std::endl;
	};

	virtual int				GetAsInt()		{ return (int)mIntValue; };
	virtual long			GetAsLong()		{ return mIntValue; };
	virtual float			GetAsFloat()	{ return mIntValue; };
	virtual std::string		GetAsString()	{ char	numStr[256]; snprintf(numStr, 256, "%ld", mIntValue); return std::string( numStr ); };
	
protected:
	long		mIntValue;
};


class CFloatValueNode : public CValueNode
{
public:
	CFloatValueNode( CParseTree* inTree, float n, size_t inLineNum ) : CValueNode(inTree,inLineNum), mFloatValue(n) {};
	
	virtual void				GenerateCode( CCodeBlock* inCodeBlock );	// Generate the actual bytecode so it leaves the result on the stack.

	virtual bool				IsConstant()		{ return true; };

	virtual CFloatValueNode*	Copy()		{ return new CFloatValueNode( mParseTree, mFloatValue, mLineNum ); };

	virtual void				DebugPrint( std::ostream& destStream, size_t indentLevel )
	{
		INDENT_PREPARE(indentLevel);
		
		destStream << indentChars << "float( " << mFloatValue << " )" << std::endl;
	};
	
	virtual float				GetAsFloat()	{ return mFloatValue; };
	virtual int					GetAsInt()
	{
		if( mFloatValue == truncf(mFloatValue) )
			return mFloatValue;
		else
			throw CForgeParseError( "Can't make floating point number into integer.", GetLineNum() );
		return 0.0;
	};
	virtual std::string			GetAsString()	{ char	numStr[256]; snprintf(numStr, 256, "%f", mFloatValue); return std::string( numStr ); };

protected:
	float		mFloatValue;
};


class CBoolValueNode : public CValueNode
{
public:
	CBoolValueNode( CParseTree* inTree, bool n, size_t inLineNum ) : CValueNode(inTree,inLineNum), mBoolValue(n) {};
	
	virtual void				GenerateCode( CCodeBlock* inCodeBlock );	// Generate the actual bytecode so it leaves the result on the stack.
	
	virtual bool				IsConstant()		{ return true; };

	virtual CBoolValueNode*		Copy()		{ return new CBoolValueNode( mParseTree, mBoolValue, mLineNum ); };

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
	CStringValueNode( CParseTree* inTree, const std::string& n, size_t inLineNum ) : CValueNode(inTree,inLineNum), mStringValue(n) {};
	
	virtual void				GenerateCode( CCodeBlock* inCodeBlock );	// Generate the actual bytecode so it leaves the result on the stack.
	
	virtual bool				IsConstant()		{ return true; };

	virtual CStringValueNode*	Copy()				{ return new CStringValueNode( mParseTree, mStringValue, mLineNum ); };

	virtual void				DebugPrint( std::ostream& destStream, size_t indentLevel )
	{
		INDENT_PREPARE(indentLevel);
		
		destStream << indentChars << "\"" << mStringValue.c_str() << "\"" << std::endl;
	};
	
	virtual std::string			GetAsString()	{ return mStringValue; };
	virtual bool				GetAsBool()
	{
		if( mStringValue.compare("true") == 0 )
			return true;
		else if( mStringValue.compare("false") != 0 )
			throw CForgeParseError( "Can't make string into boolean.", GetLineNum() );
		return false;
	};

protected:
	std::string		mStringValue;
};


class CLocalVariableRefValueNode : public CValueNode
{
public:
	CLocalVariableRefValueNode( CParseTree* inTree, CCodeBlockNodeBase *inCodeBlockNode, const std::string& inVarName, const std::string& inRealVarName, size_t inLineNum );
	
	virtual void				Simplify();
	virtual void				GenerateCode( CCodeBlock* inCodeBlock );	// Generate the actual bytecode so it leaves the result on the stack.
	
	virtual CLocalVariableRefValueNode*	Copy()							{ return new CLocalVariableRefValueNode( mParseTree, mCodeBlockNode, mVarName, mRealVarName, mLineNum ); };
	
	virtual void				DebugPrint( std::ostream& destStream, size_t indentLevel )
	{
		INDENT_PREPARE(indentLevel);
		
		destStream << indentChars << "localVar( " << mVarName.c_str() << " )" << std::endl;
	};
	
	long					GetBPRelativeOffset();
	
	std::string				GetVarName()		{ return mVarName; };
	std::string				GetRealVarName()	{ return mRealVarName; };

protected:
	std::string				mVarName;
	std::string				mRealVarName;
	CCodeBlockNodeBase *	mCodeBlockNode;
};

}
