/*
 *  CParser.cpp
 *  HyperC
 *
 *  Created by Uli Kusterer on 29.07.06.
 *  Copyright 2006 Uli Kusterer. All rights reserved.
 *
 */

// -----------------------------------------------------------------------------
//	Headers:
// -----------------------------------------------------------------------------

#include "CParser.h"
#include "CToken.h"
#include "CParseTree.h"
#include "CFunctionDefinitionNode.h"
#include "CCommandNode.h"
#include "CFunctionCallNode.h"
#include "CWhileLoopNode.h"
#include "CCodeBlockNode.h"
#include "CIfNode.h"
#include "CPushValueCommandNode.h"
#include "CAssignCommandNode.h"
#include "CPutCommandNode.h"
#include "CGetParamCommandNode.h"
#include "CReturnCommandNode.h"
#include "COperatorNode.h"
#include "CAddCommandNode.h"
#include "CSubtractCommandNode.h"
#include "CMultiplyCommandNode.h"
#include "CDivideCommandNode.h"
#include "CLineMarkerNode.h"
#include "CAssignChunkArrayNode.h"
#include "CGetArrayItemCountNode.h"
#include "CGetArrayItemNode.h"
#include "CMakeChunkRefNode.h"
#include "CMakeChunkConstNode.h"
#include "CObjectPropertyNode.h"
#include "CGlobalPropertyNode.h"
#include "CDownloadCommandNode.h"
#include "CParseErrorCommandNode.h"
#include "CForgeExceptions.h"

extern "C" {
#include "LEOInstructions.h"
#include "LEOPropertyInstructions.h"
#include "LEOObjCCallInstructions.h"
}

#include <iostream>
#include <stdexcept>
#include <string>
#include <fstream>
#include <cmath>


using namespace Carlson;


#define DEBUG_HOST_ENTITIES			0
#define ADJUST_LINE_NUM(a,b)


// -----------------------------------------------------------------------------
//	Globals and static ivars:
// -----------------------------------------------------------------------------

namespace Carlson
{

// Static ivars:
int										CVariableEntry::mTempCounterSeed = 0;	// Counter we use for generating unique temp variable names.
std::map<std::string,CObjCMethodEntry>	CParser::sObjCMethodTable;				// Table of ObjC method signature -> types mappings for calling Cocoa.
std::map<std::string,CObjCMethodEntry>	CParser::sCFunctionTable;				// Table of C function name -> types mappings for calling native system calls.
std::map<std::string,CObjCMethodEntry>	CParser::sCFunctionPointerTable;		// Table of C function pointer type name -> types mappings for generating callback trampolines.
std::map<std::string,std::string>		CParser::sSynonymToTypeTable;			// Table of C type synonym name -> real name mappings.
std::map<std::string,std::string>		CParser::sConstantToValueTable;			// Table of C system constant name -> constant value mappings.
LEOFirstNativeCallCallbackPtr			CParser::sFirstNativeCallCallback = NULL;


static CFunctionDefinitionNode*			sLastErrorFunction = NULL;


#pragma mark -
#pragma mark [Operator lookup table]
// LOOKUP TABLES:
// Operator token(s), precedence and instruction function name:
static TOperatorEntry	sOperators[] =
{
	{ EAndIdentifier, ELastIdentifier_Sentinel, 100, AND_INSTR, EAndIdentifier },
	{ EOrIdentifier, ELastIdentifier_Sentinel, 100, OR_INSTR, EOrIdentifier },
	{ ELessThanOperator, EGreaterThanOperator, 200, NOT_EQUAL_OPERATOR_INSTR, ENotEqualOperator },
	{ ELessThanOperator, EEqualsOperator, 200, LESS_THAN_EQUAL_OPERATOR_INSTR, ELessThanEqualOperator },
	{ ELessThanEqualOperator, ELastIdentifier_Sentinel, 200, LESS_THAN_EQUAL_OPERATOR_INSTR, ELessThanEqualOperator },
	{ ELessThanOperator, ELastIdentifier_Sentinel, 200, LESS_THAN_OPERATOR_INSTR, ELessThanOperator },
	{ EGreaterThanOperator, EEqualsOperator, 200, GREATER_THAN_EQUAL_OPERATOR_INSTR, EGreaterThanEqualOperator },
	{ EGreaterThanEqualOperator, ELastIdentifier_Sentinel, 200, GREATER_THAN_EQUAL_OPERATOR_INSTR, EGreaterThanEqualOperator },
	{ EGreaterThanOperator, ELastIdentifier_Sentinel, 200, GREATER_THAN_OPERATOR_INSTR, EGreaterThanOperator },
	{ EEqualsOperator, ELastIdentifier_Sentinel, 200, EQUAL_OPERATOR_INSTR, EEqualsOperator },
	{ EIsIdentifier, ENotIdentifier, 200, NOT_EQUAL_OPERATOR_INSTR, ENotEqualOperator },
	{ ENotEqualOperator, ELastIdentifier_Sentinel, 200, NOT_EQUAL_OPERATOR_INSTR, ENotEqualOperator },
	{ EIsIdentifier, ELastIdentifier_Sentinel, 200, EQUAL_OPERATOR_INSTR, EEqualsOperator },
	{ EAmpersandOperator, EAmpersandOperator, 300, CONCATENATE_VALUES_WITH_SPACE_INSTR, EDoubleAmpersandPseudoOperator },
	{ EAmpersandOperator, ELastIdentifier_Sentinel, 300, CONCATENATE_VALUES_INSTR, EAmpersandOperator },
	{ EPlusOperator, ELastIdentifier_Sentinel, 500, ADD_OPERATOR_INSTR, EPlusOperator },
	{ EMinusOperator, ELastIdentifier_Sentinel, 500, SUBTRACT_OPERATOR_INSTR, EMinusOperator },
	{ EMultiplyOperator, ELastIdentifier_Sentinel, 1000, MULTIPLY_OPERATOR_INSTR, EMultiplyOperator },
	{ EDivideOperator, ELastIdentifier_Sentinel, 1000, DIVIDE_OPERATOR_INSTR, EDivideOperator },
	{ EDivideSymbolOperator, ELastIdentifier_Sentinel, 1000, DIVIDE_OPERATOR_INSTR, EDivideOperator },
	{ EModIdentifier, ELastIdentifier_Sentinel, 1000, MODULO_OPERATOR_INSTR, EModuloIdentifier },
	{ EModuloIdentifier, ELastIdentifier_Sentinel, 1000, MODULO_OPERATOR_INSTR, EModuloIdentifier },
	{ EExponentOperator, ELastIdentifier_Sentinel, 1100, POWER_OPERATOR_INSTR, EExponentOperator },
	{ ELastIdentifier_Sentinel, ELastIdentifier_Sentinel, 0, INVALID_INSTR, ELastIdentifier_Sentinel }
};

static TUnaryOperatorEntry	sUnaryOperators[] =
{
	{ ENotIdentifier, NEGATE_BOOL_INSTR },
	{ EMinusOperator, NEGATE_NUMBER_INSTR },
	{ ELastIdentifier_Sentinel, INVALID_INSTR }
};


static TGlobalPropertyEntry	sDefaultGlobalProperties[] =
{
	{ EItemDelIdentifier, ELastIdentifier_Sentinel, SET_ITEMDELIMITER_INSTR, PUSH_ITEMDELIMITER_INSTR },
	{ EItemDelimIdentifier, ELastIdentifier_Sentinel, SET_ITEMDELIMITER_INSTR, PUSH_ITEMDELIMITER_INSTR },
	{ EItemDelimiterIdentifier, ELastIdentifier_Sentinel, SET_ITEMDELIMITER_INSTR, PUSH_ITEMDELIMITER_INSTR },
	{ ELastIdentifier_Sentinel, ELastIdentifier_Sentinel, INVALID_INSTR }
};

static TBuiltInFunctionEntry	sDefaultBuiltInFunctions[] =
{
	{ EParamCountIdentifier, PARAMETER_COUNT_INSTR, BACK_OF_STACK, 0 },
	{ EParametersIdentifier, PUSH_PARAMETERS_INSTR, 0, 0 },
	{ ELastIdentifier_Sentinel, NULL, 0, 0 }
};

static TGlobalPropertyEntry*	sGlobalProperties = NULL;

static THostCommandEntry*		sHostCommands = NULL;

static THostCommandEntry*		sHostFunctions = NULL;

static TBuiltInFunctionEntry*	sBuiltInFunctions = NULL;

static TIdentifierSubtype		sUnitIdentifiers[] =
{
#define X4(constName,stringSuffix,identifierSubtype,unitGroup)	identifierSubtype,
	LEO_UNITS
#undef X4

};


#pragma mark [Chunk type lookup table]
// Chunk expression start token -> Chunk type constant (as string for code generation):
static TChunkTypeEntry	sChunkTypes[] =
{
	{ EByteIdentifier, EBytesIdentifier, TChunkTypeByte },
	{ ECharIdentifier, ECharsIdentifier, TChunkTypeCharacter },
	{ ECharacterIdentifier, ECharactersIdentifier, TChunkTypeCharacter },
	{ ELineIdentifier, ELinesIdentifier, TChunkTypeLine },
	{ EItemIdentifier, EItemsIdentifier, TChunkTypeItem },
	{ EWordIdentifier, EWordsIdentifier, TChunkTypeWord },
	{ ELastIdentifier_Sentinel, ELastIdentifier_Sentinel, TChunkTypeInvalid }
};

#pragma mark [Constant lookup table]
// Constant identifier and actual code to generate the value:
struct TConstantEntry	sConstants[] =
{
	{ { ETrueIdentifier, ELastIdentifier_Sentinel, ELastIdentifier_Sentinel }, new CBoolValueNode( NULL, true, SIZE_T_MAX ) },
	{ { EFalseIdentifier, ELastIdentifier_Sentinel, ELastIdentifier_Sentinel }, new CBoolValueNode( NULL, false, SIZE_T_MAX ) },
	{ { EEmptyIdentifier, ELastIdentifier_Sentinel, ELastIdentifier_Sentinel }, new CStringValueNode( NULL, std::string(""), SIZE_T_MAX ) },
	{ { ECommaIdentifier, ELastIdentifier_Sentinel, ELastIdentifier_Sentinel }, new CStringValueNode( NULL, std::string(","), SIZE_T_MAX ) },
	{ { EColonIdentifier, ELastIdentifier_Sentinel, ELastIdentifier_Sentinel }, new CStringValueNode( NULL, std::string(":"), SIZE_T_MAX ) },
	{ { ECrIdentifier, ELastIdentifier_Sentinel, ELastIdentifier_Sentinel }, new CStringValueNode( NULL, std::string("\r"), SIZE_T_MAX ) },
	{ { ELineFeedIdentifier, ELastIdentifier_Sentinel, ELastIdentifier_Sentinel }, new CStringValueNode( NULL, std::string("\n"), SIZE_T_MAX ) },
	{ { ENullIdentifier, ELastIdentifier_Sentinel, ELastIdentifier_Sentinel }, new CStringValueNode( NULL, std::string("\0"), SIZE_T_MAX ) },
	{ { EQuoteIdentifier, ELastIdentifier_Sentinel, ELastIdentifier_Sentinel }, new CStringValueNode( NULL, std::string("\""), SIZE_T_MAX ) },
	{ { EReturnIdentifier, ELastIdentifier_Sentinel, ELastIdentifier_Sentinel }, new CStringValueNode( NULL, std::string("\r"), SIZE_T_MAX ) },
	{ { ENewlineIdentifier, ELastIdentifier_Sentinel, ELastIdentifier_Sentinel }, new CStringValueNode( NULL, std::string("\n"), SIZE_T_MAX ) },
	{ { ESpaceIdentifier, ELastIdentifier_Sentinel, ELastIdentifier_Sentinel }, new CStringValueNode( NULL, std::string(" "), SIZE_T_MAX ) },
	{ { ETabIdentifier, ELastIdentifier_Sentinel, ELastIdentifier_Sentinel }, new CStringValueNode( NULL, std::string("\t"), SIZE_T_MAX ) },
	{ { EPiIdentifier, ELastIdentifier_Sentinel, ELastIdentifier_Sentinel }, new CFloatValueNode( NULL, (float) M_PI, SIZE_T_MAX ) },
	{ { EBarnIdentifier, EDoorIdentifier, EOpenIdentifier }, new CStringValueNode( NULL, std::string("barn door open"), SIZE_T_MAX ) },
	{ { EBarnIdentifier, EDoorIdentifier, ECloseIdentifier }, new CStringValueNode( NULL, std::string("barn door close"), SIZE_T_MAX ) },
	{ { EIrisIdentifier, EOpenIdentifier, ELastIdentifier_Sentinel }, new CStringValueNode( NULL, std::string("iris open"), SIZE_T_MAX ) },
	{ { EIrisIdentifier, ECloseIdentifier, ELastIdentifier_Sentinel }, new CStringValueNode( NULL, std::string("iris close"), SIZE_T_MAX ) },
	{ { EPushIdentifier, EUpIdentifier, ELastIdentifier_Sentinel }, new CStringValueNode( NULL, std::string("push up"), SIZE_T_MAX ) },
	{ { EPushIdentifier, EDownIdentifier, ELastIdentifier_Sentinel }, new CStringValueNode( NULL, std::string("push down"), SIZE_T_MAX ) },
	{ { EPushIdentifier, ELeftIdentifier, ELastIdentifier_Sentinel }, new CStringValueNode( NULL, std::string("push left"), SIZE_T_MAX ) },
	{ { EPushIdentifier, ERightIdentifier, ELastIdentifier_Sentinel }, new CStringValueNode( NULL, std::string("push right"), SIZE_T_MAX ) },
	{ { EScrollIdentifier, EUpIdentifier, ELastIdentifier_Sentinel }, new CStringValueNode( NULL, std::string("scroll up"), SIZE_T_MAX ) },
	{ { EScrollIdentifier, EDownIdentifier, ELastIdentifier_Sentinel }, new CStringValueNode( NULL, std::string("scroll down"), SIZE_T_MAX ) },
	{ { EScrollIdentifier, ELeftIdentifier, ELastIdentifier_Sentinel }, new CStringValueNode( NULL, std::string("scroll left"), SIZE_T_MAX ) },
	{ { EScrollIdentifier, ERightIdentifier, ELastIdentifier_Sentinel }, new CStringValueNode( NULL, std::string("scroll right"), SIZE_T_MAX ) },
	{ { EShrinkIdentifier, EToIdentifier, ETopIdentifier }, new CStringValueNode( NULL, std::string("shrink to top"), SIZE_T_MAX ) },
	{ { EShrinkIdentifier, EToIdentifier, ECenterIdentifier }, new CStringValueNode( NULL, std::string("shrink to center"), SIZE_T_MAX ) },
	{ { EShrinkIdentifier, EToIdentifier, EBottomIdentifier }, new CStringValueNode( NULL, std::string("shrink to bottom"), SIZE_T_MAX ) },
	{ { EStretchIdentifier, EFromIdentifier, ETopIdentifier }, new CStringValueNode( NULL, std::string("stretch from top"), SIZE_T_MAX ) },
	{ { EStretchIdentifier, EFromIdentifier, ECenterIdentifier }, new CStringValueNode( NULL, std::string("stretch from center"), SIZE_T_MAX ) },
	{ { EStretchIdentifier, EFromIdentifier, EBottomIdentifier }, new CStringValueNode( NULL, std::string("stretch from bottom"), SIZE_T_MAX ) },
	{ { EVenetianIdentifier, EBlindsIdentifier, ELastIdentifier_Sentinel }, new CStringValueNode( NULL, std::string("venetian blinds"), SIZE_T_MAX ) },
	{ { EWipeIdentifier, EUpIdentifier, ELastIdentifier_Sentinel }, new CStringValueNode( NULL, std::string("wipe up"), SIZE_T_MAX ) },
	{ { EWipeIdentifier, EDownIdentifier, ELastIdentifier_Sentinel }, new CStringValueNode( NULL, std::string("wipe down"), SIZE_T_MAX ) },
	{ { EWipeIdentifier, ELeftIdentifier, ELastIdentifier_Sentinel }, new CStringValueNode( NULL, std::string("wipe left"), SIZE_T_MAX ) },
	{ { EWipeIdentifier, ERightIdentifier, ELastIdentifier_Sentinel }, new CStringValueNode( NULL, std::string("wipe right"), SIZE_T_MAX ) },
	{ { EZoomIdentifier, ECloseIdentifier, ELastIdentifier_Sentinel }, new CStringValueNode( NULL, std::string("zoom close"), SIZE_T_MAX ) },
	{ { EZoomIdentifier, EInIdentifier, ELastIdentifier_Sentinel }, new CStringValueNode( NULL, std::string("zoom in"), SIZE_T_MAX ) },
	{ { EZoomIdentifier, EOpenIdentifier, ELastIdentifier_Sentinel }, new CStringValueNode( NULL, std::string("zoom open"), SIZE_T_MAX ) },
	{ { EZoomIdentifier, EOutIdentifier, ELastIdentifier_Sentinel }, new CStringValueNode( NULL, std::string("zoom out"), SIZE_T_MAX ) },
	{ { ELastIdentifier_Sentinel, ELastIdentifier_Sentinel, ELastIdentifier_Sentinel }, NULL }
};


#pragma mark -
	
	
void	PrintStringStream( std::stringstream& sstr );


void	PrintStringStream( std::stringstream& sstr )
{
	std::cout << sstr.str() << std::endl;
}


// -----------------------------------------------------------------------------
//	GetNewTempName:
//		Generate a unique name for a temp variable.
// -----------------------------------------------------------------------------

const std::string CVariableEntry::GetNewTempName()
{
	char tempName[40];
	snprintf( tempName, 40, "temp%d", mTempCounterSeed++ );
	
	return std::string( tempName );
}


// -----------------------------------------------------------------------------
//	* CONSTRUCTOR:
// -----------------------------------------------------------------------------

CParser::CParser()
{
	if( !sGlobalProperties )
		sGlobalProperties = sDefaultGlobalProperties;
	if( !sBuiltInFunctions )
		sBuiltInFunctions = sDefaultBuiltInFunctions;
}


// -----------------------------------------------------------------------------
//	AddBuiltInFunctionsAndOffsetInstructions:
//		Add additional global properties to the ones the parser understands.
// -----------------------------------------------------------------------------

/*static*/ void	CParser::AddBuiltInFunctionsAndOffsetInstructions( TBuiltInFunctionEntry* inEntries, size_t firstGlobalPropertyInstruction )
{
	if( !sBuiltInFunctions )
		sBuiltInFunctions = sDefaultBuiltInFunctions;
	
	size_t		numOldEntries = 0,
				numNewEntries = 0;
	
	for( size_t x = 0; sBuiltInFunctions[x].mType != ELastIdentifier_Sentinel; x++ )
		numOldEntries++;
	for( size_t x = 0; inEntries[x].mType != ELastIdentifier_Sentinel; x++ )
		numNewEntries++;
	
	TBuiltInFunctionEntry*	newTable = NULL;
	if( sBuiltInFunctions == sDefaultBuiltInFunctions )
	{
		newTable = (TBuiltInFunctionEntry*) calloc( numOldEntries +numNewEntries +1, sizeof(TBuiltInFunctionEntry) );
		if( !newTable )
			throw std::runtime_error( "Couldn't resize list of built-in functions." );
		memmove( newTable, sBuiltInFunctions, numOldEntries *sizeof(TBuiltInFunctionEntry) );
		memmove( newTable +numOldEntries, inEntries, (numNewEntries +1) *sizeof(TBuiltInFunctionEntry) );
	}
	else
	{
		newTable = (TBuiltInFunctionEntry*) realloc( sBuiltInFunctions, (numOldEntries +numNewEntries +1) * sizeof(TBuiltInFunctionEntry) );
		if( !newTable )
			throw std::runtime_error( "Couldn't resize list of global properties." );
		memmove( newTable +numOldEntries, inEntries, (numNewEntries +1) *sizeof(TBuiltInFunctionEntry) );
	}
	
	// Fix up instruction IDs to account for the ones that were already there:
	for( size_t x = numOldEntries; newTable[x].mType != ELastIdentifier_Sentinel; x++ )
	{
		if( newTable[x].mInstructionID == INVALID_INSTR2 )
			newTable[x].mInstructionID = INVALID_INSTR;
		else
			newTable[x].mInstructionID += firstGlobalPropertyInstruction;
	}
	
	sBuiltInFunctions = newTable;
}


// -----------------------------------------------------------------------------
//	AddGlobalPropertiesAndOffsetInstructions:
//		Add additional global properties to the ones the parser understands.
// -----------------------------------------------------------------------------

/*static*/ void	CParser::AddGlobalPropertiesAndOffsetInstructions( TGlobalPropertyEntry* inEntries, size_t firstGlobalPropertyInstruction )
{
	if( !sGlobalProperties )
		sGlobalProperties = sDefaultGlobalProperties;
	
	size_t		numOldEntries = 0,
				numNewEntries = 0;
	
	for( size_t x = 0; sGlobalProperties[x].mType != ELastIdentifier_Sentinel; x++ )
		numOldEntries++;
	for( size_t x = 0; inEntries[x].mType != ELastIdentifier_Sentinel; x++ )
		numNewEntries++;
	
	TGlobalPropertyEntry*	newTable = NULL;
	if( sGlobalProperties == sDefaultGlobalProperties )
	{
		newTable = (TGlobalPropertyEntry*) calloc( numOldEntries +numNewEntries +1, sizeof(TGlobalPropertyEntry) );
		if( !newTable )
			throw std::runtime_error( "Couldn't resize list of global properties." );
		memmove( newTable, sGlobalProperties, numOldEntries *sizeof(TGlobalPropertyEntry) );
		memmove( newTable +numOldEntries, inEntries, (numNewEntries +1) *sizeof(TGlobalPropertyEntry) );
	}
	else
	{
		newTable = (TGlobalPropertyEntry*) realloc( sGlobalProperties, (numOldEntries +numNewEntries +1) * sizeof(TGlobalPropertyEntry) );
		if( !newTable )
			throw std::runtime_error( "Couldn't resize list of global properties." );
		memmove( newTable +numOldEntries, inEntries, (numNewEntries +1) *sizeof(TGlobalPropertyEntry) );
	}
	
	// Fix up instruction IDs to account for the ones that were already there:
	for( size_t x = numOldEntries; newTable[x].mType != ELastIdentifier_Sentinel; x++ )
	{
		if( newTable[x].mSetterInstructionID == INVALID_INSTR2 )
			newTable[x].mSetterInstructionID = INVALID_INSTR;
		else
			newTable[x].mSetterInstructionID += firstGlobalPropertyInstruction;
		
		if( newTable[x].mGetterInstructionID == INVALID_INSTR2 )
			newTable[x].mGetterInstructionID = INVALID_INSTR;
		else
			newTable[x].mGetterInstructionID += firstGlobalPropertyInstruction;
	}
	
	sGlobalProperties = newTable;
}


// -----------------------------------------------------------------------------
//	AddHostCommandsAndOffsetInstructions:
//		Add additional commands to the ones the parser understands.
// -----------------------------------------------------------------------------

/*static*/ void	CParser::AddHostCommandsAndOffsetInstructions( THostCommandEntry* inEntries, size_t firstHostCommandInstruction )
{
	size_t		numOldEntries = 0,
				numNewEntries = 0;
	
	if( sHostCommands )
	{
		for( size_t x = 0; sHostCommands[x].mType != ELastIdentifier_Sentinel; x++ )
			numOldEntries++;
	}
	for( size_t x = 0; inEntries[x].mType != ELastIdentifier_Sentinel; x++ )
	{
		numNewEntries++;
	}
	
	THostCommandEntry*	newTable = NULL;
	if( sHostCommands == NULL )
	{
		newTable = (THostCommandEntry*) calloc( numOldEntries +numNewEntries +1, sizeof(THostCommandEntry) );
		if( !newTable )
			throw std::runtime_error( "Couldn't resize list of host commands." );
		memmove( newTable, inEntries, (numNewEntries +1) *sizeof(THostCommandEntry) );
	}
	else
	{
		newTable = (THostCommandEntry*) realloc( sHostCommands, (numOldEntries +numNewEntries +1) * sizeof(THostCommandEntry) );
		if( !newTable )
			throw std::runtime_error( "Couldn't resize list of host commands." );
		memmove( newTable +numOldEntries, inEntries, (numNewEntries +1) *sizeof(THostCommandEntry) );
	}
	
	// Fix up instruction IDs to account for the ones that were already there:
	for( size_t x = numOldEntries; newTable[x].mType != ELastIdentifier_Sentinel; x++ )
	{
		newTable[x].mInstructionID += firstHostCommandInstruction;
		for( size_t y = 0; newTable[x].mParam[y].mType != EHostParam_Sentinel; y++ )
		{
			if( newTable[x].mParam[y].mInstructionID == INVALID_INSTR2 )
				newTable[x].mParam[y].mInstructionID = INVALID_INSTR;
			else
				newTable[x].mParam[y].mInstructionID += firstHostCommandInstruction;
		}
	}
	
	sHostCommands = newTable;
}


// -----------------------------------------------------------------------------
//	AddHostFunctionsAndOffsetInstructions:
//		Add additional commands to the ones the parser understands.
// -----------------------------------------------------------------------------

/*static*/ void	CParser::AddHostFunctionsAndOffsetInstructions( THostCommandEntry* inEntries, size_t firstHostCommandInstruction )
{
	size_t		numOldEntries = 0,
				numNewEntries = 0;
	
	if( sHostFunctions )
	{
		for( size_t x = 0; sHostFunctions[x].mType != ELastIdentifier_Sentinel; x++ )
			numOldEntries++;
	}
	for( size_t x = 0; inEntries[x].mType != ELastIdentifier_Sentinel; x++ )
	{
		numNewEntries++;
	}
	
	THostCommandEntry*	newTable = NULL;
	if( sHostFunctions == NULL )
	{
		newTable = (THostCommandEntry*) calloc( numOldEntries +numNewEntries +1, sizeof(THostCommandEntry) );
		if( !newTable )
			throw std::runtime_error( "Couldn't resize list of host commands." );
		memmove( newTable, inEntries, (numNewEntries +1) *sizeof(THostCommandEntry) );
	}
	else
	{
		newTable = (THostCommandEntry*) realloc( sHostFunctions, (numOldEntries +numNewEntries +1) * sizeof(THostCommandEntry) );
		if( !newTable )
			throw std::runtime_error( "Couldn't resize list of host commands." );
		memmove( newTable +numOldEntries, inEntries, (numNewEntries +1) *sizeof(THostCommandEntry) );
	}
	
	// Fix up instruction IDs to account for the ones that were already there:
	for( size_t x = numOldEntries; newTable[x].mType != ELastIdentifier_Sentinel; x++ )
	{
		newTable[x].mInstructionID += firstHostCommandInstruction;
		for( size_t y = 0; newTable[x].mParam[y].mType != EHostParam_Sentinel; y++ )
		{
			if( newTable[x].mParam[y].mInstructionID == INVALID_INSTR2 )
				newTable[x].mParam[y].mInstructionID = INVALID_INSTR;
			else
				newTable[x].mParam[y].mInstructionID += firstHostCommandInstruction;
		}
	}
	
	sHostFunctions = newTable;
}


// -----------------------------------------------------------------------------
//	Parse:
//		Main entrypoint. This takes a script that's been tokenised and generates
//		the proper parse tree.
// -----------------------------------------------------------------------------

void	CParser::Parse( const char* fname, std::deque<CToken>& tokens, CParseTree& parseTree )
{
	// -------------------------------------------------------------------------
	// First recursively parse our script for top-level constructs:
	//	(functions, commands, CompileIt-style globals, whatever...)
	std::deque<CToken>::iterator	tokenItty = tokens.begin();
	
	mFileName = fname;
	
	while( tokenItty != tokens.end() )
	{
		try
		{
			ParseTopLevelConstruct( tokenItty, tokens, parseTree );
		}
		catch( const CForgeParseErrorProcessed& err )
		{
			// Message has already been pushed, just try to find the next handler start or so.
		}
		catch( const CForgeParseError& err )
		{
			sLastErrorFunction = NULL;
			mMessages.push_back( CMessageEntry( err.what(), mFileName, err.GetLineNum(), err.GetOffset() ) );
			throw;	// Re-throw, don't really know how to postpone this error until runtime.
		}
	}
	
	sLastErrorFunction = NULL;
}


void	CParser::ParseCommandOrExpression( const char* fname, std::deque<CToken>& tokens, CParseTree& parseTree )
{
	bool						tryCommand = true;
	CFunctionDefinitionNode*	currFunctionNode = NULL;
	try
	{
		std::deque<CToken>::iterator	tokenItty = tokens.begin();
		std::string						handlerName( ":run" );
		mFileName = fname;
		
		currFunctionNode = new CFunctionDefinitionNode( &parseTree, true, handlerName, handlerName, 1 );
		
		// Make built-in system variables so they get declared below like other local vars:
		currFunctionNode->AddLocalVar( "result", "result", TVariantTypeEmptyString, false, false, false, false );
		
		size_t		endLineNum = 1;
		ParseFunctionBody( handlerName, parseTree, currFunctionNode, tokenItty, tokens, NULL, ENewlineOperator, true );	// Parse one line as an expression wrapped in a "return" statement.
		currFunctionNode->SetEndLineNum( endLineNum );
		
		if( tokenItty != tokens.end() )	// Didn't parse all of it? Guess it wasn't a command :-S
		{
			if( currFunctionNode )
				delete currFunctionNode;	// Throw all of this away.
			tryCommand = true;	// Try parsing a command below.
		}
		else	// Parsed all of it! Good, add it to our script properly:
		{
			if( mFirstHandlerName.length() == 0 )
			{
				mFirstHandlerName = handlerName;
				mFirstHandlerIsFunction = false;
			}

			parseTree.AddNode( currFunctionNode );
			tryCommand = false;
		}
	}
	catch( const std::exception& err )	// Error? Guess it wasn't an expression after all.
	{
		if( currFunctionNode )
			delete currFunctionNode;	// Throw any previous parse progress away.
		tryCommand = true;	// Try parsing a command below.
	}
	
	if( tryCommand )
	{
		std::deque<CToken>::iterator	tokenItty = tokens.begin();
		std::string						handlerName( ":run" );
		
		currFunctionNode = new CFunctionDefinitionNode( &parseTree, true, handlerName, handlerName, 1 );
		
		// Make built-in system variables so they get declared below like other local vars:
		currFunctionNode->AddLocalVar( "result", "result", TVariantTypeEmptyString, false, false, false, false );

		size_t		endLineNum = 1;
		ParseFunctionBody( handlerName, parseTree, currFunctionNode, tokenItty, tokens, NULL, ENewlineOperator );
		currFunctionNode->SetEndLineNum( endLineNum );
		
		mFileName = fname;
		if( mFirstHandlerName.length() == 0 )
		{
			mFirstHandlerName = handlerName;
			mFirstHandlerIsFunction = false;
		}

		parseTree.AddNode( currFunctionNode );
	}
}


void	CParser::ParseTopLevelConstruct( std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens, CParseTree& parseTree )
{
	if( tokenItty == tokens.end() )
		;
	else if( tokenItty->IsIdentifier( ENewlineOperator ) )
	{
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip the newline.
	}
	else if( tokenItty->IsIdentifier( EFunctionIdentifier ) )
	{
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "function" 
		ParseFunctionDefinition( false, tokenItty, tokens, parseTree );
	}
	else if( tokenItty->IsIdentifier( EOnIdentifier ) || tokenItty->IsIdentifier( EWhenIdentifier ) )
	{
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "on"/"when"
		ParseFunctionDefinition( true, tokenItty, tokens, parseTree );
	}
	else if( tokenItty->IsIdentifier( EToIdentifier ) )
	{
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "to" 
		ParseFunctionDefinition( true, tokenItty, tokens, parseTree );
	}
	else
	{
		std::stringstream errMsg;
		errMsg << mFileName << ":" << tokenItty->mLineNum << ": warning: Skipping \"" << tokenItty->GetShortDescription() << "\".";
		
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Just skip it, whatever it may be.
		while( !tokenItty->IsIdentifier( ENewlineOperator ) && tokenItty != tokens.end() )	// Now skip until the end of the line.
		{
			errMsg << " " << tokenItty->GetShortDescription();
			CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
		}
		errMsg << "." << std::endl;
		
		mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
	}
}


void	CParser::ParseFunctionDefinition( bool isCommand, std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens, CParseTree& parseTree )
{
	std::string								handlerName( tokenItty->GetIdentifierText() );
	std::string								userHandlerName( tokenItty->GetIdentifierText() );
	std::stringstream						fcnHeader;
	std::stringstream						fcnSignature;
	size_t									fcnLineNum = tokenItty->mLineNum;
	
	if( sLastErrorFunction )	// Last function had an error and never ended?
	{
		sLastErrorFunction->SetEndLineNum(fcnLineNum);	// We're now parsing a new function, so make sure we undo its indentation in the code formatter.
		sLastErrorFunction = NULL;	// All good now.
	}
	
	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );

