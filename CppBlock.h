/*
 *  CppBlock.h
 *  HyperCompiler
 *
 *  Created by Uli Kusterer on 21.11.08.
 *  Copyright 2008 The Void Software. All rights reserved.
 *
 */

#pragma once

// -----------------------------------------------------------------------------
//	Headers:
// -----------------------------------------------------------------------------

#include "CodeBlock.h"	// We should extract the stuff we need from here into its own header.
#include <sstream>


// -----------------------------------------------------------------------------
//	CppBlock:
// -----------------------------------------------------------------------------

// This class is used for generating code in a more high-level language. For
//	starters, we'll generate C++, but what we mainly need are user-defined stack
//	objects with constructors and destructors, so every language that does that
//	should be easy, and others could probably be made to work by faking that.

class CppBlock
{
public:
	CppBlock( CodeBlockProgressDelegate* dele = NULL );
	
	void	generate_binary_operator_start( const std::string& opName );
	void	generate_binary_operator_middle( const std::string& opName );
	void	generate_binary_operator_end( const std::string& opName );

	void	generate_binary_operator_cmd_start( const std::string& opName );
	void	generate_binary_operator_cmd_middle( const std::string& opName );
	void	generate_binary_operator_cmd_end( const std::string& opName );

	void	generate_return_statement_start();
	void	generate_return_statement_end();
	
	void	generate_function_call_start( const std::string& inFcnName );
	void	generate_function_call_param_start( const std::string& inFcnName );
	void	generate_function_call_param_delim( const std::string& inFcnName );
	void	generate_function_call_param_end( const std::string& inFcnName );
	void	generate_function_call_end( const std::string& inFcnName );
	
	void	generate_command_call_start( const std::string& inFcnName );
	void	generate_command_call_param_start( const std::string& inFcnName );
	void	generate_command_call_param_delim( const std::string& inFcnName );
	void	generate_command_call_param_end( const std::string& inFcnName );
	void	generate_command_call_end( const std::string& inFcnName );
	
	void	generate_function_definition_start( const std::string& inFcnName );
	void	generate_function_definition_end( const std::string& inFcnName );
	
	void	declare_local_var( const std::string& inVarName );
	
	void	generate_while_loop_start();	// start before condition.
	void	generate_while_loop_middle();	// after condition before commands
	void	generate_while_loop_end();		// after commands.
	
	void	generate_int( int num )						{ mHeaderCode << "/* int " << num << " */"; mBodyCode << "CVariant( " << num << " )"; };
	void	generate_float( float num )					{ mHeaderCode << "/* float " << num << " */"; mBodyCode << "CVariant( " << num << " )"; };
	void	generate_bool( bool bl )					{ mHeaderCode << "/* bool " << bl << " */"; mBodyCode << "CVariant( " << bl << " )"; };
	void	generate_string( const std::string s );
	void	generate_var_ref( const std::string s )		{ mBodyCode << s; };
	
	void	dump();
	
protected:
	std::stringstream				mHeaderCode;
	std::stringstream				mIncludesCode;
	std::stringstream				mBodyCode;
	CodeBlockProgressDelegate*		mProgressDelegate;
};