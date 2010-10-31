/*
 *  CCommandNode.cpp
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 10.05.07.
 *  Copyright 2007 M. Uli Kusterer. All rights reserved.
 *
 */

#include "CCommandNode.h"
#include "CValueNode.h"


namespace Carlson
{

void	CCommandNode::DebugPrint( std::ostream& destStream, size_t indentLevel )
{
	INDENT_PREPARE(indentLevel);
	
	destStream << indentChars << "Command \"" << mSymbolName << "\"" << std::endl
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


void	CCommandNode::GenerateCode( CodeBlock& codeBlock )
{
	std::vector<ParamEntry>				paramList;
	
	if( mSymbolName.compare( "=" ) == 0 )	// Low-level raw assignment:
	{
		// TODO: Actually compile assignment:
		std::cout << "Warning: INTERNAL ERROR: Raw assignment not yet implemented, but used!" << std::endl;
		paramList.resize(2);
		mParams[0]->GenerateCodeWithReturnOffset( codeBlock, codeBlock.offset_for_param( &paramList[0], 0 ) );
		mParams[1]->GenerateCodeWithReturnOffset( codeBlock, codeBlock.offset_for_param( &paramList[0], 1 ) );
	}
	else if( mSymbolName.compare( "+=" ) == 0 )	// Low-level raw assignment:
	{
		// TODO: Actually compile assignment:
		paramList.resize(2);
		EBPRelativeOffset	theOffset = 0;
		if( mParams[0]->GenerateCodeAndProvideEBPRelativeOffset( theOffset ) && mParams[1]->IsConstant() )
		{
			codeBlock.push_back_leal_ebp_eax( theOffset );			// Load current value from dest.
			codeBlock.push_back_addl_eax( mParams[1]->GetAsInt() );	// Add that much to it.
			codeBlock.push_back_movl_eax_ebp( theOffset );			// Write it back to dest.
		}
		else
		{
			std::cout << "Warning: INTERNAL ERROR: Raw assignment not yet implemented, but used!" << std::endl;

			mParams[0]->GenerateCodeWithReturnOffset( codeBlock, codeBlock.offset_for_param( &paramList[0], 0 ) );
			mParams[1]->GenerateCodeWithReturnOffset( codeBlock, codeBlock.offset_for_param( &paramList[0], 1 ) );
		}
	}
	else if( mSymbolName.compare( "return" ) == 0 )	// Low-level raw result assignment:
	{
		// TODO: Actually compile assignment:
		std::cout << "Warning: INTERNAL ERROR: Return statement not yet implemented, but used!" << std::endl;
		paramList.resize(2);
		mParams[0]->GenerateCodeWithReturnOffset( codeBlock, codeBlock.offset_for_param( &paramList[0], 0 ) );
	}
	else	// Any other method or function call:
	{
		// Find out how much stack we need for params and add instructions to allocate it:
		std::vector<CValueNode*>::iterator	itty;
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
}


void	CCommandNode::GenerateCpp( CppBlock& codeBlock )
{
	std::vector<ParamEntry>				paramList;
	
	if( mSymbolName.compare( "=" ) == 0 )	// Low-level raw assignment:
	{
		codeBlock.generate_binary_operator_cmd_start(mSymbolName);
		mParams[0]->GenerateCpp( codeBlock );
		codeBlock.generate_binary_operator_cmd_middle(mSymbolName);
		mParams[1]->GenerateCpp( codeBlock );
		codeBlock.generate_binary_operator_cmd_end(mSymbolName);
	}
	else if( mSymbolName.compare( "return" ) == 0 )	// Low-level raw result assignment:
	{
		codeBlock.generate_return_statement_start();
		mParams[0]->GenerateCpp( codeBlock );
		codeBlock.generate_return_statement_end();
	}
	else	// Any other method or function call:
	{
		codeBlock.generate_command_call_start( mSymbolName );
		
		if( mParams.size() > 0 )
		{
			codeBlock.generate_command_call_param_start( mSymbolName );
			std::vector<CValueNode*>::iterator	itty;
			for( itty = mParams.begin(); itty != mParams.end(); itty++ )
			{
				if( itty != mParams.begin() )
					codeBlock.generate_command_call_param_delim( mSymbolName );
				(*itty)->GenerateCpp( codeBlock );
			}
			codeBlock.generate_command_call_param_end( mSymbolName );
		}
		
		codeBlock.generate_command_call_end( mSymbolName );
	}
}

} // namespace Carlson