	if( mFirstHandlerName.length() == 0 )
	{
		mFirstHandlerName = handlerName;
		mFirstHandlerIsFunction = !isCommand;
	}
	
	CFunctionDefinitionNode*		currFunctionNode = NULL;
	currFunctionNode = new CFunctionDefinitionNode( &parseTree, isCommand, handlerName, userHandlerName, fcnLineNum );
	parseTree.AddNode( currFunctionNode );
	
	// Make built-in system variables so they get declared below like other local vars:
	currFunctionNode->AddLocalVar( "result", "result", TVariantTypeEmptyString, false, false, false, false );

	int		currParamIdx = 0;
	
	while( !tokenItty->IsIdentifier( ENewlineOperator ) )
	{
		std::string	realVarName( tokenItty->GetIdentifierText() );
		std::string	varName("var_");
		varName.append( realVarName );
		CCommandNode*		theVarCopyCommand = new CGetParamCommandNode( &parseTree, tokenItty->mLineNum );
		theVarCopyCommand->AddParam( new CLocalVariableRefValueNode(&parseTree, currFunctionNode, varName, realVarName, tokenItty->mLineNum) );
		theVarCopyCommand->AddParam( new CIntValueNode( &parseTree, currParamIdx++, tokenItty->mLineNum ) );
		currFunctionNode->AddCommand( theVarCopyCommand );
		
		currFunctionNode->AddLocalVar( varName, realVarName, TVariantTypeEmptyString, false, true, false );	// Create param var and mark as parameter in variable list.
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
		if( !tokenItty->IsIdentifier( ECommaOperator ) )
		{
			if( tokenItty->IsIdentifier( ENewlineOperator ) )
				break;
			std::stringstream		errMsg;
			errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected comma or end of line here, found "
									<< tokenItty->GetShortDescription() << ".";
			
			mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
			throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
		}
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	}
	
	while( tokenItty->IsIdentifier( ENewlineOperator ) )
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	
	try
	{
		size_t		endLineNum = fcnLineNum;
		currFunctionNode->SetCommandsLineNum( tokenItty->mLineNum );
		ParseFunctionBody( userHandlerName, parseTree, currFunctionNode, tokenItty, tokens, &endLineNum );
		currFunctionNode->SetEndLineNum( endLineNum );
	}
	catch( const CForgeParseError& err )
	{
		sLastErrorFunction = currFunctionNode;
		mMessages.push_back( CMessageEntry( err.what(), mFileName, err.GetLineNum(), err.GetOffset() ) );
		
		CParseErrorCommandNode	*	theErrorCmd = new CParseErrorCommandNode( &parseTree, err.what(), mFileName, err.GetLineNum(), err.GetOffset() );
		currFunctionNode->AddCommand( theErrorCmd );
		
		throw CForgeParseErrorProcessed( err );
	}
}


CValueNode	*	CParser::ParseFunctionCall( CParseTree& parseTree, CCodeBlockNodeBase* currFunction, bool isMessagePassing, std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens )
{
	CValueNode*	theTerm = NULL;
	std::string	handlerName( tokenItty->GetIdentifierText() );
	std::string	realHandlerName( tokenItty->GetOriginalIdentifierText() );
	size_t		callLineNum = tokenItty->mLineNum;
	
	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	
	if( tokenItty->IsIdentifier(EOpenBracketOperator) )	// Yes! Function call!
	{
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip opening bracket.
		
		std::map<std::string,CObjCMethodEntry>::iterator funcItty = sCFunctionTable.find( realHandlerName );
		if( funcItty == sCFunctionTable.end() )	// No native function of that name? Call function handler:
		{
			CFunctionCallNode*	fcall = new CFunctionCallNode( &parseTree, false, handlerName, callLineNum );
			if( isMessagePassing )
				fcall->SetIsMessagePassing(true);
			theTerm = fcall;
			ParseParamList( ECloseBracketOperator, parseTree, currFunction, tokenItty, tokens, fcall );
			
			CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip closing bracket.
		}
		else if( !isMessagePassing )	// Native call!
			theTerm = ParseNativeFunctionCallStartingAtParams( realHandlerName, funcItty->second, parseTree, currFunction, tokenItty, tokens );
	}
	else
		CTokenizer::GoPreviousToken( mFileName, tokenItty, tokens );	// Backtrack over what wasn't a bracket.
	
	return theTerm;
}

