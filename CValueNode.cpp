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


namespace Carlson
{

void	CValueNode::GenerateCode( CodeBlock& codeBlock )
{
	throw std::runtime_error( "INTERNAL ERROR: Can't generate code for a value node without a destination." );
}


void	CValueNode::GenerateCpp( CppBlock& codeBlock )
{
	throw std::runtime_error( "INTERNAL ERROR: Value node missing implementation." );
}


void	CIntValueNode::GenerateCodeWithReturnOffset( CodeBlock& codeBlock, ESPRelativeOffset resultOffs )
{
	codeBlock.push_back_movl_n_m_esp( mIntValue, resultOffs );
	codeBlock.progress_added_int( mIntValue );
}

void	CIntValueNode::GenerateCpp( CppBlock& codeBlock )
{
	codeBlock.generate_int( mIntValue );
}


void	CFloatValueNode::GenerateCodeWithReturnOffset( CodeBlock& codeBlock, ESPRelativeOffset resultOffs )
{
	codeBlock.push_back_movl_n_m_esp( (*(int32_t*)&mFloatValue), resultOffs );
	codeBlock.progress_added_float( mFloatValue );
}

void	CFloatValueNode::GenerateCpp( CppBlock& codeBlock )
{
	codeBlock.generate_float( mFloatValue );
}

void	CBoolValueNode::GenerateCodeWithReturnOffset( CodeBlock& codeBlock, ESPRelativeOffset resultOffs )
{
	codeBlock.push_back_movl_n_m_esp( mBoolValue, resultOffs );
	codeBlock.progress_added_bool( mBoolValue );
}

void	CBoolValueNode::GenerateCpp( CppBlock& codeBlock )
{
	codeBlock.generate_bool( mBoolValue );
}

void	CStringValueNode::GenerateCodeWithReturnOffset( CodeBlock& codeBlock, ESPRelativeOffset resultOffs )
{
	// Generate a length long in our data section, followed by that many bytes of string:
	size_t				strLength = mStringValue.length();
	codeBlock.push_front_data( mStringValue.c_str(), strLength );
	OffsetFromCodeStart	dataRawOffs = codeBlock.push_front_data( &strLength, sizeof(strLength) );
	EBXRelativeOffset	dataEBXOffs = dataRawOffs -codeBlock.current_reference_offset();
	codeBlock.push_back_leal_ebx_eax( dataEBXOffs );	// Get address of that data bit.
	
	// Call MakeStringObject with address of our counted string data to create the string:
	ParamEntry		params[] = { kPointerParam, kEndOfParamList };
	int32_t	stackNeeded = codeBlock.stackspace_for_params( params );
	codeBlock.push_back_subl_esp( stackNeeded );	// Get stack space for params.
	ESPRelativeOffset	paramOffs = codeBlock.offset_for_param( params, 0 );
	codeBlock.push_back_movl_eax_m_esp( paramOffs );
	codeBlock.push_back_call("vcy_string_alloc");
	codeBlock.push_back_addl_esp( stackNeeded );	// Free stack space for params.
	
	// Stash the new string object on the stack, where we're supposed to put it:
	codeBlock.push_back_movl_eax_m_esp( resultOffs );
	codeBlock.progress_added_string( mStringValue );
}

void	CStringValueNode::GenerateCpp( CppBlock& codeBlock )
{
	codeBlock.generate_string( mStringValue );
}

void	CLocalVariableRefValueNode::GenerateCodeWithReturnOffset( CodeBlock& codeBlock, ESPRelativeOffset resultOffs )
{
	EBPRelativeOffset	offs = codeBlock.get_local_var_offset( mVarName );
	codeBlock.push_back_leal_ebp_eax( offs );
	codeBlock.push_back_movl_eax_m_esp( resultOffs );
	codeBlock.progress_used_local_var( mVarName, 1 );
}

void	CLocalVariableRefValueNode::GenerateCpp( CppBlock& codeBlock )
{
	codeBlock.generate_var_ref( mVarName );
}


bool	CLocalVariableRefValueNode::GenerateCodeAndProvideEBPRelativeOffset( CodeBlock& codeBlock, EBPRelativeOffset &outOffset )
{
	outOffset = codeBlock.get_local_var_offset( mVarName );
	
	return true;
}

} // namespace Carlson
