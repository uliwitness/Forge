/*
 *  CValueNode.cpp
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 10.05.07.
 *  Copyright 2007 M. Uli Kusterer. All rights reserved.
 *
 */

// -----------------------------------------------------------------------------
//	Headers:
// -----------------------------------------------------------------------------

#include "CValueNode.h"
#include "CCodeBlock.h"
#include "CCodeBlockNode.h"


namespace Carlson
{

void	CIntValueNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	inCodeBlock->GeneratePushIntInstruction( mIntValue );
}


void	CFloatValueNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	inCodeBlock->GeneratePushFloatInstruction( mFloatValue );
}


void	CBoolValueNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	inCodeBlock->GeneratePushBoolInstruction( mBoolValue );
}


void	CStringValueNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	inCodeBlock->GeneratePushStringInstruction( mStringValue );
}


CLocalVariableRefValueNode::CLocalVariableRefValueNode( CParseTree* inTree, CCodeBlockNodeBase *inCodeBlockNode, const std::string& inVarName, const std::string& inRealVarName )
	: CValueNode(inTree), mCodeBlockNode(inCodeBlockNode), mVarName(inVarName), mRealVarName(inRealVarName)
{
	mCodeBlockNode->AddLocalVar( inVarName, inRealVarName, TVariantType_INVALID );
}


void	CLocalVariableRefValueNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	inCodeBlock->GeneratePushVariableInstruction( GetBPRelativeOffset() );
}


size_t	CLocalVariableRefValueNode::GetBPRelativeOffset()
{
	return mCodeBlockNode->GetBPRelativeOffsetForLocalVar(mVarName);
}

} // namespace Carlson