void	CParser::ParsePassStatement( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
									std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens )
{
	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "pass".
	
	CFunctionDefinitionNode* theFunction = dynamic_cast<CFunctionDefinitionNode*>( currFunction->GetContainingFunction() );
	if( !theFunction )
	{
		std::stringstream		errMsg;
		errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Can only pass messages in command or function handlers.";
		mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
		throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
	}
	
	if( theFunction->IsCommand() )
		ParseHandlerCall( parseTree, currFunction, true, tokenItty, tokens );
	else
	{
		CCommandNode*	theReturnCommand = new CReturnCommandNode( &parseTree, tokenItty->mLineNum );
		CValueNode*		theWhatNode = ParseFunctionCall( parseTree, currFunction, true, tokenItty, tokens );
		theReturnCommand->AddParam( theWhatNode );
		
		currFunction->AddCommand( theReturnCommand );
	}
}


void	CParser::ParseHandlerCall( CParseTree& parseTree, CCodeBlockNodeBase* currFunction, bool isMessagePassing, std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens )
{
	std::string	handlerName;
	size_t		currLineNum = tokenItty->mLineNum;
	
	handlerName.append( tokenItty->GetIdentifierText() );
	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );

	CFunctionCallNode*	currFunctionCall = new CFunctionCallNode( &parseTree, true, handlerName, currLineNum );
	ParseParamList( ENewlineOperator, parseTree, currFunction, tokenItty, tokens, currFunctionCall );
	
	CCommandNode*			theVarAssignCommand = NULL;
	if( isMessagePassing )
	{
		currFunctionCall->SetIsMessagePassing( true );
		theVarAssignCommand = new CReturnCommandNode( &parseTree, tokenItty->mLineNum );
		theVarAssignCommand->AddParam( currFunctionCall );
	}
	else
	{
		theVarAssignCommand = new CAssignCommandNode( &parseTree, currLineNum );
		theVarAssignCommand->AddParam( new CLocalVariableRefValueNode(&parseTree, currFunction, "result", "result", tokenItty->mLineNum) );
		theVarAssignCommand->AddParam( currFunctionCall );
	}
	currFunction->AddCommand( theVarAssignCommand );
}


void	CParser::ParsePutStatement( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
								std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens )
{
	// Put:
	CCommandNode*			thePutCommand = NULL;
	CNode*					resultNode = NULL;
	size_t					startLine = tokenItty->mLineNum;
	
	try {
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
		
		// What:
		CValueNode*	whatExpression = ParseExpression( parseTree, currFunction, tokenItty, tokens, ELastIdentifier_Sentinel );
		
		// [into|after|before]
		if( tokenItty->IsIdentifier( EIntoIdentifier ) )
		{
			resultNode = thePutCommand = new CPutCommandNode( &parseTree, startLine );
			thePutCommand->AddParam( whatExpression );
			CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
			
			// container:
			CValueNode*	destContainer = ParseContainer( false, false, parseTree, currFunction, tokenItty, tokens, ELastIdentifier_Sentinel );
			thePutCommand->AddParam( destContainer );
			resultNode = thePutCommand;
		}
		else if( tokenItty->IsIdentifier( EAfterIdentifier ) )
		{
			CTokenizer::GoNextToken( mFileName, tokenItty, tokens );

			CValueNode*	destContainer = ParseContainer( false, false, parseTree, currFunction, tokenItty, tokens, ELastIdentifier_Sentinel );
			
			resultNode = thePutCommand = new CPutCommandNode( &parseTree, startLine );
			COperatorNode	*	concatOperation = new COperatorNode( &parseTree, CONCATENATE_VALUES_INSTR, startLine );
			concatOperation->AddParam( destContainer->Copy() );
			concatOperation->AddParam( whatExpression );
			thePutCommand->AddParam( concatOperation );
			thePutCommand->AddParam( destContainer );
			resultNode = thePutCommand;
		}
		else if( tokenItty->IsIdentifier( EBeforeIdentifier ) )
		{
			CTokenizer::GoNextToken( mFileName, tokenItty, tokens );

			CValueNode*	destContainer = ParseContainer( false, false, parseTree, currFunction, tokenItty, tokens, ELastIdentifier_Sentinel );
			
			resultNode = thePutCommand = new CPutCommandNode( &parseTree, startLine );
			COperatorNode	*	concatOperation = new COperatorNode( &parseTree, CONCATENATE_VALUES_INSTR, startLine );
			concatOperation->AddParam( whatExpression );
			concatOperation->AddParam( destContainer->Copy() );
			thePutCommand->AddParam( concatOperation );
			thePutCommand->AddParam( destContainer );
		}
		else
		{
			// Look for a host command named "put" with exactly 1 parameter & use instruction from that:
			LEOInstructionID	printInstrID = INVALID_INSTR;
			uint16_t			param1 = 0;
			uint32_t			param2 = 0;
			
			if( sHostCommands )
			{
				for( size_t x = 0; sHostCommands[x].mType != ELastIdentifier_Sentinel; x++ )
				{
					if( sHostCommands[x].mType == EPutIdentifier )
					{
						printInstrID = sHostCommands[x].mInstructionID;
						param1 = sHostCommands[x].mInstructionParam1;
						param2 = sHostCommands[x].mInstructionParam2;
						if( sHostCommands[x].mParam[0].mType == EHostParamExpression && sHostCommands[x].mParam[1].mType == EHostParam_Sentinel )
						{
							if( sHostCommands[x].mParam[0].mInstructionID != INVALID_INSTR )
							{
								printInstrID = sHostCommands[x].mParam[0].mInstructionID;
								param1 = sHostCommands[x].mParam[0].mInstructionParam1;
								param2 = sHostCommands[x].mParam[0].mInstructionParam2;
							}
							break;
						}
						else
							printInstrID = INVALID_INSTR;
					}
				}
			}
			
			// Found one?
			if( printInstrID != INVALID_INSTR )
			{
				COperatorNode* opNode = new COperatorNode( &parseTree, printInstrID, startLine );
				resultNode = opNode;
				opNode->SetInstructionParams( param1, param2 );
				opNode->AddParam( whatExpression );
			}
			else
			{
				std::stringstream		errMsg;
				errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: expected \"into\", \"before\" or \"after\" here, found "
										<< tokenItty->GetShortDescription() << ".";
				mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
				CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
			}
		}
		
		currFunction->AddCommand( resultNode );
	}
	catch( ... )
	{
		if( resultNode )
			delete resultNode;
		
		throw;
	}
}


void	CParser::ParseSetStatement( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
									std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens )
{
	// Set:
	CCommandNode*	thePutCommand = NULL;
	size_t			startLine = tokenItty->mLineNum;
	
	try {
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
		
		// container:
		CValueNode*	destContainer = ParseContainer( false, false, parseTree, currFunction, tokenItty, tokens, EToIdentifier );
		
		// to:
		if( !tokenItty->IsIdentifier( EToIdentifier ) )
		{
			std::stringstream		errMsg;
			errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: expected \"to\" here, found "
									<< tokenItty->GetShortDescription() << ".";
			mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
			throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
		}
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "to".

		// what:
		CValueNode*	whatExpression = ParseExpression( parseTree, currFunction, tokenItty, tokens, ELastIdentifier_Sentinel );
		
		// Just build a put command:
		thePutCommand = new CPutCommandNode( &parseTree, startLine );
		thePutCommand->AddParam( whatExpression );
		thePutCommand->AddParam( destContainer );
		
		currFunction->AddCommand( thePutCommand );
	}
	catch( ... )
	{
		if( thePutCommand )
			delete thePutCommand;
		
		// TODO: Don't leak destContainer and whatExpression if error before they're added to command.
		
		throw;
	}
}


CValueNode*	CParser::ParseHostFunction( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
									std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens )
{
	return ParseHostEntityWithTable( parseTree, currFunction, tokenItty, tokens, sHostFunctions );
}


void	CParser::ParseHostCommand( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
									std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens )
{
	CNode*	theNode = ParseHostEntityWithTable( parseTree, currFunction,
												tokenItty, tokens, sHostCommands );
	if( theNode )
		currFunction->AddCommand( theNode );
	else if( tokenItty != tokens.end() && tokenItty->IsIdentifier(EEndIdentifier) )
	{
		std::stringstream		errMsg;
		errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected handler call here, found \""
							<< tokenItty->GetShortDescription() << "\".";
		mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
		throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
	}
	else
		ParseHandlerCall( parseTree, currFunction, false, tokenItty, tokens );
}


#if DEBUG_HOST_ENTITIES
#define HE_PRINT(args...)	printf(args)
#else
#define HE_PRINT(...)		do{}while(0)
#endif


CValueNode*	CParser::ParseHostEntityWithTable( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
									std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens,
									THostCommandEntry* inHostTable )
{
	CValueNode			*theNode = NULL;
	TIdentifierSubtype	firstIdentifier = tokenItty->GetIdentifierSubType();
	
	if( inHostTable != NULL )
	{
		for( size_t commandIdx = 0; inHostTable[commandIdx].mType != ELastIdentifier_Sentinel; commandIdx++ )
		{
			THostCommandEntry	*	currCmd = inHostTable +commandIdx;
			if( currCmd->mType == firstIdentifier )
			{
				HE_PRINT("First identifier match: \"%s\"\n", tokenItty->GetOriginalIdentifierText().c_str());
				long long			identifiersToBacktrack = 0;
				CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
				identifiersToBacktrack++;
				
				uint8_t					currMode = '\0';
				THostCommandEntry*		cmd = inHostTable + commandIdx;
				THostParameterEntry*	par = cmd->mParam;
				COperatorNode*			hostCommand = new COperatorNode( &parseTree, cmd->mInstructionID, tokenItty->mLineNum );
				hostCommand->SetInstructionParams( cmd->mInstructionParam1, cmd->mInstructionParam2 );
				theNode = hostCommand;
				bool	abortThisCommand = false;
				
				while( par->mType != EHostParam_Sentinel && !abortThisCommand )
				{
					if( par->mModeRequired == '\0' || par->mModeRequired == currMode )
					{
						switch( par->mType )
						{
							case EHostParamImmediateValue:
							{
								HE_PRINT("\tImmediate value\n");
								CValueNode	*	term = ParseTerm( parseTree, currFunction, tokenItty, tokens, par->mIdentifierType );
								if( !term && par->mIsOptional )
								{
									HE_PRINT("\t\tNot found\n");
									if( par->mInstructionID == INVALID_INSTR )
									{
										hostCommand->AddParam( new CStringValueNode( &parseTree, "", tokenItty->mLineNum ) );
										HE_PRINT("\t\tAdding empty string because no instruction has been set to otherwise indicate this value isn't there.\n");
									}
								}
								else if( !term )
								{
									HE_PRINT("\t\tNot found.\n");
									delete hostCommand;
									hostCommand = NULL;
									theNode = NULL;
									abortThisCommand = true;
									std::stringstream		errMsg;
									if( tokenItty != tokens.end() )
									{
										errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected term here, found \""
																<< tokenItty->GetShortDescription() << "\".";
									}
									else
									{
										--tokenItty;
										errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected term here.";
									}
									mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
									HE_PRINT("\t\tTHROWING: %s\n",errMsg.str().c_str());
									throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
								}
								else
								{
									HE_PRINT("\t\tFound a term\n");
									hostCommand->AddParam( term );
									if( par->mInstructionID != INVALID_INSTR )
									{
										HE_PRINT("\t\tChanging instruction type to %d (%d,%d)\n", par->mInstructionID, par->mInstructionParam1, par->mInstructionParam2);
										hostCommand->SetInstructionID( par->mInstructionID );
										hostCommand->SetInstructionParams( par->mInstructionParam1, par->mInstructionParam2 );
									}
									if( par->mModeToSet != 0 )
									{
										HE_PRINT("\t\tChanging mode to '%c'\n", par->mModeToSet);
										currMode = par->mModeToSet;
									}
								}
								HE_PRINT("\t\tForget about backtracking.\n");
								identifiersToBacktrack = -1;
								break;
							}

							case EHostParamContainer:
							{
								HE_PRINT("\tContainer\n");
								CValueNode	*	term = ParseContainer( false, false, parseTree, currFunction, tokenItty, tokens, par->mIdentifierType );
								if( !term && par->mIsOptional )
								{
									HE_PRINT("\t\tNot found.\n");
									if( par->mInstructionID == INVALID_INSTR )
									{
										hostCommand->AddParam( new CStringValueNode( &parseTree, "", tokenItty->mLineNum ) );
									}
								}
								else if( !term )
								{
									HE_PRINT("\t\tNot found.\n");
									delete hostCommand;
									hostCommand = NULL;
									theNode = NULL;
									abortThisCommand = true;
									std::stringstream		errMsg;
									if( tokenItty != tokens.end() )
									{
										errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected term here, found \""
																<< tokenItty->GetShortDescription() << "\".";
									}
									else
									{
										--tokenItty;
										errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected term here.";
									}
									mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
									HE_PRINT("\t\tTHROWING: %s\n",errMsg.str().c_str());
									throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
								}
								else
								{
									hostCommand->AddParam( term );
									if( par->mInstructionID != INVALID_INSTR )
									{
										hostCommand->SetInstructionID( par->mInstructionID );
										hostCommand->SetInstructionParams( par->mInstructionParam1, par->mInstructionParam2 );
									}
									if( par->mModeToSet != 0 )
									{
										HE_PRINT("\t\tChanging mode to '%c'\n", par->mModeToSet);
										currMode = par->mModeToSet;
									}
								}
								HE_PRINT("\t\tForget about backtracking.\n");
								identifiersToBacktrack = -1;
								break;
							}

							case EHostParamExpression:
							case EHostParamExpressionOrIdentifiersTillLineEnd:
							{
								HE_PRINT("\tExpression\n");
								CValueNode	*	term = ParseExpression( parseTree, currFunction, tokenItty, tokens, par->mIdentifierType );
								if( !term && par->mIsOptional )
								{
									HE_PRINT("\t\tNot found.\n");
									if( par->mInstructionID == INVALID_INSTR )
										hostCommand->AddParam( new CStringValueNode( &parseTree, "", tokenItty->mLineNum ) );
								}
								else if( !term )
								{
									HE_PRINT("\t\tNot found.\n");
									delete hostCommand;
									hostCommand = NULL;
									theNode = NULL;
									abortThisCommand = true;
									std::stringstream		errMsg;
									if( tokenItty != tokens.end() )
									{
										errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected expression here, found \""
																<< tokenItty->GetShortDescription() << "\".";
									}
									else
									{
										--tokenItty;
										errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected expression here.";
									}
									mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
									HE_PRINT("\t\tTHROWING: %s\n",errMsg.str().c_str());
									throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
								}
								else
								{
									// We parsed an expression, but only got an unquoted literal (variable) and that's not the end of the line? AND we're doing an identifier list?
									if( par->mType == EHostParamExpressionOrIdentifiersTillLineEnd && dynamic_cast<CLocalVariableRefValueNode*>(term) && tokenItty != tokens.end() && !tokenItty->IsIdentifier( ENewlineOperator ) )
									{
										HE_PRINT("\t\tIdentifiers till line end.\n");
										std::string		theStr = ((CLocalVariableRefValueNode*)term)->GetRealVarName();
										delete term;
										term = NULL;
										for( ; tokenItty != tokens.end() && tokenItty->mType == EIdentifierToken; tokenItty++ )
										{
											theStr.append( 1, ' ' );
											theStr.append( tokenItty->GetIdentifierText() );
										}
										
										term = new CStringValueNode( &parseTree, theStr, tokenItty->mLineNum );
									}

									hostCommand->AddParam( term );
									if( par->mInstructionID != INVALID_INSTR )
									{
										hostCommand->SetInstructionID( par->mInstructionID );
										hostCommand->SetInstructionParams( par->mInstructionParam1, par->mInstructionParam2 );
									}
									if( par->mModeToSet != 0 )
									{
										HE_PRINT("\t\tChanging mode to '%c'\n", par->mModeToSet);
										currMode = par->mModeToSet;
									}
								}
								HE_PRINT("\t\tForget about backtracking.\n");
								identifiersToBacktrack = -1;
								break;
							}

							case EHostParamIdentifier:
							case EHostParamInvisibleIdentifier:
							{
								if( par->mIdentifierType == ELastIdentifier_Sentinel )
									HE_PRINT("\tAny Identifier\n");
								else
									HE_PRINT("\tIdentifier \"%s\"\n", gIdentifierStrings[par->mIdentifierType]);
								if( (tokenItty->mType == EIdentifierToken && par->mIdentifierType == ELastIdentifier_Sentinel)
									|| tokenItty->IsIdentifier(par->mIdentifierType) )
								{
									if( par->mInstructionID == INVALID_INSTR )
									{
										if( par->mType != EHostParamInvisibleIdentifier )
										{
											HE_PRINT("\t\tAdding identifier with this string.\n");
											hostCommand->AddParam( new CStringValueNode( &parseTree, tokenItty->GetShortDescription(), tokenItty->mLineNum ) );
										}
										else
											HE_PRINT("\t\tInvisibble identifier accepted.\n");
									}
									else
									{
										HE_PRINT("\t\tSetting instruction %d (%d,%d)\n", par->mInstructionID, par->mInstructionParam1, par->mInstructionParam2 );
										hostCommand->SetInstructionID( par->mInstructionID );
										hostCommand->SetInstructionParams( par->mInstructionParam1, par->mInstructionParam2 );
									}
									CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
									if( par->mModeToSet != 0 )
									{
										HE_PRINT("\t\tChanging mode to '%c'\n", par->mModeToSet);
										currMode = par->mModeToSet;
									}
									if( identifiersToBacktrack >= 0 )
										identifiersToBacktrack++;
								}
								else if( par->mIsOptional )
								{
									HE_PRINT("\t\tNot found.\n");
									if( par->mInstructionID == INVALID_INSTR && par->mType != EHostParamInvisibleIdentifier )
									{
										HE_PRINT("\t\tSetting empty string parameter.\n");
										hostCommand->AddParam( new CStringValueNode( &parseTree, "", (tokenItty != tokens.end()) ? tokenItty->mLineNum  : 0) );
									}
								}
								else
								{
									HE_PRINT("\t\tNot found.\n");
									abortThisCommand = true;
									if( identifiersToBacktrack < 0 )
									{
										delete hostCommand;
										hostCommand = NULL;
										theNode = NULL;
										
										std::stringstream		errMsg;
										if( tokenItty != tokens.end() )
											errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected \"" << gIdentifierStrings[par->mIdentifierType] << "\" here, found \""
																<< tokenItty->GetShortDescription() << "\".";
										else
										{
											--tokenItty;
											errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected \"" << gIdentifierStrings[par->mIdentifierType] << "\" here.";
										}
										mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
										HE_PRINT("\t\tTHROWING: %s\n",errMsg.str().c_str());
										throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
									}
								}
								break;
							}

							case EHostParamLabeledValue:
							case EHostParamLabeledExpression:
							case EHostParamLabeledContainer:
							{
								if( par->mIdentifierType == ELastIdentifier_Sentinel )
									HE_PRINT("\tAny Identifier + ");
								else
									HE_PRINT("\tIdentifier \"%s\" +", gIdentifierStrings[par->mIdentifierType]);
								if( par->mType == EHostParamLabeledExpression
									|| par->mType == EHostParamExpressionOrIdentifiersTillLineEnd )
								{
									HE_PRINT("expression\n");
								}
								else if( par->mType == EHostParamLabeledContainer )
								{
									HE_PRINT("container\n");
								}
								else
								{
									HE_PRINT("term\n");
								}
								if( tokenItty->IsIdentifier(par->mIdentifierType) )
								{
									CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
									
									CValueNode	*	term = NULL;
									const char	*	valType = "term";
									if( par->mType == EHostParamLabeledExpression
										|| par->mType == EHostParamExpressionOrIdentifiersTillLineEnd )
									{
										term = ParseExpression( parseTree, currFunction, tokenItty, tokens, ELastIdentifier_Sentinel );
										valType = "expression";
									}
									else if( par->mType == EHostParamLabeledContainer )
									{
										term = ParseContainer( false, false, parseTree, currFunction, tokenItty, tokens, ELastIdentifier_Sentinel );
										valType = "container";
									}
									else
									{
										term = ParseTerm( parseTree, currFunction, tokenItty, tokens, ELastIdentifier_Sentinel );
									}
									if( !term )
									{
										HE_PRINT("\t\tNot Found.\n");
										delete hostCommand;
										hostCommand = NULL;
										theNode = NULL;
										abortThisCommand = true;
										std::stringstream		errMsg;
										if( tokenItty != tokens.end() )
										{
											errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected " << valType << " after \"" << gIdentifierStrings[par->mIdentifierType] << "\", found \""
																<< tokenItty->GetShortDescription() << "\".";
										}
										else
										{
											CTokenizer::GoPreviousToken(mFileName, tokenItty, tokens);
											errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected " << valType << " after \"" << gIdentifierStrings[par->mIdentifierType] << "\".";
										}
										mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
										HE_PRINT("\t\tTHROWING: %s\n",errMsg.str().c_str());
										throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
									}
									else
									{
										HE_PRINT("\t\tFound.\n");
										hostCommand->AddParam( term );
										if( par->mInstructionID != INVALID_INSTR )
										{
											HE_PRINT("\t\tSetting instruction %d (%d,%d)\n", par->mInstructionID, par->mInstructionParam1, par->mInstructionParam2 );
											hostCommand->SetInstructionID( par->mInstructionID );
											hostCommand->SetInstructionParams( par->mInstructionParam1, par->mInstructionParam2 );
										}
									}
									if( par->mModeToSet != 0 )
									{
										HE_PRINT("\t\tChanging mode to '%c'\n", par->mModeToSet);
										currMode = par->mModeToSet;
									}
								}
								else if( par->mIsOptional )
								{
									HE_PRINT("\t\tSetting empty string parameter.\n");
									hostCommand->AddParam( new CStringValueNode( &parseTree, "", tokenItty->mLineNum ) );
								}
								else
								{
									delete hostCommand;
									hostCommand = NULL;
									theNode = NULL;
									abortThisCommand = true;
									std::stringstream		errMsg;
									if( tokenItty != tokens.end() )
									{
										errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected \"" << gIdentifierStrings[par->mIdentifierType] << "\" here, found \""
															<< tokenItty->GetShortDescription() << "\".";
									}
									else
									{
										--tokenItty;
										errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected \"" << gIdentifierStrings[par->mIdentifierType] << "\" here.";
									}
									mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
									HE_PRINT("\t\tTHROWING: %s\n",errMsg.str().c_str());
									throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
									currMode = currCmd->mTerminalMode;	// Otherwise backtracking code below tries again & errors out.
								}
								identifiersToBacktrack = -1;
								break;
							}
							
							case EHostParam_Sentinel:
								HE_PRINT("\tEnd of param list\n");
								break;
						}
					}
					else
						HE_PRINT("\tSkipping, mode '%c' doesn't match '%c'\n", currMode, par->mModeRequired);
					
					par++;
				}
				
				if( currCmd->mTerminalMode != '\0' && currMode != currCmd->mTerminalMode )
				{
					HE_PRINT("\tDidn't encounter end mode\n");
					if( hostCommand )	// Should have one by now, so if none, backtracked.
					{
						delete hostCommand;
						theNode = NULL;
						hostCommand = NULL;
					}
					if( identifiersToBacktrack >= 0 )
					{
						HE_PRINT("\t\tBacktracking %lld\n",identifiersToBacktrack);
						for( long long x = 0; x < identifiersToBacktrack; x++ )
							CTokenizer::GoPreviousToken( mFileName, tokenItty, tokens );
						// Now that we've backtracked, try the next host command.
					}
					else
					{
						std::stringstream		errMsg;
						errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Unexpected  \""
							<< tokenItty->GetShortDescription() << "\" following \"" << gIdentifierStrings[currCmd->mType] << "\".";
						mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
						HE_PRINT("\t\tTHROWING: %s\n",errMsg.str().c_str());
						throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
					}
				}
				else
				{
					HE_PRINT("\tFOUND A MATCH.\n");
					break;	// Found a command that matches, stop looping.
				}
			}
		}
	}
	
	return theNode;
}


