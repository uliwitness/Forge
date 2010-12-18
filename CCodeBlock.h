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
struct LEOHandler;
struct LEOContextGroup;

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
	CCodeBlock( LEOContextGroup * inGroup, LEOScript* inScript, CCodeBlockProgressDelegate * progressDelegate );
	virtual ~CCodeBlock();
	
	size_t		GetCodeSize()		{ return 0; };
	size_t		GetDataSize()		{ return 0; };
	
	void		GenerateFunctionPrologForName( const std::string& inName );
	void		GenerateFunctionEpilogForName( const std::string& inName );
	
	void		GeneratePushIntInstruction( int inNumber );
	void		GeneratePushFloatInstruction( float inNumber );
	void		GeneratePushBoolInstruction( bool inBoolean );
	void		GeneratePushStringInstruction( const std::string& inString );
	void		GeneratePushVariableInstruction( size_t bpRelativeOffset );
	void		GeneratePopIntoVariableInstruction( size_t bpRelativeOffset );
	
protected:
	CCodeBlockProgressDelegate	*	mProgressDelegate;
	LEOScript*						mScript;
	LEOContextGroup*				mGroup;
	LEOHandler*						mCurrentHandler;
};

}