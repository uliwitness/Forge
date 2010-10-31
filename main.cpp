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
}


using namespace Carlson;


class PrintfProgressDelegate : public CCodeBlockProgressDelegate, public CParseTreeProgressDelegate
{
public:
	virtual void	CodeBlockPreparing( CCodeBlock* blk )																{ printf( "\tPreparing...\n" ); };
	virtual void	CodeBlockAddingFunction( CCodeBlock* blk, const std::string& methodName )							{ printf( "\tAdding function \"%s\".\n", methodName.c_str() ); };
	virtual void	CodeBlockAddedData( CCodeBlock* blk )																{ printf( "\t%lu bytes of data.\n", blk->data_size() ); };	// This gets called a lot, and may even be caused by the other callbacks.
	virtual void	CodeBlockAddedCode( CCodeBlock* blk )																{ printf( "\t%lu bytes of code.\n", blk->code_size() ); };	// This gets called a lot, and may even be caused by the other callbacks.
	virtual void	CodeBlockUpdateSymbolUseCount( CCodeBlock* blk, const std::string& methodName, int32_t numUses )	{ printf( "\tSymbol \"%s\" used %d times so far.\n", methodName.c_str(), numUses ); };
	virtual void	CodeBlockAddingSymbol( CCodeBlock* blk, const std::string& symbolName, bool isExternal )			{ printf( "\tAdding %s symbol \"%s\" to symbol table.\n", (isExternal ? "external" : "internal"), symbolName.c_str() ); };
	virtual void	CodeBlockFinished( CCodeBlock* blk )																{ printf( "\tFinished.\n\t\t%lu bytes of code.\n\t\t%lu bytes of data.\n", blk->code_size(), blk->data_size() ); };

	virtual void	CodeBlockAddedStringData( CCodeBlock* blk, const std::string& str )									{ printf( "\tAdded string \"%s\".\n", str.c_str() ); };
	virtual void	CodeBlockAddedIntData( CCodeBlock* blk, int n )														{ printf( "\tAdded int %d.\n", n ); };
	virtual void	CodeBlockAddedFloatData( CCodeBlock* blk, float n )													{ printf( "\tAdded int %f.\n", n ); };
	virtual void	CodeBlockAddedBoolData( CCodeBlock* blk, bool n )													{ printf( "\tAdded bool %s.\n", (n ? "true" : "false") ); };
	virtual void	CodeBlockUsedLocalVariable( CCodeBlock* blk, const std::string& str, int32_t numUses )				{ printf( "\tLocal variable \"%s\" used %d times so far.\n", str.c_str(), numUses ); };
	
	virtual void	ParseTreeBegunParsing( CParseTree* tree )															{ printf( "\tCreated tree...\n" ); };
	virtual void	ParseTreeAddedNode( CParseTree* tree, CNode* inNode, size_t inNumNodes )							{ printf( "\t%lu tree nodes parsed.\n", inNumNodes ); };
	virtual void	ParseTreeFinishedParsing( CParseTree* tree )														{ printf( "\tTearing down tree...\n" ); };
};


using namespace Carlson;

int main( int argc, char * const argv[] )
{
	time_t					theDate;
	struct tm*				dateParts;
	theDate = time( NULL );
	dateParts = localtime( &theDate );
	
	if( dateParts->tm_year > 112 )	// Valid till end of 2012 (-1900).
	{
		std::cerr << "error: This pre-release version has run out. Please get a new one from http://www.zathras.de/." << std::endl;
		return 99;
	}
	
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
			std::cerr << "error: First parameter should be name of script to compile." << std::endl;
		else
			std::cerr << "error: Couldn't find file \"" << filename << "\"." << std::endl;
		return 2;
	}
		
	try
	{
		PrintfProgressDelegate	progressDelegate;
		CParseTree				parseTree( &progressDelegate );
		
		std::cout << "Tokenizing file \"" << filename << "\"..." << std::endl;
		tokens = CToken::TokenListFromText( code, strlen(code) );
		#if 0
		for( std::deque<CToken>::iterator currToken = tokens.begin(); currToken != tokens.end(); currToken++ )
			std::cout << "Token: " << currToken->GetDescription() << std::endl;
		#endif
		
		std::cout << "Parsing file \"" << filename << "\"..." << std::endl;
		parser.Parse( filename, tokens, parseTree );
		
		#if 1
		parseTree.DebugPrint( std::cout, 1 );
		#endif
		
		LEOScript	*	script = LEOScriptCreateForOwner( 0, 0 );
		CCodeBlock		block( script, &progressDelegate );
		LEOScriptRelease( script );
		
		parseTree.GenerateCode( &block );
	}
	catch( std::exception& err )
	{
		std::cerr << err.what() << std::endl;
		return 3;
	}
	
	std::cout << "Finished successfully." << std::endl;
	
    return 0;
}