void	CParser::ParseGlobalStatement( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens )
{
	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "global".
	
	std::string		globalName( "var_" );
	globalName.append( tokenItty->GetIdentifierText() );
	
	currFunction->AddLocalVar( globalName, tokenItty->GetIdentifierText(), TVariantType_INVALID, false, false, true );
	
	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip global name.
}


void	CParser::ParseGetStatement( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
									std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens )
{
	CCommandNode*	thePutCommand = new CPutCommandNode( &parseTree, tokenItty->mLineNum );
	
	// We map "get" to "put <what> into it":
	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "get".
	
	// What:
	CValueNode*	theWhatNode = ParseExpression( parseTree, currFunction, tokenItty, tokens, ELastIdentifier_Sentinel );
	thePutCommand->AddParam( theWhatNode );
		
	// Make sure we have an "it":
	CreateVariable( "var_it", "it", false, currFunction );
	thePutCommand->AddParam( new CLocalVariableRefValueNode( &parseTree, currFunction, "var_it", "it", tokenItty->mLineNum ) );
	
	currFunction->AddCommand( thePutCommand );
}


void	CParser::ParseReturnStatement( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens )
{
	CCommandNode*	theReturnCommand = new CReturnCommandNode( &parseTree, tokenItty->mLineNum );
	
	// Return:
	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	
	// What:
	CValueNode*	theWhatNode = ParseExpression( parseTree, currFunction, tokenItty, tokens, ELastIdentifier_Sentinel );
	theReturnCommand->AddParam( theWhatNode );
	
	currFunction->AddCommand( theReturnCommand );
}


void	CParser::ParseDownloadStatement( std::string& userHandlerName, CParseTree& parseTree,
										CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens )
{
	CDownloadCommandNode*	theDownloadCommand = new CDownloadCommandNode( &parseTree, tokenItty->mLineNum );
	currFunction->AddCommand( theDownloadCommand );
	
	// Download:
	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	
	// What:
	CValueNode*	theWhatNode = ParseExpression( parseTree, currFunction, tokenItty, tokens, ELastIdentifier_Sentinel );
	theDownloadCommand->AddParam( theWhatNode );
	
	// [In]to:
	if( !tokenItty->IsIdentifier( EIntoIdentifier ) && !tokenItty->IsIdentifier( EToIdentifier ) )
	{
		std::stringstream		errMsg;
		errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected \"into\" here, found "
								<< tokenItty->GetShortDescription() << ".";
		mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
		throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
	}
	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	
	// Dest:
	CValueNode*	theContainerNode = ParseContainer( false, false, parseTree, currFunction, tokenItty, tokens, ELastIdentifier_Sentinel );
	theDownloadCommand->AddParam( theContainerNode );
	
	bool		needEndDownload = false;
	bool		haveProgressBlock = false;
	bool		haveCompletionBlock = false;
	
	// For ease of reading, we allow the 'for' and 'when' clauses on a new line, if desired:
	if( tokenItty->IsIdentifier( ENewlineOperator ) )	// +++ Cope with more than 1 line break.
	{
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
		if( !tokenItty->IsIdentifier( EForIdentifier ) && !tokenItty->IsIdentifier( EWhenIdentifier ) )
			CTokenizer::GoPreviousToken( mFileName, tokenItty, tokens );
	}
	
	std::deque<CToken>::iterator	beforeReturnPos;
	
	// For each chunk:
	if( tokenItty->IsIdentifier( EForIdentifier ) )
	{
		theDownloadCommand->SetProgressLineNum( tokenItty->mLineNum );
		
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	
		if( !tokenItty->IsIdentifier(EEachIdentifier) )
		{
			std::stringstream		errMsg;
			errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected \"each chunk\" after \"for\" here, found "
									<< tokenItty->GetShortDescription() << ".";
			mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
			throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
		}
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	
		if( !tokenItty->IsIdentifier(EChunkIdentifier) )
		{
			std::stringstream		errMsg;
			errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected \"chunk\" after \"for each\" here, found "
									<< tokenItty->GetShortDescription() << ".";
			mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
			throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
		}
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
		
		CCodeBlockNodeBase*		progressNode = theDownloadCommand->CreateProgressBlock( tokenItty->mLineNum );

		// Make sure function declares "the result" for use by handler calls:
		progressNode->AddLocalVar( "result", "result", TVariantTypeEmptyString, false, false, false, false );

		// Make sure parameter 1 is available under "the download". It's an array
		//	containing info like download size (current, total) etc.:
		std::string	realVarName( "download" );
		std::string	varName("download");
		CCommandNode*		theVarCopyCommand = new CGetParamCommandNode( &parseTree, tokenItty->mLineNum );
		theVarCopyCommand->AddParam( new CLocalVariableRefValueNode(&parseTree, progressNode, varName, realVarName, tokenItty->mLineNum) );
		theVarCopyCommand->AddParam( new CIntValueNode( &parseTree, 0, tokenItty->mLineNum ) );
		progressNode->AddCommand( theVarCopyCommand );
		
		progressNode->AddLocalVar( varName, realVarName, TVariantTypeEmptyString, false, true, false );	// Create param var and mark as parameter in variable list.

		if( tokenItty->IsIdentifier( ENewlineOperator ) )
		{
			CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
			needEndDownload = true;
			
			// Commands:
			theDownloadCommand->SetProgressCommandsLineNum( tokenItty->mLineNum );
			while( !tokenItty->IsIdentifier( EEndIdentifier ) && !tokenItty->IsIdentifier( EWhenIdentifier ) )
			{
				ParseOneLine( userHandlerName, parseTree, progressNode, tokenItty, tokens );
			}
			beforeReturnPos = tokenItty;
		}
		else
		{
			theDownloadCommand->SetProgressCommandsLineNum( tokenItty->mLineNum );
			ParseOneLine( userHandlerName, parseTree, progressNode, tokenItty, tokens, true );
			beforeReturnPos = tokenItty;
			if( tokenItty->IsIdentifier(ENewlineOperator) )
				CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
		}
		
		haveProgressBlock = true;
	}
	else
		beforeReturnPos = tokenItty;
	
	// when done:
	if( tokenItty->IsIdentifier( EWhenIdentifier ) )
	{
		theDownloadCommand->SetCompletionLineNum( tokenItty->mLineNum );

		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	
		if( !tokenItty->IsIdentifier(EDoneIdentifier) )
		{
			std::stringstream		errMsg;
			errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected \"done\" after \"when\" here, found "
									<< tokenItty->GetShortDescription() << ".";
			mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
			throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
		}
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
		
		CCodeBlockNodeBase*		completionNode = theDownloadCommand->CreateCompletionBlock( tokenItty->mLineNum );
		
		// Make sure function declares "the result" for use by handler calls:
		completionNode->AddLocalVar( "result", "result", TVariantTypeEmptyString, false, false, false, false );

		// Make sure parameter 1 is available under "the download". It's an array
		//	containing info like download size (current, total) etc.:
		std::string	realVarName( "download" );
		std::string	varName("download");
		CCommandNode*		theVarCopyCommand = new CGetParamCommandNode( &parseTree, tokenItty->mLineNum );
		theVarCopyCommand->AddParam( new CLocalVariableRefValueNode(&parseTree, completionNode, varName, realVarName, tokenItty->mLineNum) );
		theVarCopyCommand->AddParam( new CIntValueNode( &parseTree, 0, tokenItty->mLineNum ) );
		completionNode->AddCommand( theVarCopyCommand );
		
		completionNode->AddLocalVar( varName, realVarName, TVariantTypeEmptyString, false, true, false );	// Create param var and mark as parameter in variable list.

		if( tokenItty->IsIdentifier( ENewlineOperator ) )
		{
			CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
			needEndDownload = true;
			
			// Commands:
			theDownloadCommand->SetCompletionCommandsLineNum( tokenItty->mLineNum );
			while( !tokenItty->IsIdentifier( EEndIdentifier ) )
			{
				ParseOneLine( userHandlerName, parseTree, completionNode, tokenItty, tokens );
			}
		}
		else
		{
			needEndDownload = false;	// The previous guy may have wanted an 'end'. She gets the 'when done' clause instead.
			theDownloadCommand->SetCompletionCommandsLineNum( tokenItty->mLineNum );
			ParseOneLine( userHandlerName, parseTree, completionNode, tokenItty, tokens, true );
		}
		
		haveCompletionBlock = true;
	}
	else
		tokenItty = beforeReturnPos;
	
	if( needEndDownload )
	{
		theDownloadCommand->SetEndDownloadLineNum( tokenItty->mLineNum );

		if( !tokenItty->IsIdentifier(EEndIdentifier) )
		{
			std::stringstream		errMsg;
			
			errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected \"end download\" here, found "
									<< tokenItty->GetShortDescription() << ".";
			mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
			throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
		}
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
		
		if( !tokenItty->IsIdentifier(EDownloadIdentifier) )
		{
			std::stringstream		errMsg;
			
			errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected \"download\" after \"end\" here, found "
									<< tokenItty->GetShortDescription() << ".";
			mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
			throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
		}
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	}
	else
		theDownloadCommand->SetEndDownloadLineNum( tokenItty->mLineNum );
	
	if( !tokenItty->IsIdentifier(ENewlineOperator) )
	{
		const char*		expectations = "end of line";
		if( !haveCompletionBlock && haveProgressBlock )
			expectations = "\"when done\" or end of line";
		else if( !haveCompletionBlock && !haveProgressBlock )
			expectations = "\"for each chunk\", \"when done\" or end of line";
		std::stringstream		errMsg;
		errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected " << expectations << " here, found "
								<< tokenItty->GetShortDescription() << ".";
		mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
		throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
	}
}


void	CParser::ParseAddStatement( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens )
{
	CCommandNode*	theAddCommand = new CAddCommandNode( &parseTree, tokenItty->mLineNum );
	
	// Add:
	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	
	// What:
	CValueNode*	theWhatNode = ParseExpression( parseTree, currFunction, tokenItty, tokens, EToIdentifier );
	
	// To:
	if( !tokenItty->IsIdentifier( EToIdentifier ) )
	{
		delete theWhatNode;
		std::stringstream		errMsg;
		errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected \"to\" here, found "
								<< tokenItty->GetShortDescription() << ".";
		mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
		throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
	}
	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	
	// Dest:
	CValueNode*	theContainerNode = ParseContainer( false, false, parseTree, currFunction, tokenItty, tokens, ELastIdentifier_Sentinel );
	theAddCommand->AddParam( theContainerNode );
	theAddCommand->AddParam( theWhatNode );
	
	currFunction->AddCommand( theAddCommand );
}


void	CParser::ParseSubtractStatement( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens )
{
	CCommandNode*	theAddCommand = new CSubtractCommandNode( &parseTree, tokenItty->mLineNum );
	
	// Subtract:
	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	
	// What:
	CValueNode*	theWhatNode = ParseExpression( parseTree, currFunction, tokenItty, tokens, EFromIdentifier );
	theAddCommand->AddParam( theWhatNode );
	
	// From:
	if( !tokenItty->IsIdentifier( EFromIdentifier ) )
	{
		std::stringstream		errMsg;
		errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected \"from\" here, found "
								<< tokenItty->GetShortDescription() << ".";
		mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
		throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
	}
	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	
	// Dest:
	CValueNode*	theContainerNode = ParseContainer( false, false, parseTree, currFunction, tokenItty, tokens, ELastIdentifier_Sentinel );
	theAddCommand->AddParam( theContainerNode );
	
	currFunction->AddCommand( theAddCommand );
}


void	CParser::ParseMultiplyStatement( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens )
{
	CCommandNode*	theAddCommand = new CMultiplyCommandNode( &parseTree, tokenItty->mLineNum );
	
	// Multiply:
	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	
	// Dest:
	CValueNode*	theContainerNode = ParseContainer( false, false, parseTree, currFunction, tokenItty, tokens, EWithIdentifier );
	theAddCommand->AddParam( theContainerNode );
	
	// With:
	if( !tokenItty->IsIdentifier( EWithIdentifier ) && !tokenItty->IsIdentifier( EByIdentifier ) )
	{
		std::stringstream		errMsg;
		errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected \"with\" or \"by\" here, found "
								<< tokenItty->GetShortDescription() << ".";
		mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
		throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
	}
	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	
	// What:
	CValueNode*	theWhatNode = ParseExpression( parseTree, currFunction, tokenItty, tokens, ELastIdentifier_Sentinel );
	theAddCommand->AddParam( theWhatNode );
	
	currFunction->AddCommand( theAddCommand );
}


void	CParser::ParseDivideStatement( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens )
{
	CCommandNode*	theAddCommand = new CDivideCommandNode( &parseTree, tokenItty->mLineNum );
	
	// Divide:
	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	
	// Dest:
	CValueNode*	theContainerNode = ParseContainer( false, false, parseTree, currFunction, tokenItty, tokens, EByIdentifier );
	theAddCommand->AddParam( theContainerNode );
	
	// By:
	if( !tokenItty->IsIdentifier( EByIdentifier ) )
	{
		std::stringstream		errMsg;
		errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected \"by\" here, found "
								<< tokenItty->GetShortDescription() << ".";
		mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
		throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
	}
	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	
	// What:
	CValueNode*	theWhatNode = ParseExpression( parseTree, currFunction, tokenItty, tokens, ELastIdentifier_Sentinel );
	theAddCommand->AddParam( theWhatNode );
	
	currFunction->AddCommand( theAddCommand );
}


