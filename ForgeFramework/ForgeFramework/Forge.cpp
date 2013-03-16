//
//  Forge.c
//  Forge
//
//  Created by Uli Kusterer on 09.04.11.
//  Copyright 2011 The Void Software. All rights reserved.
//

extern "C" {
#include "Forge.h"
#include "ForgeTypes.h"
}

#include "CToken.h"
#include "CParser.h"
#include "CParseTree.h"
#include "CCodeBlock.h"
#include "CChunkPropertyNodeTransformation.h"
#include "CConcatOperatorNodeTransformation.h"
#include "CConcatSpaceOperatorNodeTransformation.h"


using namespace Carlson;




extern "C" void LEOInitializeNodeTransformationsIfNeeded()
{
	static bool	sAlreadyInitializedThem = false;
	if( !sAlreadyInitializedThem )
	{
//		CConcatOperatorNodeTransformation::Initialize();
//		CConcatSpaceOperatorNodeTransformation::Initialize();
		CChunkPropertyNodeTransformation::Initialize();
		
		sAlreadyInitializedThem = true;
	}
}


char	gLEOLastErrorString[1024] = { 0 };


extern "C" LEOParseTree*	LEOParseTreeCreateFromUTF8Characters( const char* inCode, size_t codeLength, uint16_t inFileID )
{
	LEOInitializeNodeTransformationsIfNeeded();
	
	CParseTree	*	parseTree = NULL;
	gLEOLastErrorString[0] = 0;
	
	try
	{
		parseTree = new CParseTree;
		CParser				parser;
		std::deque<CToken>	tokens = CToken::TokenListFromText( inCode, codeLength );
		parser.Parse( LEOFileNameForFileID( inFileID ), tokens, *parseTree );
		
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


extern "C" LEOParseTree*	LEOParseTreeCreateForCommandOrExpressionFromUTF8Characters( const char* inCode, size_t codeLength, uint16_t inFileID )
{
	LEOInitializeNodeTransformationsIfNeeded();
	
	CParseTree	*	parseTree = NULL;
	gLEOLastErrorString[0] = 0;
	
	try
	{
		parseTree = new CParseTree;
		CParser				parser;
		std::deque<CToken>	tokens = CToken::TokenListFromText( inCode, codeLength );
		parser.ParseCommandOrExpression( LEOFileNameForFileID( inFileID ), tokens, *parseTree );

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


extern "C" void		LEOScriptCompileAndAddParseTree( LEOScript* inScript, LEOContextGroup* inGroup, LEOParseTree* inTree, uint16_t inFileID )
{
	LEOInitializeNodeTransformationsIfNeeded();
	
	gLEOLastErrorString[0] = 0;
	
	try
	{
		CCodeBlock			block( inGroup, inScript, inFileID );
		
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


extern "C" void	LEOAddGlobalPropertiesAndOffsetInstructions( struct TGlobalPropertyEntry* inEntries, size_t firstGlobalPropertyInstruction )
{
	gLEOLastErrorString[0] = 0;
	
	try
	{
		CParser::AddGlobalPropertiesAndOffsetInstructions( inEntries, firstGlobalPropertyInstruction );
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


extern "C" void	LEOAddHostCommandsAndOffsetInstructions( struct THostCommandEntry* inEntries, size_t firstHostCommandInstruction )
{
	gLEOLastErrorString[0] = 0;
	
	try
	{
		CParser::AddHostCommandsAndOffsetInstructions( inEntries, firstHostCommandInstruction );
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


extern "C" void	LEOAddHostFunctionsAndOffsetInstructions( struct THostCommandEntry* inEntries, size_t firstHostFunctionInstruction )
{
	gLEOLastErrorString[0] = 0;
	
	try
	{
		CParser::AddHostFunctionsAndOffsetInstructions( inEntries, firstHostFunctionInstruction );
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


