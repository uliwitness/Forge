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

void	CValueNode::GenerateCode( CCodeBlock* inCodeBlock )
{
#pragma unused(inCodeBlock)
}
	
	
void	CIntValueNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	inCodeBlock->GeneratePushIntInstruction( mIntValue, mUnit );
}


void	CFloatValueNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	inCodeBlock->GeneratePushFloatInstruction( mFloatValue, mUnit );
}


void	CBoolValueNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	inCodeBlock->GeneratePushBoolInstruction( mBoolValue );
}


void	CStringValueNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	inCodeBlock->GeneratePushStringInstruction( mStringValue );
}


void	CUnsetValueNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	inCodeBlock->GeneratePushUnsetValueInstruction();
}


CLocalVariableRefValueNode::CLocalVariableRefValueNode( CParseTree* inTree, CCodeBlockNodeBase *inCodeBlockNode,
														const std::string& inVarName, const std::string& inRealVarName, size_t inLineNum )
	: CValueNode(inTree,inLineNum), mCodeBlockNode(inCodeBlockNode), mVarName(inVarName), mRealVarName(inRealVarName)
{
	mCodeBlockNode->AddLocalVar( inVarName, inRealVarName, TVariantType_INVALID );
}


void	CLocalVariableRefValueNode::Simplify()
{
	GetBPRelativeOffset();	// Make sure we are assigned a slot NOW, so we know how many variables we need by the time we generate the function prolog.
	
	CNode::Simplify();
}


void	CLocalVariableRefValueNode::GenerateCode( CCodeBlock* inCodeBlock )
{
	inCodeBlock->GeneratePushVariableInstruction( GetBPRelativeOffset() );
}


long	CLocalVariableRefValueNode::GetBPRelativeOffset()
{
	return mCodeBlockNode->GetBPRelativeOffsetForLocalVar(mVarName);
}
	
	
CArrayValueNode*	CArrayValueNode::Copy()
{
	CArrayValueNode * array = new CArrayValueNode( mParseTree, mLineNum );
	for( CValueNode * currValue : mArray )
	{
		array->AddItem( currValue->Copy() );
	}
	return array;
}


} // namespace Carlson