// When you enter this, "repeat for each" has already been parsed, and you should be at the chunk type token:
void	CParser::ParseRepeatForEachStatement( std::string& userHandlerName, CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens )
{
	// chunk type:
	TChunkType	chunkTypeConstant = GetChunkTypeNameFromIdentifierSubtype( tokenItty->GetIdentifierSubType() );
	if( chunkTypeConstant == TChunkTypeInvalid )
	{
		std::stringstream		errMsg;
		errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected chunk type identifier here, found "
								<< tokenItty->GetShortDescription() << ".";
		mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
		throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
	}
	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip chunk type.
	
	// <varName>:
	std::string	counterVarName("var_");
	counterVarName.append( tokenItty->GetIdentifierText() );
	
	CreateVariable( counterVarName, tokenItty->GetIdentifierText(), false, currFunction );
	
	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	
	// of:
	if( !tokenItty->IsIdentifier( EOfIdentifier ) && !tokenItty->IsIdentifier( EInIdentifier ) )
	{
		std::stringstream		errMsg;
		errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected \"of\" or \"in\" here, found "
								<< tokenItty->GetShortDescription() << ".";
		mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
		throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
	}
	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	
	// <expression>
	size_t			currLineNum = tokenItty->mLineNum;
	CValueNode* theExpressionNode = ParseExpression( parseTree, currFunction, tokenItty, tokens, ELastIdentifier_Sentinel );
	
	// AssignChunkArray( tempName, chunkType, <expression> );
	std::string		tempName = CVariableEntry::GetNewTempName();
	std::string		tempCounterName = CVariableEntry::GetNewTempName();
	std::string		tempMaxCountName = CVariableEntry::GetNewTempName();
	
	CCommandNode*			theVarChunkListCommand = new CAssignChunkArrayNode( &parseTree, currLineNum );
	theVarChunkListCommand->AddParam( new CLocalVariableRefValueNode(&parseTree, currFunction, tempName, tempName, currLineNum) );
	theVarChunkListCommand->AddParam( new CIntValueNode(&parseTree, chunkTypeConstant, currLineNum) );
	theVarChunkListCommand->AddParam( theExpressionNode );
	currFunction->AddCommand( theVarChunkListCommand );
	
	// tempCounterName = 1;
	CCommandNode*			theVarAssignCommand = new CAssignCommandNode( &parseTree, currLineNum );
	theVarAssignCommand->AddParam( new CLocalVariableRefValueNode(&parseTree, currFunction, tempCounterName, tempCounterName, currLineNum) );
	theVarAssignCommand->AddParam( new CIntValueNode(&parseTree, 1, currLineNum) );
	currFunction->AddCommand( theVarAssignCommand );
	
	// tempMaxCountName = GetArrayItemCount( tempName );
	CGetArrayItemCountNode*	currFunctionCall = new CGetArrayItemCountNode( &parseTree, currLineNum);
	currFunctionCall->AddParam( new CLocalVariableRefValueNode(&parseTree, currFunction, tempMaxCountName, tempMaxCountName, currLineNum) );
	currFunctionCall->AddParam( new CLocalVariableRefValueNode(&parseTree, currFunction, tempName, tempName, currLineNum) );
	currFunction->AddCommand( currFunctionCall );
	
	// while( tempCounterName <= tempMaxCountName )
	CWhileLoopNode*		whileLoop = new CWhileLoopNode( &parseTree, currLineNum, currFunction );
	currFunction->AddCommand( whileLoop );
	COperatorNode	*	opNode = new COperatorNode( &parseTree, LESS_THAN_EQUAL_OPERATOR_INSTR, currLineNum );
	whileLoop->SetCondition( opNode );
	opNode->AddParam( new CLocalVariableRefValueNode(&parseTree, currFunction, tempCounterName, tempCounterName, currLineNum) );
	opNode->AddParam( new CLocalVariableRefValueNode(&parseTree, currFunction, tempMaxCountName, tempMaxCountName, currLineNum) );
	
	// counterVarName = GetArrayItem( tempName, tempCounterName );
	CGetArrayItemNode*	getItemNode = new CGetArrayItemNode( &parseTree, currLineNum );
	getItemNode->AddParam( new CLocalVariableRefValueNode(&parseTree, currFunction, counterVarName, counterVarName, currLineNum) );
	getItemNode->AddParam( new CLocalVariableRefValueNode(&parseTree, currFunction, tempCounterName, tempCounterName, currLineNum) );
	getItemNode->AddParam( new CLocalVariableRefValueNode(&parseTree, currFunction, tempName, tempName, currLineNum) );
	whileLoop->AddCommand( getItemNode );
	
	whileLoop->SetCommandsLineNum( tokenItty->mLineNum );
	while( !tokenItty->IsIdentifier( EEndIdentifier ) )
	{
		ParseOneLine( userHandlerName, parseTree, whileLoop, tokenItty, tokens );
	}
	
	// tempCounterName += 1;	-- increment loop counter.
	CAddCommandNode	*	theIncrementOperation = new CAddCommandNode( &parseTree, tokenItty->mLineNum );
	theIncrementOperation->AddParam( new CLocalVariableRefValueNode(&parseTree, currFunction, tempCounterName, tempCounterName, tokenItty->mLineNum) );
	theIncrementOperation->AddParam( new CIntValueNode(&parseTree, 1, tokenItty->mLineNum) );
	whileLoop->AddCommand( theIncrementOperation );	// TODO: Need to dispose this on exceptions above.
	
	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	if( !tokenItty->IsIdentifier(ERepeatIdentifier) )	// end repeat
	{
		std::stringstream		errMsg;
		errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected \"end repeat\" here, found "
								<< tokenItty->GetShortDescription() << ".";
		mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
		throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
	}
	whileLoop->SetEndRepeatLineNum( tokenItty->mLineNum );
	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
}


void	CParser::ParseRepeatStatement( std::string& userHandlerName, CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens )
{
	size_t		conditionLineNum = tokenItty->mLineNum;
	
	// Repeat:
	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	
	if( tokenItty->IsIdentifier( EWhileIdentifier ) || tokenItty->IsIdentifier( EUntilIdentifier ) )	// While:
	{
		bool			doUntil = (tokenItty->mSubType == EUntilIdentifier);
		
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
		
		CWhileLoopNode*		whileLoop = new CWhileLoopNode( &parseTree, conditionLineNum, currFunction );
		CValueNode*			conditionNode = NULL;
		
		currFunction->AddCommand( whileLoop );
		
		// Condition:
		conditionNode = ParseExpression( parseTree, currFunction, tokenItty, tokens, ELastIdentifier_Sentinel );

		if( doUntil )
		{
			COperatorNode	*funcNode = new COperatorNode( &parseTree, NEGATE_BOOL_INSTR, conditionLineNum );
			funcNode->AddParam( conditionNode );
			conditionNode = funcNode;
		}
		
		whileLoop->SetCondition( conditionNode );
		
		// Commands:
		whileLoop->SetCommandsLineNum( tokenItty->mLineNum );
		while( !tokenItty->IsIdentifier( EEndIdentifier ) )
		{
			ParseOneLine( userHandlerName, parseTree, whileLoop, tokenItty, tokens );
		}

		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
		tokenItty->ExpectIdentifier( mFileName, ERepeatIdentifier, EEndIdentifier );
		whileLoop->SetEndRepeatLineNum( tokenItty->mLineNum );
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	}
	else if( tokenItty->IsIdentifier( EWithIdentifier ) )	// With:
	{
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
		
		std::string	counterVarName("var_");
		counterVarName.append( tokenItty->GetIdentifierText() );
		
		CreateVariable( counterVarName, tokenItty->GetIdentifierText(), false, currFunction );
		
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
		
		// From:
		if( !tokenItty->IsIdentifier( EFromIdentifier ) && !tokenItty->IsIdentifier( EEqualsOperator )
			 && !tokenItty->IsIdentifier( EIsIdentifier ) )
		{
			std::stringstream		errMsg;
			errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected \"from\" or \"=\" here, found "
								<< tokenItty->GetShortDescription() << ".";
			mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
			throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
		}
		
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
		
		// startNum:
		CValueNode*			startNumExpr = ParseExpression( parseTree, currFunction, tokenItty, tokens, EToIdentifier );
		
		LEOInteger			stepSize = 1;
		LEOInstructionID	compareOp = LESS_THAN_EQUAL_OPERATOR_INSTR;
		
		// [down] ?
		if( tokenItty->IsIdentifier( EDownIdentifier ) )
		{
			stepSize = -1;
			compareOp = GREATER_THAN_EQUAL_OPERATOR_INSTR;
			
			CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
		}
		
		// To:
		if( !tokenItty->IsIdentifier( EToIdentifier ) && !tokenItty->IsIdentifier( EThroughIdentifier ) )
		{
			std::stringstream		errMsg;
			errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected \"to\" or \"through\" here, found "
								<< tokenItty->GetShortDescription() << ".";
			mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
			throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
		}
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
		
		// endNum:
		CValueNode*		endNumExpr = ParseExpression( parseTree, currFunction, tokenItty, tokens, ELastIdentifier_Sentinel );
		std::string		tempName = CVariableEntry::GetNewTempName();
		currFunction->AddLocalVar( tempName, tempName, TVariantTypeInt );
		
		CWhileLoopNode*		whileLoop = new CWhileLoopNode( &parseTree, conditionLineNum, currFunction );
		
		// tempName = startNum;
		CCommandNode*	theAssignCommand = new CAssignCommandNode( &parseTree, conditionLineNum );
		theAssignCommand->AddParam( new CLocalVariableRefValueNode(&parseTree, currFunction, tempName, tempName, conditionLineNum) );
		theAssignCommand->AddParam( startNumExpr );
		currFunction->AddCommand( theAssignCommand );
		
		// while( tempName <= endNum )
		COperatorNode*	theComparison = new COperatorNode( &parseTree, compareOp, conditionLineNum );
		theComparison->AddParam( new CLocalVariableRefValueNode(&parseTree, currFunction, tempName, tempName, conditionLineNum) );
		theComparison->AddParam( endNumExpr );
		whileLoop->SetCondition( theComparison );
		
		// counterVarName = tempName;
		theAssignCommand = new CPutCommandNode( &parseTree, conditionLineNum );
		theAssignCommand->AddParam( new CLocalVariableRefValueNode(&parseTree, currFunction, tempName, tempName, conditionLineNum) );
		theAssignCommand->AddParam( new CLocalVariableRefValueNode(&parseTree, currFunction, counterVarName, counterVarName, conditionLineNum) );
		whileLoop->AddCommand( theAssignCommand );
		
		do
		{
			// If there is no command to repeat, we don't want to parse the
			//	"end" statement as a handler call named "end", so we need to
			//	make sure we eliminate that case beforehand.
			
			whileLoop->SetCommandsLineNum( tokenItty->mLineNum );
			while( tokenItty != tokens.end() && tokenItty->IsIdentifier( ENewlineOperator) )
				CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
			
			if( tokenItty == tokens.end() || tokenItty->IsIdentifier( EEndIdentifier ) )
				break;
			
			ParseOneLine( userHandlerName, parseTree, whileLoop, tokenItty, tokens );
		}
		while( true );
		
		// tempName += 1;
		CAddCommandNode	*	theIncrementOperation = new CAddCommandNode( &parseTree, tokenItty->mLineNum );
		theIncrementOperation->AddParam( new CLocalVariableRefValueNode(&parseTree, currFunction, tempName, tempName, tokenItty->mLineNum) );
		theIncrementOperation->AddParam( new CIntValueNode(&parseTree, stepSize, tokenItty->mLineNum) );
		whileLoop->AddCommand( theIncrementOperation );	// TODO: Need to dispose this on exceptions above.
		
		currFunction->AddCommand( whileLoop );
		
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
		tokenItty->ExpectIdentifier( mFileName, ERepeatIdentifier, EEndIdentifier );
		whileLoop->SetEndRepeatLineNum( tokenItty->mLineNum );
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	}
	else
	{
		// [for] ?
		if( tokenItty->IsIdentifier( EForIdentifier ) )
		{
			CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "for".
			if( tokenItty->IsIdentifier( EEachIdentifier ) )
			{
				CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "each".
				ParseRepeatForEachStatement( userHandlerName, parseTree,
											currFunction, tokenItty, tokens );
				return;
			}
		}
		
		// tempName = 0;
		std::string			tempName = CVariableEntry::GetNewTempName();
		CCommandNode*		theAssignCommand = new CAssignCommandNode( &parseTree, conditionLineNum );
		theAssignCommand->AddParam( new CLocalVariableRefValueNode(&parseTree, currFunction, tempName, tempName, conditionLineNum) );
		theAssignCommand->AddParam( new CIntValueNode(&parseTree, 0, tokenItty->mLineNum) );
		currFunction->AddCommand( theAssignCommand );
		
		// countNum:
		CValueNode*		countExpression = ParseExpression( parseTree, currFunction, tokenItty, tokens, ELastIdentifier_Sentinel );
		
		// [times] ?
		if( tokenItty->IsIdentifier( ETimesIdentifier ) )
			CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "times".
		
		CWhileLoopNode*		whileLoop = new CWhileLoopNode( &parseTree, conditionLineNum, currFunction );
		currFunction->AddCommand( whileLoop );
		
		// while( tempName < countExpression )
		COperatorNode*	theComparison = new COperatorNode( &parseTree, LESS_THAN_OPERATOR_INSTR, conditionLineNum );
		theComparison->AddParam( new CLocalVariableRefValueNode(&parseTree, currFunction, tempName, tempName, conditionLineNum) );
		theComparison->AddParam( countExpression );
		whileLoop->SetCondition( theComparison );
		
		whileLoop->SetCommandsLineNum( tokenItty->mLineNum );
		while( !tokenItty->IsIdentifier( EEndIdentifier ) )
		{
			ParseOneLine( userHandlerName, parseTree, whileLoop, tokenItty, tokens );
		}
		
		// tempName += 1;
		CAddCommandNode	*	theIncrementOperation = new CAddCommandNode( &parseTree, tokenItty->mLineNum );
		theIncrementOperation->AddParam( new CLocalVariableRefValueNode(&parseTree, currFunction, tempName, tempName, tokenItty->mLineNum) );
		theIncrementOperation->AddParam( new CIntValueNode(&parseTree, 1, tokenItty->mLineNum) );
		whileLoop->AddCommand( theIncrementOperation );
		
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
		tokenItty->ExpectIdentifier( mFileName, ERepeatIdentifier, EEndIdentifier );
		whileLoop->SetEndRepeatLineNum( tokenItty->mLineNum );
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	}
}


void	CParser::ParseIfStatement( std::string& userHandlerName, CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens )
{
	size_t			conditionLineNum = tokenItty->mLineNum;
	CIfNode*		ifNode = new CIfNode( &parseTree, conditionLineNum, currFunction );
	try
	{
		// If:
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
		
		// Condition:
		CValueNode*			condition = ParseExpression( parseTree, currFunction, tokenItty, tokens, ELastIdentifier_Sentinel );
		ifNode->SetCondition( condition );
		
		while( tokenItty->IsIdentifier(ENewlineOperator) )
			CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
		
		// Then:
		tokenItty->ExpectIdentifier( mFileName, EThenIdentifier );
		ifNode->SetThenLineNum( tokenItty->mLineNum );
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
		
		bool	needEndIf = true;
		
		if( tokenItty->IsIdentifier( ENewlineOperator ) )
		{
			CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
			// Commands:
			ifNode->SetIfCommandsLineNum( tokenItty->mLineNum );
			while( !tokenItty->IsIdentifier( EEndIdentifier ) && !tokenItty->IsIdentifier( EElseIdentifier ) )
			{
				ParseOneLine( userHandlerName, parseTree, ifNode, tokenItty, tokens );
			}
		}
		else
		{
			ifNode->SetIfCommandsLineNum( tokenItty->mLineNum );
			ParseOneLine( userHandlerName, parseTree, ifNode, tokenItty, tokens, true );
			needEndIf = false;
		}
		
		std::deque<CToken>::iterator	beforeLineEnd = tokenItty;	// Remember position before line end in case there's no 'else'. We need to leave a line break for ParseOneLine() to parse.

		while( tokenItty->IsIdentifier(ENewlineOperator) )
			CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
		
		// Else:
		if( tokenItty->IsIdentifier( EElseIdentifier ) )	// It's an "else"! Parse another block!
		{
			ifNode->SetElseLineNum( tokenItty->mLineNum );
			CCodeBlockNode*		elseNode = ifNode->CreateElseBlock( tokenItty->mLineNum );
			
			CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
			
			if( tokenItty->IsIdentifier(ENewlineOperator) )	// Followed by a newline! Multi-line if!
			{
				CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
				ifNode->SetElseCommandsLineNum( tokenItty->mLineNum );
				while( tokenItty->IsIdentifier(ENewlineOperator) )
					CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
				while( !tokenItty->IsIdentifier( EEndIdentifier ) )
				{
					ParseOneLine( userHandlerName, parseTree, elseNode, tokenItty, tokens );
				}
				needEndIf = true;
			}
			else
			{
				ifNode->SetElseCommandsLineNum( tokenItty->mLineNum );
				ifNode->SetEndIfLineNum( tokenItty->mLineNum );
				ParseOneLine( userHandlerName, parseTree, elseNode, tokenItty, tokens, true );	// Don't swallow return.
				needEndIf = false;
			}
		}
		else if( needEndIf == false )
			tokenItty = beforeLineEnd;	// Leave a return at the end of the line (which the code above skipped) so ParseOneLine() can detect we're really at the end of the line.
		
		// End If:
		if( needEndIf && tokenItty->IsIdentifier( EEndIdentifier ) )
		{
			CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
			if( !tokenItty->IsIdentifier(EIfIdentifier) )
			{
				std::stringstream		errMsg;
				errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected \"end if\" here, found "
										<< tokenItty->GetShortDescription() << ".";
				mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
				throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
			}
			ifNode->SetEndIfLineNum( tokenItty->mLineNum );
			CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
		}
	}
	catch( ... )
	{
		delete ifNode;
		throw;
	}
	
	currFunction->AddCommand( ifNode );
		
	// We leave the last line break after the 'end if' in the token stream so ParseOneLine() can parse it.
}


CValueNode*	CParser::ParseArrayItem( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
								std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens )
{
	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	
	// itemNumber:
	CValueNode*	theIndex = ParseExpression( parseTree, currFunction, tokenItty, tokens, EOfIdentifier );
	
	// of:
	tokenItty->ExpectIdentifier( mFileName, EOfIdentifier );
	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	
	// container:
	size_t				containerLineNum = tokenItty->mLineNum;
	CValueNode*			theTarget = ParseContainer( false, true, parseTree, currFunction, tokenItty, tokens, ELastIdentifier_Sentinel );
	CFunctionCallNode*	fcall = new CFunctionCallNode( &parseTree, true, "GetItemOfListWithKey", containerLineNum );
	fcall->AddParam( theTarget );
	fcall->AddParam( theIndex );
	
	return fcall;	// TODO: delete stuff on exceptions before this.
}


