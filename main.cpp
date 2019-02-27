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
#include "LEOMsgInstructionsGeneric.h"

#include <fstream>
#include "AnsiFiles.h"


using namespace Carlson;


struct TBuiltInVariableEntry	gBuiltInVariables[] =
{
	{ EPageIdentifier, "page", "page", true },
	{ ELastIdentifier_Sentinel, nullptr, nullptr }
};


struct ForgeToolResourceEntry
{
	ForgeToolResourceEntry( std::string src, std::string dst ) : mSourceFile(src), mDestFile(dst) {}
	
	bool operator == ( const ForgeToolResourceEntry& inOriginal ) const { return (mSourceFile.compare( inOriginal.mSourceFile ) == 0) && (mDestFile.compare( inOriginal.mDestFile ) == 0); }
	
	std::string	mSourceFile;
	std::string mDestFile;
};


struct ForgeToolSummaryEntry
{
	ForgeToolSummaryEntry( std::string fileName, std::string summary, std::string modified, std::string created ) : mFileName(fileName), mSummary(summary), mModified(modified), mCreated(created) {}

	std::string	mFileName;
	std::string mSummary;
	std::string mModified;
	std::string mCreated;
};


struct ForgeToolOptions
{
	bool			debuggerOn = false;
	bool			runCode = true;
	bool			printInstructions = false;
	bool			printTokens = false;
	bool			printParseTree = false;
	bool			printOptimizedParseTree = false;
	bool			printIndented = false;
	bool			verbose = false;
	bool			doOptimize = true;
	bool			printresult = false;
	bool			webPageEmbedMode = false;
	const char*		debuggerHost = NULL;
	const char*		messageName = nullptr;
	int				argc = 0;
	char * const *	argv = nullptr;
	int				fnameIdx = 0;
	bool			postbuild = false;	// Ignore passed arc/argv and instead pass the resources as parameters.
	std::vector<ForgeToolResourceEntry>	resources;
	std::vector<ForgeToolSummaryEntry>	summaries;
};


int	ProcessOneScriptFile( const std::string& inFilePathString, ForgeToolOptions& toolOptions );


static bool GetFileContents( const std::string& fname, std::vector<char>& outFileContents )
{
	// Open script to run:
	FILE*	theFile = LEOFOpen( fname.c_str(), "r" );
	if( !theFile )
	{
		char theWD[1024];
		printf("ERROR: Can't open file \"%s/%s\".\n", getcwd(theWD, sizeof(theWD)), fname.c_str());
		return false;
	}
	
	// Find out file length:
	fseek( theFile, 0, SEEK_END );
	long	len = ftell( theFile ),
			readbytes = 0;
	outFileContents.resize( len +1 );
	
	// Rewind and read in whole file:
	fseek( theFile, 0, SEEK_SET );
	readbytes = fread( outFileContents.data(), 1, len, theFile );
	if( readbytes != len )
	{
		fclose( theFile );
		printf("ERROR: Couldn't read from file \"%s\" (%ld bytes read).\n",fname.c_str(),readbytes);
		return false;
	}
	outFileContents[len] = 0;	// Terminate string.
	fclose( theFile );
	
	return true;
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
			else if( strcmp( argv[x], "--printoptimizedparsetree" ) == 0 )
			{
				toolOptions.printOptimizedParseTree = true;
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
	LEOAddHostFunctionsAndOffsetInstructions( gFileHostFunctions, kFirstFileInstruction );
	
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
			{
				return errNum;
			}
		}
		
		if( toolOptions.webPageEmbedMode )
		{
			filesystem::path	postBuildFile("../library/postbuild.hc");
			if( filesystem::exists(postBuildFile) )
			{
				toolOptions.postbuild = true;
				int errNum = ProcessOneScriptFile( postBuildFile.string(), toolOptions );
				if( errNum != EXIT_SUCCESS )
				{
					return errNum;
				}
			}
		}
	}
	else if( filename )
	{
		int theResult = ProcessOneScriptFile( filename, toolOptions );
		return theResult;
	}
	else
	{
		std::cerr << "error: First parameter after options should be name of script to compile." << std::endl;
		return 2;
	}
	
	return 0;
}


void	AddResourceEntryIfUnique( const ForgeToolResourceEntry& inResourceToAdd, ForgeToolOptions& toolOptions )
{
	for( const ForgeToolResourceEntry& currResource : toolOptions.resources )
	{
		if( currResource == inResourceToAdd )
			return;	// No need to add, already have it.
	}
	
	toolOptions.resources.push_back( inResourceToAdd );
}


