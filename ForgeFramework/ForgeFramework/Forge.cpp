//
//  Forge.cpp
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


#include <iostream>

using namespace Carlson;


extern "C" void LEOInitializeNodeTransformationsIfNeeded( void );


extern "C" void LEOInitializeNodeTransformationsIfNeeded( void )
{
	static bool	sAlreadyInitializedThem = false;
	if( !sAlreadyInitializedThem )
	{
		CConcatOperatorNodeTransformation::Initialize();
		CConcatSpaceOperatorNodeTransformation::Initialize();
		CChunkPropertyNodeTransformation::Initialize();
		CChunkPropertyPutNodeTransformation::Initialize();
		
		sAlreadyInitializedThem = true;
	}
}


char	gLEOLastErrorString[1024] = { 0 };
size_t	gLEOLastErrorOffset = SIZE_T_MAX;
size_t	gLEOLastErrorLineNum = SIZE_T_MAX;


extern "C" LEOParseTree*	LEOParseTreeCreateFromUTF8Characters( const char* inCode, size_t codeLength, uint16_t inFileID )
{
	LEOInitializeNodeTransformationsIfNeeded();
	
	CParseTree	*	parseTree = NULL;
	gLEOLastErrorString[0] = 0;
	gLEOLastErrorOffset = SIZE_T_MAX;
	gLEOLastErrorLineNum = SIZE_T_MAX;
	
	try
	{
		parseTree = new CParseTree;
		CParser				parser;
		std::deque<CToken>	tokens = CToken::TokenListFromText( inCode, codeLength );
		parser.Parse( LEOFileNameForFileID( inFileID ), tokens, *parseTree );
		
		#if 0
		parseTree->DebugPrint( std::cout, 0 );
		#endif
		
		parseTree->Simplify();
		
		#if 0
		parseTree->DebugPrint( std::cout, 0 );
		#endif
	}
	catch( CForgeParseError& ferr )
	{
		strcpy( gLEOLastErrorString, ferr.what() );
		gLEOLastErrorLineNum = ferr.GetLineNum();
		gLEOLastErrorOffset = ferr.GetOffset();
		#if 0
		parseTree->DebugPrint( std::cout, 0 );
		#endif
		if( parseTree )
			delete parseTree;
		parseTree = NULL;
	}
	catch( std::exception& err )
	{
		strcpy( gLEOLastErrorString, err.what() );
		#if 0
		parseTree->DebugPrint( std::cout, 0 );
		#endif
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
	gLEOLastErrorOffset = SIZE_T_MAX;
	gLEOLastErrorLineNum = SIZE_T_MAX;
	
	try
	{
		parseTree = new CParseTree;
		CParser				parser;
		std::deque<CToken>	tokens = CToken::TokenListFromText( inCode, codeLength );
		parser.ParseCommandOrExpression( LEOFileNameForFileID( inFileID ), tokens, *parseTree );

		parseTree->Simplify();
		
		#if 0
		parseTree->DebugPrint( std::cout, 0 );
		#endif
	}
	catch( CForgeParseError& ferr )
	{
		strcpy( gLEOLastErrorString, ferr.what() );
		gLEOLastErrorLineNum = ferr.GetLineNum();
		gLEOLastErrorOffset = ferr.GetOffset();
		if( parseTree )
			delete parseTree;
		parseTree = NULL;
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


extern "C" size_t	LEOParserGetLastErrorLineNum()
{
	return gLEOLastErrorLineNum;
}


extern "C" size_t	LEOParserGetLastErrorOffset()
{
	return gLEOLastErrorOffset;
}


extern "C" void		LEOScriptCompileAndAddParseTree( LEOScript* inScript, LEOContextGroup* inGroup, LEOParseTree* inTree, uint16_t inFileID )
{
	LEOInitializeNodeTransformationsIfNeeded();
	
	gLEOLastErrorString[0] = 0;
	gLEOLastErrorOffset = SIZE_T_MAX;
	gLEOLastErrorLineNum = SIZE_T_MAX;
	
	try
	{
		CCodeBlock			block( inGroup, inScript, inFileID );
		
		#if 0
		((CParseTree*)inTree)->DebugPrint( std::cout, 0 );
		#endif
		
		((CParseTree*)inTree)->Simplify();
		
		#if 0
		((CParseTree*)inTree)->DebugPrint( std::cout, 0 );
		#endif
		
		((CParseTree*)inTree)->GenerateCode( &block );
		
		#if 0
		((CParseTree*)inTree)->DebugPrint( std::cout, 0 );
		LEODebugPrintScript( inGroup, inScript );
		#endif
	}
	catch( CForgeParseError& ferr )
	{
		strcpy( gLEOLastErrorString, ferr.what() );
		gLEOLastErrorLineNum = ferr.GetLineNum();
		gLEOLastErrorOffset = ferr.GetOffset();
		
		#if 0
		((CParseTree*)inTree)->DebugPrint( std::cout, 0 );
		#endif
	}
	catch( std::exception& err )
	{
		strcpy( gLEOLastErrorString, err.what() );
		
		#if 0
		((CParseTree*)inTree)->DebugPrint( std::cout, 0 );
		#endif
	}
	catch( ... )
	{
		strcpy( gLEOLastErrorString, "Unknown error." );
	}
}


extern "C" void	LEOAddBuiltInFunctionsAndOffsetInstructions( struct TBuiltInFunctionEntry* inEntries, size_t firstGlobalPropertyInstruction )
{
	gLEOLastErrorString[0] = 0;
	gLEOLastErrorOffset = SIZE_T_MAX;
	gLEOLastErrorLineNum = SIZE_T_MAX;
	
	try
	{
		CParser::CParser::AddBuiltInFunctionsAndOffsetInstructions( inEntries, firstGlobalPropertyInstruction );
	}
	catch( CForgeParseError& ferr )
	{
		strcpy( gLEOLastErrorString, ferr.what() );
		gLEOLastErrorLineNum = ferr.GetLineNum();
		gLEOLastErrorOffset = ferr.GetOffset();
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
	gLEOLastErrorOffset = SIZE_T_MAX;
	gLEOLastErrorLineNum = SIZE_T_MAX;
	
	try
	{
		CParser::AddGlobalPropertiesAndOffsetInstructions( inEntries, firstGlobalPropertyInstruction );
	}
	catch( CForgeParseError& ferr )
	{
		strcpy( gLEOLastErrorString, ferr.what() );
		gLEOLastErrorLineNum = ferr.GetLineNum();
		gLEOLastErrorOffset = ferr.GetOffset();
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
	gLEOLastErrorOffset = SIZE_T_MAX;
	gLEOLastErrorLineNum = SIZE_T_MAX;
	
	try
	{
		CParser::AddHostCommandsAndOffsetInstructions( inEntries, firstHostCommandInstruction );
	}
	catch( CForgeParseError& ferr )
	{
		strcpy( gLEOLastErrorString, ferr.what() );
		gLEOLastErrorLineNum = ferr.GetLineNum();
		gLEOLastErrorOffset = ferr.GetOffset();
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
	gLEOLastErrorOffset = SIZE_T_MAX;
	gLEOLastErrorLineNum = SIZE_T_MAX;
	
	try
	{
		CParser::AddHostFunctionsAndOffsetInstructions( inEntries, firstHostFunctionInstruction );
	}
	catch( CForgeParseError& ferr )
	{
		strcpy( gLEOLastErrorString, ferr.what() );
		gLEOLastErrorLineNum = ferr.GetLineNum();
		gLEOLastErrorOffset = ferr.GetOffset();
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


extern "C" void	LEOLoadNativeHeadersFromFile( const char* filepath )
{
	gLEOLastErrorString[0] = 0;
	gLEOLastErrorOffset = SIZE_T_MAX;
	gLEOLastErrorLineNum = SIZE_T_MAX;
	
	try
	{
		CParser::LoadNativeHeadersFromFile( filepath );
	}
	catch( CForgeParseError& ferr )
	{
		strcpy( gLEOLastErrorString, ferr.what() );
		gLEOLastErrorLineNum = ferr.GetLineNum();
		gLEOLastErrorOffset = ferr.GetOffset();
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


extern "C" void	LEOSetFirstNativeCallCallback( LEOFirstNativeCallCallbackPtr inCallback )
{
	CParser::SetFirstNativeCallCallback( inCallback );
}