CValueNode*	CParser::ParseContainer( bool asPointer, bool initWithName, CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
								std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens, TIdentifierSubtype inEndToken )
{
	// Try to find chunk type that matches:
	CValueNode*	container = NULL;
	TChunkType	typeConstant = GetChunkTypeNameFromIdentifierSubtype( tokenItty->mSubType );
	
	if( typeConstant != TChunkTypeInvalid )
	{
		return ParseChunkExpression( typeConstant, parseTree, currFunction, tokenItty, tokens );
	}
	
	// Try if it is a "my <property>"-style property expression:
	if( tokenItty->IsIdentifier( EMyIdentifier ) )
	{
		COperatorNode*		meContainer = new COperatorNode( &parseTree, kFirstPropertyInstruction +PUSH_ME_INSTR, tokenItty->mLineNum );
		
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// skip "my".
		
		// Look for long/abbreviated/short style qualifier:
		std::string		propName;
		if( tokenItty->IsIdentifier( ELongIdentifier ) || tokenItty->IsIdentifier( EShortIdentifier )
			|| tokenItty->IsIdentifier( EAbbreviatedIdentifier ) )
		{
			propName = tokenItty->GetIdentifierText();
			propName.append( 1, ' ' );
			
			CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Advance past style qualifier.
		}
		
		// Look for actual property name:
		propName.append( tokenItty->GetIdentifierText() );
		CObjectPropertyNode	*	propExpr = new CObjectPropertyNode( &parseTree, propName, tokenItty->mLineNum );
		propExpr->AddParam( meContainer );

		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// skip property name.
		
		return propExpr;
	}
	else if( tokenItty->IsIdentifier( EMeIdentifier ) )	// A reference to the object owning this script?
	{
		COperatorNode*		hostCommand = new COperatorNode( &parseTree, kFirstPropertyInstruction +PUSH_ME_INSTR, tokenItty->mLineNum );
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
		return hostCommand;
	}
	
	// If we know we have a variable of that name, choose that:
	std::string		realVarName( tokenItty->GetIdentifierText() );
	std::string		varName( "var_" );
	varName.append( realVarName );
	if( !container && currFunction->LocalVariableExists( varName ) )
	{
		CreateVariable( varName, realVarName, initWithName, currFunction );
		container = new CLocalVariableRefValueNode( &parseTree, currFunction, varName, realVarName, tokenItty->mLineNum );
		
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
		
		return container;
	}
	
	// Some things may be prefixed by 'the':
	if( tokenItty->IsIdentifier( ETheIdentifier ) )
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	
	// Try to parse a host-specific function (e.g. object descriptor):
	container = ParseHostFunction( parseTree, currFunction, tokenItty, tokens );
	if( container )
		return container;

	// Otherwise try to parse a built-in variable:
	if( tokenItty->IsIdentifier( EResultIdentifier ) )
	{
		std::string		realResultName( "result" );
		std::string		resultName( "result" );
		CreateVariable( resultName, realResultName, initWithName, currFunction );
		container = new CLocalVariableRefValueNode( &parseTree, currFunction, resultName, realResultName, tokenItty->mLineNum );
		
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	}
	else if( tokenItty->IsIdentifier( EDownloadIdentifier ) )
	{
		std::string		realDVarName( "download" );
		std::string		dVarName( "download" );
		CreateVariable( dVarName, realDVarName, initWithName, currFunction );
		container = new CLocalVariableRefValueNode( &parseTree, currFunction, dVarName, realDVarName, tokenItty->mLineNum );
		
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	}
	
	// Check if it could be an object property expression:
	bool			isStyleQualifiedProperty = false;
	if( !container )
	{
		std::string		propName;
		
		// Is it a qualified property?
		if( tokenItty->IsIdentifier( ELongIdentifier ) || tokenItty->IsIdentifier( EShortIdentifier )
			|| tokenItty->IsIdentifier( EAbbreviatedIdentifier ) )
		{
			propName = tokenItty->GetIdentifierText();
			propName.append( 1, ' ' );
			
			CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Advance past style qualifier.
			isStyleQualifiedProperty = true;
		}
		
		if( isStyleQualifiedProperty && tokenItty->mType != EIdentifierToken )
		{
			isStyleQualifiedProperty = false;
			CTokenizer::GoPreviousToken( mFileName, tokenItty, tokens );	// Backtrack over style qualifier.
		}
		else
		{
			size_t			lineNum = tokenItty->mLineNum;
			propName.append( tokenItty->GetIdentifierText() );
			CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
			if( inEndToken != EOfIdentifier && tokenItty->IsIdentifier( EOfIdentifier ) )
			{
				CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "of".
				CValueNode	*	targetObj = ParseTerm( parseTree, currFunction, tokenItty, tokens, ELastIdentifier_Sentinel );
				
				if( targetObj )
				{
					CObjectPropertyNode	*	propExpr = new CObjectPropertyNode( &parseTree, propName, lineNum );
					propExpr->AddParam( targetObj );
					container = propExpr;
				}
				else
					CTokenizer::GoPreviousToken( mFileName, tokenItty, tokens );	// Backtrack over what should have been "of".
			}
			else
				CTokenizer::GoPreviousToken( mFileName, tokenItty, tokens );	// Backtrack over what should have been "of".
			
			if( !container && isStyleQualifiedProperty )
				CTokenizer::GoPreviousToken( mFileName, tokenItty, tokens );	// Backtrack over style qualifier.
		}
	}
	
	// Check if it could be a global property expression:
	if( !container && sGlobalProperties )
	{
		TIdentifierSubtype		subType = tokenItty->GetIdentifierSubType();
		TIdentifierSubtype		qualifierType = ELastIdentifier_Sentinel;
		
		if( isStyleQualifiedProperty )
		{
			qualifierType = subType;
			CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "short", "long" or "abbreviated".
			subType = tokenItty->GetIdentifierSubType();
		}
		
		// Find it in our list of global properties:
		int				x = 0;
		
		for( x = 0; sGlobalProperties[x].mType != ELastIdentifier_Sentinel; x++ )
		{
			if( sGlobalProperties[x].mType == subType && sGlobalProperties[x].mPrefixType == qualifierType )
			{
				container = new CGlobalPropertyNode( &parseTree, sGlobalProperties[x].mSetterInstructionID, sGlobalProperties[x].mGetterInstructionID, gIdentifierStrings[subType], tokenItty->mLineNum );
				CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip the property name.
				break;
			}
		}
		
		if( !container && isStyleQualifiedProperty )
			CTokenizer::GoPreviousToken( mFileName, tokenItty, tokens );	// Backtrack over style qualifier.
	}
	
	// Implicit declaration of any old variable:
	if( !container )
	{
		CreateVariable( varName, realVarName, initWithName, currFunction );
		container = new CLocalVariableRefValueNode( &parseTree, currFunction, varName, realVarName, tokenItty->mLineNum );
		
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	}
	
	return container;
}


void	CParser::CreateVariable( const std::string& varName, const std::string& realVarName, bool initWithName,
									CCodeBlockNodeBase* currFunction, bool isGlobal )
{
	std::map<std::string,CVariableEntry>::iterator	theContainerItty;
	std::map<std::string,CVariableEntry>*			varMap;
	
	if( isGlobal )
		varMap = &currFunction->GetGlobals();
	else
		varMap = &currFunction->GetLocals();
	theContainerItty = varMap->find( varName );

	if( theContainerItty == varMap->end() )	// No var of that name yet?
		(*varMap)[varName] = CVariableEntry( realVarName, TVariantType_INVALID, initWithName );	// Add one to variable list.

}


void	CParser::ParseOneLine( std::string& userHandlerName, CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
								std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens,
								bool dontSwallowReturn )
{
	while( tokenItty->IsIdentifier(ENewlineOperator) )
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	
	CLineMarkerNode*	lineMarker = new CLineMarkerNode( &parseTree, tokenItty->mLineNum );
	currFunction->AddCommand( lineMarker );
	
	if( tokenItty->mType == EIdentifierToken && tokenItty->mSubType == ELastIdentifier_Sentinel )	// Unknown identifier.
		ParseHandlerCall( parseTree, currFunction, false, tokenItty, tokens );
	else if( tokenItty->IsIdentifier(EPutIdentifier) )		// put command.
		ParsePutStatement( parseTree, currFunction, tokenItty, tokens );
	else if( tokenItty->IsIdentifier(EDownloadIdentifier) )
		ParseDownloadStatement( userHandlerName, parseTree, currFunction, tokenItty, tokens );
	else if( tokenItty->IsIdentifier(EReturnIdentifier) )
		ParseReturnStatement( parseTree, currFunction, tokenItty, tokens );
	else if( tokenItty->IsIdentifier(EPassIdentifier) )
		ParsePassStatement( parseTree, currFunction, tokenItty, tokens );
	else if( tokenItty->IsIdentifier(EExitIdentifier) )
	{
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "exit".
		if( tokenItty->IsIdentifier(ERepeatIdentifier) )
		{
			CCommandNode*	theExitRepeatCommand = new CCommandNode( &parseTree, "ExitRepeat", tokenItty->mLineNum );
			currFunction->AddCommand( theExitRepeatCommand );
			CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
		}
		else if( tokenItty->GetIdentifierText().compare(userHandlerName) == 0 )
		{
			CCommandNode*	theReturnCommand = new CReturnCommandNode( &parseTree, tokenItty->mLineNum );
			currFunction->AddCommand( theReturnCommand );
			theReturnCommand->AddParam( new CStringValueNode(&parseTree, "", tokenItty->mLineNum) );
			CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
		}
		else
		{
			std::stringstream errMsg;
			errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected \"exit repeat\" or \"exit " << userHandlerName << "\", found "
					<< tokenItty->GetShortDescription() << ".";
			mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
			throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
		}
	}
	else if( tokenItty->IsIdentifier(ENextIdentifier) )
	{
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "next".
		if( tokenItty->IsIdentifier(ERepeatIdentifier) )
		{
			CCommandNode*	theNextRepeatCommand = new CCommandNode( &parseTree, "NextRepeat", tokenItty->mLineNum );
			currFunction->AddCommand( theNextRepeatCommand );
			CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
		}
		else
		{
			std::stringstream errMsg;
			errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected \"next repeat\", found "
					<< tokenItty->GetShortDescription() << ".";
			mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
			throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
		}
	}
	else if( tokenItty->IsIdentifier(ERepeatIdentifier) )
		ParseRepeatStatement( userHandlerName, parseTree, currFunction, tokenItty, tokens );
	else if( tokenItty->IsIdentifier(EIfIdentifier) )
		ParseIfStatement( userHandlerName, parseTree, currFunction, tokenItty, tokens );
	else if( tokenItty->IsIdentifier(EAddIdentifier) )
		ParseAddStatement( parseTree, currFunction, tokenItty, tokens );
	else if( tokenItty->IsIdentifier(ESubtractIdentifier) )
		ParseSubtractStatement( parseTree, currFunction, tokenItty, tokens );
	else if( tokenItty->IsIdentifier(EMultiplyIdentifier) )
		ParseMultiplyStatement( parseTree, currFunction, tokenItty, tokens );
	else if( tokenItty->IsIdentifier(EDivideIdentifier) )
		ParseDivideStatement( parseTree, currFunction, tokenItty, tokens );
	else if( tokenItty->IsIdentifier(EGetIdentifier) )
		ParseGetStatement( parseTree, currFunction, tokenItty, tokens );
	else if( tokenItty->IsIdentifier(ESetIdentifier) )
		ParseSetStatement( parseTree, currFunction, tokenItty, tokens );
	else if( tokenItty->IsIdentifier(EGlobalIdentifier) )
		ParseGlobalStatement( parseTree, currFunction, tokenItty, tokens );
	else
		ParseHostCommand( parseTree, currFunction, tokenItty, tokens );
	
	// End this line:
	if( !dontSwallowReturn && tokenItty != tokens.end() )
	{
		if( !tokenItty->IsIdentifier(ENewlineOperator) )
		{
			std::stringstream errMsg;
			errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected end of line, found "
					<< tokenItty->GetShortDescription() << ".";
			mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
			throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
		}
			
		while( tokenItty->IsIdentifier(ENewlineOperator) )
			CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	}
}


void	CParser::ParseFunctionBody( std::string& userHandlerName,
									CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
									std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens,
								    size_t *outEndLineNum, TIdentifierSubtype endIdentifier, bool parseFirstLineAsReturnExpression )
{
	if( parseFirstLineAsReturnExpression )
	{
		CCommandNode*	theReturnCommand = new CReturnCommandNode( &parseTree, (tokenItty != tokens.end()) ? tokenItty->mLineNum : 1 );
		
		CValueNode*	theWhatNode = ParseExpression( parseTree, currFunction, tokenItty, tokens, endIdentifier );
		if( !theWhatNode )	// Empty string passed in?
		{
			delete theReturnCommand;
			return;
		}
		theReturnCommand->AddParam( theWhatNode );
		
		currFunction->AddCommand( theReturnCommand );
	}
	else
	{
		while( tokenItty != tokens.end()
				&& !tokenItty->IsIdentifier( endIdentifier ) )	// Sub-constructs will swallow their own "end XXX" instructions, so we can exit the loop. Either it's our "end", or it's unbalanced.
		{
			ParseOneLine( userHandlerName, parseTree, currFunction, tokenItty, tokens );
		}
		
		if( tokenItty != tokens.end() )
			CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	}
	
	if( endIdentifier == EEndIdentifier && tokenItty != tokens.end() )
	{
		if( tokenItty->GetIdentifierText().compare(userHandlerName) != 0 )
		{
			std::stringstream		errMsg;
			errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected \"end " << userHandlerName << "\" here, found "
									<< tokenItty->GetShortDescription() << ".";
			mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
			throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
		}
		if( outEndLineNum ) *outEndLineNum = tokenItty->mLineNum;
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	}
}


// Parse a list of expressions separated by commas for passing to a handler as a parameter list:
void	CParser::ParseParamList( TIdentifierSubtype identifierToEndOn,
								CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
								std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens,
								CFunctionCallNode* inFCallToAddTo )
{
	if( tokenItty == tokens.end() )
		return;
	while( !tokenItty->IsIdentifier( identifierToEndOn ) )
	{
		CValueNode*		paramExpression = ParseExpression( parseTree, currFunction, tokenItty, tokens, ECommaOperator );
		if( !paramExpression )
			return;
		inFCallToAddTo->AddParam( paramExpression );
		
		if( tokenItty == tokens.end() )
			return;
		
		if( !tokenItty->IsIdentifier( ECommaOperator ) )
		{
			if( tokenItty->IsIdentifier( identifierToEndOn ) )
				break;	// Exit loop.
			std::stringstream		errMsg;
			errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected comma here, found \""
									<< tokenItty->GetShortDescription() << "\".";
			mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
			throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
		}
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
	}
}


TIdentifierSubtype	CParser::ParseOperator( std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens, int *outPrecedence, LEOInstructionID *outOpName )
{
	if( tokenItty->mType != EIdentifierToken )
		return ELastIdentifier_Sentinel;
	
	int		x = 0;
	for( ; sOperators[x].mType != ELastIdentifier_Sentinel; x++ )
	{
		if( tokenItty->IsIdentifier( sOperators[x].mType ) )
		{
			CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
			
			// Is single-token operator and matches?
			if( sOperators[x].mSecondType == ELastIdentifier_Sentinel )
			{
				*outPrecedence = sOperators[x].mPrecedence;
				*outOpName = sOperators[x].mInstructionID;
				
				return sOperators[x].mTypeToReturn;
			}
			else if( tokenItty->IsIdentifier(sOperators[x].mSecondType) )
			{
				CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Swallow second operator token, too.
				*outPrecedence = sOperators[x].mPrecedence;
				*outOpName = sOperators[x].mInstructionID;
				
				return sOperators[x].mTypeToReturn;
			}
			else
				CTokenizer::GoPreviousToken( mFileName, tokenItty, tokens );	// Backtrack so we don't accidentally swallow the token following this operator.
		}
	}
	
	return ELastIdentifier_Sentinel;
}


// -----------------------------------------------------------------------------
//	CollapseExpressionStack ():
//		Take the passed lists of terms and operators and go over them from
//		the right end, generating a function call for the rightmost operator/two
//		terms combination and than pushing that call back on the tree for use
//		as the rightmost argument of the next operator.
// -----------------------------------------------------------------------------

CValueNode*	CParser::CollapseExpressionStack( CParseTree& parseTree, std::deque<CValueNode*> &terms, std::deque<LEOInstructionID> &operators )
{
	CValueNode*			operandA = NULL;
	LEOInstructionID	opName = NULL;

	while( terms.size() > 1 )	// More than 1 operand? Process stuff on stack.
	{
		CValueNode*			operandB = NULL;
		
		opName = operators.back();
		operators.pop_back();
		
		operandB = terms.back();
		terms.pop_back();
		
		operandA = terms.back();
		terms.pop_back();
		
		COperatorNode*	currOperation = new COperatorNode( &parseTree, opName, operandA->GetLineNum() );
		currOperation->AddParam( operandA );
		currOperation->AddParam( operandB );
		
		terms.push_back( currOperation );
	}
	
	operandA = terms.back();
	terms.pop_back();
	
	return operandA;
}


// -----------------------------------------------------------------------------
//	ParseExpression ():
//		Parse an expression from the given token stream, adding any variables
//		and commands needed to the given function. This uses a stack to collect
//		all terms and operators, and collapses subexpressions whenever the
//		operator precedence goes down.
// -----------------------------------------------------------------------------

CValueNode*	CParser::ParseExpression( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty,
										std::deque<CToken>& tokens, TIdentifierSubtype inEndToken )
{
	if( tokenItty == tokens.end() )
		return NULL;
	
	std::deque<CValueNode*>			terms;
	std::deque<LEOInstructionID>	operators;
	CValueNode*						currArg;
	TIdentifierSubtype				currOpType = ELastIdentifier_Sentinel;
	int								currPrecedence = 0,
									prevPrecedence = 0;
	LEOInstructionID				opName = INVALID_INSTR;
	
	currArg = ParseTerm( parseTree, currFunction, tokenItty, tokens, inEndToken );
	if( !currArg )
		return NULL;
	terms.push_back( currArg );
	currArg = NULL;
	
	while( (currOpType = ParseOperator( tokenItty, tokens, &currPrecedence, &opName )) != ELastIdentifier_Sentinel )
	{
		if( prevPrecedence > currPrecedence )
		{
			CValueNode*	collapsedOp = CollapseExpressionStack( parseTree, terms, operators );
			terms.push_back( collapsedOp );
		}

		currArg = ParseTerm( parseTree, currFunction, tokenItty, tokens, inEndToken );
		if( !currArg )
		{
			std::stringstream		errMsg;
			errMsg << mFileName << ":0: error: Expected term here, found end of script.";
			mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, 0 ) );
			throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
		}
		terms.push_back( currArg );
		currArg = NULL;
		
		operators.push_back( opName );
		
		prevPrecedence = currPrecedence;
	}
	
	return CollapseExpressionStack( parseTree, terms, operators );
}


