/*
 *  CDownloadCommandNode.cpp
 *  Stacksmith
 *
 *  Created by Uli Kusterer on 29.03.13.
 *  Copyright 2013 M. Uli Kusterer. All rights reserved.
 *
 */

#include "CDownloadCommandNode.h"
#include "CCodeBlock.h"
#include "CNodeTransformation.h"
#include "CFunctionDefinitionNode.h"
#include "LEODownloadInstructions.h"


namespace Carlson
{

void	CDownloadCommandNode::EnsureBlockNameParamsAreSet()
{
	if( !mBlockNamesAdded )
	{
		AddParam( new CStringValueNode( mParseTree, mProgressBlockName, GetLineNum() ) );
		AddParam( new CStringValueNode( mParseTree, mCompletionBlockName, GetLineNum() ) );
		mBlockNamesAdded = true;
	}
}


void	CDownloadCommandNode::GenerateCode( CCodeBlock* inBlock )
{
	EnsureBlockNameParamsAreSet();
	
	CCommandNode::GenerateCode( inBlock );
	inBlock->GenerateOperatorInstruction( kFirstDownloadInstruction +DOWNLOAD_INSTR );
}


void	CDownloadCommandNode::Simplify()
{
	EnsureBlockNameParamsAreSet();

	CCommandNode::Simplify();
	
	// Don't optimize mProgressBlock and mCompletionBlock, they're regular handlers
	//	on the parse tree and it will take care of optimizing them.
}


void	CDownloadCommandNode::DebugPrint( std::ostream& destStream, size_t indentLevel )
{
	EnsureBlockNameParamsAreSet();
	
	INDENT_PREPARE(indentLevel);
	
	CCommandNode::DebugPrint( destStream, indentLevel );
}


CCodeBlockNodeBase*	CDownloadCommandNode::CreateProgressBlock( size_t inLineNum )
{
	mProgressBlockName = mParseTree->GetUniqueIdentifierBasedOn("::downloadProgress:");
	mProgressBlock = new CFunctionDefinitionNode( mParseTree, true, mProgressBlockName, inLineNum );
	mParseTree->AddNode( mProgressBlock );
		
	return mProgressBlock;
}


CCodeBlockNodeBase*	CDownloadCommandNode::CreateCompletionBlock( size_t inLineNum )
{
	mCompletionBlockName = mParseTree->GetUniqueIdentifierBasedOn("::downloadCompletion:");
	mCompletionBlock = new CFunctionDefinitionNode( mParseTree, true, mCompletionBlockName, inLineNum );
	mParseTree->AddNode( mCompletionBlock );

	return mCompletionBlock;
}

}
