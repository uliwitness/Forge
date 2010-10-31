/*
 *  CppBlock.cpp
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 21.11.08.
 *  Copyright 2008 The Void Software. All rights reserved.
 *
 */

// -----------------------------------------------------------------------------
//	Headers:
// -----------------------------------------------------------------------------

#include "CppBlock.h"


CppBlock::CppBlock( CodeBlockProgressDelegate* dele )
{
	mProgressDelegate = dele;
}


// -----------------------------------------------------------------------------

void	CppBlock::generate_binary_operator_start( const std::string& opName )
{
	mBodyCode << "((";
}


void	CppBlock::generate_binary_operator_middle( const std::string& opName )
{
	mBodyCode << ") " << opName << " (";
}


void	CppBlock::generate_binary_operator_end( const std::string& opName )
{
	mBodyCode << "))";
}


// -----------------------------------------------------------------------------

void	CppBlock::generate_binary_operator_cmd_start( const std::string& opName )
{
	mBodyCode << "	((";
}


void	CppBlock::generate_binary_operator_cmd_middle( const std::string& opName )
{
	mBodyCode << ") " << opName << " (";
}


void	CppBlock::generate_binary_operator_cmd_end( const std::string& opName )
{
	mBodyCode << "));\n";
}


// -----------------------------------------------------------------------------

void	CppBlock::generate_return_statement_start()
{
	mBodyCode << "	return( ";
}


void	CppBlock::generate_return_statement_end()
{
	mBodyCode << " );\n";
}


// -----------------------------------------------------------------------------

void	CppBlock::generate_function_call_start( const std::string& inFcnName )
{
	mBodyCode << inFcnName << "( ";
}

void	CppBlock::generate_function_call_param_start( const std::string& inFcnName )
{
	mBodyCode << "(";
}

void	CppBlock::generate_function_call_param_delim( const std::string& inFcnName )
{
	mBodyCode << "), (";
}


void	CppBlock::generate_function_call_param_end( const std::string& inFcnName )
{
	mBodyCode << ")";
}

void	CppBlock::generate_function_call_end( const std::string& inFcnName )
{
	mBodyCode << " )";
}


// -----------------------------------------------------------------------------

void	CppBlock::generate_command_call_start( const std::string& inFcnName )
{
	mBodyCode << "	" << inFcnName << "( ";
}

void	CppBlock::generate_command_call_param_start( const std::string& inFcnName )
{
	mBodyCode << "(";
}

void	CppBlock::generate_command_call_param_delim( const std::string& inFcnName )
{
	mBodyCode << "), (";
}


void	CppBlock::generate_command_call_param_end( const std::string& inFcnName )
{
	mBodyCode << ")";
}

void	CppBlock::generate_command_call_end( const std::string& inFcnName )
{
	mBodyCode << " );\n";
}


// -----------------------------------------------------------------------------

void	CppBlock::generate_function_definition_start( const std::string& inFcnName )
{
	mHeaderCode << "vcy_variant_t	" << inFcnName << "( vcy_variant_t inParamList );\n";
	mBodyCode << "vcy_variant_t	" << inFcnName << "( vcy_variant_t inParamList )\n{\n";
}


void	CppBlock::generate_function_definition_end( const std::string& inFcnName )
{
	mBodyCode << "}\n\n";
}


// -----------------------------------------------------------------------------

void	CppBlock::declare_local_var( const std::string& inVarName )
{
	mBodyCode << "	vcy_variant_t	" << inVarName << ";\n";
	mBodyCode << "	vcy_alloc_empty( &" << inVarName << " );\n";
}



// -----------------------------------------------------------------------------

void	CppBlock::generate_while_loop_start()
{
	mBodyCode << "	while( ";
}


void	CppBlock::generate_while_loop_middle()
{
	mBodyCode << " )\n	{\n";
}


void	CppBlock::generate_while_loop_end()
{
	mBodyCode << "	}\n";
}


// -----------------------------------------------------------------------------

void	CppBlock::generate_string( const std::string s )
{
	mBodyCode << "CVariant( \"";
	int	x = 0, count = s.size();
	for( x = 0; x < count; x++ )
	{
		switch( s[x] )
		{
			case '\"':
			case '\r':
			case '\n':
			case '\t':
				mBodyCode << "\\" << s[x];
				break;
			
			default:
				mBodyCode << s[x];
				break;
		}
	}
	mBodyCode << "\" )";
}


// -----------------------------------------------------------------------------

void	CppBlock::dump()
{
	std::cout << "#include \"vcy_lib.c\"\n\n";
	std::cout << mIncludesCode.str() << "\n\n";
	std::cout << mHeaderCode.str() << "\n\n";
	std::cout << mBodyCode.str() << "\n\n";
}

