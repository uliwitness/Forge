#include <iostream>
#include <stdexcept>
#include "CToken.h"
#include "CParser.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include "CParseTree.h"
#include "CCodeBlock.h"
extern "C" {
#include "Forge.h"
#include "LEOScript.h"
#include "LEOContextGroup.h"
#include "LEORemoteDebugger.h"
#include "LEOMsgCommandsGeneric.h"
#include "LEOFileInstructionsGeneric.h"
#include "LEOWebPageInstructionsGeneric.h"
#include "LEOInterpreter.h"
}
#include "CConcatOperatorNodeTransformation.h"
#include "CConcatSpaceOperatorNodeTransformation.h"
#include "CChunkPropertyNodeTransformation.h"

#include <unistd.h>
#include "fake_filesystem.hpp"	// until <filesystem> ships for Xcode's clang.
using namespace fake;


using namespace Carlson;


struct TBuiltInVariableEntry	gBuiltInVariables[] =
{
	{ EPageIdentifier, "page", "page", true },
	{ ELastIdentifier_Sentinel, nullptr, nullptr }
};


struct ForgeToolOptions
{
	bool			debuggerOn = false;
	bool			runCode = true;
	bool			printInstructions = false;
	bool			printTokens = false;
	bool			printParseTree = false;
	bool			printIndented = false;
	bool			verbose = false;
	bool			doOptimize = true;
	bool			printresult = false;
	bool			webPageEmbedMode = false;
	const char*		debuggerHost = NULL;
	const char*		messageName = nullptr;
	int				argc;
	char * const *	argv;
	int				fnameIdx = 0;
};


int	ProcessOneScriptFile( const std::string& inFilePathString, const ForgeToolOptions& toolOptions );


static char*	GetFileContents( const char* fname )
{
	// Open script to run:
	FILE*	theFile = fopen( fname, "r" );
	if( !theFile )
	{
		printf("ERROR: Can't open file \"%s\".\n", fname);
		return NULL;
	}
	
	// Find out file length:
	fseek( theFile, 0, SEEK_END );
	long	len = ftell( theFile ),
			readbytes;
	char*	codeStr = (char*) calloc( len +1, sizeof(char) );
	
	// Rewind and read in whole file:
	fseek( theFile, 0, SEEK_SET );
	readbytes = fread( codeStr, 1, len, theFile );
	if( readbytes != len )
	{
		free( codeStr );
		fclose( theFile );
		printf("ERROR: Couldn't read from file \"%s\" (%ld bytes read).\n",fname,readbytes);
		return NULL;
	}
	codeStr[len] = 0;	// Terminate string.
	fclose( theFile );
	
	return codeStr;
}


