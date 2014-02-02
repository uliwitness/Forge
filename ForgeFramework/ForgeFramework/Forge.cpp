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
#include "CFunctionDefinitionNode.h"
#include "CWhileLoopNode.h"
#include "CIfNode.h"
#include "CDownloadCommandNode.h"


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
		std::deque<CToken>	tokens = CTokenizer::TokenListFromText( inCode, codeLength );
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
		std::deque<CToken>	tokens = CTokenizer::TokenListFromText( inCode, codeLength );
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


struct CLineNumEntry
{
	CLineNumEntry( size_t ln = 0, int ic = 0 )	{ mLineNum = ln; mIndentChange = ic; };
	size_t	mLineNum;
	int		mIndentChange;
};


extern "C" LEOLineIndentTable*	LEOLineIndentTableCreateForParseTree( LEOParseTree* inTree )
{
	std::vector<CLineNumEntry>*	lineIndentTable = new std::vector<CLineNumEntry>;
	((CParseTree*)inTree)->Visit( [lineIndentTable]( CNode* currNode )
	{
		CFunctionDefinitionNode*	handler = dynamic_cast<CFunctionDefinitionNode*>(currNode);
		if( handler )
		{
			if( handler->GetCommandsLineNum() > 0 ) lineIndentTable->push_back( CLineNumEntry(handler->GetCommandsLineNum(), 1) );
			if( handler->GetEndLineNum() > 0 ) lineIndentTable->push_back( CLineNumEntry(handler->GetEndLineNum(), -1) );
		}
		CWhileLoopNode*	loop = dynamic_cast<CWhileLoopNode*>(currNode);
		if( loop )
		{
			if( loop->GetLineNum() > 0 ) lineIndentTable->push_back( CLineNumEntry(loop->GetLineNum(), 1) );
			if( loop->GetCommandsLineNum() > 0 ) lineIndentTable->push_back( CLineNumEntry(loop->GetCommandsLineNum(), 1) );
			if( loop->GetEndRepeatLineNum() > 0 ) lineIndentTable->push_back( CLineNumEntry(loop->GetEndRepeatLineNum(), -1) );
		}
		CIfNode*	conditional = dynamic_cast<CIfNode*>(currNode);
		if( loop )
		{
			if( conditional->GetIfCommandsLineNum() > 0 && conditional->GetThenLineNum() != conditional->GetIfCommandsLineNum() )
				lineIndentTable->push_back( CLineNumEntry(conditional->GetIfCommandsLineNum(), 1) );
			if( conditional->GetElseLineNum() > 0 && conditional->GetElseLineNum() != conditional->GetThenLineNum() )
				lineIndentTable->push_back( CLineNumEntry(conditional->GetElseLineNum(), -1) );
			if( conditional->GetElseCommandsLineNum() > 0 && conditional->GetElseLineNum() != conditional->GetElseCommandsLineNum() )
				lineIndentTable->push_back( CLineNumEntry(conditional->GetElseCommandsLineNum(), 1) );
			if( conditional->GetEndIfLineNum() > 0 )
				lineIndentTable->push_back( CLineNumEntry(conditional->GetEndIfLineNum(), -1) );
		}
		CDownloadCommandNode*	download = dynamic_cast<CDownloadCommandNode*>(currNode);
		if( download )
		{
			
		}
	});
	
	for( auto currEntry : *lineIndentTable )
	{
		printf("line %zu indent %d\n", currEntry.mLineNum, currEntry.mIndentChange );
	}
	
	return (LEOLineIndentTable*)lineIndentTable;
}


static int	GetIndentChangeForLine( LEOLineIndentTable* inTable, size_t inLineNum )
{
	for( auto currEntry : *(std::vector<CLineNumEntry>*)inTable )
	{
		if( currEntry.mLineNum == inLineNum )
			return currEntry.mIndentChange;
	}
	
	return 0;
}


extern "C" void	LEOLineIndentTableApplyToText( LEOLineIndentTable* inTable, const char*	code, size_t codeLen, char** outText, size_t *outLength, size_t *ioCursorPosition )
{
	std::string		currText;
	size_t			lineNum = 1;
	size_t			indentCharsSkipped = 0;
	size_t			currIndent = 0;
	bool			startOfLine = true;
	
	for( size_t x = 0; x < codeLen; x++ )
	{
		if( code[x] == '\n' || code[x] == '\r' )
		{
			currText.append( 1, code[x] );
			lineNum++;
			startOfLine = true;
			indentCharsSkipped = 0;
			continue;
		}
		
		if( startOfLine && (code[x] == '\t' || code[x] == ' ') )	// Skip any old indentation at start of line.
		{
			indentCharsSkipped++;
			continue;
		}
		else if( startOfLine )	// Actual text in line starts? Now get in our indentation!
		{
			startOfLine = false;
			currIndent += GetIndentChangeForLine( inTable, lineNum );
			if( *ioCursorPosition >= x )
				*ioCursorPosition += currIndent -indentCharsSkipped;
			currText.append( currIndent, '\t' );
		}
		
		currText.append( 1, code[x] );
	}
	
	*outLength = currText.size();
	*outText = (char*) malloc( (*outLength) +1 );
	memmove( *outText, currText.c_str(), (*outLength) +1 );
}



extern "C" void		LEOCleanUpLineIndentTable( LEOLineIndentTable* inTable )
{
	delete (std::vector<CLineNumEntry>*)inTable;
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

