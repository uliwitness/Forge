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


char							gLEOLastErrorString[1024] = { 0 };
size_t							gLEOLastErrorOffset = SIZE_T_MAX;
size_t							gLEOLastErrorLineNum = SIZE_T_MAX;
std::vector<CMessageEntry>		gMessages;
std::vector<CHandlerNotesEntry>	gHandlerNotes;


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
		parser.Parse( LEOFileNameForFileID( inFileID ), tokens, *parseTree, inCode );
		
		gMessages.assign( parser.GetMessages().begin(), parser.GetMessages().end() );
		gHandlerNotes.assign( parser.GetHandlerNotes().begin(), parser.GetHandlerNotes().end() );
		
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
	CLineNumEntry( size_t ln = 0, int ic = 0, const std::string& hn = "", bool isc = false ) : mLineNum(ln), mIndentChange(ic),mHandlerName(hn), mIsCommand(isc) {};
	size_t		mLineNum;		// Line number this indent change is at.
	int			mIndentChange;	// Amount of inset (+ive) or outset (-ive) to apply to this line and all following it.
	std::string	mHandlerName;	// If this is the start of a handler, its name, otherwise an empty string.
	bool		mIsCommand;		// If mHandlerName is not empty, whether this is a command or function message handler.
};


extern "C" LEODisplayInfoTable*	LEODisplayInfoTableCreateForParseTree( LEOParseTree* inTree )
{
	std::vector<CLineNumEntry>*	lineIndentTable = new std::vector<CLineNumEntry>;
	((CParseTree*)inTree)->Visit( [lineIndentTable]( CNode* currNode )
	{
		CFunctionDefinitionNode*	handler = dynamic_cast<CFunctionDefinitionNode*>(currNode);
		if( handler )
		{
			if( handler->GetLineNum() > 0 )
				lineIndentTable->push_back( CLineNumEntry(handler->GetLineNum(), 0, handler->GetUserHandlerName(), handler->IsCommand() ) );
			if( handler->GetCommandsLineNum() > 0 && handler->GetCommandsLineNum() != handler->GetEndLineNum() )
				lineIndentTable->push_back( CLineNumEntry(handler->GetCommandsLineNum(), 1 ) );	// Don't indent an empty handler's last line.
			if( handler->GetEndLineNum() > 0 )
				lineIndentTable->push_back( CLineNumEntry(handler->GetEndLineNum(), -1, "", false) );
		}
		CWhileLoopNode*	loop = dynamic_cast<CWhileLoopNode*>(currNode);
		if( loop )
		{
			if( loop->GetLineNum() > 0 ) lineIndentTable->push_back( CLineNumEntry(loop->GetLineNum(), 1, "", false) );
			if( loop->GetCommandsLineNum() > 0 && loop->GetCommandsLineNum() != loop->GetEndRepeatLineNum() )
			{
				if( loop->GetCommandsLineNum() > 0 ) lineIndentTable->push_back( CLineNumEntry(loop->GetCommandsLineNum(), 1, "", false) );
				if( loop->GetEndRepeatLineNum() > 0 ) lineIndentTable->push_back( CLineNumEntry(loop->GetEndRepeatLineNum(), -1, "", false) );
			}
		}
		CIfNode*	conditional = dynamic_cast<CIfNode*>(currNode);
		if( conditional )
		{
			if( conditional->GetIfCommandsLineNum() > 0 && conditional->GetThenLineNum() != conditional->GetIfCommandsLineNum() && conditional->GetCommandsCount() != 0 )
				lineIndentTable->push_back( CLineNumEntry(conditional->GetIfCommandsLineNum(), 1, "", false) );
			if( conditional->GetElseLineNum() > 0 // Have an else block
				&& conditional->GetElseLineNum() != conditional->GetThenLineNum()	// Not on same line as the "then".
				&& conditional->GetLineNum() != conditional->GetIfCommandsLineNum()
				&& conditional->GetCommandsCount() != 0 )	// There wasn't a one-line-if before this else.
				lineIndentTable->push_back( CLineNumEntry(conditional->GetElseLineNum(), -1, "", false) );
			if( conditional->GetElseCommandsLineNum() > 0	// We have an else.
				&& conditional->GetElseLineNum() != conditional->GetElseCommandsLineNum()	// It's not a one-line else
				&& conditional->GetElseCommandsLineNum() != conditional->GetEndIfLineNum() )	// It's not an empty else immediately followed by an "end if".
				lineIndentTable->push_back( CLineNumEntry(conditional->GetElseCommandsLineNum(), 1, "", false) );
			if( conditional->GetEndIfLineNum() > 0
				&& conditional->GetElseCommandsLineNum() != conditional->GetEndIfLineNum() )
				lineIndentTable->push_back( CLineNumEntry(conditional->GetEndIfLineNum(), -1, "", false) );
		}
		CDownloadCommandNode*	download = dynamic_cast<CDownloadCommandNode*>(currNode);
		if( download )
		{
			bool	didProgressIndent = false;
			if( download->GetProgressCommandsLineNum() != 0	// Have progress block.
				&& download->GetProgressCommandsLineNum() != download->GetProgressLineNum() ) // It's not on the same line as the 'for each chunk'?
			{
				lineIndentTable->push_back( CLineNumEntry(download->GetProgressCommandsLineNum(), 1) );
				didProgressIndent = true;
			}
			if( download->GetCompletionLineNum() != 0	// Have completion block.
				&& download->GetCompletionLineNum() != download->GetLineNum()	// It's not on the first line.
				&& download->GetCompletionLineNum() > download->GetProgressLineNum()	// There was a progress block above it, but not on same line with it.
				&& didProgressIndent )	// And it was multi-line.
			{
				lineIndentTable->push_back( CLineNumEntry(download->GetCompletionLineNum(), -1) );
			}
			if( download->GetCompletionCommandsLineNum() != 0	// Have completion block.
				&& download->GetCompletionCommandsLineNum() != download->GetCompletionLineNum() )	// Not on same line as 'when done'?
			{
				lineIndentTable->push_back( CLineNumEntry(download->GetCompletionCommandsLineNum(), 1) );
			}
			if( download->GetEndDownloadLineNum() != 0	// Didn't error b/c there's no end?
				&& download->GetEndDownloadLineNum() != download->GetCompletionLineNum()	// We didn't have a one-line completion block
				&& download->GetEndDownloadLineNum() != download->GetProgressLineNum() )	// Nor one-line progress block.
			{
				lineIndentTable->push_back( CLineNumEntry(download->GetEndDownloadLineNum(), -1) );
			}
		}
	});
	
	#if 0
	printf("====================\n");
	for( auto currEntry : *lineIndentTable )
	{
		printf("line %zu indent %d\n", currEntry.mLineNum, currEntry.mIndentChange );
	}
	#endif
	
	return (LEODisplayInfoTable*)lineIndentTable;
}