void	CParser::LoadNativeHeadersFromFile( const char* filepath )
{
	std::ifstream		headerFile(filepath);
	char				theCh = 0;
	std::string			headerPath;		
	std::string			frameworkPath;
	size_t				lineNum = 1;
	
	while( theCh != std::ifstream::traits_type::eof() )
	{
		theCh = headerFile.get();
		//std::cout << theCh << std::endl;
		
		if( theCh == std::ifstream::traits_type::eof() )
			break;
		
		switch( theCh )
		{
			case '#':	// comment.
			case '*':	// class name.
			case '<':	// protocol class/category implements.
			case '(':	// category name.
			case ':':	// superclass.
				while( (theCh = headerFile.get()) != std::ifstream::traits_type::eof() && theCh != '\n' )
					;	// Do nothing, just swallow characters on this line.
				break;
			
			case '\n':	// Empty line? Just skip.
				break;
			
			case 'F':
				frameworkPath.clear();
				while( (theCh = headerFile.get()) != std::ifstream::traits_type::eof() && theCh != '\n' )
					frameworkPath.append( 1, theCh );
				break;

			case 'H':
				headerPath.clear();
				while( (theCh = headerFile.get()) != std::ifstream::traits_type::eof() && theCh != '\n' )
					headerPath.append( 1, theCh );
				break;
			
			case '~':
			{
				std::string		typeStr;
				std::string		synonymousStr;
				bool			hadComma = false;
				
				while( (theCh = headerFile.get()) != std::ifstream::traits_type::eof() && theCh != '\n' )
				{
					if( theCh == ',' )
						hadComma = true;
					else if( hadComma )
						typeStr.append( 1, theCh );
					else
						synonymousStr.append( 1, theCh );
				}
				sSynonymToTypeTable[synonymousStr] = typeStr;
				break;
			}
			
			case 'e':
			{
				std::string		constantStr;
				std::string		synonymousStr;
				bool			hadComma = false;
				
				while( (theCh = headerFile.get()) != std::ifstream::traits_type::eof() && theCh != '\n' )
				{
					if( theCh == ',' )
						hadComma = true;
					else if( hadComma )
						constantStr.append( 1, theCh );
					else
						synonymousStr.append( 1, theCh );
				}
				sConstantToValueTable[constantStr] = synonymousStr;
				break;
			}
			
			case '=':
			case '-':
			case '+':
			case '&':
			{
				std::string		typesLine;
				std::string		selectorStr;
				std::string		currType;
				bool			isFunction = (theCh == '=');
				bool			isFunctionPtr = (theCh == '&');
				bool			isInMethodName = true;
				bool			doneWithLine = false;
				
				while(  !doneWithLine && (theCh = headerFile.get()) != std::ifstream::traits_type::eof() )
				{
					switch( theCh )
					{
						case ',':
							if( isInMethodName )	// Found comma? We're finished getting selector name. Rest of line goes in types.
								isInMethodName = false;
							else
							{
								std::map<std::string,std::string>::const_iterator foundSynonym;
								while( (foundSynonym = sSynonymToTypeTable.find(currType)) != sSynonymToTypeTable.end() )
								{
									currType = foundSynonym->second;
								}
								typesLine.append( currType );
								typesLine.append( 1, ',' );
								currType.clear();
							}
							break;
						
						case '\n':
						{
							std::map<std::string,std::string>::const_iterator foundSynonym;
							while( (foundSynonym = sSynonymToTypeTable.find(currType)) != sSynonymToTypeTable.end() )
							{
								currType = foundSynonym->second;
							}
							typesLine.append( currType );
							currType.clear();
							doneWithLine = true;
							break;
						}
						
						default:
							if( isInMethodName )
								selectorStr.append( 1, theCh );
							else
								currType.append( 1, theCh );
							break;
					}
				}
				
				if( isFunction )
					sCFunctionTable[selectorStr] = CObjCMethodEntry( headerPath, frameworkPath, typesLine );
				else if( isFunctionPtr )
					sCFunctionPointerTable[selectorStr] = CObjCMethodEntry( headerPath, frameworkPath, typesLine );
				else
					sObjCMethodTable[selectorStr] = CObjCMethodEntry( headerPath, frameworkPath, typesLine );
				//std::cout << selectorStr << " = " << typesLine << std::endl;
				break;
			}
			
			default:	// unknown.
				std::stringstream errMsg;
				errMsg << "warning: Ignoring unknown data of type \"" << theCh << "\" in framework headers:" << std::endl;
				while( (theCh = headerFile.get()) != std::ifstream::traits_type::eof() && theCh != '\n' )
					errMsg << theCh;	// Print characters on this line.
				errMsg << std::endl;
				break;
		}
		
		lineNum++;
	}
}


void	CParser::SetFirstNativeCallCallback( LEOFirstNativeCallCallbackPtr inCallback )
{
	sFirstNativeCallCallback = inCallback;
}


// This parses an *editable* chunk expression that is a CVariant. This is a
//	little more complex and dangerous, as it simply points to the target value.
//	If you can, use the more efficient call for constant (i.e. non-changeable)
//	chunk expressions.

CValueNode*	CParser::ParseChunkExpression( TChunkType typeConstant, CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
											std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens )
{
	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "char" or "item" or whatever chunk type token this was.
	
	std::stringstream		valueStr;
	std::stringstream		startOffs;
	std::stringstream		endOffs;
	bool					hadTo = false;
	
	// Start offset:
	CValueNode*	startOffsObj = ParseExpression( parseTree, currFunction, tokenItty, tokens, ELastIdentifier_Sentinel );
	CValueNode*	endOffsObj = NULL;
	
	size_t		lineNum = tokenItty->mLineNum;
	
	// (Optional) end offset:
	if( tokenItty->IsIdentifier( EToIdentifier ) || tokenItty->IsIdentifier( EThroughIdentifier )
		|| tokenItty->IsIdentifier( EThruIdentifier ) )
	{
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "to"/"through"/"thru".
		
		endOffsObj = ParseExpression( parseTree, currFunction, tokenItty, tokens, EOfIdentifier );
		hadTo = true;
	}
	
	// Target value:
	if( !tokenItty->IsIdentifier( EOfIdentifier ) )
	{
		std::stringstream		errMsg;
		errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected ";
		if( !hadTo )
			errMsg << "\"to\" or ";
		errMsg << "\"of\" here, found " << tokenItty->GetShortDescription() << ".";
		mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
		throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
	}
	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "of".
	
	CValueNode*	targetValObj = ParseTerm( parseTree, currFunction, tokenItty, tokens, ELastIdentifier_Sentinel );
	
	// Now output code:
	CMakeChunkRefNode*	currOperation = new CMakeChunkRefNode( &parseTree, lineNum );
	currOperation->AddParam( targetValObj );
	currOperation->AddParam( new CIntValueNode( &parseTree, typeConstant, tokenItty->mLineNum ) );
	currOperation->AddParam( startOffsObj );
	currOperation->AddParam( hadTo ? endOffsObj : startOffsObj->Copy() );
	
	return currOperation;
}


// This parses an *un-editable* chunk expression that can only be read. This
//	pretty much just fetches the chunk value right then and there.

CValueNode*	CParser::ParseConstantChunkExpression( TChunkType typeConstant, CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens )
{
	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "char" or "item" or whatever chunk type token this was.
	
	bool					hadTo = false;
	CValueNode*				endOffsObj = NULL;
	
	// Start offset:
	CValueNode*		startOffsObj = ParseTerm( parseTree, currFunction, tokenItty, tokens, ELastIdentifier_Sentinel );
	size_t			lineNum = tokenItty->mLineNum;
	
	// (Optional) end offset:
	if( tokenItty->IsIdentifier( EToIdentifier ) || tokenItty->IsIdentifier( EThroughIdentifier )
		|| tokenItty->IsIdentifier( EThruIdentifier ) )
	{
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "to"/"through"/"thru".
		
		endOffsObj = ParseExpression( parseTree, currFunction, tokenItty, tokens, EOfIdentifier );
		hadTo = true;
	}
	
	// Target value:
	if( !tokenItty->IsIdentifier( EOfIdentifier ) )
	{
		std::stringstream		errMsg;
		errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected ";
		if( !hadTo )
			errMsg << "\"to\" or ";
		errMsg << "\"of\" here, found " << tokenItty->GetShortDescription() << ".";
		mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
		throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
	}
	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "of".
	
	CValueNode*	targetValObj = ParseTerm( parseTree, currFunction, tokenItty, tokens, ELastIdentifier_Sentinel );
	
	CMakeChunkConstNode*	currOperation = new CMakeChunkConstNode( &parseTree, currFunction, lineNum );
	currOperation->AddParam( targetValObj );
	currOperation->AddParam( new CIntValueNode( &parseTree, typeConstant, tokenItty->mLineNum ) );
	currOperation->AddParam( startOffsObj );
	currOperation->AddParam( hadTo ? endOffsObj : startOffsObj );

	return currOperation;
}


CValueNode*	CParser::ParseObjCMethodCall( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
										std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens )
{
	if( sFirstNativeCallCallback )
	{
		LEOFirstNativeCallCallbackPtr	callback = sFirstNativeCallCallback;
		sFirstNativeCallCallback = NULL;
		callback();
	}
	
	// We parse either a class name or an expression that evaluates to an object
	// as type "native object", followed by parameters with labels. We build the
	// method name from that and look up that method in our table of system calls.
	
	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip open bracket.
	
	COperatorNode	*	methodCall = new COperatorNode( &parseTree, CALL_OBJC_METHOD_INSTR +kFirstObjCCallInstruction, tokenItty->mLineNum );
	
	if( tokenItty->IsIdentifier(ELastIdentifier_Sentinel) )	// No reserved word identifier?
	{
		std::string				className( tokenItty->GetOriginalIdentifierText() );
		std::string				varName( "var_" );
		varName.append( ToLowerString( className ) );
		std::map<std::string,CVariableEntry>::iterator	theContainerItty = currFunction->GetLocals().find( varName );

		if( theContainerItty == currFunction->GetLocals().end() )	// No variable of that name? Must be ObjC class name:
		{
			methodCall->AddParam( new CStringValueNode( &parseTree, className, tokenItty->mLineNum ) );
			CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Move past target token.
		}
		else	// Otherwise get it out of the expression:
			methodCall->AddParam( ParseExpression( parseTree, currFunction, tokenItty, tokens, ELastIdentifier_Sentinel ) );
	}
	else
		methodCall->AddParam( ParseExpression( parseTree, currFunction, tokenItty, tokens, ELastIdentifier_Sentinel ) );
	
	methodCall->AddParam( NULL );	// Leave space for method name.
	methodCall->AddParam( NULL );	// Leave space for method signature.
	methodCall->AddParam( NULL );	// Leave space for framework name.
	
	if( tokenItty->mType != EIdentifierToken )
	{
		delete methodCall;
		
		std::stringstream		errMsg;
		errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected an identifier as a method name here, found "
								<< tokenItty->GetShortDescription() << ".";
		mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
		throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
	}
	
	int						numParams = 0;
	std::stringstream		methodName;
	methodName << tokenItty->GetOriginalIdentifierText();
	
	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip method name.

	std::stringstream		currLabel;	// temp we compose our param labels in.

	if( tokenItty->IsIdentifier(EColonOperator) )	// Takes params.
	{
		methodName << ":";
		
		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip colon.
		
		CValueNode	*	currParam = ParseExpression( parseTree, currFunction, tokenItty, tokens, ELastIdentifier_Sentinel );	// Read 1st param that immediately follows method name.
		methodCall->AddParam( currParam );
		numParams++;
		
		while( !tokenItty->IsIdentifier(ECloseSquareBracketOperator) )
		{
			if( tokenItty->mType != EIdentifierToken )
			{
				std::stringstream		errMsg;
				errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected an identifier as a parameter label here, found "
										<< tokenItty->GetShortDescription() << ".";
				mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
				throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
			}
			
			methodName << tokenItty->GetOriginalIdentifierText() << ":";
			CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip identifier label.
			
			if( !tokenItty->IsIdentifier( EColonOperator ) )
			{
				std::stringstream		errMsg;
				errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected colon after parameter label here, found "
										<< tokenItty->GetShortDescription() << ".";
				mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
				throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
			}
			
			CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip colon after label.
			
			currParam = ParseExpression( parseTree, currFunction, tokenItty, tokens, ELastIdentifier_Sentinel );	// Read param value.
			methodCall->AddParam( currParam );
			numParams++;
		}
	}
	
	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip close bracket (ECloseSquareBracketOperator).

	// Get data types for this method's params and return value:
	std::map<std::string,CObjCMethodEntry>::iterator	foundTypes = sObjCMethodTable.find( methodName.str() );
	if( foundTypes == sObjCMethodTable.end() )
	{
		std::stringstream		errMsg;
		errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Couldn't find definition of Objective C method "
								<< methodName.str() << ".";
		mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
		throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
	}
	
	// Fill out the info we accumulated parsing the parameters:
	methodCall->SetParamAtIndex( 1, new CStringValueNode( &parseTree, methodName.str(), tokenItty->mLineNum ) );
	methodCall->SetParamAtIndex( 2, new CStringValueNode( &parseTree, foundTypes->second.mMethodSignature, tokenItty->mLineNum ) );
	methodCall->SetParamAtIndex( 3, new CStringValueNode( &parseTree, foundTypes->second.mFrameworkName, tokenItty->mLineNum ) );
	
	methodCall->SetInstructionParams( numParams, 0 );
	
	return methodCall;
}


TChunkType	CParser::GetChunkTypeNameFromIdentifierSubtype( TIdentifierSubtype identifierToCheck )
{
	// Try to find chunk type that matches:
	TChunkType	foundType = TChunkTypeInvalid;
	int			x = 0;
	
	for( x = 0; sChunkTypes[x].mType != ELastIdentifier_Sentinel; x++ )
	{
		if( identifierToCheck == sChunkTypes[x].mType || identifierToCheck == sChunkTypes[x].mPluralType )
			foundType = sChunkTypes[x].mChunkTypeConstant;
	}
	
	return foundType;
}


CValueNode*	CParser::ParseNativeFunctionCallStartingAtParams( std::string& methodName, CObjCMethodEntry& methodInfo,
							CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
							std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens )
{
//	int						numParams = 0;
//	std::stringstream		paramsCode;	// temp we compose our params in.
//	std::deque<std::string>	params;		
//
//	while( !tokenItty->IsIdentifier(ECloseBracketOperator) )
//	{
//		ParseExpression( parseTree, paramsCode, tokenItty, tokens );	// Read param value.
//
//		params.push_back( paramsCode.str() );	// Add to params list.
//		paramsCode.str( std::string() );		// Clear temp so we can compose next param.
//		numParams++;
//		
//		if( !tokenItty->IsIdentifier(ECommaOperator) )
//			break;
//		
//		CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip comma.
//	}
//	
//	if( !tokenItty->IsIdentifier(ECloseBracketOperator) )
//	{
//		std::stringstream		errMsg;
//		errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected closing bracket after parameter list, found "
//					<< tokenItty->GetShortDescription() << ".";
//		mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
//		throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
//	}
//	
//	CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip close bracket (ECloseSquareBracketOperator).
//
//	// Make sure we include the needed headers:
//	std::deque<std::string>::iterator	needItty;
//	bool								haveHeader = false;
//	std::string							hdrName( methodInfo.mHeaderName );
//	for( needItty = mHeadersNeeded.begin(); needItty != mHeadersNeeded.end() && !haveHeader; needItty++ )
//		haveHeader = needItty->compare( hdrName ) == 0;
//	if( !haveHeader )
//		mHeadersNeeded.push_back( hdrName );
//	
//	// Make sure we link to the needed frameworks:
//	haveHeader = false;
//	hdrName.assign( methodInfo.mFrameworkName );
//	for( needItty = mFrameworksNeeded.begin(); needItty != mFrameworksNeeded.end() && !haveHeader; needItty++ )
//		haveHeader = needItty->compare( hdrName ) == 0;
//	if( !haveHeader )
//		mFrameworksNeeded.push_back( hdrName );
//	
//	// Get data types for this method's params and return value. Build an array of the types:
//	std::deque<std::string> typesList;
//	
//	FillArrayWithComponentsSeparatedBy( methodInfo.mMethodSignature.c_str(), ',', typesList );
//	
//	// Now generate actual code for a native function call:
//	std::deque<std::string>::iterator	typeItty = typesList.begin();
//	std::string							retValPrefix, retValSuffix;
//	GenerateObjCTypeToVariantCode( *typeItty, retValPrefix, retValSuffix );
//	theFunctionBody << retValPrefix;
//	typeItty++;
//	theFunctionBody << methodName << "( ";
//	std::deque<std::string>::iterator	valItty = params.begin();
//	bool								isFirst = true;
//	
//	for( ; valItty != params.end(); valItty++, typeItty++ )
//	{
//		if( isFirst )
//			isFirst = false;
//		else
//			theFunctionBody << ", ";
//		
//		std::string	paramPrefix, paramSuffix, paramItself(*valItty);
//		GenerateVariantToObjCTypeCode( *typeItty, paramPrefix, paramSuffix, paramItself );
//		theFunctionBody << paramPrefix << paramItself << paramSuffix;
//	}
//	theFunctionBody << " )" << retValSuffix;
	
	return NULL;
}


