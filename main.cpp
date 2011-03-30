#include <iostream>
#include <stdexcept>
#include "CToken.h"
#include "CParser.h"
#include "GetFileContents.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include "CParseTree.h"
#include "CCodeBlock.h"
extern "C" {
#include "LEOScript.h"
#include "LEOContextGroup.h"
#include "LEORemoteDebugger.h"
}


using namespace Carlson;

int main( int argc, char * const argv[] )
{
	bool	debuggerOn = false,
			runCode = true,
			printInstructions = false,
			printTokens = false,
			printParseTree = false,
			verbose = false;
	
	int		fnameIdx = 0, outputFNameIdx = 0;
	for( int x = 1; x < argc; )
	{
		if( argv[x][0] == '-' )
		{
			if( strcmp( argv[x], "--output" ) == 0 )
			{
				x++;
				if( x >= argc )
				{
					std::cerr << "Expected file name after --output." << std::endl;
					return 4;
				}
				outputFNameIdx = x;
			}
			else if( strcmp( argv[x], "--debug" ) == 0 )
			{
				debuggerOn = true;
			}
			else if( strcmp( argv[x], "--dontrun" ) == 0 )
			{
				runCode = false;
			}
			else if( strcmp( argv[x], "--printinstructions" ) == 0 )
			{
				printInstructions = true;
			}
			else if( strcmp( argv[x], "--printtokens" ) == 0 )
			{
				printTokens = true;
			}
			else if( strcmp( argv[x], "--printparsetree" ) == 0 )
			{
				printParseTree = true;
			}
			else if( strcmp( argv[x], "--verbose" ) == 0 )
			{
				verbose = true;
			}
			else
			{
				std::cerr << "Unknown option \"" << argv[x] << "\"." << std::endl;
				return 5;
			}
		}
		else	// end of options, file name.
			fnameIdx = x;
		
		x++;
	}
	
	// Do actual work:
	char*				filename = (fnameIdx > 0) ? argv[fnameIdx] : NULL;
	char*				code = filename ? GetFileContents( filename ) : NULL;
	std::deque<CToken>	tokens;
	CParser				parser;
	
	if( !code )
	{
		if( !filename )
			std::cerr << "error: Last parameter should be name of script to compile." << std::endl;
		else
			std::cerr << "error: Couldn't find file \"" << filename << "\"." << std::endl;
		return 2;
	}
		
	try
	{
		CParseTree				parseTree;
		
		if( verbose )
			std::cout << "Tokenizing file \"" << filename << "\"..." << std::endl;
		tokens = CToken::TokenListFromText( code, strlen(code) );
		if( printTokens )
		{
			for( std::deque<CToken>::iterator currToken = tokens.begin(); currToken != tokens.end(); currToken++ )
				std::cout << "Token: " << currToken->GetDescription() << std::endl;
		}
		
		if( verbose )
			std::cout << "Parsing file \"" << filename << "\"..." << std::endl;
		parser.Parse( filename, tokens, parseTree );
		
		if( printInstructions || runCode || printParseTree )
			LEOInitInstructionArray();
		
		if( printParseTree )
			parseTree.DebugPrint( std::cout, 1 );
		
		LEOScript		*	script = LEOScriptCreateForOwner( 0, 0 );
		LEOContextGroup	*	group = LEOContextGroupCreate();
		CCodeBlock			block( group, script );
		
		parseTree.Simplify();
		parseTree.GenerateCode( &block );
		
		if( printInstructions )
			LEODebugPrintScript( group, script );
		
		if( runCode )
		{
			if( verbose )
				printf( "\nRun the code:\n" );
			
			LEOHandlerID	handlerID = LEOContextGroupHandlerIDForHandlerName( group, "startUp" );
			LEOHandler*		theHandler = LEOScriptFindCommandHandlerWithID( script, handlerID );
			
			LEOContext		ctx;
			LEOInitContext( &ctx, group );
			
			if( debuggerOn )
			{
				if( LEOInitRemoteDebugger( "127.0.0.1" ) )
				{
					ctx.preInstructionProc = LEORemoteDebuggerPreInstructionProc;	// Activate the debugger.
					LEORemoteDebuggerAddBreakpoint( theHandler->instructions );		// Set a breakpoint on the first instruction, so we can step through everything with the debugger.
					LEORemoteDebuggerAddFile( filename, code, script );
				}
			}
			
			LEOPushEmptyValueOnStack( &ctx );	// Reserve space for return value.
				// params would go here...
			LEOPushIntegerOnStack( &ctx, 0 );	// Parameter count.
			
			LEOContextPushHandlerScriptReturnAddressAndBasePtr( &ctx, theHandler, script, NULL, NULL );	// NULL return address is same as exit to top. basePtr is set to NULL as well on exit.
			LEORunInContext( theHandler->instructions, &ctx );
			if( ctx.errMsg[0] != 0 )
				printf("ERROR: %s\n", ctx.errMsg );
			
			LEOCleanUpContext( &ctx );
		}
		
		LEOScriptRelease( script );
		LEOContextGroupRelease( group );
	}
	catch( std::exception& err )
	{
		std::cerr << err.what() << std::endl;
		return 3;
	}
	
	if( verbose )
		std::cout << "Finished successfully." << std::endl;
	
    return 0;
}
