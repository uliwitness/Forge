//
//  ForgeFramework.c
//  ForgeFramework
//
//  Created by Uli Kusterer on 09.04.11.
//  Copyright 2011 The Void Software. All rights reserved.
//

extern "C" {
#include "ForgeFramework.h"
}

#include "CToken.h"
#include "CParser.h"
#include "CParseTree.h"
#include "CCodeBlock.h"

using namespace Carlson;


char	gLEOLastErrorString[1024] = { 0 };


extern "C" LEOParseTree*	LEOParseTreeCreateFromUTF8Characters( const char* inCode, size_t codeLength, const char* filename )
{
	CParseTree	*	parseTree = NULL;
	gLEOLastErrorString[0] = 0;
	
	try
	{
		parseTree = new CParseTree;
		CParser				parser;
		std::deque<CToken>	tokens = CToken::TokenListFromText( inCode, codeLength );
		parser.Parse( filename, tokens, *parseTree );
		
		parseTree->Simplify();
	}
	catch( std::exception& err )
	{
		strcpy( gLEOLastErrorString, err.what() );
		if( parseTree )
			delete parseTree;
		parseTree = NULL;
	}
	catch( ... )
	{
		strcpy( gLEOLastErrorString, "Unknown error." );
		if( parseTree )
			delete parseTree;
		parseTree = NULL;
	}
	
	return (LEOParseTree*)parseTree;
}


extern "C" void		LEOCleanUpParseTree( LEOParseTree* inTree )
{
	try
	{
		delete (CParseTree*)inTree;
	}
	catch( std::exception& err )
	{
		printf( "Internal error in LEOFreeParseTree: \"%s\".\n", err.what() );
	}
	catch( ... )
	{
		printf( "Internal error in LEOFreeParseTree.\n" );
	}
}


extern "C" const char*	LEOParserGetLastErrorMessage()
{
	if( gLEOLastErrorString[0] == 0 )
		return NULL;
	return gLEOLastErrorString;
}


extern "C" void		LEOScriptCompileAndAddParseTree( LEOScript* inScript, LEOContextGroup* inGroup, LEOParseTree* inTree )
{
	gLEOLastErrorString[0] = 0;
	
	try
	{
		CCodeBlock			block( inGroup, inScript );
		
		((CParseTree*)inTree)->GenerateCode( &block );
	}
	catch( std::exception& err )
	{
		strcpy( gLEOLastErrorString, err.what() );
	}
	catch( ... )
	{
		strcpy( gLEOLastErrorString, "Unknown error." );
	}
}