/*
 *  CFunctionCallNode.cpp
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 12.05.07.
 *  Copyright 2007 M. Uli Kusterer. All rights reserved.
 *
 */

#include "CFunctionCallNode.h"


namespace Carlson
{

void	CFunctionCallNode::GenerateCodeWithReturnOffset( CodeBlock& codeBlock, ESPRelativeOffset resultOffset )
{
	GenerateCode( codeBlock );
	
	// Move result into the desired slot:
	codeBlock.push_back_movl_eax_m_esp( resultOffset );
}


void	CFunctionCallNode::GenerateCode( CodeBlock& codeBlock )
{
	// Find out how much stack we need for params and add instructions to allocate it:
	std::vector<CValueNode*>::iterator	itty;
	std::vector<ParamEntry>				paramList;
	int									x = 0;
	
	paramList.resize( mParams.size() +1, kEndOfParamList );
	for( x = 0, itty = mParams.begin(); itty != mParams.end(); itty++ )
		(*itty)->FillOutParamEntry( paramList[x++] );
	
	int32_t	stackNeeded = codeBlock.stackspace_for_params( &paramList[0] );
	codeBlock.push_back_subl_esp( stackNeeded );
	
	// Add instructions that push params on stack:
	for( x = 0, itty = mParams.begin(); itty != mParams.end(); itty++ )
		(*itty)->GenerateCodeWithReturnOffset( codeBlock, codeBlock.offset_for_param( &paramList[0], x++ ) );
	
	// Add instruction that actually executes the call:
	codeBlock.push_back_call( mSymbolName );
	codeBlock.push_back_addl_esp( stackNeeded );					// Remove parameters.
}


void	CFunctionCallNode::GenerateCpp( CppBlock& codeBlock )
{
	std::vector<ParamEntry>				paramList;
	
	codeBlock.generate_function_call_start( mSymbolName );
	
	if( mSymbolName.compare( "=" ) == 0 || mSymbolName.compare( "+" ) == 0
		|| mSymbolName.compare( "-" ) == 0 || mSymbolName.compare( "*" ) == 0
		|| mSymbolName.compare( "/" ) == 0 || mSymbolName.compare( "==" ) == 0
		|| mSymbolName.compare( "!=" ) == 0 || mSymbolName.compare( ">" ) == 0
		|| mSymbolName.compare( "<" ) == 0 || mSymbolName.compare( "<=" ) == 0
		|| mSymbolName.compare( ">=" ) == 0 || mSymbolName.compare( "->" ) == 0
		|| mSymbolName.compare( "." ) == 0 || mSymbolName.compare( "," ) == 0
		|| mSymbolName.compare( "&" ) == 0 || mSymbolName.compare( "*" ) == 0
		|| mSymbolName.compare( "%" ) == 0 )
	{
		codeBlock.generate_binary_operator_start(mSymbolName);
		mParams[0]->GenerateCpp( codeBlock );
		codeBlock.generate_binary_operator_middle(mSymbolName);
		mParams[1]->GenerateCpp( codeBlock );
		codeBlock.generate_binary_operator_end(mSymbolName);
	}
	else if( mParams.size() > 0 )
	{
		codeBlock.generate_function_call_param_start( mSymbolName );
		std::vector<CValueNode*>::iterator	itty;
		for( itty = mParams.begin(); itty != mParams.end(); itty++ )
		{
			if( itty != mParams.begin() )
				codeBlock.generate_function_call_param_delim( mSymbolName );
			(*itty)->GenerateCpp( codeBlock );
		}
		codeBlock.generate_function_call_param_end( mSymbolName );
	}
	
	codeBlock.generate_function_call_end( mSymbolName );
}


void	CFunctionCallNode::DebugPrint( std::ostream& destStream, size_t indentLevel )
{
	INDENT_PREPARE(indentLevel);
	
	destStream << indentChars << "Function Call \"" << mSymbolName << "\"" << std::endl
				<< indentChars << "{" << std::endl
				<< indentChars << "\tparams" << std::endl
				<< indentChars << "\t{" << std::endl;
	
	std::vector<CValueNode*>::iterator itty;
	
	for( itty = mParams.begin(); itty != mParams.end(); itty++ )
	{
		(*itty)->DebugPrint( destStream, indentLevel +2 );
	}
	
	destStream << indentChars << "\t}" << std::endl;
	
	destStream << indentChars << "}" << std::endl;
}



} // namespace Carlson