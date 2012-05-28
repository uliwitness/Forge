/*
 *  CObjectPropertyNode.cpp
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 12.05.07.
 *  Copyright 2007 M. Uli Kusterer. All rights reserved.
 *
 */

#include "CObjectPropertyNode.h"
#include "CParseTree.h"
#include "CCodeBlock.h"
#include "CMakeChunkConstNode.h"
#include "CMakeChunkRefNode.h"


namespace Carlson
{

CValueNode*	CObjectPropertyNode::Copy()
{
	CObjectPropertyNode	*	nodeCopy = new CObjectPropertyNode( mParseTree, mSymbolName, mLineNum );
	
	std::vector<CValueNode*>::const_iterator	itty;
	for( itty = mParams.begin(); itty != mParams.end(); itty++ )
	{
		nodeCopy->AddParam( (*itty)->Copy() );
	}
	
	return nodeCopy;
}


void	CObjectPropertyNode::DebugPrint( std::ostream& destStream, size_t indentLevel )
{
	INDENT_PREPARE(indentLevel);
	
	destStream << indentChars << "Property \"" << mSymbolName << "\"" << std::endl
				<< indentChars << "{" << std::endl;
	
	std::vector<CValueNode*>::iterator itty;
	
	for( itty = mParams.begin(); itty != mParams.end(); itty++ )
	{
		(*itty)->DebugPrint( destStream, indentLevel +1 );
	}
	
	destStream << indentChars << "}" << std::endl;
}


void	CObjectPropertyNode::AddParam( CValueNode* val )
{
	mParams.push_back( val );
	mParseTree->NodeWasAdded(val);
}


void	CObjectPropertyNode::Simplify()
{
	std::vector<CValueNode*>::iterator itty;
	
	for( itty = mParams.begin(); itty != mParams.end(); itty++ )
		(*itty)->Simplify();
	
	CValueNode::Simplify();
}


void	CObjectPropertyNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	std::vector<CValueNode*>::reverse_iterator itty;
	
	// Push all params on stack (in reverse order!):
	inCodeBlock->GeneratePushStringInstruction( mSymbolName );
	for( itty = mParams.rbegin(); itty != mParams.rend(); itty++ )
		(*itty)->GenerateCode( inCodeBlock );
	
	inCodeBlock->GeneratePushPropertyOfObjectInstruction();
}

} // namespace Carlson
