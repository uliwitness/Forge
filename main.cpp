#include <iostream>
#include <stdexcept>
#include "CToken.h"
#include "CParser.h"
#include "GetFileContents.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include "CodeBlock.h"
#include "CParseTree.h"
#include "VelocityProgram.h"

using namespace Carlson;


class PrintfProgressDelegate : public CodeBlockProgressDelegate, public ParseTreeProgressDelegate
{
public:
	virtual void	CodeBlockPreparing( CodeBlock* blk )															{ printf( "\tPreparing...\n" ); };
	virtual void	CodeBlockAddingFunction( CodeBlock* blk, const std::string& methodName )						{ printf( "\tAdding function \"%s\".\n", methodName.c_str() ); };
	virtual void	CodeBlockAddedData( CodeBlock* blk )															{ printf( "\t%d bytes of data.\n", blk->data_size() ); };	// This gets called a lot, and may even be caused by the other callbacks.
	virtual void	CodeBlockAddedCode( CodeBlock* blk )															{ printf( "\t%d bytes of code.\n", blk->code_size() ); };	// This gets called a lot, and may even be caused by the other callbacks.
	virtual void	CodeBlockUpdateSymbolUseCount( CodeBlock* blk, const std::string& methodName, int32_t numUses )	{ printf( "\tSymbol \"%s\" used %d times so far.\n", methodName.c_str(), numUses ); };
	virtual void	CodeBlockAddingSymbol( CodeBlock* blk, const std::string& symbolName, bool isExternal )			{ printf( "\tAdding %s symbol \"%s\" to symbol table.\n", (isExternal ? "external" : "internal"), symbolName.c_str() ); };
	virtual void	CodeBlockFinished( CodeBlock* blk )																{ printf( "\tFinished.\n\t\t%d bytes of code.\n\t\t%d bytes of data.\n", blk->code_size(), blk->data_size() ); };

	virtual void	CodeBlockAddedStringData( CodeBlock* blk, const std::string& str )								{ printf( "\tAdded string \"%s\".\n", str.c_str() ); };
	virtual void	CodeBlockAddedIntData( CodeBlock* blk, int n )													{ printf( "\tAdded int %d.\n", n ); };
	virtual void	CodeBlockAddedFloatData( CodeBlock* blk, float n )												{ printf( "\tAdded int %f.\n", n ); };
	virtual void	CodeBlockAddedBoolData( CodeBlock* blk, bool n )												{ printf( "\tAdded bool %s.\n", (n ? "true" : "false") ); };
	virtual void	CodeBlockUsedLocalVariable( CodeBlock* blk, const std::string& str, int32_t numUses )			{ printf( "\tLocal variable \"%s\" used %d times so far.\n", str.c_str(), numUses ); };
	
	virtual void	ParseTreeBegunParsing( CParseTree* tree )														{ printf( "\tCreated tree...\n" ); };
	virtual void	ParseTreeAddedNode( CParseTree* tree, CNode* inNode, size_t inNumNodes )						{ printf( "\t%u tree nodes parsed.\n", (unsigned)inNumNodes ); };
	virtual void	ParseTreeFinishedParsing( CParseTree* tree )													{ printf( "\tTearing down tree...\n" ); };
};


extern "C" void	test_vcy_lib();


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
	
	test_vcy_lib();
	
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
	
	std::string			destFilename;
	if( outputFNameIdx > 0 )
		destFilename.append( argv[outputFNameIdx] );
	else
	{
		destFilename.append( filename );
		destFilename.append( ".vld" );
	}
	
	try
	{
		PrintfProgressDelegate	progressDelegate;
		CParseTree				parseTree( &progressDelegate );
		
		std::cout << "Tokenizing file \"" << filename << "\"..." << std::endl;
		tokens = CToken::TokenListFromText( code, strlen(code) );
		#if 1
		for( std::deque<CToken>::iterator currToken = tokens.begin(); currToken != tokens.end(); currToken++ )
			std::cout << "Token: " << currToken->GetDescription() << std::endl;
		#endif
		
		std::cout << "Parsing file \"" << filename << "\"..." << std::endl;
		parser.Parse( filename, tokens, parseTree );
		
		parseTree.DebugPrint( std::cout, 1 );
		
		std::cout << "Compiling file \"" << filename << "\"..." << std::endl;
		
		#if 1
		CodeBlock				codeBlock( &progressDelegate );
		
		parseTree.GenerateCode( codeBlock );
		
		codeBlock.finish();
		
		std::cout << "Writing code file \"" << destFilename.c_str() << "\"..." << std::endl;
		FILE*		aFile = fopen( destFilename.c_str(), "w" );
		if( !aFile )
			throw std::runtime_error( "Couldn't open destination file." );
		codeBlock.write( aFile );
		fclose( aFile );
		
		std::cout << "Disassembly of code file \"" << destFilename.c_str() << "\":" << std::endl;
		VelocityProgram		program;
		program.LoadFromFile( destFilename.c_str() );
		#else
		CppBlock				cppBlock( &progressDelegate );
		
		parseTree.GenerateCpp( cppBlock );
		
		cppBlock.dump();
		#endif
	}
	catch( std::exception& err )
	{
		std::cerr << err.what() << std::endl;
		return 3;
	}
	
	std::cout << "Finished successfully." << std::endl;
	
    return 0;
}
