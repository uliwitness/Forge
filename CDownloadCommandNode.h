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
	CDownloadCommandNode( CParseTree* inTree, size_t inLineNum ) : CCommandNode( inTree, "download", inLineNum ), mProgressBlock(NULL), mCompletionBlock(NULL), mBlockNamesAdded(false) {};

	virtual CCodeBlockNodeBase*	CreateProgressBlock( size_t inLineNum );
	virtual CCodeBlockNodeBase*	GetProgressBlock()							{ return mProgressBlock; };		// May return NULL!
	virtual CCodeBlockNodeBase*	CreateCompletionBlock( size_t inLineNum );
	virtual CCodeBlockNodeBase*	GetCompletionBlock()						{ return mCompletionBlock; };	// May return NULL!
	
	virtual void			DebugPrint( std::ostream& destStream, size_t indentLevel );
	virtual void			GenerateCode( CCodeBlock* inBlock );
	virtual void			Simplify();
	
protected:
	virtual void			EnsureBlockNameParamsAreSet();
	
protected:
	std::string			mProgressBlockName;
	CCodeBlockNodeBase*	mProgressBlock;		// Owned by parse tree.
	std::string			mCompletionBlockName;
	CCodeBlockNodeBase*	mCompletionBlock;	// Owned by parse tree.
	bool				mBlockNamesAdded;
};

}