int main( int argc, char * const argv[] )
{
	ForgeToolOptions toolOptions;
	toolOptions.argc = argc;
	toolOptions.argv = argv;
	
	bool		fnameIsFolder = false;
	for( int x = 1; x < argc; )
	{
		if( argv[x][0] == '-' )
		{
			if( strcmp( argv[x], "--debug" ) == 0 )
			{
				toolOptions.debuggerOn = true;
				if( (argc -1) < (x +1) )	// No parameter after debug option?
				{
					std::cerr << "Error: Expected host name after --debug option." << std::endl;
					return 6;
				}
				toolOptions.debuggerHost = argv[x+1];
				x++;
			}
			else if( strcmp( argv[x], "--dontrun" ) == 0 )
			{
				toolOptions.runCode = false;
			}
			else if( strcmp( argv[x], "--printinstructions" ) == 0 )
			{
				toolOptions.printInstructions = true;
			}
			else if( strcmp( argv[x], "--printtokens" ) == 0 )
			{
				toolOptions.printTokens = true;
			}
			else if( strcmp( argv[x], "--printparsetree" ) == 0 )
			{
				toolOptions.printParseTree = true;
			}
			else if( strcmp( argv[x], "--printindented" ) == 0 )
			{
				toolOptions.printIndented = true;
			}
			else if( strcmp( argv[x], "--verbose" ) == 0 )
			{
				toolOptions.verbose = true;
			}
			else if( strcmp( argv[x], "--printresult" ) == 0 )
			{
				toolOptions.printresult = true;
			}
			else if( strcmp( argv[x], "--message" ) == 0 )
			{
				if( (argc -1) < (x +1) )	// No parameter after message option?
				{
					std::cerr << "Error: Expected handler name after --message option." << std::endl;
					return 7;
				}
				toolOptions.messageName = argv[x+1];
				x++;
			}
			else if( strcmp( argv[x], "--dont-optimize" ) == 0 )
				toolOptions.doOptimize = false;
			else if( strcmp( argv[x], "--folder" ) == 0 )
				fnameIsFolder = true;
			else if( strcmp( argv[x], "--webpage" ) == 0 )
				toolOptions.webPageEmbedMode = true;
			else
			{
				std::cerr << "Unknown option \"" << argv[x] << "\"." << std::endl;
				return 5;
			}
		}
		else	// end of options, file name.
		{
			toolOptions.fnameIdx = x;
			break;
		}
		
		x++;
	}
	
	if( toolOptions.messageName == nullptr )
	{
		if( toolOptions.webPageEmbedMode )
			toolOptions.messageName = "::generatepage";
		else
			toolOptions.messageName = "startUp";
	}
	
	LEOInitInstructionArray();
	
	if( toolOptions.doOptimize )
	{
		CConcatOperatorNodeTransformation::Initialize();
		CConcatSpaceOperatorNodeTransformation::Initialize();
		CChunkPropertyNodeTransformation::Initialize();
	}
	
	LEOAddInstructionsToInstructionArray( gMsgInstructions, LEO_NUMBER_OF_MSG_INSTRUCTIONS, &kFirstMsgInstruction );
	LEOAddHostCommandsAndOffsetInstructions( gMsgCommands, kFirstMsgInstruction );
	
	LEOAddInstructionsToInstructionArray( gFileInstructions, LEO_NUMBER_OF_FILE_INSTRUCTIONS, &kFirstFileInstruction );
	LEOAddHostCommandsAndOffsetInstructions( gFileCommands, kFirstFileInstruction );
	
	LEOAddInstructionsToInstructionArray( gPropertyInstructions, LEO_NUMBER_OF_PROPERTY_INSTRUCTIONS, &kFirstPropertyInstruction );
	LEOAddHostFunctionsAndOffsetInstructions( gPropertyHostFunctions, kFirstPropertyInstruction );
	LEOAddOperatorsAndOffsetInstructions( gPropertyOperators, kFirstPropertyInstruction );
	
	LEOAddInstructionsToInstructionArray( gWebPageInstructions, LEO_NUMBER_OF_WEB_PAGE_INSTRUCTIONS, &kFirstWebPageInstruction );
	LEOAddBuiltInFunctionsAndOffsetInstructions( gWebPageBuiltInFunctions, kFirstWebPageInstruction );
	
	if( toolOptions.webPageEmbedMode )
	{
		LEOAddBuiltInVariables( gBuiltInVariables );
	}

	char*	filename = (toolOptions.fnameIdx > 0) ? argv[toolOptions.fnameIdx] : NULL;
	if( filename && fnameIsFolder )
	{
		chdir(filename);
		
		filesystem::directory_iterator	currFile(filename);
		for( ; currFile != filesystem::directory_iterator(); ++currFile )
		{
			filesystem::path	fpath( (*currFile).path() );
			std::string			fname( fpath.filename().string() );
			if( fname.length() > 0 && fname[0] == '.' )
				continue;
			if( fname.rfind(".hc") != fname.length() -3 )
				continue;
			
			int errNum = ProcessOneScriptFile( (*currFile).path().string(), toolOptions );
			if( errNum != EXIT_SUCCESS )
				return errNum;
		}
	}
	else if( filename )
	{
		return ProcessOneScriptFile( filename, toolOptions );
	}
	else
	{
		std::cerr << "error: Last parameter should be name of script to compile." << std::endl;
		return 2;
	}
	
	return 0;
}