CValueNode*	CParser::ParseTerm( CParseTree& parseTree, CCodeBlockNodeBase* currFunction,
								std::deque<CToken>::iterator& tokenItty, std::deque<CToken>& tokens,
								TIdentifierSubtype inEndIdentifier )
{
	CValueNode*	theTerm = NULL;
	
	if( tokenItty == tokens.end() )
		return NULL;
	
	switch( tokenItty->mType )
	{
		case EStringToken:
		{
			theTerm = new CStringValueNode( &parseTree, tokenItty->mStringValue, tokenItty->mLineNum );
			CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
			break;
		}

		case ENumberToken:	// Any number (integer). We fake floats by parsing an integer/period-operator/integer sequence.
		{
			long long		theNumber = tokenItty->mNumberValue;
			
			CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
			
			if( tokenItty != tokens.end() && tokenItty->IsIdentifier( EPeriodOperator ) )	// Integer followed by period? Could be a float!
			{
				CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
				if( tokenItty != tokens.end() && tokenItty->mType == ENumberToken )	// Is a float!
				{
					std::stringstream	numStr;
					numStr << theNumber << "." << tokenItty->mNumberValue;
					char*				endPtr = NULL;
					double				theNum = strtod( numStr.str().c_str(), &endPtr );
					
					theTerm = new CFloatValueNode( &parseTree, theNum, tokenItty->mLineNum );
					
					CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
				}
				else	// Backtrack, that period was something else:
				{
					CTokenizer::GoPreviousToken( mFileName, tokenItty, tokens );
					theTerm = new CIntValueNode( &parseTree, theNumber, tokenItty->mLineNum );
				}
			}
			else
				theTerm = new CIntValueNode( &parseTree, theNumber, tokenItty->mLineNum );
			
			// If there's a unit after this number, apply that unit to the term:
			for( int x = 1; x < kLEOUnit_Last; x++ )
			{
				if( tokenItty->IsIdentifier(sUnitIdentifiers[x]) )
				{
					((CNumericValueNodeBase*)theTerm)->SetUnit(x);
					CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
					break;
				}
			}
			break;
		}

		case EIdentifierToken:	// Function call?
			if( tokenItty->mSubType == ELastIdentifier_Sentinel )	// Any user-defined identifier.
			{
				theTerm = ParseFunctionCall( parseTree, currFunction, false, tokenItty, tokens );
				if( !theTerm ) 
				{
					std::map<std::string,std::string>::iterator		sysConstItty = sConstantToValueTable.find( tokenItty->GetOriginalIdentifierText() );
					if( sysConstItty == sConstantToValueTable.end() )	// Not a system constant either? Guess it was a variable name:
						theTerm = ParseContainer( false, true, parseTree, currFunction, tokenItty, tokens, inEndIdentifier );
					else
					{
						theTerm = new CStringValueNode( &parseTree, sysConstItty->second, tokenItty->mLineNum );
						CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip the identifier for the constant we just parsed.
					}
				}
				
				break;	// Exit our switch.
			}
			else if( tokenItty->mSubType == EEndIdentifier )
			{
				return NULL;
			}
			else if( tokenItty->mSubType == EEntryIdentifier )
			{	
				theTerm = ParseArrayItem( parseTree, currFunction, tokenItty, tokens );
				break;
			}
			else if( tokenItty->mSubType == EIdIdentifier )	// "id"?
			{
				CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "id".
				
				// OF:
				if( !tokenItty->IsIdentifier(EOfIdentifier) )
				{
					std::stringstream		errMsg;
					errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected \"of\" here, found "
											<< tokenItty->GetShortDescription() << ".";
					mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
					throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
				}
				CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "of".
				
				std::string		hdlName;
				if( tokenItty->IsIdentifier(EFunctionIdentifier) )
				{
					hdlName.assign("fun_");
					CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "function".
					if( tokenItty->IsIdentifier(EHandlerIdentifier) )
						CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "handler".
				}
				else if( tokenItty->IsIdentifier(EMessageIdentifier) )
				{
					hdlName.assign("hdl_");
					CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "message".
					if( !tokenItty->IsIdentifier(EHandlerIdentifier) )
					{
						std::stringstream		errMsg;
						errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected \"function handler\" or \"message handler\" here, found "
												<< tokenItty->GetShortDescription() << ".";
						mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
						throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
					}
					CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "handler".
				}
				else
				{
					hdlName.assign("hdl_");
					if( !tokenItty->IsIdentifier(EHandlerIdentifier) )
					{
						std::stringstream		errMsg;
						errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected \"function handler\" or \"message handler\" here, found "
												<< tokenItty->GetShortDescription() << ".";
						mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
						throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
					}
					CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "handler".
				}
				
				hdlName.append( tokenItty->GetIdentifierText() );
				CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "handler".
				
				// Now that we know whether it's a function or a handler, store a pointer to it:
				theTerm = new CFunctionCallNode( &parseTree, false, "vcy_fcn_addr", tokenItty->mLineNum );
				break;
			}
			else if( tokenItty->mSubType == ENumberIdentifier )		// The identifier "number", i.e. the actual word.
			{
				CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "number".
				
				// OF:
				if( !tokenItty->IsIdentifier(EOfIdentifier) )
				{
					std::stringstream		errMsg;
					errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected \"of\" here, found "
											<< tokenItty->GetShortDescription() << ".";
					mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
					throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
				}
				CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "of".
				
				// Chunk type:
				TChunkType	typeConstant = GetChunkTypeNameFromIdentifierSubtype( tokenItty->GetIdentifierSubType() );
				if( typeConstant == TChunkTypeInvalid )
				{
					CTokenizer::GoPreviousToken( mFileName, tokenItty, tokens );	// Back to 'of'.
					CTokenizer::GoPreviousToken( mFileName, tokenItty, tokens );	// Back to 'number'.
					return ParseHostFunction( parseTree, currFunction, tokenItty, tokens );
				}
				CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "items" etc.
				
				// OF:
				if( !tokenItty->IsIdentifier(EOfIdentifier) )
				{
					std::stringstream		errMsg;
					errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected \"of\" here, found "
											<< tokenItty->GetShortDescription() << ".";
					mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
					throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
				}
				CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "of".
				
				// VALUE:
				CFunctionCallNode*	fcall = new CFunctionCallNode( &parseTree, false, "vcy_chunk_count", tokenItty->mLineNum );
				CValueNode*			valueObj = ParseTerm( parseTree, currFunction, tokenItty, tokens, inEndIdentifier );
				
				fcall->AddParam( new CIntValueNode( &parseTree, typeConstant, tokenItty->mLineNum ) );
				fcall->AddParam( valueObj );
				
				theTerm = fcall;
				break;
			}
			else if( tokenItty->mSubType == EOpenBracketOperator )
			{
				CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
				
				theTerm = ParseExpression( parseTree, currFunction, tokenItty, tokens, ECloseBracketOperator );
				
				if( !tokenItty->IsIdentifier(ECloseBracketOperator) )
				{
					std::stringstream		errMsg;
					errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected closing bracket here, found "
											<< tokenItty->GetShortDescription() << ".";
					mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
					throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
				}
				CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
				break;
			}
			else if( tokenItty->mSubType == ETheIdentifier )
			{
				CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "the".
				
				theTerm = ParseHostFunction( parseTree, currFunction, tokenItty, tokens );
				
				bool			isStyleQualifiedProperty = false;
				if( !theTerm )
				{
					std::string		propName;
					
					// Is it a qualified property?
					if( tokenItty->IsIdentifier( ELongIdentifier ) || tokenItty->IsIdentifier( EShortIdentifier )
						|| tokenItty->IsIdentifier( EAbbreviatedIdentifier ) )
					{
						propName = tokenItty->GetIdentifierText();
						propName.append( 1, ' ' );
						
						CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Advance past style qualifier.
						isStyleQualifiedProperty = true;
					}
					
					// Check if it could be an object property expression:
					if( isStyleQualifiedProperty && tokenItty->mType != EIdentifierToken )
						isStyleQualifiedProperty = false;
					else
					{
						size_t			lineNum = tokenItty->mLineNum;
						propName.append( tokenItty->GetIdentifierText() );
						CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
						if( inEndIdentifier != EOfIdentifier && tokenItty->IsIdentifier( EOfIdentifier ) )
						{
							CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "of".
							CValueNode	*	targetObj = ParseTerm( parseTree, currFunction, tokenItty, tokens, inEndIdentifier );
							
							CObjectPropertyNode	*	propExpr = new CObjectPropertyNode( &parseTree, propName, lineNum );
							propExpr->AddParam( targetObj );
							theTerm = propExpr;
						}
						else
						{
							CTokenizer::GoPreviousToken( mFileName, tokenItty, tokens );	// Backtrack over what should have been "of".
							if( isStyleQualifiedProperty )
								CTokenizer::GoPreviousToken( mFileName, tokenItty, tokens );	// Backtrack over what we took for a style qualifier.
						}
					}
				}
				
				// Check if it could be a global property expression:
				if( !theTerm && sGlobalProperties )
				{
					TIdentifierSubtype	subType = tokenItty->mSubType;
					TIdentifierSubtype	qualifierType = ELastIdentifier_Sentinel;
		
					if( isStyleQualifiedProperty )
					{
						CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
						qualifierType = subType;
						subType = tokenItty->mSubType;
					}
					
					// Find it in our list of global properties:
					int				x = 0;
					
					for( x = 0; sGlobalProperties[x].mType != ELastIdentifier_Sentinel; x++ )
					{
						if( sGlobalProperties[x].mType == subType && (sGlobalProperties[x].mPrefixType == qualifierType) )
						{
							theTerm = new CGlobalPropertyNode( &parseTree, sGlobalProperties[x].mSetterInstructionID, sGlobalProperties[x].mGetterInstructionID, gIdentifierStrings[subType], tokenItty->mLineNum );
							CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
							break;
						}
					}
					
					if( !theTerm && isStyleQualifiedProperty )
						CTokenizer::GoPreviousToken( mFileName, tokenItty, tokens );
				}
								
				if( !theTerm && sBuiltInFunctions )
				{
					TIdentifierSubtype	subType = tokenItty->mSubType;
		
					// Find it in our list of built-in functions:
					int				x = 0;
					
					for( x = 0; sBuiltInFunctions[x].mType != ELastIdentifier_Sentinel; x++ )
					{
						if( sBuiltInFunctions[x].mType == subType )
						{
							COperatorNode* fcall = new COperatorNode( &parseTree, sBuiltInFunctions[x].mInstructionID, tokenItty->mLineNum );
							fcall->SetInstructionParams( sBuiltInFunctions[x].mParam1, sBuiltInFunctions[x].mParam2 );
							theTerm = fcall;
							CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
							break;
						}
					}
				}
				
				if( !theTerm )	// Not a global property? Try a container:
				{
					CTokenizer::GoPreviousToken( mFileName, tokenItty, tokens );	// Backtrack so ParseContainer sees "the", too.
					theTerm = ParseContainer( false, true, parseTree, currFunction, tokenItty, tokens, inEndIdentifier );
				}
				break;
			}
			else if( tokenItty->mSubType == EParameterIdentifier )
			{
				size_t		lineNum = tokenItty->mLineNum;
				
				CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip "param".
				
				bool	hadOpenBracket = false;
				if( tokenItty->IsIdentifier( EOpenBracketOperator ) )	// Parse open bracket.
				{
					CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip opening bracket.
					hadOpenBracket = true;
				}
				
				COperatorNode*			fcall = new COperatorNode( &parseTree, PARAMETER_INSTR, lineNum );
				fcall->SetInstructionParams( BACK_OF_STACK, 0 );
				
				fcall->AddParam( ParseTerm( parseTree, currFunction, tokenItty, tokens, inEndIdentifier ) );
				
				if( !tokenItty->IsIdentifier( ECloseBracketOperator ) && hadOpenBracket )	// MUST have close bracket.
				{
					delete fcall;
					
					std::stringstream		errMsg;
					errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: excpected \"(\" after function name, found "
											<< tokenItty->GetShortDescription() << ".";
					mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
					throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
				}
				
				if( hadOpenBracket )
					CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip closing bracket.
				
				theTerm = fcall;
				break;
			}
			else if( tokenItty->mSubType == EResultIdentifier || tokenItty->mSubType == ETheIdentifier )
			{	
				theTerm = ParseContainer( false, true, parseTree, currFunction, tokenItty, tokens, inEndIdentifier );
				break;
			}
			else if( tokenItty->mSubType == EOpenSquareBracketOperator )
			{	
				theTerm = ParseObjCMethodCall( parseTree, currFunction, tokenItty, tokens );
				break;
			}
			else
			{
				TIdentifierSubtype	subType = tokenItty->mSubType;
	
				if( !theTerm && sBuiltInFunctions )
				{
					// Find it in our list of built-in functions:
					int				x = 0;
					
					for( x = 0; sBuiltInFunctions[x].mType != ELastIdentifier_Sentinel; x++ )
					{
						if( sBuiltInFunctions[x].mType == subType )
						{
							COperatorNode* fcall = new COperatorNode( &parseTree, sBuiltInFunctions[x].mInstructionID, tokenItty->mLineNum );
							fcall->SetInstructionParams( sBuiltInFunctions[x].mParam1, sBuiltInFunctions[x].mParam2 );
							theTerm = fcall;
							CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
							
							if( tokenItty->IsIdentifier( EOpenBracketOperator ) )
							{
								CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
								if( tokenItty->IsIdentifier( ECloseBracketOperator ) )
									CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
								else
									CTokenizer::GoPreviousToken( mFileName, tokenItty, tokens );
							}
							break;
						}
					}
				}
				
				if( !theTerm && sGlobalProperties )
				{
					// Find it in our list of global properties:
					for( int x = 0; sGlobalProperties[x].mType != ELastIdentifier_Sentinel; x++ )
					{
						if( sGlobalProperties[x].mType == subType )
						{
							theTerm = new CGlobalPropertyNode( &parseTree, sGlobalProperties[x].mSetterInstructionID, sGlobalProperties[x].mGetterInstructionID, gIdentifierStrings[subType], tokenItty->mLineNum );
							CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
							break;
						}
					}
				}
				
				if( theTerm )
					break;
				
				// Try to find chunk type that matches:
				TChunkType typeConstant = GetChunkTypeNameFromIdentifierSubtype( tokenItty->mSubType );
				if( typeConstant != TChunkTypeInvalid )
				{
					theTerm = ParseConstantChunkExpression( typeConstant, parseTree, currFunction, tokenItty, tokens );
					break;
				}

				// Now try constant:
				CValueNode		*	constantValue = NULL;
				TConstantEntry	*	currConst = sConstants;
				
				while( currConst->mType[0] != ELastIdentifier_Sentinel )
				{
					for( size_t y = 0; y < MAX_CONSTANT_IDENTS; y++ )
					{
						if( currConst->mType[y] != ELastIdentifier_Sentinel
							&& !tokenItty->IsIdentifier( currConst->mType[y] ) )
						{
							for( size_t z = 0; z < y; z++ )	// < so we don't skip to *before* first token.
								CTokenizer::GoPreviousToken( mFileName, tokenItty, tokens );
							break;
						}
						
						if( y == (MAX_CONSTANT_IDENTS -1) || currConst->mType[y+1] == ELastIdentifier_Sentinel )
						{
							constantValue = currConst->mValue;
							break;
						}
						
						CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
					}
					
					if( constantValue )
						break;
					
					currConst++;
				}
				
				if( constantValue )	// Found constant of that name!
				{
					theTerm = constantValue->Copy();
					theTerm->SetLineNum( tokenItty->mLineNum );
					CTokenizer::GoNextToken( mFileName, tokenItty, tokens );
					break;
				}

				// Try to find unary operator that matches:
				LEOInstructionID	operatorCommandName = INVALID_INSTR;
				
				for( int x = 0; sUnaryOperators[x].mType != ELastIdentifier_Sentinel; x++ )
				{
					if( tokenItty->mSubType == sUnaryOperators[x].mType )
						operatorCommandName = sUnaryOperators[x].mInstructionID;
				}
				
				if( operatorCommandName != INVALID_INSTR )
				{
					size_t	lineNum = tokenItty->mLineNum;
					CTokenizer::GoNextToken( mFileName, tokenItty, tokens );	// Skip operator token.
					
					COperatorNode*	opFCall = new COperatorNode( &parseTree, operatorCommandName, lineNum );
					opFCall->AddParam( ParseTerm( parseTree, currFunction, tokenItty, tokens, inEndIdentifier ) );
					theTerm = opFCall;
				}
				else
					theTerm = ParseContainer( false, true, parseTree, currFunction, tokenItty, tokens, inEndIdentifier );
			}
			break;
		
		default:
		{
			std::stringstream		errMsg;
			errMsg << mFileName << ":" << tokenItty->mLineNum << ": error: Expected a term here, found \""
									<< tokenItty->GetShortDescription() << "\".";
			mMessages.push_back( CMessageEntry( errMsg.str(), mFileName, tokenItty->mLineNum ) );
			throw CForgeParseError( errMsg.str(), tokenItty->mLineNum, tokenItty->mOffset );
			break;
		}
	}
	
	return theTerm;
}


// Take a list in a string delimited by a single character (e.g. comma) and fill
//	a deque with the various items:
void	CParser::FillArrayWithComponentsSeparatedBy( const char* typesStr, char delimiter, std::deque<std::string> &destTypesList )
{
	std::string				tempType;
	int						x = 0;
	
	for( x = 0; typesStr[x] != 0; x++ )
	{
		if( typesStr[x] == delimiter )
		{
			destTypesList.push_back(tempType);
			tempType.clear();
		}
		else
			tempType.append(1,typesStr[x]);
	}
	
	if( tempType.length() > 0 )
	{
		destTypesList.push_back(tempType);
		tempType.clear();
	}
}


// Create a "trampoline" function that can be handed to a system API as a callback
//	and calls a particular handler with the converted parameters:
void	CParser::CreateHandlerTrampolineForFunction( const std::string &handlerName, const std::string& procPtrName,
														const char* typesStr,
														std::stringstream& theCode, std::string &outTrampolineName )
{
//	// Build an array of the types:
//	std::deque<std::string> typesList;
//	
//	FillArrayWithComponentsSeparatedBy( typesStr, ',', typesList );
//	
//	outTrampolineName.assign("Trampoline_");
//	outTrampolineName.append(procPtrName);
//	outTrampolineName.append("_");
//	outTrampolineName.append(handlerName);
//	
//	// Generate method name and param signature:
//	theCode << "#ifndef GUARD_" << outTrampolineName << std::endl;
//	theCode << "#define GUARD_" << outTrampolineName << "\t1" << std::endl;
//	theCode << "const CVariant\t" << handlerName << "( CVariant& paramList );" << std::endl;
//	theCode << typesList[0] << "\t" << outTrampolineName << "( ";
//	std::deque<std::string>::iterator itty = typesList.begin();
//	int			x = 1;
//	bool		isFirst = true;
//	itty++;
//	while( itty != typesList.end() )
//	{
//		if( isFirst )
//			isFirst = false;
//		else
//			theCode << ", ";
//		theCode << *itty << " param" << x++;
//		
//		itty++;
//	}
//	theCode << " )" << std::endl << "{" << std::endl;
//	theCode << "\tCVariant	temp1( TVariantTypeList );" << std::endl;
//	
//	// Generate translation code that calls our handler:
//	itty = typesList.begin();
//	
//	theCode << "\t";
//	if( !itty->compare( "void" ) == 0 )
//		theCode << "CVariant\tresult = ";
//	theCode << handlerName << "( temp1.MakeList()";
//	
//		// Do each param:
//	itty++;
//	isFirst = true;
//	x = 1;
//	while( itty != typesList.end() )
//	{
//		std::string		parPrefix, parSuffix;
//		theCode << ".Append( ";
//		
//		GenerateObjCTypeToVariantCode( *itty, parPrefix, parSuffix );
//		theCode << parPrefix << "param" << x++ << parSuffix << " )";
//		
//		itty++;
//	}
//
//	theCode << " );" << std::endl;
//	
//	// Return value:
//	if( typesList[0].compare("void") != 0 )
//	{
//		std::string		resultPrefix, resultSuffix, resultItself("result");
//		theCode << "\treturn ";
//		GenerateVariantToObjCTypeCode( typesList[0], resultPrefix, resultSuffix, resultItself );
//		theCode << resultPrefix << resultItself << resultSuffix << ";" << std::endl;
//	}
//	
//	theCode << "}" << std::endl
//			<< "#endif /*GUARD_" << outTrampolineName << "*/" << std::endl; 
}

} // Namespace Carlson.