static int	GetIndentChangeForLine( LEODisplayInfoTable* inTable, size_t inLineNum )
{
	for( auto currEntry : *(std::vector<CLineNumEntry>*)inTable )
	{
		if( currEntry.mLineNum == inLineNum )
			return currEntry.mIndentChange;
	}
	
	return 0;
}


extern "C" void	LEODisplayInfoTableApplyToText( LEODisplayInfoTable* inTable, const char*	code, size_t codeLen, char** outText, size_t *outLength, size_t *ioCursorPosition, size_t *ioCursorEndPosition )
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
			if( startOfLine )	// the previous line was empty?
			{
				// Don't miss any indent changes in it:
				int	idChange = GetIndentChangeForLine( inTable, lineNum );
				if( idChange > 0 || ((unsigned)abs(idChange)) <= currIndent )
					currIndent += idChange;
				currText.append( currIndent, '\t' );
			}
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
			int	idChange = GetIndentChangeForLine( inTable, lineNum );
			if( idChange > 0 || ((unsigned)abs(idChange)) <= currIndent )
				currIndent += idChange;
			if( ioCursorPosition && *ioCursorPosition >= x )
			{
				long		diff = currIndent -indentCharsSkipped;
				*ioCursorPosition += diff;
			}
			if( ioCursorEndPosition && *ioCursorEndPosition >= x )
			{
				long		diff = currIndent -indentCharsSkipped;
				*ioCursorEndPosition += diff;
			}
			currText.append( currIndent, '\t' );
		}
		
		currText.append( 1, code[x] );
	}
	
	*outLength = currText.size();
	*outText = (char*) malloc( (*outLength) +1 );
	memmove( *outText, currText.c_str(), (*outLength) +1 );
}


extern "C" void LEODisplayInfoTableGetHandlerInfoAtIndex( LEODisplayInfoTable* inTable, size_t inIndex, const char** outName, size_t *outLine, bool *outIsCommand )
{
	size_t			x = 0;
	for( CLineNumEntry& currEntry : *(std::vector<CLineNumEntry>*)inTable )
	{
		if( currEntry.mHandlerName.length() > 0 )
		{
			if( inIndex == x )
			{
				*outName = currEntry.mHandlerName.c_str();
				*outLine = currEntry.mLineNum;
				*outIsCommand = currEntry.mIsCommand;
				return;
			}
			x++;
		}
	}
	
	*outName = NULL;
	*outLine = 0;
}


extern "C" void		LEOCleanUpDisplayInfoTable( LEODisplayInfoTable* inTable )
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


extern "C" void	LEOParserGetNonFatalErrorMessageAtIndex( size_t inIndex, const char** outErrMsg, size_t *outLineNum, size_t *outOffset )
{
	if( inIndex >= gMessages.size() )
	{
		*outErrMsg = NULL;
		return;
	}
	
	*outErrMsg = gMessages[inIndex].mMessage.c_str();
	*outLineNum = gMessages[inIndex].mLineNum;
	*outOffset = gMessages[inIndex].mOffset;
}


extern "C" void	LEOParserGetHandlerNoteAtIndex( size_t inIndex, const char** outHandlerName, const char** outNote )
{
	if( inIndex >= gHandlerNotes.size() )
	{
		*outHandlerName = NULL;
		return;
	}
	
	*outHandlerName = gHandlerNotes[inIndex].mHandlerName.c_str();
	*outNote = gHandlerNotes[inIndex].mNotes.c_str();
}

