/*
 *  CDownloadCommandNode.h
 *  Stacksmith
 *
 *  Created by Uli Kusterer on 29.03.13.
 *  Copyright 2013 M. Uli Kusterer. All rights reserved.
 *
 */

#include "CCommandNode.h"
#include "CCodeBlockNode.h"


namespace Carlson
{

class CDownloadCommandNode : public CCommandNode
{
public:
	CDownloadCommandNode( CParseTree* inTree, size_t inLineNum, std::string inFileName ) : CCommandNode( inTree, "download", inLineNum, inFileName ), mProgressBlock(NULL), mCompletionBlock(NULL), mBlockNamesAdded(false), mProgressLineNum(0), mProgressCommandsLineNum(0), mCompletionLineNum(0), mCompletionCommandsLineNum(0), mEndDownloadLineNum(0) {};

	virtual CCodeBlockNodeBase*	CreateProgressBlock( size_t inLineNum );
	virtual CCodeBlockNodeBase*	GetProgressBlock()							{ return mProgressBlock; };		// May return NULL!
	virtual CCodeBlockNodeBase*	CreateCompletionBlock( size_t inLineNum );
	virtual CCodeBlockNodeBase*	GetCompletionBlock()						{ return mCompletionBlock; };	// May return NULL!
	
	virtual void			DebugPrint( std::ostream& destStream, size_t indentLevel );
	virtual void			GenerateCode( CCodeBlock* inBlock );
	virtual void			Simplify();
	
	size_t	GetProgressLineNum()						{ return mProgressLineNum; };
	void	SetProgressLineNum( size_t n )				{ mProgressLineNum = n; };
	size_t	GetProgressCommandsLineNum()				{ return mProgressCommandsLineNum; };
	void	SetProgressCommandsLineNum( size_t n )		{ mProgressCommandsLineNum = n; };
	size_t	GetCompletionLineNum()						{ return mCompletionLineNum; };
	void	SetCompletionLineNum( size_t n )			{ mCompletionLineNum = n; };
	size_t	GetCompletionCommandsLineNum()				{ return mCompletionCommandsLineNum; };
	void	SetCompletionCommandsLineNum( size_t n )	{ mCompletionCommandsLineNum = n; };
	size_t	GetEndDownloadLineNum()						{ return mEndDownloadLineNum; };
	void	SetEndDownloadLineNum( size_t n )				{ mEndDownloadLineNum = n; };

protected:
	virtual void			EnsureBlockNameParamsAreSet();
	
protected:
	std::string			mProgressBlockName;
	CCodeBlockNodeBase*	mProgressBlock;		// Owned by parse tree.
	std::string			mCompletionBlockName;
	CCodeBlockNodeBase*	mCompletionBlock;	// Owned by parse tree.
	bool				mBlockNamesAdded;
	size_t				mProgressLineNum;
	size_t				mProgressCommandsLineNum;
	size_t				mCompletionLineNum;
	size_t				mCompletionCommandsLineNum;
	size_t				mEndDownloadLineNum;
};

}
