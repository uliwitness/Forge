/*
 *  CCodeBlock.h
 *  Forge
 *
 *  Created by Uli Kusterer on 31.10.10.
 *  Copyright 2010 Uli Kusterer. All rights reserved.
 *
 */

#include <string>


struct LEOScript;

namespace Carlson
{

class CCodeBlock;

class CCodeBlockProgressDelegate
{
public:
	virtual ~CCodeBlockProgressDelegate()																				{};
	
	virtual void	CodeBlockPreparing( CCodeBlock* blk )																{};
	virtual void	CodeBlockAddingFunction( CCodeBlock* blk, const std::string& methodName )							{};
	virtual void	CodeBlockAddedData( CCodeBlock* blk )																{};	// This gets called a lot, and may even be caused by the other callbacks.
	virtual void	CodeBlockAddedCode( CCodeBlock* blk )																{};	// This gets called a lot, and may even be caused by the other callbacks.
	virtual void	CodeBlockUpdateSymbolUseCount( CCodeBlock* blk, const std::string& methodName, int32_t numUses )	{};
	virtual void	CodeBlockAddingSymbol( CCodeBlock* blk, const std::string& symbolName, bool isExternal )			{};
	virtual void	CodeBlockFinished( CCodeBlock* blk )																{};
	
	virtual void	CodeBlockAddedStringData( CCodeBlock* blk, const std::string& str )									{};
	virtual void	CodeBlockAddedIntData( CCodeBlock* blk, int n )														{};
	virtual void	CodeBlockAddedFloatData( CCodeBlock* blk, float n )													{};
	virtual void	CodeBlockAddedBoolData( CCodeBlock* blk, bool n )													{};
	virtual void	CodeBlockUsedLocalVariable( CCodeBlock* blk, const std::string& str, int32_t numUses )				{};
};


class CCodeBlock
{
public:
	CCodeBlock( LEOScript* inScript, CCodeBlockProgressDelegate * progressDelegate );
	virtual ~CCodeBlock();
	
	size_t		code_size()		{ return 0; };
	size_t		data_size()		{ return 0; };
	
protected:
	CCodeBlockProgressDelegate	*	mProgressDelegate;
	LEOScript*						mScript;
};

}