int	ProcessOneScriptFile( const std::string& inFilePathString, ForgeToolOptions& toolOptions )
{
	// Do actual work:
	std::vector<char>	code;
	if( !GetFileContents( inFilePathString, code ) )
	{
		std::cerr << "error: Couldn't find file \"" << inFilePathString << "\"." << std::endl;
		return 2;
	}

	std::deque<CToken>	tokens;
	CParser				parser;
	parser.SetWebPageEmbedMode(toolOptions.webPageEmbedMode);
	if( toolOptions.webPageEmbedMode )
	{
		parser.SetIncludeHandler([](const std::string& inFileName, const std::string& inRelativeToFileName, std::vector<char>&outContents)
		{
			std::string includePath;
			size_t namestart = inRelativeToFileName.find_last_of('/');
			if( namestart == std::string::npos )
				namestart = 0;
			includePath = inRelativeToFileName.substr( 0, namestart );
			if( namestart != 0 )
				includePath.append(1,'/');
			includePath.append(inFileName);
			
			return GetFileContents( includePath, outContents );
		});
	}
	
	try
	{
		CParseTree				parseTree;
		
		if( toolOptions.verbose )
			std::cout << "Tokenizing file \"" << inFilePathString << "\"..." << std::endl;
		tokens = CTokenizer::TokenListFromText( code.data(), code.size(), toolOptions.webPageEmbedMode );
		if( toolOptions.printTokens )
		{
			for( std::deque<CToken>::iterator currToken = tokens.begin(); currToken != tokens.end(); currToken++ )
				std::cout << "Token: " << currToken->GetDescription() << std::endl;
		}
		
		if( toolOptions.verbose )
			std::cout << "Parsing file \"" << inFilePathString << "\"..." << std::endl;
		parser.Parse( inFilePathString.c_str(), tokens, parseTree, code.data() );
		
		if( toolOptions.printParseTree )
			parseTree.DebugPrint( std::cout, 1 );
		
		if( toolOptions.printIndented )
		{
			if( toolOptions.verbose )
				std::cout << "Indenting file \"" << inFilePathString << "\"..." << std::endl;
			LEODisplayInfoTable*	lit = LEODisplayInfoTableCreateForParseTree( (LEOParseTree*) &parseTree );
			char*	theText = NULL;
			size_t	theLength = 0;
			LEODisplayInfoTableApplyToText( lit, code.data(), code.size(), &theText, &theLength, NULL, NULL );
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
		
		if( toolOptions.printOptimizedParseTree )
			parseTree.DebugPrint( std::cout, 1 );

		parseTree.GenerateCode( &block );
		
		if( toolOptions.printInstructions )
			LEODebugPrintScript( group, script );
		
		if( toolOptions.runCode )
		{
			std::stringstream	capturedOutput;
			if( toolOptions.webPageEmbedMode && !toolOptions.postbuild )
			{
				gLEOMsgOutputStream = &capturedOutput;
			}
			
			if( toolOptions.verbose )
				printf( "\nRun the code:\n" );
			
			const char*		handlerName = toolOptions.postbuild ? "buildEnded" : toolOptions.messageName;
			LEOHandlerID	handlerID = LEOContextGroupHandlerIDForHandlerName( group, handlerName );
			LEOHandler*		theHandler = LEOScriptFindCommandHandlerWithID( script, handlerID );
			
			if( theHandler == NULL )
			{
				printf( "ERROR: Could not find handler \"%s\" to run!", handlerName );
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
						LEORemoteDebuggerAddFile( code.data(), fileID, script );
						
						for( const CIncludeFileEntry& currInclude : parser.GetIncludedFiles() )
						{
							uint16_t 	includedFileID = LEOFileIDForFileName(currInclude.mFileName.c_str());
							LEORemoteDebuggerAddFile( currInclude.mFileData.data(), includedFileID, script );
						}
					}
				}
				
				LEOPushUnsetValueOnStack( ctx );	// Reserve space for return value.
				
				// Push params on stack in reverse order:
				LEOInteger paramCount = 0;
				if( !toolOptions.postbuild )
				{
					paramCount = 0;
					
					for( int x = (toolOptions.argc -1); x > toolOptions.fnameIdx; x-- )
					{
						LEOPushStringValueOnStack( ctx, toolOptions.argv[x], strlen(toolOptions.argv[x]) );
						paramCount++;
					}
				
					LEOPushIntegerOnStack( ctx, paramCount, kLEOUnitNone );	// Parameter count.
				}
				else
				{
					paramCount = 2;
					
					// We must push the params *backwards*!
					
					LEOValueArray * arrayValue = (LEOValueArray*) LEOPushArrayValueOnStack( ctx, NULL );
					size_t	currIndex = 0;
					for( ForgeToolSummaryEntry currEntry : toolOptions.summaries )
					{
						char	currKey[100] = {};
						snprintf( currKey, sizeof(currKey) -1, "%zu", ++currIndex );
						LEOValuePtr singleEntryArray = LEOAddArrayArrayEntryToRoot( &arrayValue->array, currKey, NULL, ctx );
						LEOAddStringArrayEntryToRoot( &singleEntryArray->array.array, "filename", currEntry.mFileName.data(), currEntry.mFileName.size(), ctx );
						LEOAddStringArrayEntryToRoot( &singleEntryArray->array.array, "summary", currEntry.mSummary.data(), currEntry.mSummary.size(), ctx );
						LEOAddStringArrayEntryToRoot( &singleEntryArray->array.array, "modified", currEntry.mModified.data(), currEntry.mModified.size(), ctx );
						LEOAddStringArrayEntryToRoot( &singleEntryArray->array.array, "created", currEntry.mCreated.data(), currEntry.mCreated.size(), ctx );
					}
					
					arrayValue = (LEOValueArray*) LEOPushArrayValueOnStack( ctx, NULL );
					currIndex = 0;
					for( ForgeToolResourceEntry currEntry : toolOptions.resources )
					{
						char	currKey[100] = {};
						snprintf( currKey, sizeof(currKey) -1, "%zu", ++currIndex );
						LEOValuePtr singleEntryArray = LEOAddArrayArrayEntryToRoot( &arrayValue->array, currKey, NULL, ctx );
						LEOAddStringArrayEntryToRoot( &singleEntryArray->array.array, "filename", currEntry.mSourceFile.data(), currEntry.mSourceFile.size(), ctx );
						LEOAddStringArrayEntryToRoot( &singleEntryArray->array.array, "destination", currEntry.mDestFile.data(), currEntry.mDestFile.size(), ctx );
					}
					
					LEOPushIntegerOnStack( ctx, paramCount, kLEOUnitNone );	// Parameter count.
				}
				
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
				
				if( toolOptions.webPageEmbedMode )
				{
					std::stringstream	desiredFilename;
					
					LEOValuePtr pageGlobal = LEOGetArrayValueForKey( group->globals, "page" );
					if( pageGlobal )
					{
						bool			filenameRequested = false;
						char			tmpStr[1024] = {};
						union LEOValue	tmp;
						LEOValuePtr theValue = LEOGetValueForKey( pageGlobal, "filename", &tmp, kLEOInvalidateReferences, ctx );
						if( theValue )
						{
							const char* str = LEOGetValueAsString( theValue, tmpStr, sizeof(tmpStr), ctx );
							if( str )
							{
								filenameRequested = true;
								desiredFilename << str;
							}
							if( theValue == &tmp )
							{
								LEOCleanUpValue( theValue, kLEOInvalidateReferences, ctx );
							}
						}
						if( !filenameRequested )
						{
							theValue = LEOGetValueForKey( pageGlobal, "filetype", &tmp, kLEOInvalidateReferences, ctx );
							if( theValue )
							{
								const char* str = LEOGetValueAsString( theValue, tmpStr, sizeof(tmpStr), ctx );
								if( str )
								{
									std::string suffixlessFileName;
									size_t namestart = inFilePathString.find_last_of('/');
									if( namestart == std::string::npos ) namestart = 0; else ++namestart;
									size_t basenameend = inFilePathString.find_last_of('.');
									suffixlessFileName = inFilePathString.substr( namestart, basenameend -namestart );
									desiredFilename << suffixlessFileName << "." << str;
									filenameRequested = true;
								}
								if( theValue == &tmp )
								{
									LEOCleanUpValue( theValue, kLEOInvalidateReferences, ctx );
								}
							}
						}
						
						if( filenameRequested )
						{
							std::cout << "Writing file: " << desiredFilename.str() << std::endl;
							filesystem::create_directories( filesystem::path(desiredFilename.str()).parent_path() );
							
							std::ofstream	outputFile;
							outputFile.open( desiredFilename.str() );
							outputFile << capturedOutput.str();
						}

						theValue = LEOGetValueForKey( pageGlobal, "resources", &tmp, kLEOInvalidateReferences, ctx );
						if( theValue )
						{
							char strBuf[1024] = {};
							const char* theGlobalStr = LEOGetValueAsString( theValue, strBuf, sizeof(strBuf), ctx );
							std::cout << "[[" << theGlobalStr << "]]" << std::endl;
							
							char			keyStr[100] = {};
							size_t			x = 0;
							while( true )
							{
								snprintf( keyStr, sizeof(keyStr) -1, "%zu", ++x );
								union LEOValue	currResourceTmp;
								LEOValuePtr currResourceValue = LEOGetValueForKey( theValue, keyStr, &currResourceTmp, kLEOInvalidateReferences, ctx );
								if( !currResourceValue )
									break;
								
								if( currResourceValue )
								{
									union LEOValue	currResourceFilenameTmp;
									LEOValuePtr currResourceFilenameValue = LEOGetValueForKey( currResourceValue, "filename", &currResourceFilenameTmp, kLEOInvalidateReferences, ctx );
									if( currResourceFilenameValue )
									{
										char currResourceFilenameStrBuf[1024];
										const char* currResourceFilenameStr = LEOGetValueAsString( currResourceFilenameValue, currResourceFilenameStrBuf, sizeof(currResourceFilenameStrBuf), ctx );
										
										const char*		currResourceDestStr = "";
										union LEOValue	currResourceDestTmp;
										LEOValuePtr currResourceDestValue = LEOGetValueForKey( currResourceValue, "destination", &currResourceDestTmp, kLEOInvalidateReferences, ctx );
										if( currResourceDestValue )
										{
											char currResourceDestStrBuf[1024];
											currResourceDestStr = LEOGetValueAsString( currResourceDestValue, currResourceDestStrBuf, sizeof(currResourceDestStrBuf), ctx );
										}
										
										AddResourceEntryIfUnique( ForgeToolResourceEntry( currResourceFilenameStr, currResourceDestStr ), toolOptions );
										
										if( currResourceDestValue == &currResourceDestTmp )
										{
											LEOCleanUpValue( currResourceDestValue, kLEOInvalidateReferences, ctx );
										}
										
										if( currResourceFilenameValue == &currResourceFilenameTmp )
										{
											LEOCleanUpValue( currResourceFilenameValue, kLEOInvalidateReferences, ctx );
										}
									}
								}
								
								if( currResourceValue == &currResourceTmp )
								{
									LEOCleanUpValue( currResourceValue, kLEOInvalidateReferences, ctx );
								}
							}
							
							if( theValue == &tmp )
							{
								LEOCleanUpValue( theValue, kLEOInvalidateReferences, ctx );
							}
						}

						union LEOValue	tmp1;
						char currSummaryStrBuf[1024];
						const char* currSummaryStr = nullptr;
						theValue = LEOGetValueForKey( pageGlobal, "summary", &tmp1, kLEOInvalidateReferences, ctx );
						if( theValue )
						{
							currSummaryStr = LEOGetValueAsString( theValue, currSummaryStrBuf, sizeof(currSummaryStrBuf), ctx );
						}

						union LEOValue	tmp2;
						char currModifiedStrBuf[1024];
						const char* currModifiedStr = nullptr;
						LEOValuePtr theValue2 = LEOGetValueForKey( pageGlobal, "modified", &tmp2, kLEOInvalidateReferences, ctx );
						if( theValue2 )
						{
							currModifiedStr = LEOGetValueAsString( theValue2, currModifiedStrBuf, sizeof(currModifiedStrBuf), ctx );
						}

						union LEOValue	tmp3;
						char currCreatedStrBuf[1024];
						const char* currCreatedStr = nullptr;
						LEOValuePtr theValue3 = LEOGetValueForKey( pageGlobal, "modified", &tmp3, kLEOInvalidateReferences, ctx );
						if( theValue3 )
						{
							currModifiedStr = LEOGetValueAsString( theValue3, currCreatedStrBuf, sizeof(currCreatedStrBuf), ctx );
						}
						
						toolOptions.summaries.push_back( ForgeToolSummaryEntry( desiredFilename.str(), currSummaryStr ? currSummaryStr : "", currModifiedStr ? currModifiedStr : "", currCreatedStr ? currCreatedStr : "" ) );
						
						if( theValue == &tmp3 )
						{
							LEOCleanUpValue( theValue, kLEOInvalidateReferences, ctx );
						}
						
						if( theValue == &tmp2 )
						{
							LEOCleanUpValue( theValue, kLEOInvalidateReferences, ctx );
						}
						
						if( theValue == &tmp1 )
						{
							LEOCleanUpValue( theValue, kLEOInvalidateReferences, ctx );
						}
					}
				}
				
				LEOContextRelease( ctx );
			}
			
			if( toolOptions.webPageEmbedMode && !toolOptions.postbuild )
			{
				gLEOMsgOutputStream = &std::cout;
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