int	ProcessOneScriptFile( const std::string& inFilePathString, const ForgeToolOptions& toolOptions )
{
	// Do actual work:
	char*				code = GetFileContents( inFilePathString.c_str() );
	std::deque<CToken>	tokens;
	CParser				parser;
	parser.SetWebPageEmbedMode(toolOptions.webPageEmbedMode);
	
	if( !code )
	{
		std::cerr << "error: Couldn't find file \"" << inFilePathString << "\"." << std::endl;
		return 2;
	}
		
	try
	{
		CParseTree				parseTree;
		
		if( toolOptions.verbose )
			std::cout << "Tokenizing file \"" << inFilePathString << "\"..." << std::endl;
		tokens = CTokenizer::TokenListFromText( code, strlen(code), toolOptions.webPageEmbedMode );
		if( toolOptions.printTokens )
		{
			for( std::deque<CToken>::iterator currToken = tokens.begin(); currToken != tokens.end(); currToken++ )
				std::cout << "Token: " << currToken->GetDescription() << std::endl;
		}
		
		if( toolOptions.verbose )
			std::cout << "Parsing file \"" << inFilePathString << "\"..." << std::endl;
		parser.Parse( inFilePathString.c_str(), tokens, parseTree, code );
		
		if( toolOptions.printParseTree )
			parseTree.DebugPrint( std::cout, 1 );
		
		if( toolOptions.printIndented )
		{
			if( toolOptions.verbose )
				std::cout << "Indenting file \"" << inFilePathString << "\"..." << std::endl;
			LEODisplayInfoTable*	lit = LEODisplayInfoTableCreateForParseTree( (LEOParseTree*) &parseTree );
			char*	theText = NULL;
			size_t	theLength = 0;
			LEODisplayInfoTableApplyToText( lit, code, strlen(code), &theText, &theLength, NULL, NULL );
			std::cout << theText << std::endl;
			free( theText );
			const char*		currName = "";
			size_t			currLine = 0;
			bool			isCommand = false;
			size_t			x = 0;
			while( true )
			{
				bool isEnd = false;
				LEODisplayInfoTableGetHandlerInfoAtIndex( lit, x++, &currName, &currLine, &isEnd, &isCommand );
				if( !currName )
					break;
				if( !isEnd )
					std::cout << (isCommand ? "[C] " : "[F] ") << currName << " (Line " << currLine << ")" << std::endl;
			}
			LEOCleanUpDisplayInfoTable( lit );
		}
		
		uint16_t 			fileID = LEOFileIDForFileName(inFilePathString.c_str());
		LEOScript		*	script = LEOScriptCreateForOwner( 0, 0, NULL );
		LEOContextGroup	*	group = LEOContextGroupCreate( NULL, NULL );
		CCodeBlock			block( group, script, fileID );
		
		parseTree.Simplify();
		parseTree.GenerateCode( &block );
		
		if( toolOptions.printInstructions )
			LEODebugPrintScript( group, script );
		
		if( toolOptions.runCode )
		{
			if( toolOptions.verbose )
				printf( "\nRun the code:\n" );
			
			LEOHandlerID	handlerID = LEOContextGroupHandlerIDForHandlerName( group, toolOptions.messageName );
			LEOHandler*		theHandler = LEOScriptFindCommandHandlerWithID( script, handlerID );
			
			if( theHandler == NULL )
			{
				printf( "ERROR: Could not find handler to run!" );
			}
			else
			{
				LEOContext	*	ctx = LEOContextCreate( group, NULL, NULL );
				
				if( toolOptions.debuggerOn )
				{
					if( LEOInitRemoteDebugger( toolOptions.debuggerHost ) )
					{
						ctx->preInstructionProc = LEORemoteDebuggerPreInstructionProc;	// Activate the debugger.
						LEORemoteDebuggerAddBreakpoint( theHandler->instructions );		// Set a breakpoint on the first instruction, so we can step through everything with the debugger.
						LEORemoteDebuggerAddFile( code, fileID, script );
					}
				}
				
				LEOPushUnsetValueOnStack( ctx );	// Reserve space for return value.
				
				// Push params on stack in reverse order:
				LEOInteger	paramCount = 0;
				
				for( int x = (toolOptions.argc -1); x > toolOptions.fnameIdx; x-- )
				{
					LEOPushStringValueOnStack( ctx, toolOptions.argv[x], strlen(toolOptions.argv[x]) );
					paramCount++;
				}

				LEOPushIntegerOnStack( ctx, paramCount, kLEOUnitNone );	// Parameter count.
				
				LEOContextPushHandlerScriptReturnAddressAndBasePtr( ctx, theHandler, script, NULL, NULL );	// NULL return address is same as exit to top. basePtr is set to NULL as well on exit.
				LEORunInContext( theHandler->instructions, ctx );
				if( ctx->errMsg[0] != 0 )
					printf("ERROR: %s\n", ctx->errMsg );
				if( toolOptions.printresult )
				{
					// Remove all parameters from the stack:
					LEOCleanUpStackToPtr( ctx, ctx->stackEndPtr -paramCount -1 );
					
					if( ctx->stack == ctx->stackEndPtr )
						printf("WARNING: No result left on stack. Bad code generated?\n");
					long	numResults = ctx->stackEndPtr -ctx->stack;
					if( numResults > 1 )
					{
						printf("WARNING: %ld results left on stack, expected 1. Bad code generated?\n", numResults);
						LEODebugPrintContext( ctx );
					}
					else if( numResults > 0 )
					{
						if( LEOGetValueIsUnset( ctx->stack, ctx ) )
							printf("Result: --\n");
						else
						{
							char		str[256] = {};
							const char* theResultAsString = LEOGetValueAsString( ctx->stack, str, sizeof(str), ctx );
							printf("Result: \"%s\"\n", theResultAsString);
						}
					}
				}
				
				LEOContextRelease( ctx );
			}
		}
		
		LEOScriptRelease( script );
		LEOContextGroupRelease( group );
	}
	catch( std::exception& err )
	{
		std::cerr << err.what() << std::endl;
		return 3;
	}
	
	if( toolOptions.verbose )
		std::cout << "Finished successfully." << std::endl;
	
    return EXIT_SUCCESS;
}
